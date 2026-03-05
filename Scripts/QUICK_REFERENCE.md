# 快速参考卡 - Profile配置系统

## 最常用命令

```bash
# ① 处理所有profiles
python main.py

# ② 处理指定profile
python main.py --profile chinese_fonts

# ③ 临时快速处理（不改config）
python main.py --font ../font/Terminus/ --sizes 12,16,24

# ④ 查看所有profiles
python main.py --list
```

---

## config.yaml结构速查

```yaml
generate:
  font_format:
    # ... 其他配置 ...
    default_char_spacing:
      left_pad: 1
      right_pad: 1

  profiles:                    # 核心！
    profile_name_1:
      font_paths:
        - ../font/FontA/
        - ../font/FontB.ttf
      sizes: [8, 12, 16]
      # char_spacing: {...}  # 可选：覆盖全局

    profile_name_2:
      font_paths: [...]
      sizes: [12, 24]
```

---

## 常见任务

| 任务 | 操作 |
|------|------|
| **添加新字体** | 在对应profile的font_paths中添加一行 |
| **添加新尺寸组** | 创建新profile并定义sizes |
| **修改某字体的padding** | 在profile中添加char_spacing块 |
| **快速测试新字体** | `python main.py --font PATH --sizes X,Y,Z` |
| **查看现有配置** | `python main.py --list` |

---

## Profile命名建议

```
embedded_small       # 小屏幕
embedded_large       # 大屏幕
compact_fonts        # 紧凑排版
bitmap_fonts         # 位图字体
ascii_only           # 英文字体
chinese_fonts        # 中文字体
monospace_fonts      # 等宽字体
```

---

## 路径写法

✅ **正确**
```yaml
font_paths:
  - ../font/Terminus/               # 文件夹
  - ../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf  # 文件
  - ../font/unifont/
```

❌ **错误**
```yaml
font_paths:
  - Terminus/                        # 缺少../font/
  - C:/fonts/Terminus/              # 绝对路径
  - ../font/Terminus/*.ttf           # 文件已经会自动扫描
```

---

## 生成结果位置

生成的C文件存放在：
```
g:\WouoUI\WouoUISDL2\WouoUI-PageVersion\Csource\font\

├── ZLabsBitmap_12px_CN.h
├── ZLabsBitmap_12px_CN.c
├── TerminessNerdFont_Bold.h
├── TerminessNerdFont_Bold.c
└── ... 其他文件
```

---

## 性能提示

- **首次运行**: 处理所有profiles，会比较慢
- **生成单个**: 用 `--profile X` 只处理需要的
- **测试新字体**: 用 `--font X --sizes Y` 快速验证

---

## 常见错误解决

### 错误：字体路径不存在
```
[!] 字体路径不存在或不可识别: ../font/XXX
```
**解决**：检查路径是否正确（需要../font/前缀）

### 错误：尺寸格式错误
```
[-] 无效的尺寸列表: 12 16 24
```
**解决**：用逗号分隔，不要用空格 → `12,16,24`

### 错误：Profile不存在
```
[-] profile 'XXX' 不存在
```
**解决**：用 `python main.py --list` 查看可用的profiles

### 生成很慢
**原因**：文件夹中有许多字体，都被扫描和生成  
**解决**：
1. 明确指定文件而不是文件夹
2. 或创建多个小profile而不是一个大profile

---

## 示例config.yaml

```yaml
generate:
  font_folder: ../font
  output_c_folder: ../Csource/font

  font_format:
    layout: row_column
    bit_order: lsb
    encoding: positive
    default_char_spacing:
      left_pad: 1
      right_pad: 1
      center_align: true

  profiles:
    # 简体中文字体
    chinese:
      font_paths:
        - ../font/ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf
      sizes: [12, 24]

    # 等宽编程字体
    monospace:
      font_paths:
        - ../font/Terminus/
      sizes: [8, 12, 16]

    # 紧凑型（不要padding）
    compact:
      font_paths:
        - ../font/04b_03/
      sizes: [8, 12]
      char_spacing:
        left_pad: 0
        right_pad: 0
        center_align: false

    # Unicode通用字体
    unicode:
      font_paths:
        - ../font/unifont/
      sizes: [16]

  charset:
    ascii_start: 32
    ascii_end: 126

preview:
  enabled: true
  print_output: true
  text: "WQghH123我"
```

---

## 工作流用例

### 情景：要为新项目生成几套字体

**步骤1** - 编辑config.yaml，创建新profile
```yaml
profiles:
  my_project:
    font_paths:
      - ../font/选中的字体文件或文件夹/
    sizes: [需要的尺寸]
```

**步骤2** - 生成
```bash
python main.py --profile my_project
```

**步骤3** - 找到生成的文件
```
Csource/font/ 文件夹下
```

---

## 最后提示

💡 **保存当前config.yaml** - 这样下次就不用重新配置  
💡 **定期用--list检查** - 确保配置没有问题  
💡 **注释多写一些** - 几个月后自己还能看懂  
💡 **profiles命名保持统一** - 便于长期维护

---

## 快速帮助

```bash
# 查看所有命令选项
python main.py --help

# 完整文档
cat PROFILE_CONFIG_GUIDE.md
cat PROFILE_SCHEME_COMPLETION.md
```

祝使用愉快！😊
