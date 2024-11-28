import sensor, image, display, time
from pyb import UART

# 初始化摄像头和LCD屏幕
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA2)  # Special 128x160 framesize for LCD Shield.
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False)  # 颜色识别必须关闭自动增益
sensor.set_auto_whitebal(False)
lcd = display.SPIDisplay()
uart = UART(3, 9600)

# 颜色阈值（根据实际情况进行调整）
thresholds = {
    "Red": (0, 28, 5, 127, -2, 19),    # 红色阈值
    "Green": (22, 40, -35, -14, 7, 40),  # 绿色阈值
    "Blue": (0, 35, -128, 14, -128, -7), # 蓝色阈值
}


MAX_DISTANCE = 50
# 存储识别到的圆心坐标
circle_centers = []
MAX_CIRCLE_COUNT = 40  # 存储最多40次识别到的圆心

# 计算平均值
def calculate_average(centers):
    if not centers:
        return None
    avg_x = sum(c[0] for c in centers) // len(centers)
    avg_y = sum(c[1] for c in centers) // len(centers)
    return (avg_x, avg_y)

# 合并矩形函数
def union_rect(rect1, rect2):
    x1, y1, w1, h1 = rect1
    x2, y2, w2, h2 = rect2
    x_min = min(x1, x2)
    y_min = min(y1, y2)
    x_max = max(x1 + w1, x2 + w2)
    y_max = max(y1 + h1, y2 + h2)
    return (x_min, y_min, x_max - x_min, y_max - y_min)

# 合并颜色块的自定义类
class MergedBlob:
    def __init__(self, rect, cx, cy):
        self._rect = rect
        self._cx = cx
        self._cy = cy

    def rect(self):
        return self._rect

    def cx(self):
        return self._cx

    def cy(self):
        return self._cy

    def x(self):  # 获取左上角 x 坐标
        return self._rect[0]

    def y(self):  # 获取左上角 y 坐标
        return self._rect[1]

    def width(self):  # 获取宽度
        return self._rect[2]

    def height(self):  # 获取高度
        return self._rect[3]

    def update(self, new_rect, new_cx, new_cy):
        self._rect = new_rect
        self._cx = new_cx
        self._cy = new_cy

def distance(blob1, blob2):
    """计算两个 blob 的中心点距离"""
    return ((blob1.cx() - blob2.cx())**2 + (blob1.cy() - blob2.cy())**2) ** 0.5

def merge_close_blobs(blobs, max_distance):
    """合并距离较近的颜色块"""
    merged_blobs = []
    for blob in blobs:
        merged = False
        for merged_blob in merged_blobs:
            if distance(blob, merged_blob) < max_distance:
                new_rect = union_rect(merged_blob.rect(), blob.rect())
                new_cx = (merged_blob.cx() + blob.cx()) // 2
                new_cy = (merged_blob.cy() + blob.cy()) // 2
                merged_blob.update(new_rect, new_cx, new_cy)
                merged = True
                break
        if not merged:
            merged_blobs.append(MergedBlob(blob.rect(), blob.cx(), blob.cy()))
    return merged_blobs

time.sleep(1)

