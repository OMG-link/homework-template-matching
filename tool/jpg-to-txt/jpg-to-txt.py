from PIL import Image
import sys

def jpg_to_grayscale_text(input_path, output_path):
    # 打开图像并转换为灰度图
    img = Image.open(input_path).convert('L')
    
    # 获取图像的宽度和高度
    width, height = img.size
    
    # 获取灰度值矩阵
    pixels = list(img.getdata())
    
    # 将灰度值矩阵重新排列为二维列表
    pixels = [pixels[i * width:(i + 1) * width] for i in range(height)]
    
    # 写入文本文件
    with open(output_path, 'w') as f:
        f.write(f"{height} {width}\n")
        for row in pixels:
            f.write(" ".join(map(str, row)) + "\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_jpg_path> <output_txt_path>")
    else:
        input_jpg_path = sys.argv[1]
        output_txt_path = sys.argv[2]
        jpg_to_grayscale_text(input_jpg_path, output_txt_path)
