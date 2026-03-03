"""
对比不同padding配置的效果
"""
import sys
sys.path.insert(0, 'g:\\WouoUI\\WouoUISDL2\\WouoUI-PageVersion\\Scripts')

from ttf_parser import TTFParser
import numpy as np

ttf_file = "../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf"
font_size = 12

print("=" * 80)
print("Padding和居中对齐效果对比")
print("=" * 80)

# 测试三种配置
configs = [
    {
        'name': '无Padding, 左对齐',
        'config': {
            'layout': 'row_column',
            'bit_order': 'lsb',
            'encoding': 'positive',
            'char_spacing': {
                'left_pad': 0,
                'right_pad': 0,
                'center_align': False
            }
        }
    },
    {
        'name': '1px Padding, 左对齐',
        'config': {
            'layout': 'row_column',
            'bit_order': 'lsb',
            'encoding': 'positive',
            'char_spacing': {
                'left_pad': 1,
                'right_pad': 1,
                'center_align': False
            }
        }
    },
    {
        'name': '1px Padding, 居中对齐',
        'config': {
            'layout': 'row_column',
            'bit_order': 'lsb',
            'encoding': 'positive',
            'char_spacing': {
                'left_pad': 1,
                'right_pad': 1,
                'center_align': True
            }
        }
    },
]

test_string = "if"  # 窄字符和宽字符的组合

for config_info in configs:
    print(f"\n[配置] {config_info['name']}")
    print("-" * 80)
    
    config = config_info['config']
    parser = TTFParser(ttf_file, font_size, config)
    
    # 获取字符数据
    char_data = parser.render_string(test_string)
    
    # 获取统一宽度
    spacing_config = config.get('char_spacing', {})
    left_pad = spacing_config.get('left_pad', 0)
    right_pad = spacing_config.get('right_pad', 0)
    center_align = spacing_config.get('center_align', False)
    
    actual_width = max([item['width'] for item in char_data])
    unified_width = actual_width + left_pad + right_pad
    
    print(f"统一宽度: {unified_width}px (max {actual_width}px + padding {left_pad+right_pad}px)")
    print(f"居中对齐: {'是' if center_align else '否'}")
    print()
    
    # 显示字符串的预览
    print("字符渲染对比:")
    print(f"{'字符':<5} | {'原始':<10} | {'对齐后':<20}")
    print("-" * 80)
    
    for item in char_data:
        char = item['char']
        width = item['width']
        pixels = item['pixels']
        height = pixels.shape[0]
        
        # 创建对齐后的矩阵
        aligned_pixels = np.zeros((height, unified_width), dtype=np.uint8)
        
        # 计算水平偏移
        available_width = unified_width - left_pad - right_pad
        if center_align and width < available_width:
            total_padding = available_width - width
            left_offset = left_pad + total_padding // 2
        else:
            left_offset = left_pad
        
        # 复制像素
        if width > 0:
            aligned_pixels[:, left_offset:left_offset + min(width, available_width)] = \
                pixels[:, :min(width, available_width)]
        
        # 显示一行的可视化
        original_vis = ""
        for pixel in pixels[height//2]:
            original_vis += "█" if pixel else "·"
        
        aligned_vis = ""
        for pixel in aligned_pixels[height//2]:
            if pixel:
                aligned_vis += "█"
            else:
                aligned_vis += "·"
        
        print(f"'{char}'({width}px) | {original_vis:<10} | {aligned_vis:<20}")

print("\n" + "=" * 80)
print("总结")
print("=" * 80)
print("""
1. 无Padding - 原始字体直接生成，字符间可能紧贴
2. Padding左对齐 - 添加padding但窄字符靠左，排版不美观
3. Padding+居中 - 添加padding且窄字符居中，排版最美观

推荐使用 "1px Padding + 居中对齐" 配置以获得最佳视觉效果。
""")
