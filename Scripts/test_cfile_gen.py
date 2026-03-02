"""
测试cfile_generater的功能
"""
import sys
sys.path.insert(0, 'g:\\WouoUI\\WouoUISDL2\\WouoUI-PageVersion\\Scripts')

from ttf_parser import TTFParser
from cfile_generater import CFileGenerator
import numpy as np

# 配置
ttf_file = "../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf"
font_size = 12
output_dir = "../Csource/font_test"

# 配置参数
config = {
    'layout': 'row_column',
    'bit_order': 'lsb',
    'encoding': 'positive'
}

print("=" * 60)
print("C文件生成测试")
print("=" * 60)
print()

# 创建parser
try:
    parser = TTFParser(ttf_file, font_size, config)
    print(f"[OK] TTF字体加载成功: {ttf_file}")
except Exception as e:
    print(f"[ERROR] 字体加载失败: {e}")
    sys.exit(1)

# 渲染ASCII字符集
print("[OK] 渲染ASCII字符集...")
char_data = parser.render_ascii(32, 126)
print(f"  渲染了 {len(char_data)} 个字符")

if char_data:
    # 获取统一宽度
    unified_width = max([item['width'] for item in char_data])
    
    # 获取基线对齐的统一高度
    max_bitmap_top = max([item.get('baseline_top', item['height']) for item in char_data])
    min_below_baseline = min([item.get('baseline_top', item['height']) - item['height'] for item in char_data])
    unified_height = max_bitmap_top - min_below_baseline
    
    print(f"  统一尺寸: {unified_width}x{unified_height}")
    # row_column layout: (height + 7) // 8 rows × width columns
    bytes_per_char = ((unified_height + 7) // 8) * unified_width
    print(f"  每个字符字节数: {bytes_per_char}")

# 创建C文件生成器
print("\n[OK] 创建C文件生成器...")
generator = CFileGenerator("Zlabs", config)

# 添加字体尺寸
print("[OK] 添加字体数据...")
generator.add_font_size(font_size, char_data, 32, 126)

# 生成文件
print("[OK] 生成C文件...")
try:
    header_path, source_path = generator.save_files(output_dir)
    print(f"  [OK] 头文件: {header_path}")
    print(f"  [OK] 源文件: {source_path}")
except Exception as e:
    print(f"  [ERROR] 生成失败: {e}")
    import traceback
    traceback.print_exc()
    sys.exit(1)

# 打印摘要
print("\n[OK] 生成摘要:")
print(generator.get_summary())

# 显示生成的C文件的前几行
print("\n生成的头文件内容:")
print("=" * 60)
try:
    with open(header_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        for line in lines[:15]:
            print(line.rstrip())
except Exception as e:
    print(f"[ERROR] 读取文件失败: {e}")

print("\n\n生成的源文件内容 (前30行):")
print("=" * 60)
try:
    with open(source_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        for i, line in enumerate(lines[:30]):
            print(line.rstrip())
except Exception as e:
    print(f"[ERROR] 读取文件失败: {e}")

print("\n\n[OK] 测试完成！")
