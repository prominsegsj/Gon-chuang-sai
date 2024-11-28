import sensor, image, display, time
from pyb import UART

clock = time.clock()  # 创建一个 clock 对象
# 初始化摄像头和LCD屏幕
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA2)  # Special 128x160 framesize for LCD Shield.
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False)  # 颜色识别必须关闭自动增益
sensor.set_auto_whitebal(False)
lcd = display.SPIDisplay()
uart = UART(3, 19200)
# 接收状态
RECV_IDLE = 0
RECV_START = 1
RECV_DATA = 2
RECV_END = 3
receive_state = RECV_IDLE

#接收缓冲区
recv_data=[]
mode=1     #模式设置，0为识别颜色，1为识别圆心
a=1
# 颜色阈值（根据实际情况进行调整）
thresholds = {
    "Red": (0, 28, 5, 127, -2, 19),    # 红色阈值
    "Green": (22, 40, -35, -14, 7, 40),  # 绿色阈值
    "Blue": (0, 35, -128, 14, -128, -7), # 蓝色阈值
}
MAX_DISTANCE = 50
def distance(blob1, blob2):
    """计算两个blob的中心点之间的距离"""
    return ((blob1.cx() - blob2.cx())**2 + (blob1.cy() - blob2.cy())**2) ** 0.5

# 假设 rect() 返回 (x, y, width, height)
def union_rect(rect1, rect2):
    x1, y1, w1, h1 = rect1
    x2, y2, w2, h2 = rect2

    # 计算两个矩形的边界
    x_min = min(x1, x2)
    y_min = min(y1, y2)
    x_max = max(x1 + w1, x2 + w2)
    y_max = max(y1 + h1, y2 + h2)

    # 返回新的外接矩形
    return (x_min, y_min, x_max - x_min, y_max - y_min)

def merge_close_blobs(blobs, max_distance):
    """合并距离较近的颜色块"""
    merged_blobs = []
    for blob in blobs:
        merged = False
        for merged_blob in merged_blobs:
            if distance(blob, merged_blob) < max_distance:
                # 合并两个区域：重新计算中心
                new_rect = merged_blob.rect().union_rect(blob.rect())  # 合并外接矩形
                new_cx = (merged_blob.cx() + blob.cx()) // 2  # 更新中心
                new_cy = (merged_blob.cy() + blob.cy()) // 2
                merged_blob.rect(new_rect)  # 更新区域
                merged_blob.cx(new_cx)
                merged_blob.cy(new_cy)
                merged = True
                break
        if not merged:
            merged_blobs.append(blob)
    return merged_blobs

time.sleep(8)
# 主循环
while True:
    while(a):
        img = sensor.snapshot()  # 获取摄像头图像

        if uart.any():
            data = uart.readchar()

            if receive_state == RECV_IDLE:
                if data == 0x5a:  # 接收到帧头
                    receive_state=RECV_START

            elif receive_state == RECV_START:
                if data == 0x5b:  # 检测到帧尾
                    if len(recv_data) == 3:  # 假设数据长度为3个字符
                        print("接收到的数据:", recv_data)
                        receive_state = RECV_IDLE
                        a=0
                        mode=recv_data[0]
                    else:
                        recv_data = []  # 清空接收数据缓冲区
                else:
                    recv_data.append(data)  # 存储数据
        # 进入颜色识别模式
        while mode == 0:
            img = sensor.snapshot()  # 获取一次快照
            detected_color = None

            # 查找颜色
            for color, threshold in thresholds.items():
                blobs = img.find_blobs([threshold], pixels_threshold=100, area_threshold=100, merge=True)
                for blob in blobs:
                    # 绘制框框和十字架
                    img.draw_rectangle(blob.rect(), color=(255, 0, 0))  # 使用红色绘制框框
                    img.draw_cross(blob.cx(), blob.cy(), color=(255, 0, 0))  # 使用红色绘制十字架
                    detected_color = color
                    print("Detected color:", color)
                    if color == "Red":
                        uart.write(bytes([1]))  # 发送字节 1
                    elif color == "Green":
                        uart.write(bytes([2]))  # 发送字节 2
                    elif color == "Blue":
                        uart.write(bytes([3]))  # 发送字节 3

                    #uart.write(color)
                    time.sleep(0.8)
                    img.draw_string(blob.x(), blob.y(), color, color=(255, 255, 255))  # 在图像上显示颜色名称
                    break  # 只处理第一个检测到的颜色

            # 如果未检测到颜色，标记为白色物块
            if not detected_color:
                img.draw_string(10, 10, "No Color Detected", color=(255, 255, 255))  # 显示没有检测到颜色
                img.draw_cross(10, 10, color=(255, 255, 255))  # 使用白色绘制十字架
            lcd.write(img)  # 更新 LCD 显示
            time.sleep(0.1)  # 休眠100毫秒
        while mode == 1:
            img = sensor.snapshot()  # 获取摄像头图像
            clock.tick()

            #lens_corr(1.8)畸变矫正
            img = sensor.snapshot().lens_corr(1.8)
            # 分别处理红色、绿色和蓝色的区域
            for color, threshold in thresholds.items():
                color_blobs = img.find_blobs([threshold], pixels_threshold=100, area_threshold=100, merge=True)
                merged_blobs = merge_close_blobs(color_blobs, MAX_DISTANCE)

                # 根据颜色设置显示颜色
                if color == "Red":
                    display_color = (255, 0, 0)  # 红色
                elif color == "Green":
                    display_color = (0, 255, 0)  # 绿色
                elif color == "Blue":
                    display_color = (0, 0, 255)  # 蓝色

                # 绘制合并后的颜色区域
                for blob in merged_blobs:
                    img.draw_rectangle(blob.rect(), color=display_color)  # 绘制框框
                    img.draw_cross(blob.cx(), blob.cy(), color=display_color)  # 绘制十字架
                    img.draw_string(blob.x(), blob.y(), color, color=(255, 255, 255))  # 显示颜色名称
            #霍夫变换法检测圆，主要通过修改thresholed来调整识别的圆
            for c in img.find_circles(threshold = 2200, x_margin = 10, y_margin = 10, r_margin = 10,r_min = 2, r_max = 37, r_step = 2):
                 print("x =", c.x(), ", y =", c.y())
                 img.draw_circle(c.x(), c.y(), c.r(), color = (255, 255, 0))
                 lcd.write(img)
                 x=c.x()
                 y=c.y()
                 img_data = bytearray([0x5a,x,y,0x5b])
                 uart.write(img_data)
                 print(img_data)
            lcd.write(img)  # 更新



