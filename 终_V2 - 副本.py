import sensor, image, display, time, math
from pyb import UART

# 初始化摄像头和LCD屏幕
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)  # Special 128x160 framesize for LCD Shield.
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False)  # 颜色识别必须关闭自动增益
sensor.set_auto_whitebal(False)
lcd = display.SPIDisplay()
uart = UART(3, 9600)

# 颜色阈值（根据实际情况进行调整）
thresholds = {
    "Red":(0, 55, 41, 127, -128, 127),  # generic_red_thresholds
    "Green":(0, 67, -128, -1, -128, 127),  # generic_green_thresholds
    "Blue":(0, 60, 4, 127, -128, -26),
}  # generic_blue_thresholds

# 存储识别到的圆心坐标
circle_centers = []
MAX_CIRCLE_COUNT = 20  # 存储最多20次识别到的圆心

# 计算平均值
def calculate_average(centers):
    if not centers:
        return None
    avg_x = sum(c[0] for c in centers) // len(centers)
    avg_y = sum(c[1] for c in centers) // len(centers)
    return (avg_x, avg_y)

time.sleep(1)

# 主循环
while True:
    img = sensor.snapshot()  # 获取一次快照
    detected_color = None
    color_count = {"Red": 0, "Green": 0, "Blue": 0}  # 用于统计颜色出现次数
    color_identified = False  # 标记是否已经识别到某个颜色
    ring_identified=False     #标记是否识别到某个圆
    start_time = time.time()  # 记录开始时间

    # 主机发送串口指令：0（颜色识别 并且回传物块坐标）  1（圆环识别 回传圆环中心位置）
    if uart.any():  # 判断是否接收到数据
        command = uart.readchar()  # 读取一个字节
        print("Received command:", command)
        color_identified = False  # 标记是否已经识别到某个颜色
        if command == 0x02:  # 如果接收到字符0，进行颜色识别
            print("开始颜色识别")
            while not color_identified:  # 循环直到识别到颜色
                img = sensor.snapshot()  # 获取一次快照
                #total_checks += 1  # 计数识别次数
                detected_color = None  # Reset the detected color flag
                for color, threshold in thresholds.items():
                    blobs = img.find_blobs(
                        [threshold],
                        pixels_threshold=300,  # 最小像素数阈值
                        area_threshold=300,    # 最小区域大小阈值
                        merge=True             # 合并重叠的 blob
                    )

                    # 检查找到的 blobs 是否为空，且过滤掉小的区域
                    if blobs and all(blob.pixels() < 500 for blob in blobs):  # 排除小区域伪目标
                        continue  # 如果是小区域，跳过当前循环，继续检查下一个颜色

                    if blobs:  # 如果找到符合条件的 blob
                        detected_color = color
                        for blob in blobs:
                            # 绘制矩形框和中心点
                            img.draw_rectangle(blob.rect(), color=(255, 0, 0))  # 红色矩形框
                            img.draw_cross(blob.cx(), blob.cy(), color=(0, 255, 0))  # 绿色交叉点
                            print(f"Detected {color}: Center=({blob.cx()}, {blob.cy()}), Size=({blob.w()}, {blob.h()})")
                            color_count[color] += 1  # 统计该颜色出现次数
                        break  # 一旦找到目标颜色，跳出循环
                lcd.write(img)

                # 如果某种颜色出现次数超过 20 次，则返回该颜色并停止识别
                for color, count in color_count.items():
                    if count > 20 and not color_identified:
                        print(f"{color} 已识别超过 20 次")
                        color_identified = True  # 设置标志，表示已经识别到该颜色
                        if color == "Red" and blobs:
                            uart.write(bytearray([0xff , 1 , blobs[0].cx(), blobs[0].cy(), 0xfe]))  # 发送字节 1 表示红色
                        elif color == "Green" and blobs:
                            uart.write(bytearray([0xff , 2 , blobs[0].cx(), blobs[0].cy(), 0xfe]))  # 发送字节 2 表示绿色
                        elif color == "Blue" and blobs:
                            uart.write(bytearray([0xff , 3 , blobs[0].cx(), blobs[0].cy(), 0xfe]))  # 发送字节 3 表示蓝色
                        break  # 识别到颜色后退出循环
                lcd.write(img)

                # 检查是否超时，10秒为最大时间
                if time.time() - start_time > 10:  # 超过10秒则结束并发送失败信号
                    print("超时未成功识别颜色")
                    uart.write(bytearray([0xff , 5, 0, 0, 0xfe]))  # 发送字节 5 表示超时失败
                    color_identified = True  # 强制跳出识别循环
                    break

            # 如果颜色识别完成，向上位机发送完成信号
            if not color_identified:
                print("没有识别到符合条件的颜色，超时失败")
                uart.write(b'1')
                usart.read(1)# 向上位机发送失败信号

            print("颜色识别结束")

        elif command == 0x01:  # 如果接收到字符1，进行圆环识别
            print("开始圆环识别")
            while not ring_identified:
                img = sensor.snapshot()  # 获取图像

                # 使用find_circles检测圆形，返回圆形的中心坐标和半径
                circles = img.find_circles(
                    threshold=2000,  # 检测阈值，值越大越严格
                    x_margin=10,  # X方向容差
                    y_margin=10,  # Y方向容差
                    r_margin=10,  # 半径方向容差
                    r_min=5,  # 最小半径（调整为适合你的圆的最小半径）
                    r_max=50,  # 最大半径（调整为适合你的圆的最大半径）
                    r_step=2  # 半径步长
                )

                # 如果有检测到圆，选择半径最小且离中心最近的圆
                if len(circles) > 0:
                    center_x = img.width() // 2  # 获取图像中心X坐标
                    center_y = img.height() // 2  # 获取图像中心Y坐标

                    closest_circle = None
                    min_distance = float('inf')  # 初始化最小距离为正无穷
                    min_radius = float('inf')  # 初始化最小半径为正无穷

                    # 遍历所有圆形，选择最小半径且距离中心最近的圆
                    for circle in circles:
                        # 计算圆心到图像中心的距离
                        distance = math.sqrt((circle[0] - center_x)**2 + (circle[1] - center_y)**2)

                        # 选择距离中心最近且半径最小的圆
                        if distance < min_distance or (distance == min_distance and circle[2] < min_radius):
                            min_distance = distance
                            min_radius = circle[2]
                            closest_circle = circle

                    # 如果选择了合适的圆，绘制它
                    if closest_circle:
                        img.draw_circle(closest_circle[0], closest_circle[1], closest_circle[2], color=(255, 0, 0))  # 绘制圆形
                        img.draw_cross(closest_circle[0], closest_circle[1], color=(255, 0, 0))  # 绘制圆心

                        print("Detected closest circle at (x, y): (%d, %d) with radius: %d" % (closest_circle[0], closest_circle[1], closest_circle[2]))  # 打印圆的坐标和半径
                        circle_centers.append((closest_circle[0], closest_circle[1]))

                        # 限制存储的圆心数量
                        if len(circle_centers) > MAX_CIRCLE_COUNT:
                            circle_centers.pop(0)

                        # 每隔50ms发送一次数据
                        if len(circle_centers) >= MAX_CIRCLE_COUNT:
                            # 计算圆心坐标的平均值
                            avg_center = calculate_average(circle_centers)
                            if avg_center:
                                print("Averaged Center:", avg_center)
                                uart.write(bytearray([0xee, avg_center[0], avg_center[1], 0xfe]))  # 通过 UART 发送数据
                                ring_identified=True
                            # 清空存储，重新开始
                            circle_centers.clear()
                            # 圆环识别代码
                            print('Ring OK')

                # 检查是否超时，5秒为最大时间
                if time.time() - start_time > 5:  # 超过5秒则结束并发送失败信号
                    print("超时未成功识别圆环")
                    uart.write(bytearray([0xee, 0, 6, 0xfe]))  # 发送字节 6 表示超时失败
                    ring_identified=True    # 强制跳出识别循环
                    break

        else:  # 如果接收到的不是 '0' 或 '1'，处理无效命令
            print("Invalid command")

    # 更新 LCD 显示
    lcd.write(img)
    #time.sleep(0.01)  # 稍微休眠，避免过于频繁的串口通讯
