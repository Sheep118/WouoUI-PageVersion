"""
测试字符padding和居中对齐功能
"""
import sys
sys.path.insert(0, 'g:\\WouoUI\\WouoUISDL2\\WouoUI-PageVersion\\Scripts')

from ttf_parser import TTFParser
from cfile_generater import CFileGenerator
import numpy as np

# 配置 - 包含padding
config = {
    'layout': 'row_column',
    'bit_order': 'lsb',
    'encoding': 'positive',
    'char_spacing': {
        'left_pad': 1,
        'right_pad': 1,
        'center_align': True
    }
}

print("=" * 70)
print("字符Padding和居中对齐测试")
print("=" * 70)

ttf_file = "../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf"
font_size = 12
output_dir = "../Csource/font_test"

# 创建parser
parser = TTFParser(ttf_file, font_size, config)
char_data = parser.render_ascii(32, 126)

print(f"\n[字体加载] {ttf_file}")
print(f"[字符数] {len(char_data)}")

# 获取统一尺寸
actual_width = max([item['width'] for item in char_data])
left_pad = config['char_spacing']['left_pad']
right_pad = config['char_spacing']['right_pad']
unified_width = actual_width + left_pad + right_pad

print(f"\n[宽度信息]")
print(f"  实际最大宽度: {actual_width}px")
print(f"  左padding: {left_pad}px")
print(f"  右padding: {right_pad}px")
print(f"  统一宽度: {unified_width}px")

# 获取统一高度
max_bitmap_top = max([item.get('baseline_top', item['height']) for item in char_data])
min_below_baseline = min([item.get('baseline_top', item['height']) - item['height'] for item in char_data])
unified_height = max_bitmap_top - min_below_baseline

print(f"  统一高度: {unified_height}px (baseline对齐)")

# 测试几个不同宽度的字符
test_chars = [
    (105, 'i', "窄字符"),
    (109, 'm', "宽字符"),
    (108, 'l', "超窄字符"),
    (87, 'W', "最宽字符"),
]

print(f"\n[字符对齐测试 - 包含padding和居中]")
print("-" * 70)

for ascii_code, char, desc in test_chars:
    for item in char_data:
        if item['ord'] == ascii_code:
            print(f"\n'{char}' ({desc}) - ASCII {ascii_code}:")
            print(f"  原始宽度: {item['width']}px")
            
            # 可视化原始字符
            print(f"  原始矩阵 (█=像素):")
            for row in item['pixels']:
                print(f"    ", end="")
                for pixel in row:
                    print("█" if pixel else "·", end="")
                print()
            
            # 计算对齐后的位置
            available_width = unified_width - left_pad - right_pad
            if item['width'] < available_width:
                total_padding = available_width - item['width']
                left_offset = left_pad + total_padding // 2
                right_offset = left_pad + (total_padding + 1) // 2
            else:
                left_offset = left_pad
                right_offset = 0
            
            print(f"  对齐信息:")
            print(f"    左offset: {left_offset}px (padding {left_pad} + 居中补充 {left_offset - left_pad})")
            print(f"    右offset: {right_offset}px")
            
            # 创建对齐后的矩阵
            aligned_pixels = np.zeros((unified_height, unified_width), dtype=np.uint8)
            
            # 计算基线偏移
            bitmap_top = item.get('baseline_top', item['height'])
            baseline_offset = max_bitmap_top - bitmap_top
            
            # 复制像素
            copy_height = min(item['pixels'].shape[0], unified_height - baseline_offset)
            copy_width = item['pixels'].shape[1]
            
            if copy_height > 0 and copy_width > 0:
                aligned_pixels[baseline_offset:baseline_offset + copy_height,
                             left_offset:left_offset + copy_width] = item['pixels'][:copy_height, :copy_width]
            
            # 可视化对齐后的字符
            print(f"  对齐后矩阵 (█=像素, 中间为字符内容, 两侧为padding):")
            for row in aligned_pixels:
                print(f"    ", end="")
                for pixel in row:
                    if pixel:
                        print("█", end="")
                    else:
                        print(" ", end="")
                print()
            
            break

# 创建C文件生成器并生成文件
print(f"\n[生成C文件]")
generator = CFileGenerator("ZlabsPad", config)
generator.add_font_size(font_size, char_data, 32, 126)

try:
    header_path, source_path = generator.save_files(output_dir)
    print(f"  [OK] 头文件: {header_path}")
    print(f"  [OK] 源文件: {source_path}")
    
    # 显示摘要
    print(f"\n[生成摘要]:")
    print(generator.get_summary())
except Exception as e:
    print(f"  [ERROR] 生成失败: {e}")
    import traceback
    traceback.print_exc()

print("\n" + "=" * 70)
print("测试完成")
print("=" * 70 + "\n")
