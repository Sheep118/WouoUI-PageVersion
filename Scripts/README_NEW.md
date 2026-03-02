# WouoUI 字体转换工具 - freetype-py 版本

## 项目概述

本项目是一个完整的字体转换工具，用于将 TTF 和 BDF 字体文件转换为 C 语言数组和头文件，适用于嵌入式系统（如单色屏 SSD1306）。

使用 **freetype-py** 库进行字体解析，提供高质量的字符渲染。并实现了 **基线对齐（Baseline Alignment）**，确保字符按照排版规则正确对齐。

## 核心特性

✅ **TTF/BDF 字体支持** - 支持 TrueType 和 BDF 位图字体
✅ **基线对齐** - 大写、小写、descender 字符自动对齐到共享的基线
✅ **灵活的位布局** - 支持 row_column、column_row 两种布局方式
✅ **多样式编码** - 支持 LSB/MSB 位序和 positive/negative 编码
✅ **统一尺寸** - 自动计算并应用统一的宽度和基线对齐高度
✅ **清晰的可视化预览** - ASCII 预览显示基线和字符关键指标

## 文件结构

```
Scripts/
├── main.py                           # 主程序入口
├── ttf_parser.py                    # TTF 字体解析器（支持基线对齐）
├── bdf_parser.py                    # BDF 字体解析器（支持基线对齐）
├── cfile_generater.py               # C 文件生成器（支持基线对齐）
├── config.yaml                      # 配置文件
├── requirements.txt                 # Python 依赖
├── BASELINE_ALIGNMENT_SUMMARY.md    # 基线对齐实现文档
├── README_NEW.md                    # 本文件
```

## 各个模块说明

### 1. `main.py` - 主程序

**职责：**
- 读取 `config.yaml` 配置文件
- 扫描字体文件夹
- 调用 TTF/BDF 解析器
- 调用 C 文件生成器（支持基线对齐）
- 管理多字体尺寸

**使用方式：**

```bash
# 处理配置中指定的字体文件夹
python main.py

# 处理指定的字体文件
python main.py ../font/04b_03/04B_03__.TTF ttf
python main.py ../font/unifont/unifont-17_0_03.bdf bdf
```

### 2. `ttf_parser.py` - TTF 字体解析器

**职责：**
- 使用 freetype-py 加载 TTF 字体
- 渲染单个字符或字符串，**提取基线信息**
- 将像素矩阵转换为字节数组
- 支持不同的位布局格式
- **显示基线对齐预览**

**主要类：**
- `TTFParser`: TTF 字体解析器类

**关键方法：**
- `render_character(char)`: 渲染单个字符，返回像素矩阵和 **baseline_top**
- `render_ascii(start_char, end_char)`: 渲染 ASCII 字符集
- `get_glyph_metrics(char)`: 获取字形指标，包括 **baseline_top**
- `pixels_to_bytes(pixels)`: 将像素矩阵转换为字节数组
- `print_pixels_preview(pixels)`: 打印 ASCII 艺术预览，**显示基线位置**

**单独测试：**

```bash
python ttf_parser.py
# 会显示包含基线对齐的预览
```

### 3. `bdf_parser.py` - BDF 字体解析器

**职责：**
- 使用 freetype-py 加载 BDF 字体
- 提供与 TTFParser 相同的接口，**包括基线信息**
- 支持 BDF 特有的格式

**主要类：**
- `BDFParser`: BDF 字体解析器类

**使用方式与 TTFParser 相同。**

**单独测试：**

```bash
python bdf_parser.py
# 会显示包含基线对齐的预览
```

### 4. `cfile_generater.py` - C 文件生成器

**职责：**
- 接收 Parser 生成的像素缓冲区和 **基线信息**
- 支持多个字体尺寸放在同一个 .c 和 .h 文件中
- 生成 **基线对齐** 的 C 代码
- **自动计算基线偏移**，确保字符正确对齐
- 生成相应的头文件

**主要类：**
- `CFileGenerator`: C 文件生成器类

**关键方法：**
- `add_font_size(font_size, char_data, start_char, end_char)`: 添加字体数据
- `_get_baseline_metrics(char_data)`: **计算基线指标**
  - `max_bitmap_top`: 最高字符顶部
  - `min_below_baseline`: 最低 descender 位置
  - `total_height`: 基线对齐的统一高度
