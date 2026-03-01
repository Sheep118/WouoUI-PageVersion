import freetype

# 加载字体
# face = freetype.Face("../font/Pixeloid_Font_1_0/TrueType (.ttf)/PixeloidMono.ttf")
face = freetype.Face("../font/04b_03/04B_03__.TTF")
# face = freetype.Face("../font/unifont/unifont-16_0_04.ttf")
# face = freetype.Face("../font/ZlabsBitmap_12px_ttf/ZlabsBitmap_12px_CN.ttf")

# 设置字符大小（比如 16 像素）
face.set_pixel_sizes(0, 8)

# 打印字体的全局度量信息
print(f"字体度量信息:")
print(f"  Ascender (上升高度): {face.size.ascender / 64:.1f} 像素")
print(f"  Descender (下降深度): {face.size.descender / 64:.1f} 像素")
print(f"  Height (行高): {face.size.height / 64:.1f} 像素")
print(f"  Max advance (最大前进): {face.size.max_advance / 64:.1f} 像素")
print()

# 要渲染的字符串
text = "0789eWouoUIghHAZ"

# 存储每个字符的位图数据
char_bitmaps = []

for char in text:
    # 加载并渲染字符
    face.load_char(char, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
    bitmap = face.glyph.bitmap
    
    # 将位图数据转换为二维数组
    char_data = []
    for row in range(bitmap.rows):
        row_data = []
        for col in range(bitmap.width):
            # 计算字节位置和位偏移
            byte_index = row * bitmap.pitch + col // 8
            bit_index = 7 - (col % 8)  # 从最高位开始
            
            # 获取对应的bit
            if byte_index < len(bitmap.buffer):
                byte_value = bitmap.buffer[byte_index]
                bit = (byte_value >> bit_index) & 1
                row_data.append(bit)
            else:
                row_data.append(0)
        char_data.append(row_data)
    
    char_bitmaps.append({
        'char': char,
        'data': char_data,
        'width': bitmap.width,
        'height': bitmap.rows,
        'bitmap_top': face.glyph.bitmap_top,  # 位图顶部到基线的距离
        'bitmap_left': face.glyph.bitmap_left  # 位图左边距
    })
    print(f"字符 '{char}': {bitmap.width} x {bitmap.rows}, bitmap_top: {face.glyph.bitmap_top}")

# 找出最大高度和最高的bitmap_top（基线上方的最大距离）
max_height = max([cb['height'] for cb in char_bitmaps]) if char_bitmaps else 0
max_bitmap_top = max([cb['bitmap_top'] for cb in char_bitmaps]) if char_bitmaps else 0
min_below_baseline = min([cb['bitmap_top'] - cb['height'] for cb in char_bitmaps]) if char_bitmaps else 0
total_height = max_bitmap_top - min_below_baseline  # 从最顶部到最底部的总高度

# 找出最大宽度（用于对齐）
max_width = max([cb['width'] for cb in char_bitmaps]) if char_bitmaps else 0

print(f"\n基线位置: {max_bitmap_top}, 最低下降: {min_below_baseline}, 总高度: {total_height}")
print(f"最大宽度: {max_width}, 将使用此宽度对齐所有字符")

# 为每个字符补齐到最大宽度
for char_bitmap in char_bitmaps:
    original_width = char_bitmap['width']
    if original_width < max_width:
        # 在右边填充0
        padding = max_width - original_width
        for row in char_bitmap['data']:
            row.extend([0] * padding)
        char_bitmap['width'] = max_width

# 打印整个字符串
print(f"\n字符串 '{text}' 的位图内容:")
print("=" * (max_width * len(char_bitmaps) + len(char_bitmaps)))
for row in range(total_height):
    # 标记基线
    line_marker = "→" if row == max_bitmap_top - 1 else " "
    for char_bitmap in char_bitmaps:
        # 计算当前行相对于该字符的位置
        # row 0 是最顶部，max_bitmap_top 是基线
        char_row = row - (max_bitmap_top - char_bitmap['bitmap_top'])
        
        # 如果该行在当前字符的位图范围内
        if 0 <= char_row < char_bitmap['height']:
            for col in range(char_bitmap['width']):
                bit = char_bitmap['data'][char_row][col]
                if bit:
                    print('#', end='')
                else:
                    print(' ', end='')
        else:
            # 如果当前行不在字符范围内，打印空格
            print(' ' * char_bitmap['width'], end='')
        
        # 字符之间加一个空格间隔
        print(' ', end='')
    print(f" {line_marker} (行{row+1})")  # 换行并标注行号

# ==================== 生成C语言数组 ====================
print("\n" + "="*80)
print("生成C语言数组代码:")
print("="*80)

# 每个字符的字节数（按8位为单位）
bytes_per_char_width = (max_width + 7) // 8
bytes_per_char = bytes_per_char_width * total_height

print(f"\n字符数: {len(char_bitmaps)}")
print(f"字符宽度: {max_width} 像素")
print(f"字符高度: {total_height} 像素")
print(f"每行字节数: {bytes_per_char_width}")
print(f"每个字符字节数: {bytes_per_char}")

# 生成数组
print("\n// 字体位图数据")
print(f"// 字符: {text}")
print(f"// 宽度: {max_width}px, 高度: {total_height}px")
print("const uint8_t font_bitmap[] = {")

total_hex_count = 0
for char_idx, char_bitmap in enumerate(char_bitmaps):
    char = char_bitmap['char']
    print(f"    // 字符 '{char}' (0x{ord(char):04X})")
    
    for row in range(total_height):
        char_row = row - (max_bitmap_top - char_bitmap['bitmap_top'])
        
        # 收集这一行的所有bit
        bits = []
        if 0 <= char_row < char_bitmap['height']:
            bits = char_bitmap['data'][char_row]
        else:
            bits = [0] * max_width
        
        # 将bits转换为字节
        for byte_idx in range(bytes_per_char_width):
            byte_val = 0
            for bit_idx in range(8):
                pixel_idx = byte_idx * 8 + bit_idx
                if pixel_idx < len(bits) and bits[pixel_idx]:
                    byte_val |= (1 << (7 - bit_idx))
            
            # 打印十六进制值
            print(f"    0x{byte_val:02X},", end="")
            total_hex_count += 1
            
            # 每8个字节换行
            if total_hex_count % 8 == 0:
                print()

print("\n};")

# 生成索引表
print(f"\n// 字符索引表")
print(f"const char font_chars[] = \"{text}\";")
print(f"const uint16_t font_offsets[] = {{")
for i in range(len(char_bitmaps)):
    offset = i * bytes_per_char
    print(f"    {offset},  // '{text[i]}'", end="")
    if i < len(char_bitmaps) - 1:
        print()
print("\n};")

print(f"\nconst uint8_t font_width = {max_width};")
print(f"const uint8_t font_height = {total_height};")
print(f"const uint8_t font_count = {len(char_bitmaps)};")