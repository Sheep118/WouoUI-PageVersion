"""
测试baseline对齐功能
"""
import sys
sys.path.insert(0, 'g:\\WouoUI\\WouoUISDL2\\WouoUI-PageVersion\\Scripts')

from ttf_parser import TTFParser
from cfile_generater import CFileGenerator

# 配置
ttf_file = "../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf"
font_size = 12

config = {
    'layout': 'row_column',
    'bit_order': 'lsb',
    'encoding': 'positive'
}

print("=" * 60)
print("测试Baseline对齐")
print("=" * 60)
print()

# 加载并渲染字体
parser = TTFParser(ttf_file, font_size, config)
char_data = parser.render_ascii(32, 126)

print(f"渲染了 {len(char_data)} 个字符")

# 获取unified尺寸
unified_width = max([item['width'] for item in char_data])
unified_height = max([item['height'] for item in char_data])
max_bitmap_top = max([item.get('baseline_top', item['height']) for item in char_data])
min_below_baseline = min([item.get('baseline_top', item['height']) - item['height'] for item in char_data])

print(f"统一宽度: {unified_width}px")
print(f"统一高度: {unified_height}px")
print(f"最大bitmap_top: {max_bitmap_top}px")
print(f"最小below_baseline: {min_below_baseline}px")
print(f"计算的总高度: {max_bitmap_top - min_below_baseline}px")
print()

# 创建生成器
generator = CFileGenerator("test_baseline", config)
generator.add_font_size(font_size, char_data)

# 获取baseline metrics
baseline_offsets = generator._get_baseline_metrics(char_data)

print("Baseline堆移分析:")
print("字符 | ASCII | 宽  | 高 | baseline_top | 堆移量")
print("-" * 55)

for char in ['A', 'H', 'a', 'd', 'g', ',', '.', 'y']:
    for item in char_data:
        if item['char'] == char:
            width = item['width']
            height = item['height']
            baseline_top = item.get('baseline_top', height)
            offset = baseline_offsets.get(item['ord'], 0)
            print(f"{char:1s}   | {ord(char):3d}  | {width:2d}   | {height:2d} | {baseline_top:12d} | {offset:3d}")
            break

print()
print("注：堆移量表示像素数据应该从顶部空出多少行（基线对齐）")
