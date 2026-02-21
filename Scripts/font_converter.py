"""
字体转换脚本 - 将TTF字体转换为C语言数组
Font Converter - Convert TTF fonts to C language arrays

支持配置:
- 排列方式 (column_row / row_column)
- 比特顺序 (lsb / msb)
- 编码方式 (positive / negative)

适用于单色屏 (如 SSD1306)
"""

import numpy as np
from PIL import Image, ImageDraw, ImageFont
import os
from datetime import datetime


class FontConverter:
    """TTF字体转C数组转换器"""
    
    def __init__(self, ttf_path, font_size, config=None):
        """
        初始化字体转换器
        
        Args:
            ttf_path (str): TTF字体文件路径
            font_size (int): 字体大小（像素）
            config (dict): 配置字典，包含:
                - layout: 'column_row' 或 'row_column'
                - bit_order: 'lsb' 或 'msb'
                - encoding: 'positive' 或 'negative'
        """
        self.ttf_path = ttf_path
        self.font_size = font_size
        
        # 默认配置 (SSD1306标准: 列行式, LSB在前, 阳码)
        self.config = {
            'layout': 'column_row',
            'bit_order': 'lsb',
            'encoding': 'positive'
        }
        if config:
            self.config.update(config)
        
        # 加载字体
        try:
            self.font = ImageFont.truetype(ttf_path, font_size)
        except Exception as e:
            raise Exception(f"无法加载字体文件: {ttf_path}\n错误信息: {e}")
        
        # 计算字符宽度和高度
        self._calculate_dimensions()
    
    def _calculate_dimensions(self):
        """计算字符的宽度和高度"""
        # 创建临时图像以测量文本
        test_img = Image.new('1', (256, 256), color=0)
        test_draw = ImageDraw.Draw(test_img)
        
        # 测量字符 'M' 来获取宽度
        bbox = test_draw.textbbox((0, 0), 'M', font=self.font)
        self.char_width = bbox[2] - bbox[0]
        
        # 字高 = 字体大小
        self.char_height = self.font_size
    
    def render_character(self, char):
        """
        渲染单个字符为像素数组
        
        Args:
            char (str): 要渲染的字符
            
        Returns:
            numpy.ndarray: 像素矩阵 (height x width), 0=黑, 1=白
        """
        # 创建临时图像
        img = Image.new('1', (self.char_width, self.char_height), color=0)
        draw = ImageDraw.Draw(img)
        
        # 绘制字符
        draw.text((0, 0), char, font=self.font, fill=1)
        
        # 转换为numpy数组
        pixels = np.array(img, dtype=np.uint8)
        return pixels
    
    def pixels_to_bytes(self, pixels):
        """
        将像素数组转换为字节数组
        
        Args:
            pixels (numpy.ndarray): 像素矩阵 (height x width)
            
        Returns:
            list: 字节列表
        """
        height, width = pixels.shape
        bytes_list = []
        
        if self.config['layout'] == 'column_row':
            # 列行式: 先按列，再按行
            for col in range(width):
                for row in range(0, height, 8):
                    # 提取这一列的8行像素
                    byte_val = 0
                    for bit in range(8):
                        pixel_row = row + bit
                        if pixel_row < height:
                            pixel = pixels[pixel_row, col]
                            
                            if self.config['encoding'] == 'positive':
                                # 阳码: 亮点为1
                                bit_val = 1 if pixel else 0
                            else:
                                # 阴码: 亮点为0
                                bit_val = 0 if pixel else 1
                            
                            if self.config['bit_order'] == 'lsb':
                                # LSB在前: 最低位对应最上面的像素
                                byte_val |= (bit_val << bit)
                            else:
                                # MSB在前: 最高位对应最上面的像素
                                byte_val |= (bit_val << (7 - bit))
                    
                    bytes_list.append(byte_val)
        
        elif self.config['layout'] == 'row_column':
            # 行列式: 先按行，再按列
            for row in range(0, height, 8):
                for col in range(width):
                    byte_val = 0
                    for bit in range(8):
                        pixel_row = row + bit
                        if pixel_row < height:
                            pixel = pixels[pixel_row, col]
                            
                            if self.config['encoding'] == 'positive':
                                bit_val = 1 if pixel else 0
                            else:
                                bit_val = 0 if pixel else 1
                            
                            if self.config['bit_order'] == 'lsb':
                                byte_val |= (bit_val << bit)
                            else:
                                byte_val |= (bit_val << (7 - bit))
                    
                    bytes_list.append(byte_val)
        
        return bytes_list
    
    def convert_ascii(self, start_char=32, end_char=126):
        """
        转换ASCII字符集 (默认32-126)
        
        Args:
            start_char (int): 起始ASCII码
            end_char (int): 结束ASCII码
            
        Returns:
            tuple: (字节数组, 字符个数)
        """
        all_bytes = []
        
        for ascii_code in range(start_char, end_char + 1):
            char = chr(ascii_code)
            pixels = self.render_character(char)
            bytes_list = self.pixels_to_bytes(pixels)
            all_bytes.extend(bytes_list)
        
        char_count = end_char - start_char + 1
        return all_bytes, char_count
    
    def generate_c_header(self, font_name, all_bytes, char_count, start_char=32, end_char=126):
        """
        生成C语言头文件内容
        
        Args:
            font_name (str): 字体名称
            all_bytes (list): 所有字节数据
            char_count (int): 字符个数
            start_char (int): 起始ASCII码
            end_char (int): 结束ASCII码
            
        Returns:
            str: 头文件内容
        """
        bytes_per_char = (self.char_height + 7) // 8
        if self.config['layout'] == 'column_row':
            bytes_per_char *= self.char_width
        else:
            bytes_per_char *= self.char_width
        
        width_height = self.char_width * 100 + self.char_height
        
        header = f"""/**
 * Auto-generated font file from {os.path.basename(self.ttf_path)}
 * Font Size: {self.font_size}px
 * Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
 * 
 * Layout: {self.config['layout']}
 * Bit Order: {self.config['bit_order']}
 * Encoding: {self.config['encoding']}
 */

#ifndef __{font_name.upper()}_H
#define __{font_name.upper()}_H

#include "WouoUI_font.h"

/* Font data for {font_name} */
extern const uint8_t {font_name}[];

/* Font structure */
extern const sFONT Font_Width_Height;

#endif

"""
        return header
    
    def generate_c_source(self, font_name, all_bytes, char_count, start_char=32, end_char=126):
        """
        生成C语言源文件内容
        
        Args:
            font_name (str): 字体名称
            all_bytes (list): 所有字节数据
            char_count (int): 字符个数
            start_char (int): 起始ASCII码
            end_char (int): 结束ASCII码
            
        Returns:
            str: 源文件内容
        """
        bytes_per_char = (self.char_height + 7) // 8 * self.char_width
        width_height = self.char_width * 100 + self.char_height
        
        # 生成字节数组
        bytes_str = ""
        for i, byte_val in enumerate(all_bytes):
            if i % 12 == 0:
                bytes_str += "\n    "
            bytes_str += f"0x{byte_val:02X}"
            if i < len(all_bytes) - 1:
                bytes_str += ", "
        
        source = f"""/**
 * Auto-generated font file from {os.path.basename(self.ttf_path)}
 * Font Size: {self.font_size}px
 * Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
 * 
 * Layout: {self.config['layout']}
 * Bit Order: {self.config['bit_order']}
 * Encoding: {self.config['encoding']}
 * ASCII Range: {start_char}-{end_char} (Total: {char_count} characters)
 */

#include "WouoUI_font.h"

/* Font data array */
const uint8_t {font_name}[] = {{{bytes_str}
}};

/* Font structure definition */
const sFONT Font_Width_Height = {{
    .table = {font_name},
    .WidthHeight = {width_height},
    .Width = {self.char_width},
    .Height = {self.char_height}
}};

"""
        return source
    
    def _sanitize_name(self, name):
        """
        清理不符合C命名规范的字符
        
        Args:
            name (str): 原始名称
            
        Returns:
            str: 清理后的名称
        """
        # 将非字母数字和下划线的字符替换为下划线
        result = ""
        for char in name:
            if char.isalnum() or char == '_':
                result += char
            else:
                result += "_"
        
        # 确保不以数字开头（C语言规范）
        if result and result[0].isdigit():
            result = "_" + result
        
        return result
    
    def save_font(self, output_dir, font_name, start_char=32, end_char=126):
        """
        保存生成的C文件
        
        Args:
            output_dir (str): 输出目录
            font_name (str): 字体名称 (不带扩展名)
            start_char (int): 起始ASCII码
            end_char (int): 结束ASCII码
            
        Returns:
            tuple: (header_file_path, source_file_path)
        """
        # 清理字体名称以符合C命名规范
        font_name = self._sanitize_name(font_name)
        
        # 创建输出目录
        os.makedirs(output_dir, exist_ok=True)
        
        # 转换字体
        all_bytes, char_count = self.convert_ascii(start_char, end_char)
        
        # 生成C文件内容
        header_content = self.generate_c_header(font_name, all_bytes, char_count, start_char, end_char)
        source_content = self.generate_c_source(font_name, all_bytes, char_count, start_char, end_char)
        
        # 保存文件
        header_path = os.path.join(output_dir, f"{font_name}.h")
        source_path = os.path.join(output_dir, f"{font_name}.c")
        
        with open(header_path, 'w', encoding='utf-8') as f:
            f.write(header_content)
        
        with open(source_path, 'w', encoding='utf-8') as f:
            f.write(source_content)
        
        return header_path, source_path


def main():
    """主函数 - 用于测试"""
    import sys
    
    if len(sys.argv) < 3:
        print("用法: python font_converter.py <ttf_path> <font_size> [output_dir]")
        print("例如: python font_converter.py ../font/arial.ttf 16 ../Csource/font")
        return
    
    ttf_path = sys.argv[1]
    font_size = int(sys.argv[2])
    output_dir = sys.argv[3] if len(sys.argv) > 3 else os.path.dirname(__file__)
    
    # 默认配置
    config = {
        'layout': 'column_row',
        'bit_order': 'lsb',
        'encoding': 'positive'
    }
    
    converter = FontConverter(ttf_path, font_size, config)
    font_name = os.path.splitext(os.path.basename(ttf_path))[0] + f"_{font_size}"
    
    header_path, source_path = converter.save_font(output_dir, font_name)
    
    print(f"✓ 字体转换成功!")
    print(f"  Header: {header_path}")
    print(f"  Source: {source_path}")
    print(f"  字体大小: {converter.char_width}x{converter.char_height}")


if __name__ == "__main__":
    main()
