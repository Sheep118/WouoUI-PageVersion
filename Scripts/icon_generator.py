"""
ICON数组生成器

支持两类来源：
1) 图片文件（PNG/JPG/BMP等）
2) 图标字体（TTF/BDF，适合 Nerd Font 图标）

输出为 C 语言 .c/.h 文件，编码布局复用 generate.font_format 配置。
"""

import argparse
import os
from pathlib import Path

import numpy as np
import yaml
from PIL import Image

from bdf_parser import BDFParser
from cfile_generater import CFileGenerator
from ttf_parser import TTFParser


class IconGenerator:
    def __init__(self, config_path):
        self.config_path = config_path
        self.script_dir = os.path.dirname(__file__)
        self.config = self._load_config(config_path)
        self._name_helper = None

    def _load_config(self, config_path):
        with open(config_path, 'r', encoding='utf-8') as f:
            return yaml.safe_load(f)

    def _resolve_path(self, relative_path):
        return os.path.abspath(os.path.join(self.script_dir, relative_path))

    def _get_icon_config(self):
        icon_cfg = self.config.get('icon', {})
        if not icon_cfg:
            raise ValueError("config.yaml 缺少 icon 配置段")
        return icon_cfg

    def _get_render_config(self):
        font_format_cfg = self.config.get('generate', {}).get('font_format', {})
        return {
            'layout': font_format_cfg.get('layout', 'row_column'),
            'bit_order': font_format_cfg.get('bit_order', 'lsb'),
            'encoding': font_format_cfg.get('encoding', 'positive'),
        }

    def _normalize_name(self, name, fallback='icon'):
        # 延迟初始化helper以避免循环依赖
        if self._name_helper is None:
            self._name_helper = CFileGenerator('IconNameSanitizer', self._get_render_config())
        text = str(name or '').strip() or fallback
        normalized = self._name_helper._sanitize_name(text, fallback)
        if not normalized:
            normalized = fallback
        return normalized

    def _fit_pixels_to_canvas(self, pixels, width, height):
        if pixels is None:
            return np.zeros((height, width), dtype=np.uint8)

        src_h, src_w = pixels.shape
        canvas = np.zeros((height, width), dtype=np.uint8)

        dst_x = max((width - src_w) // 2, 0)
        dst_y = max((height - src_h) // 2, 0)
        src_x = max((src_w - width) // 2, 0)
        src_y = max((src_h - height) // 2, 0)

        copy_w = min(src_w, width)
        copy_h = min(src_h, height)

        canvas[dst_y:dst_y + copy_h, dst_x:dst_x + copy_w] = (
            pixels[src_y:src_y + copy_h, src_x:src_x + copy_w] > 0
        ).astype(np.uint8)

        return canvas

    def _load_image_icon_pixels(self, image_path, width, height, threshold):
        with Image.open(image_path) as img:
            gray = img.convert('L')
            resized = gray.resize((width, height), Image.Resampling.LANCZOS)
            arr = np.array(resized, dtype=np.uint8)
        return (arr >= threshold).astype(np.uint8)

    def _normalize_encoding_mode(self, value, default='positive'):
        token = str(value or '').strip().lower()
        mapping = {
            'positive': 'positive',
            'negative': 'negative',
            'pos': 'positive',
            'neg': 'negative',
            'yang': 'positive',
            'yin': 'negative',
            '阳码': 'positive',
            '阴码': 'negative',
        }
        return mapping.get(token, default)

    def _parse_image_paths_config(self, image_paths_cfg):
        grouped = {
            'positive': [],
            'negative': [],
        }

        if isinstance(image_paths_cfg, list):
            grouped['positive'].extend(image_paths_cfg)
            return grouped

        if not isinstance(image_paths_cfg, dict):
            return grouped

        key_aliases = {
            'positive': 'positive',
            'negative': 'negative',
            'pos': 'positive',
            'neg': 'negative',
            'yang': 'positive',
            'yin': 'negative',
            '阳码': 'positive',
            '阴码': 'negative',
        }

        for raw_key, raw_value in image_paths_cfg.items():
            bucket = key_aliases.get(str(raw_key).strip().lower())
            if not bucket:
                print(f"[!] image_paths 分组键无效，已忽略: {raw_key}")
                continue

            if isinstance(raw_value, list):
                grouped[bucket].extend(raw_value)
            elif raw_value:
                grouped[bucket].append(raw_value)

        return grouped

    def _parse_codepoint_text(self, text):
        token = str(text).strip().upper()
        if token.startswith('U+'):
            token = token[2:]
        if token.startswith('0X'):
            token = token[2:]
        return chr(int(token, 16))

    def _parse_font_icon_item(self, item):
        if isinstance(item, str):
            raw = item.strip()
            if not raw:
                return None
            if len(raw) == 1:
                char = raw
            else:
                char = self._parse_codepoint_text(raw)
            return {
                'name': f"U{ord(char):04X}",
                'char': char,
            }

        if not isinstance(item, dict):
            return None

        name = item.get('name')
        if item.get('char'):
            char = str(item.get('char'))[0]
        elif item.get('glyph'):
            char = str(item.get('glyph'))[0]
        elif item.get('codepoint'):
            char = self._parse_codepoint_text(item.get('codepoint'))
        else:
            return None

        if not name:
            name = f"U{ord(char):04X}"

        return {
            'name': name,
            'char': char,
        }

    def _create_font_parser(self, font_path, size, render_config):
        lower = font_path.lower()
        if lower.endswith('.ttf') or lower.endswith('.otf'):
            return TTFParser(font_path, size, render_config)
        if lower.endswith('.bdf'):
            return BDFParser(font_path, size, render_config)
        raise ValueError(f"不支持的图标字体类型: {font_path}")

    def _collect_icons_from_images(self, icon_cfg, width, height, render_config):
        image_paths_cfg = icon_cfg.get('image_paths', []) or []
        grouped_paths = self._parse_image_paths_config(image_paths_cfg)
        base_encoding = self._normalize_encoding_mode(render_config.get('encoding', 'positive'))
        threshold = int(icon_cfg.get('image_threshold', 128))

        entries = []
        for image_encoding in ('positive', 'negative'):
            for image_rel in grouped_paths[image_encoding]:
                full_path = self._resolve_path(image_rel)
                if not os.path.exists(full_path):
                    print(f"[!] 图片不存在，跳过: {image_rel}")
                    continue

                try:
                    pixels = self._load_image_icon_pixels(full_path, width, height, threshold)
                    if image_encoding != base_encoding:
                        pixels = (1 - pixels).astype(np.uint8)

                    name = self._normalize_name(Path(image_rel).stem, 'image_icon')
                    entries.append({'name': name, 'pixels': pixels})
                    print(
                        f"[+] 图片图标: {image_rel} -> {name} "
                        f"(image={image_encoding}, global={base_encoding})"
                    )
                except Exception as e:
                    print(f"[!] 图片解析失败，跳过 {image_rel}: {e}")

        return entries

    def _collect_icons_from_fonts(self, icon_cfg, width, height, render_config):
        font_sources = icon_cfg.get('icon_fonts', []) or []
        entries = []

        for source in font_sources:
            if not isinstance(source, dict):
                print("[!] icon_fonts 项必须是对象，已跳过")
                continue

            font_rel = source.get('font_path')
            if not font_rel:
                print("[!] icon_fonts 项缺少 font_path，已跳过")
                continue

            font_full = self._resolve_path(font_rel)
            if not os.path.exists(font_full):
                print(f"[!] 字体文件不存在，跳过: {font_rel}")
                continue

            font_size = int(source.get('size', height))
            icon_items = source.get('icons', []) or []
            if not icon_items:
                print(f"[!] 字体来源未配置 icons，跳过: {font_rel}")
                continue

            try:
                parser = self._create_font_parser(font_full, font_size, render_config)
            except Exception as e:
                print(f"[!] 字体加载失败，跳过 {font_rel}: {e}")
                continue

            print(f"[+] 处理图标字体: {font_rel} (size={font_size})")
            for raw_item in icon_items:
                parsed = self._parse_font_icon_item(raw_item)
                if not parsed:
                    print(f"    [!] 无法解析图标项，已跳过: {raw_item}")
                    continue

                char = parsed['char']
                name = self._normalize_name(parsed['name'], f"U{ord(char):04X}")
                pixels, glyph_w, glyph_h = parser.render_character(char)

                if pixels is None or glyph_w <= 0 or glyph_h <= 0:
                    print(f"    [!] 字形缺失，已跳过: {name} ({char})")
                    continue

                fitted = self._fit_pixels_to_canvas(pixels, width, height)
                entries.append({'name': name, 'pixels': fitted})
                print(f"    [+] 图标字形: {name} (U+{ord(char):04X}, {glyph_w}x{glyph_h})")

        return entries

    def generate_icons(self):
        icon_cfg = self._get_icon_config()
        render_config = self._get_render_config()

        width = int(icon_cfg.get('width', 30))
        height = int(icon_cfg.get('height', 30))
        output_name = icon_cfg.get('output_file_name', 'IconAssets')
        output_folder = self._resolve_path(icon_cfg.get('output_c_folder', '../Csource/font'))

        image_entries = self._collect_icons_from_images(icon_cfg, width, height, render_config)
        font_entries = self._collect_icons_from_fonts(icon_cfg, width, height, render_config)

        all_entries = image_entries + font_entries
        if not all_entries:
            print("[-] 没有可生成的ICON数据")
            return False, None

        generator = CFileGenerator(output_name, render_config)
        header_path, source_path = generator.save_icon_files(
            output_folder,
            output_name,
            all_entries,
            width,
            height,
        )

        print("\n[+] ICON数组生成完成")
        print(f"    输出头文件: {header_path}")
        print(f"    输出源文件: {source_path}")
        print(f"    图标数量: {len(all_entries)}")
        print(f"    图标尺寸: {width}x{height}")
        print(
            f"    编码格式: layout={render_config['layout']}, bit_order={render_config['bit_order']}, encoding={render_config['encoding']}"
        )

        return True, (header_path, source_path)


def main():
    parser = argparse.ArgumentParser(description='ICON数组生成器')
    parser.add_argument('--config', default='config.yaml', help='配置文件路径（默认: config.yaml）')
    args = parser.parse_args()

    script_dir = os.path.dirname(__file__)
    config_path = args.config
    if not os.path.isabs(config_path):
        config_path = os.path.join(script_dir, config_path)
    config_path = os.path.abspath(config_path)

    if not os.path.exists(config_path):
        print(f"[-] 配置文件不存在: {config_path}")
        return

    try:
        generator = IconGenerator(config_path)
        ok, _ = generator.generate_icons()
        if not ok:
            raise SystemExit(1)
    except Exception as e:
        print(f"[-] ICON生成失败: {e}")
        raise SystemExit(1)


if __name__ == '__main__':
    main()
