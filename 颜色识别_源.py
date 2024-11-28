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
uart = UART(3, 19200)

# 颜色阈值（根据实际情况进行调整）
thresholds = {
    "Red": (0, 28, 5, 127, -2, 19),    # 红色阈值
    "Green": (22, 40, -35, -14, 7, 40),  # 绿色阈值
    "Blue": (0, 35, -128, 14, -128, -7), # 蓝色阈值
}
time.sleep(8)
# 主循环
while True:
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

