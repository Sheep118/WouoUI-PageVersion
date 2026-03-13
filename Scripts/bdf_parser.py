"""
BDF字体文件解析器 - 使用freetype-py
BDF Font Parser - Using freetype-py

使用freetype库解析BDF字体，将字符渲染为像素缓冲区
"""

import freetype
import numpy as np
from datetime import datetime
import yaml
import os


class BDFParser:
    """BDF字体解析器（使用freetype）"""
    
    def __init__(self, bdf_path, font_size, config=None):
        """
        初始化BDF解析器
        
        Args:
            bdf_path (str): BDF字体文件路径
            font_size (int): 字体大小（像素）
            config (dict): 配置字典，包含:
                - layout: 'column_row' 或 'row_column'
                - bit_order: 'lsb' 或 'msb'
                - encoding: 'positive' 或 'negative'
        """
        self.bdf_path = bdf_path
        self.font_size = font_size
        
        # 默认配置
        self.config = {
            'layout': 'row_column',
            'bit_order': 'lsb',
            'encoding': 'positive'
        }
        if config:
            self.config.update(config)
        
        # 加载字体
        try:
            self.face = freetype.Face(bdf_path)
            self.face.set_pixel_sizes(0, font_size)
        except Exception as e:
            raise Exception(f"无法加载BDF字体文件: {bdf_path}\n错误信息: {e}")
        
        # 获取字体度量信息
        self.ascender = self.face.size.ascender / 64
        self.descender = self.face.size.descender / 64
        self.height = self.face.size.height / 64
        self.max_advance = self.face.size.max_advance / 64
    
    def render_character(self, char):
        """
        渲染单个字符为像素缓冲区
        
        Args:
            char (str): 要渲染的字符
            
        Returns:
            tuple: (像素矩阵, 宽度, 高度)
                  像素矩阵为numpy数组 (height x width)，0=黑, 1=白
        """
        try:
            self.face.load_char(char, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
        except Exception as e:
            return None, 0, 0
        
        bitmap = self.face.glyph.bitmap
        
        if bitmap.width == 0 or bitmap.rows == 0:
            return None, 0, 0
        
        # 从freetype位图数据转换为像素矩阵
        pixels = np.zeros((bitmap.rows, bitmap.width), dtype=np.uint8)
        
        for row in range(bitmap.rows):
            for col in range(bitmap.width):
                # 计算字节位置和位偏移
                byte_index = row * bitmap.pitch + col // 8
                bit_index = 7 - (col % 8)
                
                # 获取对应的bit
                if byte_index < len(bitmap.buffer):
                    byte_value = bitmap.buffer[byte_index]
                    bit = (byte_value >> bit_index) & 1
                    pixels[row, col] = bit
        
        return pixels, bitmap.width, bitmap.rows
    
    def get_glyph_metrics(self, char):
        """
        获取字符的度量信息
        
        Args:
            char (str): 要查询的字符
            
        Returns:
            dict: 度量信息
        """
        try:
            self.face.load_char(char, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
        except:
            return None
        
        bitmap = self.face.glyph.bitmap
        return {
            'width': bitmap.width,
            'height': bitmap.rows,
            'bitmap_top': self.face.glyph.bitmap_top,
            'bitmap_left': self.face.glyph.bitmap_left,
            'advance_x': self.face.glyph.advance.x // 64
        }
    
    def render_string(self, text):
        """
        渲染字符串
        
        Args:
            text (str): 要渲染的字符串
            
        Returns:
            list: 每个字符的 (像素矩阵, 宽度, 高度) 列表
        """
        result = []
        for char in text:
            pixels, width, height = self.render_character(char)
            if pixels is not None:
                result.append({
                    'char': char,
                    'pixels': pixels,
                    'width': width,
                    'height': height,
                    'ord': ord(char)
                })
        return result
    
    def pixels_to_bytes(self, pixels):
        """
        将像素矩阵转换为字节数组
        
        Args:
            pixels (numpy.ndarray): 像素矩阵 (height x width)
            
        Returns:
            list: 字节列表
        """
        if pixels is None:
            return []
        
        height, width = pixels.shape
        bytes_list = []
        
        if self.config['layout'] == 'column_row':
            # 列行式: 先列后行
            for col in range(width):
                for row in range((height + 7) // 8):
                    byte_val = 0
                    for bit in range(8):
                        pixel_row = row * 8 + bit
                        if pixel_row < height:
                            if self.config['bit_order'] == 'lsb':
                                byte_val |= (pixels[pixel_row, col] << bit)
                            else:  # msb
                                byte_val |= (pixels[pixel_row, col] << (7 - bit))
                    
                    if self.config['encoding'] == 'negative':
                        byte_val = ~byte_val & 0xFF
                    bytes_list.append(byte_val)
        
        else:  # row_column
            # 行列式: 先行后列
            for row in range((height + 7) // 8):
                for col in range(width):
                    byte_val = 0
                    for bit in range(8):
                        pixel_row = row * 8 + bit
                        if pixel_row < height:
                            if self.config['bit_order'] == 'lsb':
                                byte_val |= (pixels[pixel_row, col] << bit)
                            else:  # msb
                                byte_val |= (pixels[pixel_row, col] << (7 - bit))
                    
                    if self.config['encoding'] == 'negative':
                        byte_val = ~byte_val & 0xFF
                    bytes_list.append(byte_val)
        
        return bytes_list
    
    def render_ascii(self, start_char=32, end_char=126):
        """
        渲染ASCII字符集
        
        Args:
            start_char (int): 起始ASCII码
            end_char (int): 结束ASCII码
            
        Returns:
            list: 每个字符的信息列表
        """
        result = []
        for ascii_code in range(start_char, end_char + 1):
            char = chr(ascii_code)
            pixels, width, height = self.render_character(char)
            if pixels is not None:
                result.append({
                    'char': char,
                    'pixels': pixels,
                    'width': width,
                    'height': height,
                    'ord': ascii_code,
                    'bytes': self.pixels_to_bytes(pixels)
                })
        return result

    def render_charset(self, chars):
        """渲染指定字符集合，返回成功字形与缺失字符。"""
        result = []
        missing_chars = []

        for char in chars:
            pixels, width, height = self.render_character(char)
            if pixels is None or width <= 0 or height <= 0:
                missing_chars.append(char)
                continue

            metrics = self.get_glyph_metrics(char)
            baseline_top = metrics['bitmap_top'] if metrics else height
            result.append({
                'char': char,
                'pixels': pixels,
                'width': width,
                'height': height,
                'ord': ord(char),
                'bytes': self.pixels_to_bytes(pixels),
                'baseline_top': baseline_top
            })

        return result, missing_chars
    
    def print_pixels_preview(self, pixels, label=None):
        """
        打印像素矩阵的ASCII艺术预览
        
        Args:
            pixels (numpy.ndarray): 像素矩阵
            label (str): 标签
        """
        if label:
            print(f"  {label}")
        
        for row in pixels:
            line = ""
            for pixel in row:
                line += "#" if pixel else " "
            print(f"  {line}")


def main():
    """主函数 - 用于单独测试"""
    print("=" * 60)
    print("BDF字体解析器 - freetype-py 版本")
    print("=" * 60)
    print()
    
    # 读取config文件中的preview配置
    config_path = os.path.join(os.path.dirname(__file__), 'config.yaml')
    preview_config = {}
    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            full_config = yaml.safe_load(f)
            preview_config = full_config.get('preview', {})
    except Exception as e:
        print(f"警告: 无法读取config文件: {e}")
        preview_config = {
            'enabled': True,
            'print_output': True,
            'text': 'Hello WouoUI'
        }
    
    # 配置参数
    bdf_file = r"../font/unifont/unifont-17_0_03.bdf"
    font_size = 16
    text_to_render = preview_config.get('text', 'Hello WouoUI')
    
    print(f"字体文件: {bdf_file}")
    print(f"字体大小: {font_size}px")
    print(f"渲染文本: {text_to_render}")
    print()
    
    # 创建解析器
    try:
        parser = BDFParser(bdf_file, font_size)
        print(f"[OK] 字体加载成功")
        print(f"  字体度量: ascender={parser.ascender:.1f}, descender={parser.descender:.1f}, height={parser.height:.1f}")
        print()
    except Exception as e:
        print(f"[ERROR] 字体加载失败: {e}")
        return
    
    # 渲染字符串
    if preview_config.get('enabled', True) and preview_config.get('print_output', True):
        char_list = parser.render_string(text_to_render)
        
        if not char_list:
            print("无法渲染任何字符")
            return
        
        # 计算统一的宽度
        max_width = max([item['width'] for item in char_list])
        
        print(f"渲染结果 ({len(char_list)} 个字符):")
        print()
        
        # 打印各个字符的宽高
        print("字符度量信息:")
        for item in char_list:
            print(f"  '{item['char']}' (U+{item['ord']:04X}): {item['width']}x{item['height']}")
        
        print()
        print(f"统一宽度: {max_width}px")
        print()
        
        # 获取每个字符的度量信息用于对齐
        metrics_list = []
        for item in char_list:
            metrics = parser.get_glyph_metrics(item['char'])
            metrics_list.append(metrics if metrics else {})
        
        if metrics_list and all(metrics_list):
            # 计算基线信息
            max_bitmap_top = max([m['bitmap_top'] for m in metrics_list if m])
            min_below_baseline = min([m['bitmap_top'] - m['height'] for m in metrics_list if m])
            total_height = max_bitmap_top - min_below_baseline
            
            print(f"基线对齐:")
            print(f"  最高点: {max_bitmap_top}px")
            print(f"  最低点: {min_below_baseline}px")
            print(f"  总高度: {total_height}px")
            print()
            
            # 打印对齐的字符串
            print("字符串预览 (#=像素, 空格=空白):")
            print("=" * (max_width * len(char_list) + len(char_list) + 10))
            
            for row in range(total_height):
                line_marker = "→" if row == max_bitmap_top - 1 else " "
                for char_item, metrics in zip(char_list, metrics_list):
                    if not metrics:
                        print(' ' * max_width, end=' ')
                        continue
                    
                    # 计算当前行相对于该字符的位置
                    char_row = row - (max_bitmap_top - metrics['bitmap_top'])
                    
                    # 如果该行在当前字符的位图范围内
                    if 0 <= char_row < char_item['height']:
                        pixels = char_item['pixels']
                        for col in range(min(char_item['width'], max_width)):
                            if col < pixels.shape[1]:
                                print("#" if pixels[char_row, col] else " ", end="")
                            else:
                                print(" ", end="")
                        # 补齐宽度
                        for _ in range(char_item['width'], max_width):
                            print(" ", end="")
                    else:
                        # 该行不在字符范围内，打印空格
                        for _ in range(max_width):
                            print(" ", end="")
                    
                    print(" ", end="")
                
                print(f" {line_marker}")
            
            print("=" * (max_width * len(char_list) + len(char_list) + 10))


if __name__ == "__main__":
    main()