# 主循环
while True:
    img = sensor.snapshot()  # 获取一次快照
    detected_color = None
    color_count = {"Red": 0, "Green": 0, "Blue": 0}  # 用于统计颜色出现次数
    #total_checks = 0  # 当前识别周期内的识别次数
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
                    blobs = img.find_blobs([threshold], pixels_threshold=150, area_threshold=150, merge=False)
                    for blob in blobs:
                        img.draw_rectangle(blob.rect(), color=(255, 0, 0))  # 使用红色绘制框框
                        img.draw_cross(blob.cx(), blob.cy(), color=(255, 0, 0))  # 使用红色绘制十字架
                        detected_color = color
                        print(f"Detected {color} at ({blob.cx()}, {blob.cy()}) with size ({blob.w()}, {blob.h()})")
                        lcd.write(img)
                        color_count[color] += 1  # 统计该颜色出现次数
                # 如果某种颜色出现次数超过 30 次，则返回该颜色并停止识别
                for color, count in color_count.items():
                    if count > 30 and not color_identified:
                        print(f"{color} 已识别超过 30 次")
                        color_identified = True  # 设置标志，表示已经识别到该颜色
                        if color == "Red":
                            time.sleep_ms(50)
                            uart.write(bytearray([0xff , 1 , blob.cx(), blob.cy(), 0xee]))  # 发送字节 1 表示红色
                        elif color == "Green":
                            time.sleep_ms(50)
                            uart.write(bytearray([0xff , 2 , blob.cx(), blob.cy(), 0xee]))  # 发送字节 2 表示绿色
                        elif color == "Blue":
                            time.sleep_ms(50)
                            uart.write(bytearray([0xff , 3 , blob.cx(), blob.cy(), 0xee]))  # 发送字节 3 表示蓝色
                        break  # 识别到颜色后退出循环
                lcd.write(img)

                # 检查是否超时，15秒为最大时间
                if time.time() - start_time > 15:  # 超过15秒则结束并发送失败信号
                    print("超时未成功识别颜色")
                    uart.write(bytes([5]))  # 发送字节 5 表示超时失败
                    color_identified = True  # 强制跳出识别循环
                    break

            # 如果颜色识别完成，向上位机发送完成信号
            if not color_identified:
                print("没有识别到符合条件的颜色，超时失败")
                uart.write(b'1')
                usart.read(1)# 向上位机发送失败信号

            print("颜色识别结束")

        elif command == 0x01:  # 如果接收到字符1，进行圆环识别
            while not ring_identified:
                img = sensor.snapshot()  # 获取一次快照
                #img = sensor.snapshot().lens_corr(1.8)  # 矫正镜头畸变
                largest_circle = None  # 用于保存半径最大的圆

                # 处理每种颜色的区域
                for color, threshold in thresholds.items():
                    blobs = img.find_blobs([threshold], pixels_threshold=100, area_threshold=100, merge=True)
                    merged_blobs = merge_close_blobs(blobs, MAX_DISTANCE)

                    for blob in merged_blobs:
                        img.draw_rectangle(blob.rect(), color=(255, 0, 0))
                        img.draw_cross(blob.cx(), blob.cy(), color=(255, 0, 0))
                        img.draw_string(blob.x(), blob.y(), color, color=(255, 255, 255))

                # 霍夫变换检测圆
                for c in img.find_circles(threshold=2200, x_margin=10, y_margin=10, r_margin=10, r_min=2, r_max=37, r_step=2):
                    if largest_circle is None or c.r() > largest_circle.r():
                        largest_circle = c  # 更新半径最大的圆

                # 如果找到最大圆，则绘制并发送数据
                if largest_circle:
                    img.draw_circle(largest_circle.x(), largest_circle.y(), largest_circle.r(), color=(255, 255, 0))

                    circle_centers.append((c.x(), c.y()))
                    # 限制存储的圆心数量
                    if len(circle_centers) > MAX_CIRCLE_COUNT:
                        circle_centers.pop(0)

                    # 每隔50ms发送一次数据
                    if len(circle_centers) >= MAX_CIRCLE_COUNT:
                        # 计算圆心坐标的平均值
                        avg_center = calculate_average(circle_centers)
                        if avg_center:
                            print("Averaged Center:", avg_center)
                            uart.write(bytearray([0xff, avg_center[0], avg_center[1], 0xee]))  # 通过 UART 发送数据
                            ring_identified=True
                        # 清空存储，重新开始
                        circle_centers.clear()


                    print("Largest Circle:", largest_circle.x(), largest_circle.y(), largest_circle.r())
                # 更新 LCD 显示
                lcd.write(img)
                time.sleep_ms(50)

            # 圆环识别代码
            print('Ring OK')

        else:  # 如果接收到的不是 '0' 或 '1'，处理无效命令
            print("Invalid command")

    # 更新 LCD 显示
    lcd.write(img)
    #time.sleep(0.01)  # 稍微休眠，避免过于频繁的串口通讯
