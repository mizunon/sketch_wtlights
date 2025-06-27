from PIL import Image
import numpy as np
import sys
import argparse

# > python png_to_c_array.py nakamu.png

def png_to_c_array(input_file):
    # 画像を読み込む
    img = Image.open(input_file)
    
    # 画像サイズを確認
    if img.size != (32, 32):
        raise ValueError("Image must be 32x32 pixels")
    
    # RGBAの場合はRGBに変換
    if img.mode == 'RGBA':
        img = img.convert('RGB')
    
    # NumPy配列に変換
    pixels = np.array(img)
    
    # C言語形式の配列文字列を生成
    c_array = []
    for y in range(32):
        for x in range(32):
            r, g, b = pixels[y, x]
            # 24ビットRGB値を16進数で表現
            hex_color = f"0x{r:02x}{g:02x}{b:02x}"
            c_array.append(hex_color)
    
    # カンマ区切りの文字列に変換
    result = "{\n"
    for i in range(0, len(c_array), 8):  # 8個ずつ改行
        result += "    " + ", ".join(c_array[i:i+8])
        if i < len(c_array) - 8:
            result += ","
        result += "\n"
    result += "};"
    
    return result

def main():
    # コマンドライン引数のパース
    parser = argparse.ArgumentParser(description='Convert 32x32 PNG to C array')
    parser.add_argument('input_file', help='Input PNG file path (must be 32x32 pixels)')
    args = parser.parse_args()

    try:
        output = png_to_c_array(args.input_file)
        print("const uint32_t image_data[1024] = ")
        print(output)
    except FileNotFoundError:
        print(f"Error: File '{args.input_file}' not found")
    except ValueError as e:
        print(f"Error: {e}")
    except Exception as e:
        print(f"Error: An unexpected error occurred: {e}")

if __name__ == "__main__":
    main()