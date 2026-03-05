# 多字体源文件配置方案对比

## 场景说明
- 字体源文件1(Terminus)：适合 8, 12, 16
- 字体源文件2(ZLabsBitmap)：适合 12, 24  
- 字体源文件3(JinzisheFont)：适合 8, 16, 24
- 字体源文件N：各自不同的尺寸需求

---

## 方案1：字体文件夹元数据（⭐⭐⭐⭐⭐ 推荐）

**思路**：每个字体文件夹下放一个 `font_config.yaml` 元数据文件，统一定义该文件夹内所有字体的尺寸。

### 结构
```
font/
├── Terminus/
│   ├── Terminus.ttf
│   ├── Terminus.otf
│   └── font_config.yaml          ← 元数据
├── ZLabsBitmap_12px_ttf/
│   ├── ZLabsBitmap_12px_CN.ttf
│   └── font_config.yaml          ← 元数据
└── JinzisheFont/
    ├── JinzisheFont-Regular.ttf
    └── font_config.yaml          ← 元数据
```

### 各个 font_config.yaml 的内容

**Terminus/font_config.yaml：**
```yaml
# Terminus字体编集配置
fonts:
  - name: Terminus
    sizes: [8, 12, 16]        # 只生成这三个尺寸
    char_spacing:
      left_pad: 1
      right_pad: 1
      center_align: true
```

**ZLabsBitmap_12px_ttf/font_config.yaml：**
```yaml
fonts:
  - name: ZLabsBitmap_12px_CN
    sizes: [12, 24]           # 只生成这两个尺寸
    char_spacing:
      left_pad: 1
      right_pad: 1
      center_align: true
```

**JinzisheFont/font_config.yaml：**
```yaml
fonts:
  - name: JinzisheFont-Regular
    sizes: [8, 16, 24]        # 这个需要三个尺寸
    char_spacing:
      left_pad: 0
      right_pad: 0
      center_align: false
```

### 全局 config.yaml
```yaml
generate:
  font_folder: ../font
  output_c_folder: ../Csource/font
  
  font_format:
    layout: row_column
    bit_order: lsb
    encoding: positive
    # 全局默认 char_spacing（如果字体folder的config没定义就用此默认值）
    default_char_spacing:
      left_pad: 1
      right_pad: 1
      center_align: true

  # 不再需要全局的 font_sizes 列表！
```

### 优势
- ✅ **不需要修改全局config** - 只需在字体文件夹中创建元数据文件
- ✅ **可扩展** - 添加新字体时只需创建新的 font_config.yaml
- ✅ **清晰** - 每个字体的配置自成一体，互不干扰
- ✅ **可维护** - 后续修改某个字体的尺寸只需改那个folder下的文件
- ✅ **灵活** - 可以对每个字体独立配置 char_spacing

### 实现要点
修改 `main.py` 的 `process_font_folder()` 方法：
```python
def process_font_folder(self):
    # 扫描字体文件夹
    font_folder = gen_config.get('font_folder', '../font')
    
    for subfolder in os.listdir(full_folder_path):
        subfolder_path = os.path.join(full_folder_path, subfolder)
        
        # 检查该文件夹下是否有 font_config.yaml
        font_meta_path = os.path.join(subfolder_path, 'font_config.yaml')
        if os.path.exists(font_meta_path):
            # 加载该子文件夹的元数据
            with open(font_meta_path) as f:
                meta = yaml.safe_load(f)
            
            # 按照元数据中定义的字体和尺寸生成
            for font_config in meta.get('fonts', []):
                font_name = font_config.get('name')
                sizes = font_config.get('sizes', [12])  # 默认12
                
                # 寻找该字体文件（.ttf or .bdf）
                for ext in ['.ttf', '.bdf']:
                    font_path = os.path.join(subfolder_path, font_name + ext)
                    if os.path.exists(font_path):
                        # 使用元数据中的尺寸列表生成
                        self.generate_font(font_path, sizes, ...)
                        break
```

---

## 方案2：全局字体映射表（⭐⭐⭐⭐ 次推荐）

**思路**：在全局 `config.yaml` 中定义一个 字体名称→尺寸 的映射表。

### 配置示例
```yaml
generate:
  font_folder: ../font
  output_c_folder: ../Csource/font
  
  font_format:
    layout: row_column
    bit_order: lsb
    encoding: positive
    char_spacing:
      left_pad: 1
      right_pad: 1
      center_align: true

  # 字体规格定义表
  font_specs:
    # 键是字体文件路径（相对于font_folder）
    "Terminus/Terminus.ttf":
      sizes: [8, 12, 16]
      
    "ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf":
      sizes: [12, 24]
      
    "JinzisheFont/JinzisheFont-Regular.ttf":
      sizes: [8, 16, 24]
    
    # 可选：用文件夹名称作为通配符
    "tom-thumb/*":
      sizes: [8, 16]
```

### 优势
- ✅ 集中管理，所有配置在一个文件
- ✅ 可以轻松看出全局字体的尺寸规划
- ✅ 支持通配符匹配

### 劣势
- ❌ config.yaml会很长（字体多的话）
- ❌ 需要知道准确的相对路径
- ❌ 添加新字体时需要修改全局config

---

## 方案3：命令行参数扩展（⭐⭐⭐⭐ 实用）

**思路**：支持在命令行指定字体和尺寸，config作为默认值。

### 使用示例
```bash
# 方式1：直接指定字体和尺寸
python main.py --font Terminus/Terminus.ttf --sizes 8,12,16

# 方式2：按字体文件夹批量处理（使用folder内的元数据）
python main.py --folder ZLabsBitmap_12px_ttf --use-meta

# 方式3：交互模式
python main.py --interactive

# 方式4：使用配置文件（走方案1或2）
python main.py --use-config
```

