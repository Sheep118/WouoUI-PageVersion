"""
BDF字体文件解析器 - 生成C语言数组
BDF Font Parser - Generate C language arrays

解析BDF格式字体文件，渲染指定字符串，生成C语言数组格式输出
"""

class BDFParser:
    """BDF字体解析器（基础版 - 使用标准库）"""
    
    def __init__(self, bdf_path):
        """初始化解析器"""
        self.bdf_path = bdf_path
        self.glyphs = {}
        self.font_info = {}
        self._parse()
    
    def _parse(self):
        """解析BDF文件"""
        with open(self.bdf_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        # 解析字体全局信息
        i = 0
        while i < len(lines):
            line = lines[i].strip()
            
            if line.startswith('FONTBOUNDINGBOX'):
                parts = line.split()
                self.font_info['bbox_width'] = int(parts[1])
                self.font_info['bbox_height'] = int(parts[2])
                self.font_info['bbox_x'] = int(parts[3])
                self.font_info['bbox_y'] = int(parts[4])
            
            elif line.startswith('STARTCHAR'):
                # 解析单个字符
                glyph = self._parse_glyph(lines, i)
                if glyph:
                    self.glyphs[glyph['encoding']] = glyph
                    # 跳过已解析的行
                    while i < len(lines) and not lines[i].strip().startswith('ENDCHAR'):
                        i += 1
            
            i += 1
    
    def _parse_glyph(self, lines, start_idx):
        """解析单个字形"""
        glyph = {}
        i = start_idx
        
        while i < len(lines):
            line = lines[i].strip()
            
            if line.startswith('STARTCHAR'):
                glyph['name'] = line.split(None, 1)[1] if len(line.split(None, 1)) > 1 else ""
            
            elif line.startswith('ENCODING'):
                glyph['encoding'] = int(line.split()[1])
            
            elif line.startswith('BBX'):
                parts = line.split()
                glyph['width'] = int(parts[1])
                glyph['height'] = int(parts[2])
                glyph['x_offset'] = int(parts[3])
                glyph['y_offset'] = int(parts[4])
            
            elif line.startswith('BITMAP'):
                # 读取位图数据
                bitmap_lines = []
                i += 1
                while i < len(lines) and not lines[i].strip().startswith('ENDCHAR'):
                    hex_line = lines[i].strip()
                    if hex_line:
                        bitmap_lines.append(hex_line)
                    i += 1
                glyph['bitmap'] = bitmap_lines
                return glyph
            
            i += 1
        
        return None
    
    def get_glyph(self, char_code):
        """获取字符的字形数据"""
        return self.glyphs.get(char_code)


class BDFToCConverter:
    """BDF字体转C数组转换器"""
    
    def __init__(self, bdf_path):
        """
        初始化转换器
        
        Args:
            bdf_path: BDF字体文件路径
        """
        self.bdf_path = bdf_path
        
        print(f"✓ 解析BDF文件: {bdf_path}")
        print(f"  (正在解析，请稍候...)")
        self.parser = BDFParser(bdf_path)
        print(f"  ✓ 解析完成，共 {len(self.parser.glyphs)} 个字符")
    
    def get_char_bitmap(self, char):
        """获取字符的位图数据"""
        char_code = ord(char)
        return self._get_char_bitmap_basic(char_code)
    
    def _get_char_bitmap_basic(self, char_code):
        """使用基础解析器获取字符位图"""
        glyph = self.parser.get_glyph(char_code)
        if not glyph:
            return None, 0, 0
        
        # 解析十六进制位图数据
        bitmap = []
        width = glyph['width']
        height = glyph['height']
        
        for hex_line in glyph['bitmap']:
            # 将十六进制转换为二进制字符串
            value = int(hex_line, 16)
            binary = bin(value)[2:].zfill(width)
            bitmap.append(binary)
        
        return bitmap, width, height
    
    def bitmap_to_ascii_art(self, bitmap, width, height):
        """将位图转换为ASCII艺术形式（# 和空格）"""
        lines = []
        for row in bitmap:
            if isinstance(row, str):
                # 二进制字符串
                line = row.replace('1', '█').replace('0', ' ')
            else:
                # 可能是其他格式
                line = ''.join(['█' if x else ' ' for x in row])
            lines.append(line)
        return '\n'.join(lines)
    
    def bitmap_to_c_array(self, bitmap, width, height, var_name="char_data"):
        """
        将位图转换为C语言数组
        
        Args:
            bitmap: 位图数据（二进制字符串列表）
            width: 宽度
            height: 高度
            var_name: C数组变量名
        
        Returns:
            C语言数组字符串
        """
        if not bitmap:
            return f"// 字符无位图数据\nconst uint8_t {var_name}[] = {{}};\n"
        
        # 按字节组织数据（每8行为一组，列优先）
        bytes_per_row = (width + 7) // 8
        total_bytes = bytes_per_row * height
        
        c_array = []
        c_lines = []
        
        # 逐行处理
        for row_idx, row in enumerate(bitmap):
            # 确保位图行长度足够
            if isinstance(row, str):
                row_bits = row.ljust(width, '0')[:width]
            else:
                row_bits = ''.join(['#' if x else '.' for x in row]).replace('#', '1').replace('.', '0')
                row_bits = row_bits.ljust(width, '0')[:width]
            
            # 按字节分割
            for byte_idx in range(bytes_per_row):
                start_bit = byte_idx * 8
                end_bit = min(start_bit + 8, width)
                byte_bits = row_bits[start_bit:end_bit].ljust(8, '0')
                byte_value = int(byte_bits, 2)
                c_array.append(f"0x{byte_value:02X}")
        
        # 格式化输出（每行16个字节）
        c_lines.append(f"const uint8_t {var_name}[{len(c_array)}] = {{")
        for i in range(0, len(c_array), 16):
            chunk = c_array[i:i+16]
            c_lines.append("    " + ", ".join(chunk) + ",")
        c_lines.append("};")
        
        return "\n".join(c_lines)
    
    def render_string_to_c_array(self, text, array_name="font_data"):
        """
        渲染字符串并生成C数组
        
        Args:
            text: 要渲染的字符串
            array_name: C数组名称前缀
        
        Returns:
            C语言代码字符串
        """
        result = []
        result.append(f"// 字体数据: \"{text}\"")
        result.append(f"// 生成时间: {self._get_timestamp()}")
        result.append("")
        
        for idx, char in enumerate(text):
            bitmap, width, height = self.get_char_bitmap(char)
            
            if bitmap is None:
                result.append(f"// 字符 '{char}' (U+{ord(char):04X}) 未找到")
                result.append("")
                continue
            
            var_name = f"{array_name}_char_{idx}_{ord(char):04X}"
            result.append(f"// 字符: '{char}' (U+{ord(char):04X}), 尺寸: {width}x{height}")
            
            # 添加ASCII艺术形式的位图预览
            result.append(f"// 位图预览:")
            ascii_art = self.bitmap_to_ascii_art(bitmap, width, height)
            for line in ascii_art.split('\n'):
                result.append(f"// {line}")
            
            result.append(self.bitmap_to_c_array(bitmap, width, height, var_name))
            result.append("")
        
        return "\n".join(result)
    
    def _get_timestamp(self):
        """获取时间戳"""
        from datetime import datetime
        return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def main():
    """主函数"""
    print("=" * 60)
    print("BDF字体文件解析器 - C数组生成工具")
    print("=" * 60)
    print()
    
    # 配置参数
    bdf_file = r"g:\WouoUI\WouoUISDL2\WouoUI-PageVersion\font\unifont\unifont-17_0_03.bdf"
    text_to_render = "Hello 你好"  # 要渲染的字符串
    
    print(f"字体文件: {bdf_file}")
    print(f"渲染文本: {text_to_render}")
    print()
    
    # 创建转换器
    converter = BDFToCConverter(bdf_file)
    
    # 生成C数组
    c_code = converter.render_string_to_c_array(text_to_render, array_name="my_text")
    
    # 打印结果
    print()
    print("=" * 60)
    print("生成的C语言数组:")
    print("=" * 60)
    print()
    print(c_code)


if __name__ == "__main__":
    main()
