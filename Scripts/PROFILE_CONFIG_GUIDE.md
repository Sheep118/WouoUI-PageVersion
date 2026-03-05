# B方案（Profile-based）实现使用文档

## 概览

已完成实现用户提案的B方案（描述性命名的profile配置），具有以下特性：

✅ **Profile配置**：按字体尺寸规格分组，集中管理  
✅ **灵活扩展**：新增尺寸组只需添加新profile  
✅ **文件或文件夹**：支持单个字体文件和文件夹混合  
✅ **CLI快速模式**：临时快速处理，无需改config  
✅ **向后兼容**：旧代码逻辑仍有保留  

---

## 配置结构

### config.yaml 新结构

```yaml
generate:
  font_format:
    layout: row_column
    bit_order: lsb
    encoding: positive
    # 全局默认char_spacing（可被profile覆盖）
    default_char_spacing:
      left_pad: 1
      right_pad: 1
      center_align: true
  
  # Profile定义（核心配置）
  profiles:
    embedded_small:              # Profile名称（使用描述性命名）
      font_paths:                # 该profile的字体列表
        - Terminus/Terminus.ttf  # 支持单个文件
        - tom-thumb/             # 支持文件夹（会自动扫描）
      sizes: [8, 12, 16]         # 该profile的尺寸组合
      # 可选：覆盖全局char_spacing
      # char_spacing:
      #   left_pad: 0
      #   right_pad: 0
    
    display_medium:
      font_paths:
        - ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf
        - Pixeloid_Font_1_0/
      sizes: [12, 24]
    
    universal:
      font_paths:
        - JinzisheFont/
      sizes: [8, 16, 24]
```

---

## 使用方法

### 1️⃣ 常规方式：处理所有profile

```bash
python main.py
```

效果：自动扫描config.yaml中定义的所有profile，按顺序处理。

**输出示例：**
```
============================================================
处理 profile: embedded_small
============================================================
处理字体: Terminus
...
生成摘要: ...

============================================================
处理 profile: display_medium
============================================================
...

============================================================
处理 profile: universal
============================================================
...

============================================================
✓ 字体生成完成！
生成了 3 个字体包
============================================================
```

---

### 2️⃣ 指定profile方式：处理单个或多个profile

```bash
# 处理单个profile
python main.py --profile embedded_small

# 处理多个profiles（用逗号分隔）
python main.py --profiles embedded_small,universal

# 等同于上面
python main.py --profile embedded_small --profile universal  # 支持多次指定
```

效果：只处理指定的profiles，忽略其他。

---

### 3️⃣ 临时快速模式：CLI参数直接指定

```bash
# 处理单个字体文件，指定尺寸
python main.py --font TestFont/test.ttf --sizes 12,16,24

# 处理文件夹下所有字体，使用相同的尺寸
python main.py --font TestFont/ --sizes 8,12,16
```

效果：不需要修改config.yaml，直接从命令行处理。

**特点：**
- 使用全局默认的char_spacing配置
- 自动检测字体类型（.ttf 或 .bdf）
- 支持文件和文件夹混合

---

### 4️⃣ 列出所有可用profiles

```bash
python main.py --list-profiles
# 或简写
python main.py --list
```

效果：显示所有定义的profiles及其配置。

**输出示例：**
```
可用的 profiles:
============================================================

  embedded_small:
    尺寸: [8, 12, 16]
    字体文件:
      - Terminus/Terminus.ttf
      - tom-thumb/tom-thumb.ttf
      - tom-thumb/tom-thumb-italic.ttf

  display_medium:
    尺寸: [12, 24]
    字体文件:
      - ZLabsBitmap_12px_ttf/ZLabsBitmap_12px_CN.ttf
      - Pixeloid_Font_1_0/Pixeloid.ttf

  universal:
    尺寸: [8, 16, 24]
    字体文件:
      - JinzisheFont/JinzisheFont-Regular.ttf

============================================================
```

---

## 典型工作流

### 场景1：首次设置

**需求**：需要为Terminus字体生成8、12、16三个尺寸

**步骤：**
```yaml
# 在config.yaml中添加profile
profiles:
  embedded_small:
    font_paths:
      - Terminus/Terminus.ttf
    sizes: [8, 12, 16]
```

```bash
# 运行生成
python main.py --profile embedded_small
```

---

### 场景2：添加新字体到现有profile

**需求**：已有embedded_small profile，再添加tom-thumb字体

**步骤：**
```yaml
# 修改config.yaml - 只需改一处
profiles:
  embedded_small:
    font_paths:
      - Terminus/Terminus.ttf
      - tom-thumb/              # 添加新字体（新增行）
    sizes: [8, 12, 16]
```

```bash
# 重新生成（自动处理两个字体）
python main.py --profile embedded_small
```

---

### 场景3：添加新的尺寸组

**需求**：需要支持32、48两个大尺寸