- `_rebuild_bytes_with_unified_width(char_item, unified_width, unified_height, baseline_offset)`: **应用基线偏移**到字符像素
- `generate_source()`: 生成源文件，**使用基线对齐高度**
- `save_files(output_dir)`: 保存 .c 和 .h 文件
- `save_files(output_dir)`: 保存 .c 和 .h 文件
- `get_summary()`: 获取生成的文件摘要

**特点：**
- 多个尺寸的数据放在同一个 .c 和 .h 文件中
- 自动生成结构体定义
- 支持多种配置组合

### 5. `config.yaml` - 配置文件

**配置项详解：**

```yaml
generate:
  font_folder: ../font                          # 字体文件夹（相对于脚本目录）
  output_c_folder: ../Csource/font             # 输出文件夹

  font_format:
    layout: row_column                          # 排列方式: column_row 或 row_column
    bit_order: lsb                              # 比特顺序: lsb 或 msb
    encoding: positive                          # 编码方式: positive 或 negative

  font_sizes:
    default:                                    # 默认字体大小列表
      - 12
      - 16
      - 24

  charset:
    ascii_start: 32                             # ASCII 起始码
    ascii_end: 126                              # ASCII 结束码

## 基线对齐（Baseline Alignment）说明

### 概念

基线对齐是一个 **排版规则**，确保字体中的不同字符能正确对齐：

- **Ascenders** - 大写字母（A、H）和上升字符靠近顶部
- **X-height** - 小写字母（a、x）对齐到中间层
- **Descenders** - 下伸字符（g、y、,）有底部空间

### 实现方式

系统自动计算：

```
max_bitmap_top = 字体中最高字符的起始位置（例如 9px）
min_below_baseline = 最低descender的结束位置（例如 -2px）
unified_height = max_bitmap_top - min_below_baseline = 11px
```

每个字符的偏移：

```
baseline_offset = max_bitmap_top - character_bitmap_top
```

### 示例

对于 ZLabsBitmap_12px (max=9, min=-2, unified=11)：

| 字符 | 类型 | offset | 说明 |
|------|------|--------|------|
| 'A' | Ascender | 1px | 顶部1个空行 |
| 'a' | X-height | 3px | 顶部3个空行 |
| 'g' | Descender | 3px | 下伸在底部2行 |
| ',' | Sitting | 8px | 坐在baseline下，顶部8个空行 |

### 生成文件中的体现

- 头文件注释：`(baseline aligned)`
- 结构体Height值：使用 unified_height（11），而不是简单的max height（10）
- 字节数据：每个字符的像素数据会根据offset正确定位

## 工作流程

```
config.yaml
    ↓
main.py (读取配置)
    ↓
TTFParser 或 BDFParser (解析字体)
    ↓
CFileGenerator (生成 C 代码，支持基线对齐)
    ↓
输出 .c 和 .h 文件
```

## 数据流

```
字体文件 (TTF/BDF)
    ↓
freetype 库解析 + baseline_top 提取
    ↓
像素矩阵 (numpy array, height x width) + baseline info
    ↓
基线对齐处理 (计算偏移、应用指标)
    ↓
字节数组转换 (pixels_to_bytes)
    ↓
C 代码生成（使用统一高度）
    ↓
.c 和 .h 文件（包含baseline aligned标记）
```

## 配置支持

### 布局方式 (layout)

- **row_column**: 先行后列（行列式）
  - 适用于大多数单色屏驱动（如 SSD1306）
  - 8 行为一个字节单位

- **column_row**: 先列后行（列行式）
  - 适用于某些特殊驱动

### 比特顺序 (bit_order)

- **lsb**: 最低位在前 (LSB First)
  - 第 0 位对应像素 0，第 1 位对应像素 1

- **msb**: 最高位在前 (MSB First)
  - 第 7 位对应像素 0，第 6 位对应像素 1

### 编码方式 (encoding)

- **positive**: 阳码
  - 1 = 亮点（显示），0 = 暗点（不显示）

- **negative**: 阴码
  - 0 = 亮点（显示），1 = 暗点（不显示）

## 示例

### 处理字体文件（自动基线对齐）

```bash
python main.py
```

这会自动：
1. 读取 config.yaml
2. 扫描 `../font` 文件夹
3. **提取每个字符的基线信息**
4. **计算统一的基线对齐高度**
5. **为每个字符应用基线偏移**
6. 为每个 TTF/BDF 文件和配置的每个字体尺寸生成 C 代码
7. **生成的C文件包含baseline aligned标记**
8. 将相同字体的多个尺寸放在同一个 .c 和 .h 文件中

### 生成的文件结构

```
../Csource/font/
├── 04B_03__.h                 # 头文件
├── 04B_03__.c                 # 源文件（包含 12px、16px、24px 的数据）
├── unifont.h
├── unifont.c
└── ...
```

## 依赖

```
pyyaml>=6.0
freetype-py>=2.5.1
numpy>=1.24.0
```

## 验证基线对齐（Baseline Alignment Verification）

提供了多个验证脚本来检查基线对齐是否正确应用：

### 1. `verify_baseline.py` - 基线偏移验证

验证生成的 C 文件中字符的基线对齐是否正确。

```bash
python verify_baseline.py
```

输出示例：
```
',' (逗号):
  原始: 2x3, bitmap_top=1
  对齐偏移: 8px (应该坐在基线下方)
  C文件字节: 0x00 0x00 0x00 0x00 0x00 0x04 0x03 0x00 0x00 0x00
  ✓ 基线对齐正确
