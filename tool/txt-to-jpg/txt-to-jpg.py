from PIL import Image
import sys

def text_to_grayscale_jpg(input_path, output_path):
    # 读取文本文件中的数据
    with open(input_path, 'r') as f:
        lines = f.readlines()
    
    # 第一行包含高度和宽度
    height, width = map(int, lines[0].strip().split())
    
    # 解析像素数据
    pixels = []
    for line in lines[1:]:
        pixels.extend(map(int, line.strip().split()))
    
    # 创建一个灰度图像
    img = Image.new('L', (width, height))
    img.putdata(pixels)
    
    # 保存图像
    img.save(output_path)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_txt_path> <output_jpg_path>")
    else:
        input_txt_path = sys.argv[1]
        output_jpg_path = sys.argv[2]
        text_to_grayscale_jpg(input_txt_path, output_jpg_path)
