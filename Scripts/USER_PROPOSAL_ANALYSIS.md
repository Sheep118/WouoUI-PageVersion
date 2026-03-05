# 用户提案方案分析与完善

## 核心方案

用户提案结构：
```yaml
fonts:
  set1:
    font_path: [...]
    sizes: [8, 12, 16]
  set2:
    font_path: [...]
    sizes: [12, 24]
```

## 可行性分析 ✅

### 优点
- ✅ **清晰逻辑** - 按尺寸规格分组，而不是按字体分组
- ✅ **易于扩展** - 新增尺寸组只需添加set3、set4
- ✅ **减少重复** - 同一尺寸组的字体共享配置
- ✅ **支持灵活** - 字体可以是文件或文件夹（通配）
- ✅ **维护简单** - 修改某个字体的尺寸只需改对应的set
- ✅ **混合搭配** - 一个字体可能属于多个set（如果需要）

### 功能实现难度 ⭐⭐ 简单
- 修改config.yaml结构 - 1分钟
- 修改main.py读取逻辑 - 10分钟
- 支持CLI参数 - 15分钟
- 测试 - 5分钟

---

## 命名建议

### 选项对比

| 方案 | 示例 | 优点 | 缺点 |
|------|------|------|------|
| set1, set2 | ❌ | 简单 | 无语义，看不出差别 |
| **profiles** | `8_12_16`, `12_24` | 专业，能看出尺寸 | 稍长 |
| **groups** | `small_set`, `large_set` | 直观 | 需要自己命名 |
| **presets** | `embedded`, `display` | 通用 | 需要自己理解 |
| **specs** | `compact`, `standard` | 简洁 | 模糊 |

### 我的推荐

**用尺寸作为profile名称**（最清晰）：

```yaml
fonts:
  # 按usize命名，能直观看出支持的尺寸
  8_12_16:
    font_paths: [...]
    sizes: [8, 12, 16]
  
  12_24:
    font_paths: [...]
    sizes: [12, 24]
  
  8_16_24:
    font_paths: [...]
    sizes: [8, 16, 24]
```

或者**用描述性名称**（更灵活）：

```yaml
fonts:
  embedded_small:     # 嵌入式小屏幕
    font_paths: [...]
    sizes: [8, 12, 16]
  
  embedded_medium:    # 嵌入式中等屏幕
    font_paths: [...]
    sizes: [12, 24]
  
  desktop:            # 桌面应用
    font_paths: [...]
    sizes: [8, 16, 24]
```

---

## 完整的config.yaml示例

### 结构1：尺寸为key（推荐）

```yaml
generate:
  font_folder: ../font
  output_c_folder: ../Csource/font
  
  font_format:
    layout: row_column
    bit_order: lsb
    encoding: positive
    # 全局默认char_spacing
    default_char_spacing:
      left_pad: 1
      right_pad: 1
      center_align: true
  
  # 关键改动：按尺寸规格分组
  fonts:
    8_12_16:  # 第一个尺寸组 - 嵌入式小屏
      font_paths:
        - Terminus/Terminus.ttf
        - tom-thumb/
      sizes: [8, 12, 16]
      char_spacing:  # 可选：该组可覆盖全局默认
        left_pad: 0
        right_pad: 0
    
    12_24:    # 第二个尺寸组 - 中等屏幕
      font_paths:
        - ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf
        - Pixeloid_Font_1_0/
      sizes: [12, 24]
      # 不指定char_spacing则使用全局默认
    
    8_16_24:  # 第三个尺寸组 - 完整尺寸
      font_paths:
        - JinzisheFont/JinzisheFont-Regular.ttf
        - unifont/
      sizes: [8, 16, 24]
  
  charset:
    ascii_start: 32
    ascii_end: 126

preview:
  enabled: true
  print_output: true
  text: "WQghH123我"
```

### 结构2：用描述性名称

```yaml
fonts:
  embedded_small:
    font_paths:
      - Terminus/Terminus.ttf
      - tom-thumb/
    sizes: [8, 12, 16]
  
  display_medium:
    font_paths:
      - ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf
      - Pixeloid_Font_1_0/
    sizes: [12, 24]
  
  universal:
    font_paths:
      - JinzisheFont/JinzisheFont-Regular.ttf
    sizes: [8, 16, 24]
```

---

## 实现思路

### main.py 修改要点

