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

    def __init__(self, ttf_path, font_size, target_width=None, target_height=None, config=None):
        """
        初始化字体转换器
        
        Args:
            ttf_path (str): TTF字体文件路径
            font_size (int): 字体大小（像素）
            target_width (int): 目标字宽（像素），为None时使用字体原始宽度
            target_height (int): 目标字高（像素），为None时使用字体原始高度
            config (dict): 配置字典，包含:
                - layout: 'column_row' 或 'row_column'
                - bit_order: 'lsb' 或 'msb'
                - encoding: 'positive' 或 'negative'
        """
        self.ttf_path = ttf_path
        self.font_size = font_size
        self.target_width = target_width
        self.target_height = target_height
        
        # 默认配置 (SSD1306标准: 行列式, LSB在前, 阳码)
        self.config = {
            'layout': 'row_column',
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
        test_img = Image.new('L', (256, 256), color=0)
        test_draw = ImageDraw.Draw(test_img)

        # 测量字符 'M' 来获取宽度
        bbox = test_draw.textbbox((0, 0), 'M', font=self.font)
        measured_width = bbox[2] - bbox[0]
        measured_height = bbox[3] - bbox[1]

        # 使用目标尺寸（若提供），否则使用测量值
        self.char_width = int(self.target_width) if self.target_width else measured_width
        self.char_height = int(self.target_height) if self.target_height else measured_height
    
    def render_character(self, char):
        """
        渲染单个字符为像素数组
        
        Args:
            char (str): 要渲染的字符
            
        Returns:
            numpy.ndarray: 像素矩阵 (height x width), 0=黑, 1=白
        """
        # 使用字体度量确定完整高度（基线以上 + 基线以下）
        ascent, descent = self.font.getmetrics()
        base_height = max(1, ascent + descent)

        # 估算字符宽度，避免紧凑bbox导致的拥挤
        if hasattr(self.font, "getlength"):
            base_width = max(1, int(round(self.font.getlength(char))))
        else:
            bbox = self.font.getbbox(char)
            base_width = max(1, bbox[2] - bbox[0])

        glyph_img = Image.new('L', (base_width, base_height), color=0)
        glyph_draw = ImageDraw.Draw(glyph_img)

        baseline_shift = int(self.config.get('baseline_shift', 0))
        baseline_y = ascent - baseline_shift

        # 使用基线锚点，避免小写字母上下错位
        try:
            glyph_draw.text((0, baseline_y), char, font=self.font, fill=255, anchor="ls")
        except TypeError:
            glyph_draw.text((0, baseline_y), char, font=self.font, fill=255)

        # 按目标尺寸缩放（避免保持宽高比）
        if self.char_width != base_width or self.char_height != base_height:
            glyph_img = glyph_img.resize((self.char_width, self.char_height), Image.NEAREST)

        # 转换为二值像素
        pixels = (np.array(glyph_img) > 0).astype(np.uint8)
        return pixels

    def render_preview_text(self, text, spacing=1, line_spacing=1):
        """
        生成预览用的文字图像

        Args:
            text (str): 预览字符串
            spacing (int): 字符间距
            line_spacing (int): 行间距

        Returns:
            PIL.Image.Image: 预览图像
        """
        if not text:
            return Image.new('L', (1, 1), color=0)

        lines = text.split('\n')
        line_images = []

        for line in lines:
            if not line:
                line_images.append(Image.new('L', (1, self.char_height), color=0))
                continue

            widths = [self.char_width for _ in line]
            total_width = sum(widths) + max(0, len(widths) - 1) * spacing
            line_img = Image.new('L', (max(1, total_width), self.char_height), color=0)

            x = 0
            for ch in line:
                pixels = self.render_character(ch)
                packed = self.pixels_to_bytes(pixels)
                roundtrip = self.bytes_to_pixels(packed, self.char_width, self.char_height)
                glyph = Image.fromarray((roundtrip * 255).astype(np.uint8), mode='L')
                line_img.paste(glyph, (x, 0))
                x += self.char_width + spacing

            line_images.append(line_img)

        total_height = len(line_images) * self.char_height + max(0, len(line_images) - 1) * line_spacing
        preview = Image.new('L', (max(img.width for img in line_images), total_height), color=0)

        y = 0
        for img in line_images:
            preview.paste(img, (0, y))
            y += self.char_height + line_spacing

        return preview

    def bytes_to_pixels(self, packed_bytes, width, height):
        """
        将字节数组还原为像素矩阵，用于真实预览

        Args:
            packed_bytes (list): 字节数组
            width (int): 字符宽度
            height (int): 字符高度

        Returns:
            numpy.ndarray: 像素矩阵 (height x width)
        """
        pixels = np.zeros((height, width), dtype=np.uint8)
        index = 0

        if self.config['layout'] == 'column_row':
            for col in range(width):
                for row in range(0, height, 8):
                    if index >= len(packed_bytes):
                        break
                    byte_val = packed_bytes[index]
                    index += 1
                    for bit in range(8):
                        pixel_row = row + bit
                        if pixel_row >= height:
                            continue
                        bit_val = (byte_val >> bit) & 0x01 if self.config['bit_order'] == 'lsb' else (byte_val >> (7 - bit)) & 0x01
                        pixel = bit_val if self.config['encoding'] == 'positive' else (1 - bit_val)
                        pixels[pixel_row, col] = pixel

        elif self.config['layout'] == 'row_column':
            for row in range(0, height, 8):
                for col in range(width):
                    if index >= len(packed_bytes):
                        break
                    byte_val = packed_bytes[index]
                    index += 1
                    for bit in range(8):
                        pixel_row = row + bit
                        if pixel_row >= height:
                            continue
                        bit_val = (byte_val >> bit) & 0x01 if self.config['bit_order'] == 'lsb' else (byte_val >> (7 - bit)) & 0x01
                        pixel = bit_val if self.config['encoding'] == 'positive' else (1 - bit_val)
                        pixels[pixel_row, col] = pixel

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
        
        font_struct_name = f"{font_name}_t"

        header = f"""/**
 * Auto-generated font file from {os.path.basename(self.ttf_path)}
 * Font Size: {self.font_size}px
 * Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
 * 
 * Layout: {self.config['layout']}
 * Bit Order: {self.config['bit_order']}cd
 * Encoding: {self.config['encoding']}
 */

#ifndef __{font_name.upper()}_H
#define __{font_name.upper()}_H

#include "WouoUI_font.h"

/* Font data for {font_name} */
extern const uint8_t {font_name}[];

/* Font structure */
extern const sFONT {font_struct_name};

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
        
        # 生成字节数组 (每行8个字节, 每个字符末尾追加注释)
        bytes_per_char = (self.char_height + 7) // 8 * self.char_width
        lines = []
        current_line = []

        def flush_line(comment=None):
            if not current_line:
                return
            line = "    " + " ".join(current_line)
            if comment:
                line += f" /* {comment} */"
            lines.append(line)
            current_line.clear()

        byte_index = 0
        for ascii_code in range(start_char, end_char + 1):
            for _ in range(bytes_per_char):
                byte_val = all_bytes[byte_index]
                current_line.append(f"0x{byte_val:02X},")
                byte_index += 1
                if len(current_line) == 8:
                    flush_line()

            # 该字符结束时，在当前行追加注释
            if ascii_code == 32:
                comment = "sp"
            elif ascii_code == 92:
                comment = "\\\\"
            else:
                comment = chr(ascii_code)

            if current_line:
                flush_line(comment=comment)
            else:
                lines[-1] = lines[-1] + f" /* {comment} */"

        bytes_str = "\n" + "\n".join(lines)
        
        font_struct_name = f"{font_name}_t"

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
const sFONT {font_struct_name} = {{
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
    
    if len(sys.argv) < 4:
        print("用法: python font_converter.py <ttf_path> <font_width> <font_height> [output_dir] [preview_text]")
        print("例如: python font_converter.py ../font/arial.ttf 8 16 ../Csource/font 'Hello'")
        return
    
    ttf_path = sys.argv[1]
    target_width = int(sys.argv[2])
    target_height = int(sys.argv[3])
    output_dir = sys.argv[4] if len(sys.argv) > 4 else os.path.dirname(__file__)
    preview_text = sys.argv[5] if len(sys.argv) > 5 else "Hello WouoUI ghb123!@"
    
    # 默认配置
    config = {
        'layout': 'row_column',
        'bit_order': 'lsb',
        'encoding': 'positive'
    }
    
    converter = FontConverter(ttf_path, target_height, target_width, target_height, config)
    font_name = os.path.splitext(os.path.basename(ttf_path))[0] + f"_{target_width}x{target_height}"

    if preview_text:
        try:
            preview_img = converter.render_preview_text(preview_text)
            preview_img.show(title="WouoUI Font Preview")
        except Exception as e:
            print(f"预览显示失败: {e}")
    
    header_path, source_path = converter.save_font(output_dir, font_name)
    
    print(f"✓ 字体转换成功!")
    print(f"  Header: {header_path}")
    print(f"  Source: {source_path}")
    print(f"  字体大小: {converter.char_width}x{converter.char_height}")


if __name__ == "__main__":
    main()
