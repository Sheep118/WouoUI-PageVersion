"""
C文件生成器 - 将字体buff转换为C语言代码
C File Generator - Convert font buffers to C code

用于生成.c和.h文件，支持多个字体尺寸放在同一文件中
"""

from datetime import datetime
import os


class CFileGenerator:
    """C文件生成器"""
    
    def __init__(self, font_name, config=None):
        """
        初始化C文件生成器
        
        Args:
            font_name (str): 字体名称（用于生成.c和.h文件名）
            config (dict): 配置字典，包含:
                - layout: 'column_row' 或 'row_column'
                - bit_order: 'lsb' 或 'msb'
                - encoding: 'positive' 或 'negative'
        """
        self.font_name = self._sanitize_name(font_name)
        
        # 默认配置
        self.config = {
            'layout': 'row_column',
            'bit_order': 'lsb',
            'encoding': 'positive'
        }
        if config:
            self.config.update(config)
        
        # 存储多个尺寸的字体数据
        self.font_sizes = []  # 列表: [{'size': 12, 'data': {...}}, ...]
    
    def add_font_size(self, font_size, char_data, start_char=32, end_char=126):
        """
        添加一个字体尺寸的数据
        
        Args:
            font_size (int): 字体大小
            char_data (list): 字符数据列表，每项为:
                {
                    'ord': ASCII码,
                    'char': 字符,
                    'width': 宽度,
                    'height': 高度,
                    'pixels': 像素矩阵,
                    'bytes': 字节列表
                }
            start_char (int): 起始ASCII码
            end_char (int): 结束ASCII码
        """
        self.font_sizes.append({
            'size': font_size,
            'data': char_data,
            'start_char': start_char,
            'end_char': end_char
        })
    
    def _sanitize_name(self, name):
        """
        清理不符合C命名规范的字符
        
        Args:
            name (str): 原始名称
            
        Returns:
            str: 清理后的名称
        """
        result = ""
        for char in name:
            if char.isalnum() or char == '_':
                result += char
            else:
                result += "_"
        
        # 确保不以数字开头
        if result and result[0].isdigit():
            result = "_" + result
        
        return result
    
    def _get_struct_name(self, width, height):
        """获取指定实际宽高的结构体名称"""
        return f"{self.font_name}_{width}x{height}_t"
    
    def _get_array_name(self, width, height):
        """获取指定实际宽高的数组名称"""
        return f"{self.font_name}_{width}x{height}"
    
    def generate_header(self):
        """
        生成头文件内容
        
        Returns:
            str: 头文件内容
        """
        guard_name = f"__FONT_{self.font_name.upper()}_H"
        
        header = f"""/**
 * Auto-generated font header file
 * Font: {self.font_name}
 * Sizes: {', '.join([str(s['size']) + 'px' for s in self.font_sizes])}
 * Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
 * 
 * Layout: {self.config['layout']}
 * Bit Order: {self.config['bit_order']}
 * Encoding: {self.config['encoding']}
 */

#ifndef {guard_name}
#define {guard_name}

#include <stdint.h>
#include "WouoUI_font.h"

"""
        
        # 为每个尺寸添加外部声明
        for size_info in self.font_sizes:
            size = size_info['size']
            char_data = size_info['data']

            if char_data:
                unified_width = self._get_unified_width(char_data)
                baseline_metrics = self._get_baseline_metrics(char_data)
                unified_height = baseline_metrics['total_height']
            else:
                unified_width = 0
                unified_height = 0

            array_name = self._get_array_name(unified_width, unified_height)
            struct_name = self._get_struct_name(unified_width, unified_height)
            
            header += f"/* Font data for {size}px */\n"
            header += f"extern const uint8_t {array_name}[];\n"
            header += f"extern const sFONT {struct_name};\n\n"
        
        header += f"#endif /* {guard_name} */\n"
        return header
    
    def _get_unified_width(self, char_data):
        """
        获取统一的字符宽度（所有字符的最大宽度 + padding）
        
        Args:
            char_data (list): 字符数据列表
            
        Returns:
            int: 统一的宽度
        """
        if not char_data:
            return 0
        
        # 获取最大字符宽度
        max_width = max([item['width'] for item in char_data])
        
        # 获取padding配置
        spacing_config = self.config.get('char_spacing', {})
        left_pad = spacing_config.get('left_pad', 0)
        right_pad = spacing_config.get('right_pad', 0)
        
        # 统一宽度 = 最大宽度 + 左padding + 右padding
        unified_width = max_width + left_pad + right_pad
        
        return unified_width
    
    def _get_unified_height(self, char_data):
        """
        获取统一的字符高度（所有字符的最大高度）
        
        Args:
            char_data (list): 字符数据列表
            
        Returns:
            int: 统一的高度
        """
        if not char_data:
            return 0
        return max([item['height'] for item in char_data])
    
    def _rebuild_bytes_with_unified_width(self, char_item, unified_width, unified_height, baseline_offset=0):
        """
        使用统一的宽度和高度重新生成字节数据（按基线对齐）
        支持字符居中对齐和padding
        
        Args:
            char_item (dict): 字符项
            unified_width (int): 统一的宽度
            unified_height (int): 统一的高度
            baseline_offset (int): 该字符相对于顶部的偏移量（用于基线对齐）
            
        Returns:
            list: 新的字节列表
        """
        import numpy as np
        
        original_width = char_item['width']
        original_height = char_item['height']
        
        # 获取padding和对齐配置
        spacing_config = self.config.get('char_spacing', {})
        left_pad = spacing_config.get('left_pad', 0)
        right_pad = spacing_config.get('right_pad', 0)
        center_align = spacing_config.get('center_align', True)
        
        # 如果有pixels，从pixels重新生成字节（最可靠的方法）
        if 'pixels' in char_item and char_item['pixels'] is not None:
            pixels = char_item['pixels']
            
            # 创建统一大小的新像素矩阵
            new_pixels = np.zeros((unified_height, unified_width), dtype=np.uint8)
            
            # 计算字符的水平位置（左侧偏移）
            # 首先考虑左padding
            available_width = unified_width - left_pad - right_pad
            
            if center_align and original_width < available_width:
                # 居中对齐：计算需要补充的空白
                total_padding = available_width - original_width
                left_offset = left_pad + total_padding // 2
            else:
                # 左对齐：直接加上left_pad
                left_offset = left_pad
            
            # 将原始像素复制到对齐的位置
            copy_height = min(pixels.shape[0], unified_height - baseline_offset)
            copy_width = min(pixels.shape[1], unified_width - left_offset)
            
            if copy_height > 0 and copy_width > 0:
                new_pixels[baseline_offset:baseline_offset + copy_height, 
                          left_offset:left_offset + copy_width] = pixels[:copy_height, :copy_width]
            
            # 重新生成字节
            return self._pixels_to_bytes(new_pixels, unified_width, unified_height)
        
        # 如果没有pixels但有原始字节，也需要调整为统一的宽度和高度
        # 这是一个回退方案，理想情况下应该总有pixels
        bytes_per_row_original = (original_width + 7) // 8
        bytes_per_row_unified = (unified_width + 7) // 8
        
        new_bytes = []
        
        # 遍历统一高度的每一行
        for row_idx in range(unified_height):

            if row_idx < original_height:
                # 原始字符有这一行，复制并补齐
                row_start = row_idx * bytes_per_row_original
                row_end = row_start + bytes_per_row_original
                row_bytes = list(char_item['bytes'][row_start:row_end])
            else:
                # 超过原始高度，全部补零
                row_bytes = []
            
            # 补齐至统一宽度对应的字节数
            while len(row_bytes) < bytes_per_row_unified:
                row_bytes.append(0)
            
            new_bytes.extend(row_bytes[:bytes_per_row_unified])
        
        return new_bytes
    
    def _pixels_to_bytes(self, pixels, width, height):
        """
        将像素矩阵转换为字节数组
        
        Args:
            pixels: numpy像素矩阵 (height x width)
            width: 宽度
            height: 高度
            
        Returns:
            list: 字节列表
        """
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
    
    def _get_baseline_metrics(self, char_data):
        """
        获取字符集的基线信息
        
        Args:
            char_data (list): 字符数据列表
            
        Returns:
            dict: 包含baseline_offsets的字典
        """
        baseline_offsets = {}
        
        if not char_data:
            return {'offsets': {}, 'total_height': 0, 'max_bitmap_top': 0}
        
        # 获取所有字符的bitmap_top信息
        max_bitmap_top = 0
        min_below_baseline = 0
        
        for item in char_data:
            bitmap_top = item.get('baseline_top', item['height'])
            max_bitmap_top = max(max_bitmap_top, bitmap_top)
            below = bitmap_top - item['height']
            min_below_baseline = min(min_below_baseline, below)
        
        # 计算总高度（包括descender）
        total_height = max_bitmap_top - min_below_baseline
        
        # 计算每个字符的偏移量（从顶部开始有多少行空白）
        for item in char_data:
            bitmap_top = item.get('baseline_top', item['height'])
            offset = max_bitmap_top - bitmap_top
            baseline_offsets[item['ord']] = offset
        
        return {
            'offsets': baseline_offsets,
            'total_height': total_height,
            'max_bitmap_top': max_bitmap_top,
            'min_below_baseline': min_below_baseline
        }
    
    def generate_source(self):
        """
        生成源文件内容
        
        Returns:
            str: 源文件内容
        """
        source = f"""/**
 * Auto-generated font source file
 * Font: {self.font_name}
 * Sizes: {', '.join([str(s['size']) + 'px' for s in self.font_sizes])}
 * Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
 * 
 * Layout: {self.config['layout']}
 * Bit Order: {self.config['bit_order']}
 * Encoding: {self.config['encoding']}
 */

#include "Font_{self.font_name}.h"

"""
        
        # 为每个尺寸生成数据
        for size_info in self.font_sizes:
            size = size_info['size']
            char_data = size_info['data']
            start_char = size_info['start_char']
            end_char = size_info['end_char']
            # 计算统一的字符宽度和高度
            if char_data:
                unified_width = self._get_unified_width(char_data)
                # 获取基线对齐信息
                baseline_metrics = self._get_baseline_metrics(char_data)
                baseline_offsets = baseline_metrics['offsets']
                unified_height = baseline_metrics['total_height']
            else:
                unified_width = 0
                unified_height = 0
                baseline_offsets = {}

            array_name = self._get_array_name(unified_width, unified_height)
            struct_name = self._get_struct_name(unified_width, unified_height)
            
            # 生成字节数据
            source += f"/* Font data for {size}px - {unified_width}x{unified_height} per character (baseline aligned) */\n"
            source += f"const uint8_t {array_name}[] = {{\n"
            
            lines = []
            current_line = []
            bytes_per_line = 8
            
            for ascii_code in range(start_char, end_char + 1):
                # 查找对应的字符数据
                char_item = None
                for item in char_data:
                    if item['ord'] == ascii_code:
                        char_item = item
                        break
                
                if char_item is None:
                    continue
                
                # 获取该字符的基线偏移量
                char_baseline_offset = baseline_offsets.get(ascii_code, 0)
                
                # 使用统一的宽度和高度重新生成字节（按基线对齐）
                bytes_list = self._rebuild_bytes_with_unified_width(
                    char_item, unified_width, unified_height, char_baseline_offset
                )
                
                if not bytes_list:
                    continue
                
                # 生成字符注释
                if ascii_code == 32:
                    char_comment = "sp"
                elif ascii_code == 92:
                    char_comment = "\\\\"
                elif 32 <= ascii_code < 127:
                    char_comment = chr(ascii_code)
                else:
                    char_comment = f"U+{ascii_code:04X}"
                
                # 添加字节到当前行
                for byte_idx, byte_val in enumerate(bytes_list):
                    # 添加逗号（除了最后）
                    hex_str = f"0x{byte_val:02X}"
                    
                    # 检查是否是最后一行的最后一个字节
                    is_last_byte = (ascii_code == end_char and byte_idx == len(bytes_list) - 1)
                    
                    if is_last_byte:
                        current_line.append(hex_str)
                    else:
                        current_line.append(hex_str + ",")
                    
                    # 检查是否需要换行
                    if len(current_line) == bytes_per_line or byte_idx == len(bytes_list) - 1:
                        # 输出当前行
                        line_str = "    " + " ".join(current_line)
                        
                        # 添加注释（如果字符结束）
                        if byte_idx == len(bytes_list) - 1:
                            line_str += f" /* {char_comment} */"
                        
                        lines.append(line_str)
                        current_line = []
            
            source += "\n".join(lines) + "\n"
            source += "};\n\n"
            
            # 生成结构体定义（与当前sFONT保持一致）
            source += f"const sFONT {struct_name} = {{\n"
            source += f"    .table = {array_name},\n"
            source += f"    .Width = {unified_width},\n"
            source += f"    .Height = {unified_height}\n"
            source += "};\n\n"
        
        return source
    
    def save_files(self, output_dir):
        """
        保存.c和.h文件
        
        Args:
            output_dir (str): 输出目录
            
        Returns:
            tuple: (头文件路径, 源文件路径)
        """
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        base_name = f"Font_{self.font_name}"
        header_path = os.path.join(output_dir, f"{base_name}.h")
        source_path = os.path.join(output_dir, f"{base_name}.c")
        
        # 保存头文件
        with open(header_path, 'w', encoding='utf-8') as f:
            f.write(self.generate_header())
        
        # 保存源文件
        with open(source_path, 'w', encoding='utf-8') as f:
            f.write(self.generate_source())
        
        return header_path, source_path
    
    def get_summary(self):
        """
        获取生成的文件摘要
        
        Returns:
            str: 摘要信息
        """
        summary = f"Font: {self.font_name}\n"
        summary += f"Sizes: {len(self.font_sizes)} sizes\n"
        
        for size_info in self.font_sizes:
            size = size_info['size']
            char_data = size_info['data']
            if char_data:
                # 获取实际字体宽度和padding信息
                actual_width = max([item['width'] for item in char_data])
                spacing_config = self.config.get('char_spacing', {})
                left_pad = spacing_config.get('left_pad', 0)
                right_pad = spacing_config.get('right_pad', 0)
                
                unified_width = self._get_unified_width(char_data)
                unified_height = self._get_unified_height(char_data)
                
                # 显示详细信息
                if left_pad > 0 or right_pad > 0:
                    summary += f"  {size}px: {unified_width}x{unified_height} (actual {actual_width}+{left_pad+right_pad} padding), {len(char_data)} characters\n"
                else:
                    summary += f"  {size}px: {unified_width}x{unified_height}, {len(char_data)} characters\n"
        
        summary += f"Config:\n"
        summary += f"  Layout: {self.config['layout']}\n"
        summary += f"  Bit Order: {self.config['bit_order']}\n"
        summary += f"  Encoding: {self.config['encoding']}\n"
        
        # 显示字符间距配置
        spacing_config = self.config.get('char_spacing', {})
        if spacing_config:
            summary += f"  Char Spacing:\n"
            summary += f"    Left Padding: {spacing_config.get('left_pad', 0)}px\n"
            summary += f"    Right Padding: {spacing_config.get('right_pad', 0)}px\n"
            summary += f"    Center Align: {spacing_config.get('center_align', True)}\n"
        
        return summary