```python
def generate_font_by_profile(self, profile_name):
    """按profile生成字体组"""
    fonts_config = self.config.get('generate', {}).get('fonts', {})
    
    if profile_name not in fonts_config:
        print(f"✗ profile '{profile_name}' 不存在")
        return
    
    profile = fonts_config[profile_name]
    font_paths = profile.get('font_paths', [])
    sizes = profile.get('sizes', [12])
    
    # 对该profile下的每个字体和尺寸组合
    for font_path in font_paths:
        # 支持文件或文件夹
        if font_path.endswith('.ttf') or font_path.endswith('.bdf'):
            # 单个字体文件
            self.generate_font(font_path, sizes, ...)
        else:
            # 文件夹 - 扫描所有ttf/bdf
            for font_file in Path(font_path).glob('*.ttf'):
                self.generate_font(str(font_file), sizes, ...)


def process_all_profiles(self):
    """处理所有profile"""
    fonts_config = self.config.get('generate', {}).get('fonts', {})
    
    for profile_name in fonts_config:
        print(f"\n处理profile: {profile_name}")
        self.generate_font_by_profile(profile_name)
```

### CLI参数支持

```bash
# 无参数：处理所有profile
python main.py

# 指定profile
python main.py --profile 8_12_16

# 临时快速处理单个字体文件
python main.py --font TestFont/test.ttf --sizes 12,16,24

# 临时快速处理文件夹
python main.py --font-folder TestFonts/ --sizes 12,16

# 列出所有可用profile
python main.py --list-profiles
```

---

## 工作流示例

### 初始化
```yaml
# config.yaml
fonts:
  embedded:
    font_paths: [Terminus/Terminus.ttf]
    sizes: [8, 12, 16]
```

**生成：** `python main.py` → 自动处理embedded profile → 生成3个尺寸

### 添加新字体到embedded组
```yaml
fonts:
  embedded:
    font_paths: 
      - Terminus/Terminus.ttf
      - tom-thumb/             # 添加新字体
    sizes: [8, 12, 16]
```

**生成：** `python main.py` → 自动处理两个字体

### 添加新的尺寸组
```yaml
fonts:
  embedded:
    font_paths: [...]
    sizes: [8, 12, 16]
  
  display:                      # 新增profile
    font_paths: [Display/display.ttf]
    sizes: [24, 32, 48]
```

**生成：** `python main.py` → 自动处理两个profile

### 临时快速处理
```bash
python main.py --font TestFont/test.ttf --sizes 12,16
# 不需要改config.yaml
```

---

## 对比总结

### vs. 全局font_sizes列表（原始）
```
原始: font_sizes: [12, 16, 24]  # 所有字体都用这个尺寸
你的: 按profile分组，不同字体可用不同尺寸     ✅ 更灵活
```

### vs. 方案2（全局映射表）
```
方案2: "Terminus/Terminus.ttf": {sizes: [...]}   # 字体→尺寸
你的: "8_12_16": {font_paths: [...]}             ✅ 反向映射，更直观
```

### vs. 方案1（folder元数据）
```
方案1: 每个folder下创建font_config.yaml        # 文件多
你的: 统一在config.yaml中管理                   ✅ 集中管理，易于全局规划
```

---

## 最终评价

**强烈推荐采用你的方案！** 原因：

1. ✅ **符合使用场景** - 实际工作中按尺寸规格分组比按字体分组更自然
2. ✅ **维护高效** - 新增字体、新增尺寸组都很简单
3. ✅ **可视化** - 看config就能理解各个字体的用途和尺寸
4. ✅ **兼具灵活** - CLI参数+profile组合，覆盖所有场景
5. ✅ **扩展性强** - 可以后续支持profile-level的char_spacing、encoding等

---

## 建议方案：user_method + my_enhancements

### 你的核心思想 ✅
- 按尺寸规格分组 → **保留**

### 我的建议补充
1. **命名** - 用 `8_12_16` 或 `embedded_small` 这样有语义的名称
2. **char_spacing覆盖** - 可选支持profile级别的char_spacing（如上面config示例）
3. **CLI参数** - 支持 `--profile` 和 `--font/--sizes` 两种模式
4. **通配符** - 支持 `Fonts/*/` 这样的glob模式

这样既保留了你的简洁逻辑，又增加了必要的灵活性。

---

## 准备好实现吗？

需要我帮你：
1. ✅ 修改config.yaml结构和示例
2. ✅ 修改main.py支持新配置逻辑
3. ✅ 添加CLI参数解析
4. ✅ 添加profile管理功能（列表、验证等）

还是你想先调整一下结构后再实现？
