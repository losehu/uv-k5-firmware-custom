import math

import cv2
import sys
import os
import numpy as np

# 在获取图像路径输入之前，对路径进行解码
# 读取命令行参数
arguments = sys.argv[1]  # 忽略第一个参数，因为它是脚本的名称

# 缩放倍数
scale_factor = 20  # 可根据需要调整放大倍数

# 是否输出图像内容
output_image = False

def process_file_path(file_path):
    file_name_with_extension = os.path.basename(file_path)  # 获取包含后缀的文件名
    file_name_without_extension, extension = os.path.splitext(file_name_with_extension)  # 分离文件名和后缀

    # 提取不带路径的文件名
    file_name_without_path = os.path.splitext(os.path.basename(file_path))[0]

    return {
        "file_name_with_extension": file_name_with_extension,
        "file_name_without_extension": file_name_without_extension,
        "file_name_without_path": file_name_without_path
    }

def clear_bit(byte_value, byte_bit):
    mask = ~(1 << byte_bit)  # 创建一个掩码，将特定位设置为0
    result = byte_value & mask  # 将特定位设置为0
    return result


def set_bit(byte_value, byte_bit):
    mask = 1 << byte_bit  # 创建一个掩码，将特定位设置为1
    result = byte_value | mask  # 将特定位设置为1
    return result


# 创建一个回调函数来获取鼠标事件的坐标和操作
def get_mouse_event(event, x, y, flags, param):
    global output_image

    if event == cv2.EVENT_LBUTTONDOWN:  # 左键点击
        # 计算在原始图像中的坐标
        original_x = x // scale_factor
        original_y = y // scale_factor

        # 反转点击位置的像素
        if 0 <= original_x < image.shape[1] and 0 <= original_y < image.shape[0]:
            if image[original_y, original_x] == 0:  # 如果是黑色像素，变为白色
                image[original_y, original_x] = 255
            else:  # 如果是白色像素，变为黑色
                image[original_y, original_x] = 0

        # 更新放大图像
        scaled_img = cv2.resize(image, (0, 0), fx=scale_factor, fy=scale_factor, interpolation=cv2.INTER_NEAREST)
        cv2.imshow('Scaled Image', scaled_img)


def on_space_pressed():
    global output_image
    output_image = True


# 获取图像路径输入

# 读取图像
#image_path = arguments
cv2.imdecode(np.fromfile(arguments, dtype=np.uint8), cv2.IMREAD_UNCHANGED)
# image_path= os.path.abspath(os.path.realpath(arguments)).encode('utf-8').decode('utf-8')
# print(image_path)
image = cv2.imread(image_path, 0)  # 以灰度模式读取图像

if image is None:
    print(f"Error: Couldn't read the image from path: {image_path}")
    sys.exit(1)  # Exit the script with an error code
image2=image.copy()
if image is None:
    print("Error: Couldn't read the image.")
else:
    result = process_file_path(image_path)

    filename = result["file_name_without_path"]
    # 创建放大图像
    scaled_image = cv2.resize(image, (0, 0), fx=scale_factor, fy=scale_factor, interpolation=cv2.INTER_NEAREST)

    # 创建窗口并将回调函数与窗口绑定
    cv2.namedWindow('Scaled Image')
    cv2.setMouseCallback('Scaled Image', get_mouse_event)

    # 显示放大后的图像
    cv2.imshow('Scaled Image', scaled_image)

    while True:
        key = cv2.waitKey(1)
        if key == ord(' '):  # 当按下空格键时
            cv2.imwrite(image_path, image)
            _, image = cv2.threshold(image, 127, 255, cv2.THRESH_BINARY)
            height, width = image.shape
            byte_value = 0
            byte_bit = 0
            print("{",end="")

            for i in range(math.ceil(height / 8)* width) :
                if i  >= height // 8*width :
                    now_height = height - math.floor(height / 8) * 8
                else:
                    now_height = 8
                now_col = i % width

                for j in range(now_height):
                    if j + i // width*8==10:
                        print(i,j,now_height)
                    if image[j + i // width*8,now_col] == 255:
                        byte_value= set_bit(byte_value, byte_bit)
                    else:
                        byte_value=  clear_bit(byte_value, byte_bit)
                    byte_bit = byte_bit + 1
                    if byte_bit == 8:
                        byte_bit = 0
                        print("0x",end="")
                        print(hex(byte_value)[2:].zfill(2),end=",")  # 输出两位十六进
            if byte_bit % 8 != 0:
                print(hex(byte_value)[2:].zfill(2),end="},/*")  # 输出两位十六进
            else:
                print("",end="},/*")  # 输出两位十六进
            print(filename,end="*/\n")
            break
        elif key == 27:  # 按下ESC键退出
            break
        elif  key == ord('B') or key == ord('b'):
            image=image2.copy()

            scaled_img = cv2.resize(image, (0, 0), fx=scale_factor, fy=scale_factor, interpolation=cv2.INTER_NEAREST)
            cv2.imshow('Scaled Image', scaled_img)
    cv2.destroyAllWindows()