**步骤：**
```yaml
# 在config.yaml中添加新profile
profiles:
  ...
  display_large:                # 新增profile
    font_paths:
      - Display/display.ttf
    sizes: [32, 48]
```

```bash
# 运行生成（只处理新profile）
python main.py --profile display_large
```

---

### 场景4：临时快速测试

**需求**：测试某个新字体是否兼容

**步骤：**
```bash
# 无需改config，直接生成
python main.py --font NewFont/new.ttf --sizes 12,16
```

完成后，如果满意再添加到config.yaml中。

---

### 场景5：修改某个profile的char_spacing

**需求**：embedded_small profile需要不同的padding

**步骤：**
```yaml
# 在profile中添加char_spacing覆盖
profiles:
  embedded_small:
    font_paths:
      - Terminus/Terminus.ttf
    sizes: [8, 12, 16]
    char_spacing:               # 新增块
      left_pad: 0
      right_pad: 0
      center_align: false
```

```bash
# 重新生成（使用新的char_spacing）
python main.py --profile embedded_small
```

---

## 对比旧方式 vs 新方式

### 旧方式（全局font_sizes）
```yaml
font_sizes:
  default:
    - 12
    - 16
    - 24

# 问题：所有字体都用同样的尺寸，不灵活
```

```bash
python main.py  # 所有字体都生成12、16、24
```

---

### 新方式（Profiles）
```yaml
profiles:
  embedded_small:
    font_paths: [...]
    sizes: [8, 12, 16]   # 自定义
  
  display_large:
    font_paths: [...]
    sizes: [32, 48]      # 自定义

# 优点：
# ✅ 不同字体可用不同尺寸
# ✅ 新增尺寸组只需添加新profile
# ✅ 新增字体只需在相应profile中添加
```

```bash
python main.py              # 处理所有
python main.py --profile embedded_small  # 处理指定
python main.py --font X --sizes 12,16   # 临时快速
```

---

## 注意事项

### 1. Profile名称

- 使用描述性英文名称（如 `embedded_small`, `display_medium`）
- 避免使用set1、set2等无语义的名称
- 支持下划线连接多个单词

### 2. 文件路径

- 所有路径相对于脚本目录（Scripts/）
- 支持单个文件：`Terminus/Terminus.ttf`
- 支持文件夹：`Terminus/` （自动扫描所有.ttf和.bdf）
- 支持混合：`[单文件, 文件夹]`

### 3. 尺寸列表

- YAML格式：`sizes: [8, 12, 16]`
- 可以是任意正整数
- 顺序不影响生成结果

### 4. char_spacing覆盖

- 如果profile中定义了char_spacing，会覆盖全局default_char_spacing
- 如果profile中没定义，使用全局默认值
- CLI快速模式始终使用全局默认值

---

## 实现细节

### 新增方法

| 方法 | 用途 |
|------|------|
| `expand_font_path()` | 将字体路径规范展开为实际文件列表 |
| `generate_font_by_profile()` | 处理单个profile |
| `process_all_profiles()` | 处理所有profiles |
| `list_profiles()` | 列出所有profiles |

### 修改的方法

| 方法 | 变更 |
|------|------|
| `generate_font()` | 新增font_sizes、char_spacing_override参数 |
| `main()` | 完整的argparse CLI支持 |

### 向后兼容

- 保留了`process_font_folder()`方法（虽然不使用了）
- 旧的font_sizes配置仍可保留在config.yaml中（只是被忽略）
- CLI工具可继续使用旧的方式调用

---

## 常见问题

### Q1: 能否修改已有profile的字体或尺寸？
**A:** 当然可以。修改config.yaml后重新运行即可。

### Q2: 一个字体能属于多个profile吗？
**A:** 可以，在多个profiles的font_paths中列出即可。

### Q3: 能否为不同的profile使用不同的char_spacing？
**A:** 可以，在每个profile中独立配置char_spacing项。

### Q4: CLI快速模式和字体文件夹怎么配合？
**A:** `python main.py --font FontFolder/ --sizes 12,16` 会处理FontFolder下所有.ttf和.bdf文件。

### Q5: profiles的执行顺序是什么？
**A:** 按照config.yaml中定义的顺序顺序执行。

---

## 下一步扩展方向

### 可选改进
- [ ] 支持profile-level的layout、bit_order、encoding配置
- [ ] 支持profile优先级和依赖关系
- [ ] 支持批量操作脚本（.bat/.sh）
- [ ] 支持配置验证和检查
- [ ] 支持dry-run模式（只显示要生成的文件，不实际生成）

---

## 总结

新的B方案（Profile-based）提供了：
- **清晰的组织**：按尺寸需求分组
- **高效的扩展**：新增字体或尺寸组都很简单
- **灵活的使用**：profile模式 + CLI快速模式
- **完整的工具**：列表、查看、临时处理等功能

享受更便捷的字体生成工作流！
