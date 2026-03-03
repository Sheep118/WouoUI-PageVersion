"""
字体生成主程序 - 使用freetype-py
Main Font Generator - Using freetype-py

读取config配置，调用bdf/ttf parser生成字体，最后生成C文件和头文件
"""

import os
import sys
import yaml
from pathlib import Path

from ttf_parser import TTFParser
from bdf_parser import BDFParser
from cfile_generater import CFileGenerator


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
    
    def generate_font(self, font_path, font_type='ttf'):
        """
        生成一个字体的多个尺寸
        
        Args:
            font_path (str): 字体文件路径（相对于脚本所在目录）
            font_type (str): 字体类型 ('ttf' 或 'bdf')
            
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
        char_spacing = font_format_config.get('char_spacing', {})
        
        config = {
            'layout': layout,
            'bit_order': bit_order,
            'encoding': encoding,
            'char_spacing': char_spacing
        }
        
        # 获取字体期望大小
        charset_config = gen_config.get('charset', {})
        start_char = charset_config.get('ascii_start', 32)
        end_char = charset_config.get('ascii_end', 126)
        
        # 解析完整路径
        full_font_path = self._resolve_path(font_path)
        
        if not os.path.exists(full_font_path):
            print(f"✗ 字体文件不存在: {full_font_path}")
            return False, None
        
        # 获取字体名称
        font_basename = os.path.splitext(os.path.basename(full_font_path))[0]
        
        print(f"\n{'='*60}")
        print(f"处理字体: {font_basename}")
        print(f"字体类型: {font_type.upper()}")
        print(f"字体路径: {full_font_path}")
        print(f"{'='*60}")
        
        # 创建C文件生成器
        generator = CFileGenerator(font_basename, config)
        
        # 获取字体尺寸列表
        font_sizes = gen_config.get('font_sizes', {}).get('default', [12, 16, 24])
        
        # 处理每个字体尺寸
        for font_size in font_sizes:
            print(f"\n处理字体尺寸: {font_size}px")
            
            try:
                # 创建parser
                if font_type.lower() == 'ttf':
                    parser = TTFParser(full_font_path, font_size, config)
                elif font_type.lower() == 'bdf':
                    parser = BDFParser(full_font_path, font_size, config)
                else:
                    print(f"✗ 不支持的字体类型: {font_type}")
                    continue
                
                print(f"  ✓ 字体加载成功")
                
                # 渲染ASCII字符集
                char_data = parser.render_ascii(start_char, end_char)
                print(f"  ✓ 渲染 {len(char_data)} 个字符")
                
                if char_data:
                    # 显示第一个字符的信息
                    first_char = char_data[0]
                    print(f"    字符尺寸: {first_char['width']}x{first_char['height']}")
                
                # 添加到生成器
                generator.add_font_size(font_size, char_data, start_char, end_char)
                
                # 是否显示预览
                if preview_config.get('enabled', True) and preview_config.get('print_output', True):
                    preview_text = preview_config.get('text', '')
                    if preview_text:
                        print(f"  预览文本: {preview_text}")
                        preview_chars = parser.render_string(preview_text)
                        for item in preview_chars[:3]:  # 只显示前3个字符的预览
                            print(f"    '{item['char']}': {item['width']}x{item['height']}")
                
            except Exception as e:
                print(f"  ✗ 处理失败: {e}")
                import traceback
                traceback.print_exc()
                return False, None
        
        # 生成C文件
        print(f"\n生成C文件...")
        output_dir = self._resolve_path(gen_config.get('output_c_folder', '../Csource/font'))
        
        try:
            header_path, source_path = generator.save_files(output_dir)
            print(f"  ✓ 头文件: {header_path}")
            print(f"  ✓ 源文件: {source_path}")
            
            # 打印摘要
            print(f"\n生成摘要:")
            print(generator.get_summary())
            
            return True, (header_path, source_path)
        
        except Exception as e:
            print(f"  ✗ 生成失败: {e}")
            import traceback
            traceback.print_exc()
            return False, None
    
    def process_font_folder(self):
        """
        处理config中指定的字体文件夹
        
        Returns:
            list: 生成的文件路表
        """
        gen_config = self.config.get('generate', {})
        font_folder = gen_config.get('font_folder', '../font')
        
        full_folder_path = self._resolve_path(font_folder)
        
        if not os.path.exists(full_folder_path):
            print(f"✗ 字体文件夹不存在: {full_folder_path}")
            return []
        
        print(f"扫描字体文件夹: {full_folder_path}")
        
        generated_files = []
        
        # 扫描TTF文件
        for ttf_file in Path(full_folder_path).rglob('*.ttf'):
            relative_path = os.path.relpath(ttf_file, self.script_dir)
            success, files = self.generate_font(relative_path, 'ttf')
            if success and files:
                generated_files.append(files)
        
        # 扫描BDF文件
        for bdf_file in Path(full_folder_path).rglob('*.bdf'):
            relative_path = os.path.relpath(bdf_file, self.script_dir)
            success, files = self.generate_font(relative_path, 'bdf')
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
        print(f"✗ 无法找到config.yaml: {config_path}")
        sys.exit(1)
    
    # 创建生成器
    try:
        generator = FontGenerator(config_path)
    except Exception as e:
        print(f"✗ 初始化失败: {e}")
        sys.exit(1)
    
    # 如果提供了命令行参数，则处理指定的字体
    if len(sys.argv) > 1:
        font_path = sys.argv[1]
        font_type = sys.argv[2] if len(sys.argv) > 2 else 'ttf'
        success, files = generator.generate_font(font_path, font_type)
        if not success:
            sys.exit(1)
    else:
        # 否则处理配置中的字体文件夹
        generated_files = generator.process_font_folder()
        
        if generated_files:
            print(f"\n{'='*60}")
            print(f"✓ 字体生成完成！")
            print(f"生成了 {len(generated_files)} 个字体包")
            print(f"{'='*60}")
        else:
            print(f"\n✗ 未生成任何字体")
            sys.exit(1)


if __name__ == "__main__":
    main()
