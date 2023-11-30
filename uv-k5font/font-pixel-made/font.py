import subprocess



from PIL import Image, ImageDraw, ImageFont

# 字体文件和字符
font_path = './ark-pixel-12px-proportional.ttc'  # 替换为你的字体文件路径

# 图像大小和字体大小
image_size = (11, 10)  # 图像大小//列 行
font_size = 11  # 字体大小


char_num=2
char_sum=['步','进']
for i in range(char_num):
    character = char_sum[i]
    out_path="./image/"+character+".png"
    # 创建图像
    image = Image.new('1', image_size, color=1)  # '1'表示二值图像，初始为白色

    # 加载字体
    font = ImageFont.truetype(font_path, font_size)

    # 设置字符在画布上的位置
    x_pos = 0  # 横坐标位置
    y_pos = -4  # 纵坐标位置

    # 在图像上绘制字符
    draw = ImageDraw.Draw(image)
    draw.text((x_pos, y_pos), character, font=font, fill=0)  # fill=0 表示黑色

    # 显示图像或保存图像
    image.save(out_path, format="PNG")


    # 执行代码
    path_name = out_path

    # 在b.py中运行，传递路径名作为参数
    subprocess.run(["python", "pic.py", path_name])


