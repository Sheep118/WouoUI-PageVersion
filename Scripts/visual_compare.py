"""
可视化对比：TTFParser预览 vs 生成的C文件字节
"""
import sys
sys.path.insert(0, 'g:\\WouoUI\\WouoUISDL2\\WouoUI-PageVersion\\Scripts')

from ttf_parser import TTFParser
from cfile_generater import CFileGenerator
import numpy as np

# 配置
ttf_file = "../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf"
font_size = 12

config = {
    'layout': 'row_column',
    'bit_order': 'lsb',
    'encoding': 'positive'
}

print("=" * 80)
print("TTFParser 预览 vs C文件字节对比")
print("=" * 80)

# 创建parser
parser = TTFParser(ttf_file, font_size, config)
char_data = parser.render_ascii(32, 126)

# 获取统一尺寸
unified_width = max([item['width'] for item in char_data])
max_bitmap_top = max([item.get('baseline_top', item['height']) for item in char_data])
min_below_baseline = min([item.get('baseline_top', item['height']) - item['height'] for item in char_data])
unified_height = max_bitmap_top - min_below_baseline

print(f"\n字体: {ttf_file}")
print(f"尺寸: {unified_width}x{unified_height} (统一对齐)")
print(f"基线位置: 从顶部 {max_bitmap_top} 像素")

# 测试几个字符
test_chars = [
    (44, ','),
    (65, 'A'),
    (103, 'g'),
]

for ascii_code, char_name in test_chars:
    print("\n" + "=" * 80)
    for item in char_data:
        if item['ord'] == ascii_code:
            print(f"\n字符: '{char_name}' (ASCII {ascii_code})")
            print(f"原始大小: {item['width']}x{item['height']}")
            
            bitmap_top = item.get('baseline_top', item['height'])
            baseline_offset = max_bitmap_top - bitmap_top
            print(f"baseline_top: {bitmap_top}, offset: {baseline_offset}")
            
            # 创建对齐后的矩阵
            new_pixels = np.zeros((unified_height, unified_width), dtype=np.uint8)
            copy_height = min(item['pixels'].shape[0], unified_height - baseline_offset)
            copy_width = min(item['pixels'].shape[1], unified_width)
            
            if copy_height > 0:
                new_pixels[baseline_offset:baseline_offset + copy_height, :copy_width] = \
                    item['pixels'][:copy_height, :copy_width]
            
            # 可视化预览
            print("\n对齐后的像素矩阵（█=1, space=0）:")
            print("   ", end="")
            for col in range(unified_width):
                print(f"{col}", end="")
            print(" <- 列")
            
            for row in range(unified_height):
                print(f"{row:2d}→", end="")
                for col in range(unified_width):
                    print("█" if new_pixels[row, col] else "·", end="")
                
                # 标记基线
                if row == max_bitmap_top - 1:
                    print(" ← 基线下方")
                elif row == max_bitmap_top:
                    print(" ← 基线（descender起点）")
                else:
                    print()
            
            # 显示字节数据（row_column layout）
            print("\nrow_column 字节数据（LSB位序）:")
            bytes_list = []
            for row_idx in range((unified_height + 7) // 8):
                row_bytes = []
                for col in range(unified_width):
                    byte_val = 0
                    for bit in range(8):
                        pixel_row = row_idx * 8 + bit
                        if pixel_row < unified_height:
                            byte_val |= (new_pixels[pixel_row, col] << bit)
                    row_bytes.append(byte_val)
                    bytes_list.append(byte_val)
                print(f"像素行{row_idx*8:2d}-{min(row_idx*8+7, unified_height-1):2d}: {' '.join([f'{b:02X}' for b in row_bytes])}")
            
            # 显示完整字节序列
            print(f"\n完整字节序列: {' '.join([f'{b:02X}' for b in bytes_list])}")
            
            break

print("\n" + "=" * 80)
print("验证完成")
print("=" * 80)
