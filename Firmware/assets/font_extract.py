from PIL import Image, ImageDraw, ImageFont
import numpy as np

def char_to_bitmap(char, font_path, font_size=5):
    # 创建一个空白图像
    image = Image.new('1', (3, 5), 0)
    draw = ImageDraw.Draw(image)
    
    # 加载字体
    font = ImageFont.truetype(font_path, font_size)
    
    # 绘制字符
    draw.text((0, 0), char, font=font, fill=1)
    
    # 将图像转换为numpy数组
    bitmap = np.array(image)
    
    # 将bitmap转换为二进制形式
    binary_bitmap = []
    for row in bitmap:
        binary_row = 0b000
        for i, pixel in enumerate(row):
            if pixel:
                binary_row |= 1 << (2 - i)
        binary_bitmap.append(binary_row)
    
    return binary_bitmap

def generate_font_mod(font_path, chars):
    font_mod = {}
    for char in chars:
        bitmap = char_to_bitmap(char, font_path)
        font_mod[char] = bitmap
    return font_mod

def print_font_mod(font_mod):
    for char, bitmap in font_mod.items():
        print(f"// {char}")
        print("{" + ", ".join([f"0b{bin(row)[2:].zfill(3)}" for row in bitmap]) + "},")

if __name__ == "__main__":
    font_path = "assets/3x5_tiny_mono_pixel_font.ttf"  # 替换为你的TTF文件路径
    chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"  # 你想要转换的字符
    
    font_mod = generate_font_mod(font_path, chars)
    print_font_mod(font_mod)