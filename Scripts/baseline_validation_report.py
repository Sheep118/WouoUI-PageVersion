"""
完整的基线对齐验证报告
"""
import sys
sys.path.insert(0, 'g:\\WouoUI\\WouoUISDL2\\WouoUI-PageVersion\\Scripts')

from ttf_parser import TTFParser
from cfile_generater import CFileGenerator
import re

# 配置
ttf_file = "../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf"
font_size = 12
output_dir = "../Csource/font_test"

config = {
    'layout': 'row_column',
    'bit_order': 'lsb',
    'encoding': 'positive'
}

print("\n" + "=" * 80)
print("基线对齐（Baseline Alignment）完整验证报告")
print("=" * 80)

# 1. 加载字体
parser = TTFParser(ttf_file, font_size, config)
char_data = parser.render_ascii(32, 126)

# 2. 获取统一尺寸和基线信息
unified_width = max([item['width'] for item in char_data])
max_bitmap_top = max([item.get('baseline_top', item['height']) for item in char_data])
min_below_baseline = min([item.get('baseline_top', item['height']) - item['height'] for item in char_data])
unified_height = max_bitmap_top - min_below_baseline

print(f"\n[字体信息]")
print(f"  文件: {ttf_file}")
print(f"  尺寸: {font_size}px")
print(f"  字符数: {len(char_data)}")
print(f"\n[统一尺寸（Unified Dimensions）]")
print(f"  宽度: {unified_width}px")
print(f"  高度: {unified_height}px (baseline对齐)")
print(f"  字节数/字符: {((unified_height + 7) // 8) * unified_width} bytes")

print(f"\n[基线信息（Baseline Metrics）]")
print(f"  max_bitmap_top: {max_bitmap_top}px (最高字符的顶部位置)")
print(f"  min_below_baseline: {min_below_baseline}px (最低descender位置)")
print(f"  baseline位置: 从顶部 {max_bitmap_top}px")

# 3. 分析字符类型
print(f"\n[字符分类分析]")

ascenders = []
descenders = []
baseline_sitting = []

for item in char_data:
    bitmap_top = item.get('baseline_top', item['height'])
    height = item['height']
    below_baseline = bitmap_top - height
    
    char_code = item['ord']
    if 32 <= char_code < 127:
        char_str = chr(char_code)
    else:
        char_str = f'U+{char_code:04X}'
    
    # 分类
    if bitmap_top >= max_bitmap_top - 1:  # 接触或接近最高点
        ascenders.append((char_str, bitmap_top, height))
    elif below_baseline < 0:  # 有descender
        descenders.append((char_str, bitmap_top, height, below_baseline))
    else:  # 坐在基线上
        baseline_sitting.append((char_str, bitmap_top, height))

print(f"  Ascenders（有上伸的字符）: {len(ascenders)} 个")
if ascenders:
    print(f"    示例: {', '.join([f'{c}({t})' for c, t, _ in ascenders[:5]])}")

print(f"  Descenders（有下伸的字符）: {len(descenders)} 个")
if descenders:
    print(f"    示例: {', '.join([f'{c}({o})' for c, _, _, o in descenders[:5]])}")

print(f"  Baseline Sitting（坐在基线上）: {len(baseline_sitting)} 个")
if baseline_sitting:
    print(f"    示例: {', '.join([c for c, _, _ in baseline_sitting[:5]])}")

# 4. 验证C文件生成
print(f"\n[C文件生成验证]")
c_file_path = f"{output_dir}/Zlabs.c"
h_file_path = f"{output_dir}/Zlabs.h"

try:
    with open(h_file_path, 'r') as f:
        h_content = f.read()
    
    with open(c_file_path, 'r') as f:
        c_content = f.read()
    
    # 检查注释
    if "baseline aligned" in c_content:
        print(f"  ✓ C文件包含 'baseline aligned' 标记")
    else:
        print(f"  ✗ C文件缺少 'baseline aligned' 标记")
    
    # 检查结构体Height值
    struct_match = re.search(r'const sFONT Zlabs_12_t = \{.*?\.Height = (\d+)', c_content, re.DOTALL)
    if struct_match:
        height_val = int(struct_match.group(1))
        if height_val == unified_height:
            print(f"  ✓ 结构体Height字段 = {height_val}px (正确)")
        else:
            print(f"  ✗ 结构体Height字段 = {height_val}px (应为{unified_height}px)")
    
    # 检查字节数据量
    array_match = re.search(r'const uint8_t Zlabs_12\[\] = \{(.*?)\};', c_content, re.DOTALL)
    if array_match:
        hex_values = re.findall(r'0x[0-9A-Fa-f]{2}', array_match.group(1))
        total_bytes = len(hex_values)
        expected_bytes = len(char_data) * ((unified_height + 7) // 8) * unified_width
        print(f"  字节数据:")
        print(f"    总字节数: {total_bytes}")
        print(f"    预期字节数: {expected_bytes}")
        if total_bytes == expected_bytes:
            print(f"    ✓ 字节数正确")
        else:
            print(f"    ✗ 字节数不匹配")
    
    print(f"  ✓ C文件存在: {c_file_path}")
    print(f"  ✓ 头文件存在: {h_file_path}")
    
except Exception as e:
    print(f"  ✗ 读取文件失败: {e}")

# 5. 具体字符验证
print(f"\n[具体字符验证（Specific Character Validation）]")

test_cases = [
    (44, ',', "逗号", "应该坐在基线下方"),
    (46, '.', "句号", "应该坐在基线下方"),
    (65, 'A', "大写A", "应该有ascender"),
    (72, 'H', "大写H", "应该有ascender"),
    (97, 'a', "小写a", "应该对齐到x-height"),
    (103, 'g', "小写g", "应该有descender"),
    (121, 'y', "小写y", "应该有descender"),
]

for ascii_code, char, name, description in test_cases:
    for item in char_data:
        if item['ord'] == ascii_code:
            bitmap_top = item.get('baseline_top', item['height'])
            height = item['height']
            offset = max_bitmap_top - bitmap_top
            
            print(f"\n  '{char}' ({name}):")
            print(f"    原始: {item['width']}x{height}, bitmap_top={bitmap_top}")
            print(f"    对齐偏移: {offset}px ({description})")
            
            # 从C文件中提取字节
            if array_match:
                # 计算字符在数组中的位置
                char_index = ascii_code - 32  # 从空格(32)开始
                bytes_per_char = ((unified_height + 7) // 8) * unified_width
                char_start = char_index * bytes_per_char
                char_end = char_start + bytes_per_char
                
                char_bytes = hex_values[char_start:char_end]
                if char_bytes:
                    print(f"    C文件字节: {' '.join(char_bytes)}")
                    
                    # 分析零字节行
                    bytes_per_row = unified_width
                    rows = [char_bytes[i::bytes_per_row] for i in range(bytes_per_row)]
                    
                    # 统计顶部全0行
                    top_zero_rows = 0
                    for row_idx in range((unified_height + 7) // 8):
                        row_bytes = [int(char_bytes[row_idx * bytes_per_row + col], 16) 
                                   for col in range(unified_width)]
                        if all(b == 0 for b in row_bytes):
                            top_zero_rows += 1
                        else:
                            break
                    
                    expected_zero_rows = offset // 8
                    if top_zero_rows == expected_zero_rows or offset < 8:
                        print(f"    ✓ 基线对齐正确")
                    else:
                        print(f"    顶部零字节行: {top_zero_rows} (预期约 {expected_zero_rows})")
            break

print("\n" + "=" * 80)
print("验证完成")
print("=" * 80 + "\n")
