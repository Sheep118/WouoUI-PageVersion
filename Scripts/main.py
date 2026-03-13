"""
字体生成主程序 - 使用freetype-py
Main Font Generator - Using freetype-py

读取config配置，调用bdf/ttf parser生成字体，最后生成C文件和头文件
"""

import os
import sys
import yaml
import argparse
from pathlib import Path

from ttf_parser import TTFParser
from bdf_parser import BDFParser
from cfile_generater import CFileGenerator
from char_scanner import CharScanner, load_scan_result, print_frequency_summary


class FontGenerator:
    """字体生成主程序"""
    
    def __init__(self, config_path):
        """
        初始化字体生成程序
        
        Args:
            config_path (str): config.yaml文件路径
        """
        self.config_path = config_path
        self.config = self._load_config()
        self.script_dir = os.path.dirname(__file__)
        self.scan_summary = None
    
    def _load_config(self):
        """
        加载config.yaml配置文件
        
        Returns:
            dict: 配置字典
        """
        try:
            with open(self.config_path, 'r', encoding='utf-8') as f:
                config = yaml.safe_load(f)
            return config
        except Exception as e:
            raise Exception(f"无法加载config文件: {self.config_path}\n错误信息: {e}")
    
    def _resolve_path(self, relative_path):
        """
        解析相对路径（相对于脚本目录）
        
        Args:
            relative_path (str): 相对路径
            
        Returns:
            str: 绝对路径
        """
        full_path = os.path.join(self.script_dir, relative_path)
        return os.path.abspath(full_path)

    def scan_chinese_charset(self):
        """执行中文扫描，并从扫描报告中重新读取字符集与频次。"""
        scanner = CharScanner(self.config_path)
        scanner.run()

        scan_cfg = self.config.get('scan', {})
        report_path = self._resolve_path(scan_cfg.get('output_report', './scan_result.txt'))
        summary = load_scan_result(report_path)
        print_frequency_summary(summary['counter'])
        self.scan_summary = summary
        return summary

    def _filter_chars_by_encoding(self, chars):
        """过滤无法用当前中文索引编码表示的字符。"""
        chinese_encoding = self.config.get('generate', {}).get('chinese_encoding', 'utf-8')
        accepted = []
        skipped = []

        for char in chars:
            try:
                char.encode(chinese_encoding)
                accepted.append(char)
            except UnicodeEncodeError:
                skipped.append(char)

        if skipped:
            print(f"[!] 以下中文字符无法使用 {chinese_encoding} 编码，已跳过: {''.join(skipped)}")

        return accepted
    
    def generate_font(self, font_path, font_sizes=None, font_type=None, char_spacing_override=None,
                      cn_chars=None):
        """
        生成一个字体的多个尺寸
        
        Args:
            font_path (str): 字体文件路径（相对于脚本所在目录）
            font_sizes (list): 字体尺寸列表 (默认从config读取)
            font_type (str): 字体类型 ('ttf'、'bdf'或None=自动检测)
            char_spacing_override (dict): 覆盖默认的char_spacing配置
            cn_chars (list[str] | None): 要额外生成的中文字符列表
            
        Returns:
            tuple: (成功标志, 生成的文件路径)
        """
        # 获取配置
        gen_config = self.config.get('generate', {})
        font_format_config = gen_config.get('font_format', {})
        preview_config = self.config.get('preview', {})
        
        # 解析配置参数
        layout = font_format_config.get('layout', 'row_column')
        bit_order = font_format_config.get('bit_order', 'lsb')
        encoding = font_format_config.get('encoding', 'positive')
        
        # 处理char_spacing：优先级 override > profile > global_default
        if char_spacing_override is not None:
            char_spacing = char_spacing_override
        else:
            char_spacing = font_format_config.get('default_char_spacing', {})
        
        config = {
            'layout': layout,
            'bit_order': bit_order,
            'encoding': encoding,
            'char_spacing': char_spacing,
            'chinese_encoding': gen_config.get('chinese_encoding', 'utf-8')
        }
        
        # 获取字体期望大小
        charset_config = gen_config.get('charset', {})
        start_char = charset_config.get('ascii_start', 32)
        end_char = charset_config.get('ascii_end', 126)
        
        # 解析完整路径
        full_font_path = self._resolve_path(font_path)
        
        if not os.path.exists(full_font_path):
            print(f"[-] 字体文件不存在: {full_font_path}")
            return False, None
        
        # 自动检测字体类型（大小写不敏感）
        if font_type is None:
            if full_font_path.lower().endswith('.ttf'):
                font_type = 'ttf'
            elif full_font_path.lower().endswith('.bdf'):
                font_type = 'bdf'
            else:
                print(f"[-] 无法识别的字体类型: {full_font_path}")
                return False, None

        font_type_lower = font_type.lower()
        
        # 获取字体名称
        font_basename = os.path.splitext(os.path.basename(full_font_path))[0]
        
        print(f"\n{'='*60}")
        print(f"处理字体: {font_basename}")
        print(f"字体类型: {font_type.upper()}")
        print(f"字体路径: {full_font_path}")
        print(f"{'='*60}")
        
        # 创建C文件生成器
        generator = CFileGenerator(font_basename, config)
        
        # 使用传入的尺寸或使用旧的默认值逻辑
        if font_sizes is None:
            # 向后兼容：如果是旧的方式调用（没传sizes），试图从profiles中找
            font_sizes = [12]  # 默认尺寸

        successful_sizes = 0
        cn_successful_sizes = 0
        
        # 处理每个字体尺寸
        for font_size in font_sizes:
            print(f"\n处理字体尺寸: {font_size}px")
            
            try:
                # 创建parser
                if font_type_lower == 'ttf':
                    parser = TTFParser(full_font_path, font_size, config)
                elif font_type_lower == 'bdf':
                    parser = BDFParser(full_font_path, font_size, config)
                else:
                    print(f"[-] 不支持的字体类型: {font_type}")
                    continue
                
                print(f"  [+] 字体加载成功")
                
                # 渲染ASCII字符集
                char_data = parser.render_ascii(start_char, end_char)
                print(f"  [+] 渲染 {len(char_data)} 个字符")

                # BDF是固定大小字体，遇到无法渲染当前尺寸时仅提示并跳过
                if font_type_lower == 'bdf' and not char_data:
                    print(f"  [!] BDF字体为固定大小，尺寸 {font_size}px 无可用字形，已跳过该尺寸")
                    continue
                
                if char_data:
                    # 显示第一个字符的信息
                    first_char = char_data[0]
                    print(f"    字符尺寸: {first_char['width']}x{first_char['height']}")
                
                # 添加到生成器
                generator.add_font_size(font_size, char_data, start_char, end_char)
                successful_sizes += 1

                if cn_chars:
                    cn_char_data, missing_chars = parser.render_charset(cn_chars)
                    if cn_char_data:
                        generator.add_cn_font_size(font_size, cn_char_data, cn_chars)
                        cn_successful_sizes += 1
                        print(f"  [+] 渲染中文 {len(cn_char_data)} 个字符")
                    else:
                        print(f"  [!] 尺寸 {font_size}px 未渲染到任何中文字形")

                    if missing_chars:
                        print(f"  [!] 尺寸 {font_size}px 缺失中文字符: {''.join(missing_chars)}")
                
                # 是否显示预览
                if preview_config.get('enabled', True) and preview_config.get('print_output', True):
                    preview_text = preview_config.get('text', '')
                    if preview_text:
                        print(f"  预览文本: {preview_text}")
                        preview_chars = parser.render_string(preview_text)
                        for item in preview_chars[:3]:  # 只显示前3个字符的预览
                            print(f"    '{item['char']}': {item['width']}x{item['height']}")
                
            except Exception as e:
                if font_type_lower == 'bdf':
                    print(f"  [!] BDF字体为固定大小，无法按 {font_size}px 解析: {e}")
                    print(f"  [!] 已跳过该尺寸，继续处理后续尺寸")
                    continue

                print(f"  [-] 处理失败: {e}")
                import traceback
                traceback.print_exc()
                return False, None

        if successful_sizes == 0:
            print(f"[-] 字体 {font_basename} 没有成功生成任何尺寸，已跳过")
            return False, None

        if cn_chars and cn_successful_sizes == 0:
            print(f"[!] 字体 {font_basename} 未生成任何中文数组，请确认字体是否包含所需中文")
        
        # 生成C文件
        print(f"\n生成C文件...")
        output_dir = self._resolve_path(gen_config.get('output_c_folder', '../Csource/font'))
        
        try:
            header_path, source_path = generator.save_files(output_dir)
            print(f"  [+] 头文件: {header_path}")
            print(f"  [+] 源文件: {source_path}")
            
            # 打印摘要
            print(f"\n生成摘要:")
            print(generator.get_summary())
            
            return True, (header_path, source_path)
        
        except Exception as e:
            print(f"  [-] 生成失败: {e}")
            import traceback
            traceback.print_exc()
            return False, None
    
    def expand_font_path(self, font_path_spec):
        """
        展开字体路径规范（可以是文件或文件夹）
        
        Args:
            font_path_spec (str): 字体文件或文件夹路径
            
        Returns:
            list: 实际存在的字体文件列表
        """
        full_path = self._resolve_path(font_path_spec)
        font_files = []
        
        # 检查文件（大小写不敏感）
        if os.path.isfile(full_path) and (full_path.lower().endswith('.ttf') or full_path.lower().endswith('.bdf')):
            # 单个文件
            font_files.append(font_path_spec)
        elif os.path.isdir(full_path):
            # 文件夹 - 扫描所有ttf和bdf文件（大小写不敏感）
            for ext in ['*.ttf', '*.bdf', '*.TTF', '*.BDF']:
                for font_file in Path(full_path).glob(ext):
                    relative_path = os.path.relpath(str(font_file), self.script_dir)
                    font_files.append(relative_path)
        else:
            print(f"[!] 字体路径不存在或不可识别: {font_path_spec}")
        
        return font_files
    
    def generate_font_by_profile(self, profile_name, profile_config):
        """
        按profile生成字体组
        
        Args:
            profile_name (str): profile名称
            profile_config (dict): profile配置 {font_paths, sizes, char_spacing(可选)}
            
        Returns:
            int: 成功生成的字体包数量
        """
        print(f"\n{'='*60}")
        print(f"处理 profile: {profile_name}")
        print(f"{'='*60}")

        gen_config = self.config.get('generate', {})
        chinese_profile_name = gen_config.get('chinese_profile_name', 'chinese_fonts')
        cn_chars = []
        if profile_name == chinese_profile_name and self.scan_summary is not None:
            cn_chars = self._filter_chars_by_encoding(self.scan_summary.get('char_list', []))
            if cn_chars:
                print(f"[+] profile '{profile_name}' 将额外生成 {len(cn_chars)} 个中文字符")
            else:
                print(f"[!] profile '{profile_name}' 没有可生成的中文字符")
        
        font_paths = profile_config.get('font_paths', [])
        sizes = profile_config.get('sizes', [12])
        char_spacing_override = profile_config.get('char_spacing', None)
        
        if not font_paths:
            print(f"[-] profile '{profile_name}' 中没有字体")
            return 0
        
        if not sizes:
            print(f"[-] profile '{profile_name}' 中没有指定尺寸")
            return 0
        
        success_count = 0
        
        # 对该profile下的每个字体路径
        for font_path_spec in font_paths:
            # 展开路径（支持文件和文件夹）
            actual_fonts = self.expand_font_path(font_path_spec)
            
            if not actual_fonts:
                continue
            
            # 处理每个实际的字体文件
            for font_file in actual_fonts:
                success, files = self.generate_font(
                    font_file, 
                    font_sizes=sizes,
                    font_type=None,  # 自动检测
                    char_spacing_override=char_spacing_override,
                    cn_chars=cn_chars,
                )
                if success and files:
                    success_count += 1
        
        return success_count
    
    def process_all_profiles(self):
        """
        处理config中所有定义的profiles
        
        Returns:
            int: 总共生成成功的字体包数量
        """
        gen_config = self.config.get('generate', {})
        profiles = gen_config.get('profiles', {})
        
        if not profiles:
            print("[-] config中没有定义profiles")
            return 0
        
        total_count = 0
        
        for profile_name, profile_config in profiles.items():
            count = self.generate_font_by_profile(profile_name, profile_config)
            total_count += count
        
        return total_count
    
    def list_profiles(self):
        """
        列出所有可用的profile
        """
        gen_config = self.config.get('generate', {})
        profiles = gen_config.get('profiles', {})
        
        if not profiles:
            print("[-] 没有定义任何profile")
            return
        
        print("\n可用的 profiles:")
        print(f"{'='*60}")
        
        for profile_name, profile_config in profiles.items():
            font_paths = profile_config.get('font_paths', [])
            sizes = profile_config.get('sizes', [])
            print(f"\n  {profile_name}:")
            print(f"    尺寸: {sizes}")
            print(f"    字体文件:")
            for font_path in font_paths:
                actual_fonts = self.expand_font_path(font_path)
                if actual_fonts:
                    for font_file in actual_fonts:
                        print(f"      - {font_file}")
                else:
                    print(f"      - {font_path} (无效或未找到)")
        
        print(f"\n{'='*60}")
    
    def process_font_folder(self):
        """
        处理config中指定的字体文件夹（向后兼容）
        
        Returns:
            list: 生成的文件路表
        """
        gen_config = self.config.get('generate', {})
        font_folder = gen_config.get('font_folder', '../font')
        
        full_folder_path = self._resolve_path(font_folder)
        
        if not os.path.exists(full_folder_path):
            print(f"[-] 字体文件夹不存在: {full_folder_path}")
            return []
        
        print(f"扫描字体文件夹: {full_folder_path}")
        
        generated_files = []
        
        # 扫描所有字体文件（大小写不敏感）
        for pattern in ['*.ttf', '*.bdf', '*.TTF', '*.BDF']:
            for font_file in Path(full_folder_path).rglob(pattern):
                # 根据文件扩展名确定类型
                if str(font_file).lower().endswith('.ttf'):
                    font_type = 'ttf'
                elif str(font_file).lower().endswith('.bdf'):
                    font_type = 'bdf'
                else:
                    continue
                
                relative_path = os.path.relpath(str(font_file), self.script_dir)
                success, files = self.generate_font(relative_path, font_type=font_type)
                if success and files:
                    generated_files.append(files)
        
        return generated_files


