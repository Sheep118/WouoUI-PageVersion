"""
验证生成的C文件中的基线对齐
"""
import sys
import re
sys.path.insert(0, 'g:\\WouoUI\\WouoUISDL2\\WouoUI-PageVersion\\Scripts')

from ttf_parser import TTFParser
from cfile_generater import CFileGenerator

# 配置
ttf_file = "../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf"
font_size = 12
output_dir = "../Csource/font_test"

config = {
    'layout': 'row_column',
    'bit_order': 'lsb',
    'encoding': 'positive'
}

print("=" * 70)
print("基线对齐验证")
print("=" * 70)

# 创建parser
parser = TTFParser(ttf_file, font_size, config)
char_data = parser.render_ascii(32, 126)

# 获取统一尺寸
unified_width = max([item['width'] for item in char_data])
max_bitmap_top = max([item.get('baseline_top', item['height']) for item in char_data])
min_below_baseline = min([item.get('baseline_top', item['height']) - item['height'] for item in char_data])
unified_height = max_bitmap_top - min_below_baseline

print(f"\n字体信息:")
print(f"  统一尺寸: {unified_width}x{unified_height}")
print(f"  max_bitmap_top: {max_bitmap_top}")
print(f"  min_below_baseline: {min_below_baseline}")

# 获取几个关键字符的基线信息
test_chars = [
    (32, 'space'),
    (44, ','),   # 应该有大量顶部空行
    (46, '.'),   # 应该有大量顶部空行
    (65, 'A'),   # 大写字母
    (72, 'H'),   # 大写字母
    (97, 'a'),   # 小写字母
    (103, 'g'),  # 有descender
    (121, 'y'),  # 有descender
]

print(f"\n关键字符的基线信息:")
print("-" * 70)
for ascii_code, char_name in test_chars:
    for item in char_data:
        if item['ord'] == ascii_code:
            bitmap_top = item.get('baseline_top', item['height'])
            height = item['height']
            below_baseline = bitmap_top - height
            baseline_offset = max_bitmap_top - bitmap_top
            
            print(f"\n'{char_name}' (ASCII {ascii_code}):")
            print(f"  原始尺寸: {item['width']}x{item['height']}")
            print(f"  bitmap_top: {bitmap_top}, below_baseline: {below_baseline}")
            print(f"  baseline_offset: {baseline_offset} (顶部空行数)")
            print(f"  字节数: {((unified_height + 7) // 8) * unified_width}")
            break

# 读取生成的C文件并提取字节
print("\n" + "=" * 70)
print("读取生成的C文件字节数据")
print("=" * 70)

c_file_path = f"{output_dir}/Zlabs.c"
try:
    with open(c_file_path, 'r') as f:
        content = f.read()
    
    # 提取数组部分
    match = re.search(r'const uint8_t.*?\[\] = \{(.*?)\};', content, re.DOTALL)
    if match:
        array_content = match.group(1)
        
        # 提取每个字符的字节序列和注释
        # 格式: 0x##, 0x##, ... /* char */
        lines = array_content.strip().split('\n')
        
        # 重新组织字节为按字符分组
        current_bytes = []
        current_char = None
        bytes_per_char = ((unified_height + 7) // 8) * unified_width
        byte_count = 0
        
        print(f"\n验证字符: {test_chars}")
        print("-" * 70)
        
        for line in lines:
            line = line.strip()
            if not line:
                continue
            
            # 提取字节值
            hex_values = re.findall(r'0x([0-9A-Fa-f]{2})', line)
            comment_match = re.search(r'/\* (.*?) \*/', line)
            
            for hex_val in hex_values:
                current_bytes.append(int(hex_val, 16))
            
            if comment_match:
                char_desc = comment_match.group(1)
                # 找到对应的ASCII码
                if char_desc == 'sp':
                    ascii_code = 32
                elif len(char_desc) == 1:
                    ascii_code = ord(char_desc)
                else:
                    ascii_code = None
                
                # 如果这是我们需要验证的字符
                for test_code, test_name in test_chars:
                    if ascii_code == test_code:
                        print(f"\n'{test_name}' (ASCII {ascii_code}):")
                        print(f"  字节数: {len(current_bytes)}")
                        print(f"  字节: {' '.join([f'{b:02X}' for b in current_bytes])}")
                        
                        # 分析顶部和底部的零字节
                        bytes_per_row = unified_width
                        rows = [current_bytes[i*bytes_per_row:(i+1)*bytes_per_row] 
                               for i in range(len(current_bytes) // bytes_per_row)]
                        
                        # 统计顶部全0的行
                        top_zero_rows = 0
                        for row in rows:
                            if all(b == 0 for b in row):
                                top_zero_rows += 1
                            else:
                                break
                        
                        # 统计底部全0的行
                        bottom_zero_rows = 0
                        for row in reversed(rows):
                            if all(b == 0 for b in row):
                                bottom_zero_rows += 1
                            else:
                                break
                        
                        print(f"  顶部全0行: {top_zero_rows}, 底部全0行: {bottom_zero_rows}")
                        if rows:
                            print(f"  总行数: {len(rows)}")
                
                current_bytes = []
except Exception as e:
    print(f"[ERROR] 读取C文件失败: {e}")
    import traceback
    traceback.print_exc()

print("\n" + "=" * 70)
print("验证完成")
print("=" * 70)