### 代码扩展
```python
import argparse

def main():
    parser = argparse.ArgumentParser(description='WouoUI 字体生成工具')
    parser.add_argument('--font', help='指定字体文件路径')
    parser.add_argument('--sizes', help='指定尺寸，用逗号分隔: 8,12,16')
    parser.add_argument('--folder', help='指定字体文件夹')
    parser.add_argument('--use-meta', action='store_true', help='使用folder内的font_config.yaml')
    
    args = parser.parse_args()
    
    generator = FontGenerator(config_path)
    
    if args.font and args.sizes:
        sizes = list(map(int, args.sizes.split(',')))
        generator.generate_font(args.font, sizes)
    elif args.folder and args.use_meta:
        generator.process_font_folder_with_meta(args.folder)
    else:
        generator.process_font_folder()  # 默认处理全部
```

### 优势
- ✅ 最灵活，不需要修改config
- ✅ 一次性指定，临时快速处理
- ✅ 支持batch脚本或自动化流程

---

## 方案4：分层配置（⭐⭐⭐ 可用）

**思路**：全局 → 文件夹级 → 文件级 的三层配置继承。

### 结构
```
config.yaml (全局默认)
  └─ font/
      └─ ZLabsBitmap_12px_ttf/
          ├── font_config.yaml (文件夹级，可覆盖全局)
          └─ ZLabsBitmap_12px_CN.ttf
              └─ metadata.yaml (文件级，可覆盖文件夹级)
```

### 优势
- ✅ 灵活性很高
- ✅ 可以渐进式覆盖配置

### 劣势
- ❌ 复杂度高
- ❌ 维护成本大
- ❌ 文件过多

---

## 方案5：脚本生成器（⭐⭐② 辅助方案）

**思路**：写一个工具脚本，自动扫描字体并生成 font_config.yaml。

### 工具脚本 `prepare_font_config.py`
```python
"""根据字体文件特性自动生成font_config.yaml"""

def auto_detect_config(font_path):
    """根据字体特征自动检测合适的尺寸"""
    parser = TTFParser(font_path, 12)  # 用12pt试探
    
    width = parser.get_max_width()
    height = parser.get_max_height()
    
    # 根据字体宽度推断
    if width <= 4:  # 很窄的字体
        return [8, 12, 16]
    elif width <= 8:  # 中等宽度
        return [12, 16, 24]
    else:  # 宽字体
        return [16, 24]

def generate_folder_config(folder_path):
    """为一个字体文件夹生成font_config.yaml"""
    config = {'fonts': []}
    
    for font_file in glob.glob(os.path.join(folder_path, '*.ttf')):
        font_name = os.path.splitext(os.path.basename(font_file))[0]
        sizes = auto_detect_config(font_file)
        
        config['fonts'].append({
            'name': font_name,
            'sizes': sizes,
            'char_spacing': {
                'left_pad': 1,
                'right_pad': 1,
                'center_align': True
            }
        })
    
    # 保存为font_config.yaml
    with open(os.path.join(folder_path, 'font_config.yaml'), 'w') as f:
        yaml.dump(config, f)
```

### 使用
```bash
# 为所有字体子文件夹生成config
python prepare_font_config.py ../font

# 或指定单个文件夹
python prepare_font_config.py ../font/ZLabsBitmap_12px_ttf
```

---

## 推荐方案排序

### 最佳实践：方案1 + 方案3 的组合

**第一阶段（方案1）**：
- 创建 `font_config.yaml` 元数据文件在每个字体文件夹
- 修改 main.py 支持读取元数据
- 这样新增字体时只需创建元数据文件，一次配置，永久生效

**第二阶段（方案3）**：
- 支持命令行参数
- 临时生成单个字体时不需要修改任何config
- `python main.py --font Terminus/Terminus.ttf --sizes 8,12,16`

**第三阶段（方案5）**：
- 可选工具：自动检测字体类型，生成推荐配置
- `python prepare_font_config.py ../font`

---

## 对比总结表

| 特性 | 方案1 | 方案2 | 方案3 | 方案4 | 方案5 |
|------|------|------|------|------|------|
| 维护难度 | ⭐ | ⭐⭐ | ⭐ | ⭐⭐⭐⭐ | ⭐⭐ |
| 灵活性 | ⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| 扩展性 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| 全局可见 | ❌ | ✅ | ❌ | ⭕ | ❌ |
| 新增字体 | 无需改config | 需改config | 无需改config | 无需改config | 自动 |
| 临时快速 | ❌ | ❌ | ✅ | ⭕ | ❌ |
| 推荐度 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ |

---

## 我的强烈推荐

**采用方案1（字体文件夹元数据）+ 方案3（命令行参数）**

### 原因

1. **最小化修改**：不需要频繁改全局config
2. **易于扩展**：新增字体只需创建元数据文件
3. **清晰可维护**：每个字体文件夹是独立的单元
4. **兼具灵活性**：支持命令行临时快速处理

### 示例工作流

**第1次设置Terminus字体：**
```
1. 创建 font/Terminus/font_config.yaml:
   fonts:
     - name: Terminus
       sizes: [8, 12, 16]

2. python main.py
   → 自动扫描发现font_config.yaml
   → 按配置生成 8, 12, 16 三个尺寸
```

**添加新的JinzisheFont字体：**
```
1. 创建 font/JinzisheFont/font_config.yaml:
   fonts:
     - name: JinzisheFont-Regular
       sizes: [8, 16, 24]

2. python main.py
   → 自动处理，无需改全局config
```

**临时快速处理某个特定字体：**
```bash
python main.py --font TestFont/test.ttf --sizes 12,16,24
```

这样既保证了日常使用的便利性，又提供了足够的灵活性！