def main():
    """主函数"""
    print("=" * 60)
    print("WouoUI 字体生成工具 - freetype-py 版本")
    print("=" * 60)
    print()
    
    # 查找config.yaml
    script_dir = os.path.dirname(__file__)
    config_path = os.path.join(script_dir, 'config.yaml')
    
    if not os.path.exists(config_path):
        print(f"[-] 无法找到config.yaml: {config_path}")
        sys.exit(1)
    
    # 创建生成器
    try:
        generator = FontGenerator(config_path)
    except Exception as e:
        print(f"[-] 初始化失败: {e}")
        sys.exit(1)
    
    # 命令行参数解析
    parser = argparse.ArgumentParser(
        description='WouoUI 字体生成工具',
        epilog='示例:\n'
               '  python main.py                    # 处理所有profile\n'
               '  python main.py --profile embedded_small  # 处理指定profile\n'
               '  python main.py --font Font/font.ttf --sizes 12,16,24  # 临时快速处理\n'
               '  python main.py --list-profiles    # 列出所有profile\n',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument('--profile', 
                        help='指定要处理的profile名称')
    parser.add_argument('--profiles', 
                        help='指定多个profile，用逗号分隔 (如: embedded_small,display_medium)')
    parser.add_argument('--font', 
                        help='指定字体文件或文件夹路径（相对于脚本目录）')
    parser.add_argument('--sizes', 
                        help='指定字体尺寸，用逗号分隔 (如: 8,12,16)')
    parser.add_argument('--list-profiles', '--list',
                        action='store_true',
                        help='列出所有可用的profile')
    
    args = parser.parse_args()
    
    # 处理 --list-profiles
    if args.list_profiles:
        generator.list_profiles()
        return
    
    # 处理临时快速方式 (--font 和 --sizes)
    if args.font and args.sizes:
        print("\n临时模式: 处理单个字体或文件夹")
        try:
            sizes = list(map(int, args.sizes.split(',')))
        except ValueError:
            print(f"[-] 无效的尺寸列表: {args.sizes}")
            sys.exit(1)
        
        # 展开字体路径（支持文件和文件夹）
        actual_fonts = generator.expand_font_path(args.font)
        if not actual_fonts:
            print(f"[-] 在指定路径中未找到字体文件: {args.font}")
            sys.exit(1)
        
        # 处理每个实际的字体文件
        success_count = 0
        for font_file in actual_fonts:
            success, files = generator.generate_font(font_file, font_sizes=sizes, font_type=None)
            if success and files:
                success_count += 1
        
        if success_count == 0:
            sys.exit(1)
        return

    generator.scan_chinese_charset()
    
    # 处理 profile 方式
    profiles_to_process = []
    
    if args.profile:
        # 单个profile
        profiles_to_process = [args.profile]
    elif args.profiles:
        # 多个profiles
        profiles_to_process = [p.strip() for p in args.profiles.split(',')]
    else:
        # 处理所有profiles
        profiles_to_process = None
    
    if profiles_to_process is not None:
        # 处理指定的profiles
        total_count = 0
        for profile_name in profiles_to_process:
            gen_config = generator.config.get('generate', {})
            profiles = gen_config.get('profiles', {})
            
            if profile_name not in profiles:
                print(f"[-] profile '{profile_name}' 不存在")
                print(f"使用 --list-profiles 查看可用的profiles")
                sys.exit(1)
            
            profile_config = profiles[profile_name]
            count = generator.generate_font_by_profile(profile_name, profile_config)
            total_count += count
        
        if total_count > 0:
            print(f"\n{'='*60}")
            print(f"[+] 字体生成完成！")
            print(f"生成了 {total_count} 个字体包")
            print(f"{'='*60}")
        else:
            print(f"\n[-] 未生成任何字体")
            sys.exit(1)
    else:
        # 处理所有profiles
        total_count = generator.process_all_profiles()
        
        if total_count > 0:
            print(f"\n{'='*60}")
            print(f"[+] 字体生成完成！")
            print(f"生成了 {total_count} 个字体包")
            print(f"{'='*60}")
        else:
            print(f"\n[-] 未生成任何字体")
            sys.exit(1)


if __name__ == "__main__":
    main()