```

### 2. `visual_compare.py` - 可视化对比

显示基线对齐后的字符矩阵和字节序列，便于视觉验证。

```bash
python visual_compare.py
```

输出示例：
```
字符: ',' (ASCII 44)
原始大小: 2x3
baseline_top: 1, offset: 8

对齐后的像素矩阵（█=1, space=0）:
   01234 <- 列
 0→·····
 1→·····
 2→·····
 3→·····
 4→·····
 5→·····
 6→·····
 7→·····
 8→·█··· ← 基线下方
 9→·█···
10→█····

完整字节序列: 00 00 00 00 00 04 03 00 00 00
```

### 3. `baseline_validation_report.py` - 完整验证报告

生成综合的基线对齐验证报告。

```bash
python baseline_validation_report.py
```

输出内容包括：
- 字体统一尺寸和字节数
- 基线指标（max_bitmap_top、min_below_baseline）
- 字符分类（Ascenders、Descenders、Baseline Sitting）
- C 文件生成验证
- 关键字符详细验证

## 迁移指南（从 pillow 版本）

### 旧版本 (font_converter.py - pillow)
- 使用 Pillow 库渲染字体
- 每个字体的每个尺寸生成独立的 .c 和 .h 文件
- 文件名包含尺寸信息，如 `font_12x16.c`

### 新版本 (freetype-py)
- 使用 freetype-py 库渲染字体
- **支持基线对齐** - 字符自动按照排版规则对齐
- 同一字体的多个尺寸放在同一个 .c 和 .h 文件中
- 文件名只包含字体名称，如 `04B_03__.c`
- 内部定义不同尺寸的数组，如 `04B_03___12`, `04B_03___16`, `04B_03___24`

## 注意事项

1. **字体路径**：所有相对路径都是相对于脚本所在目录 (`Scripts/`)

2. **基线对齐自动实现**：
   - 无需手动配置，系统自动计算和应用
   - 生成的 C 文件中 Height 值为基线对齐后的高度
   - 所有字符都根据基线偏移正确定位

3. **多尺寸处理**：不同尺寸的字体使用不同的数组名和结构体：
   - 12px: `font_name_12` 和 `font_name_12_t`
   - 16px: `font_name_16` 和 `font_name_16_t`
   - 等等

4. **文件命名**：
   - C 变量命名会自动清理特殊字符（替换为下划线）
   - 确保符合 C 语言标识符规范

5. **预览功能**：
   - 预览仅在 `preview.enabled` 为 `true` 时启用
   - 预览打印仅在 `preview.print_output` 为 `true` 时输出到控制台
   - **预览中显示基线位置标记**，方便观察基线对齐

## 故障诊断

### 字体加载失败
- 检查字体文件路径是否正确
- 确保字体文件格式支持（TTF 或 BDF）
- 查看具体错误信息

### 字符渲染为空
- 检查字体是否包含该字符
- 尝试更改字体大小

### C 文件生成错误
- 检查输出目录是否存在
- 确保有写入权限
- 检查 C 文件是否包含 "(baseline aligned)" 标记

### 基线对齐验证失败
- 运行 `baseline_validation_report.py` 检查统计信息
- 使用 `visual_compare.py` 查看字符的实际对齐情况
- 确认 Height 值是否为基线对齐后的高度（不是简单的最大高度）

## 后续删除

旧版本 `font_converter.py` 在完全迁移到新版本后可以删除。

## 相关文档

详细的基线对齐实现说明请查看 [BASELINE_ALIGNMENT_SUMMARY.md](BASELINE_ALIGNMENT_SUMMARY.md)


