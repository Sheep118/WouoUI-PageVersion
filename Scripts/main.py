"""
WouoUI 字体生成主脚本
Main Script - Font Generation Tool

功能:
1. 扫描字体文件夹中的所有TTF字体
2. 让用户选择字体
3. 让用户输入字体宽高
4. 调用字体转换脚本生成C文件
5. 生成对应的头文件
"""

import os
import sys
import yaml
from pathlib import Path
from font_converter import FontConverter


class FontGeneratorTool:
    """字体生成工具主类"""
    
    def __init__(self, config_path="config.yaml"):
        """
        初始化工具
        
        Args:
            config_path (str): 配置文件路径
        """
        self.script_dir = os.path.dirname(os.path.abspath(__file__))
        self.config_path = os.path.join(self.script_dir, config_path)
        self.config = self.load_config()

        self.scan_config = self.config.get('scan', {})
        self.generate_config = self.config.get('generate', {})
        self.preview_config = self.config.get('preview', {})
        
        # 根据相对路径计算实际路径
        self.font_folder = os.path.join(self.script_dir, self.generate_config.get('font_folder', '../font'))
        self.output_folder = os.path.join(
            self.script_dir,
            self.generate_config.get('output_c_folder', '../Csource/font')
        )
        
        # 转换为绝对路径
        self.font_folder = os.path.abspath(self.font_folder)
        self.output_folder = os.path.abspath(self.output_folder)
    
    def load_config(self):
        """
        加载配置文件
        
        Returns:
            dict: 配置字典
        """
        if not os.path.exists(self.config_path):
            print(f"错误: 配置文件不存在 {self.config_path}")
            return {}
        
        try:
            with open(self.config_path, 'r', encoding='utf-8') as f:
                config = yaml.safe_load(f)
            return config if config else {}
        except Exception as e:
            print(f"错误: 无法读取配置文件\n{e}")
            return {}
    
    def scan_fonts(self):
        """
        扫描字体文件夹，获取所有TTF文件
        
        Returns:
            list: [(索引, 文件名, 完整路径), ...]
        """
        if not os.path.exists(self.font_folder):
            print(f"错误: 字体文件夹不存在 {self.font_folder}")
            return []
        
        fonts = []
        ttf_files = list(Path(self.font_folder).glob("*.ttf"))
        
        for idx, ttf_file in enumerate(sorted(ttf_files), 1):
            fonts.append((idx, ttf_file.name, str(ttf_file)))
        
        return fonts
    
    def display_fonts(self, fonts):
        """
        显示可用的字体列表
        
        Args:
            fonts (list): 字体列表
        """
        if not fonts:
            print("当前字体文件夹中没有找到TTF字体文件")
            return
        
        print("\n" + "="*60)
        print("  可用的字体文件")
        print("="*60)
        
        for idx, name, path in fonts:
            print(f"  [{idx}] {name}")
        
        print("="*60 + "\n")
    
    def select_font(self, fonts):
        """
        让用户选择字体
        
        Args:
            fonts (list): 字体列表
            
        Returns:
            tuple: (文件名, 完整路径) 或 None
        """
        if not fonts:
            return None
        
        while True:
            try:
                choice = input("请选择字体 (输入序号): ").strip()
                
                if not choice.isdigit():
                    print("错误: 请输入数字")
                    continue
                
                idx = int(choice)
                
                # 查找对应的字体
                for font_idx, name, path in fonts:
                    if font_idx == idx:
                        return name, path
                
                print(f"错误: 无效的选择 {idx}")
            
            except KeyboardInterrupt:
                print("\n已取消操作")
                return None
            except Exception as e:
                print(f"错误: {e}")
    
    def get_font_size(self):
        """
        获取用户输入的字体宽高
        
        Returns:
            tuple: (width, height) 或 None
        """
        while True:
            try:
                width_input = input("请输入字体宽度 (像素, 例如 8, 12): ").strip()
                height_input = input("请输入字体高度 (像素, 例如 16, 24): ").strip()
                
                if not width_input.isdigit() or not height_input.isdigit():
                    print("错误: 宽高都必须是数字")
                    continue
                
                width = int(width_input)
                height = int(height_input)
                
                if width < 4 or width > 256 or height < 4 or height > 256:
                    print("错误: 字体宽高应在 4-256 像素之间")
                    continue
                
                return width, height
            
            except KeyboardInterrupt:
                print("\n已取消操作")
                return None
            except Exception as e:
                print(f"错误: {e}")
    
    def _sanitize_name(self, name):
        """
        清理不符合C命名规范的字符
        
        Args:
            name (str): 原始名称
            
        Returns:
            str: 清理后的名称
        """
        # 将非字母数字和下划线的字符替换为下划线
        result = ""
        for char in name:
            if char.isalnum() or char == '_':
                result += char
            else:
                result += "_"
        
        # 确保不以数字开头（C语言规范）
        if result and result[0].isdigit():
            result = "_" + result
        
        return result
    
    def generate_font(self, ttf_path, font_width, font_height):
        """
        生成字体的C文件
        
        Args:
            ttf_path (str): TTF文件路径
            font_width (int): 字体宽度
            font_height (int): 字体高度
            
        Returns:
            bool: 是否成功
        """
        try:
            # 获取字体配置
            font_config = self.generate_config.get('font_format', {})
            
            # 创建转换器
            converter = FontConverter(ttf_path, font_height, font_width, font_height, font_config)
            
            # 生成字体名称
            font_base_name = os.path.splitext(os.path.basename(ttf_path))[0]
            font_base_name = self._sanitize_name(font_base_name)
            font_name = f"Font_{converter.char_width}_{converter.char_height}"
            font_name = self._sanitize_name(font_name)
            
            print(f"\n转换字体...")
            print(f"  字体文件: {os.path.basename(ttf_path)}")
            print(f"  字体大小: {font_width}x{font_height}px")
            print(f"  实际尺寸: {converter.char_width}x{converter.char_height}")
            print(f"  字体名称: {font_name}")
            
            # 保存文件
            print(f"\n保存C文件...")
            header_path, source_path = converter.save_font(
                self.output_folder,
                font_name
            )
            
            print(f"✓ 成功生成字体文件!")
            print(f"  Header: {header_path}")
            print(f"  Source: {source_path}")
            
            return True
        
        except Exception as e:
            print(f"\n✗ 错误: {e}")
            return False

    def preview_font(self, ttf_path, font_width, font_height):
        """
        预览字体效果

        Args:
            ttf_path (str): TTF文件路径
            font_width (int): 字体宽度
            font_height (int): 字体高度
        """
        preview_text = self.preview_config.get('text', '')
        if not preview_text:
            return

        font_config = self.generate_config.get('font_format', {})
        converter = FontConverter(ttf_path, font_height, font_width, font_height, font_config)
        preview_img = converter.render_preview_text(preview_text)

        try:
            preview_img.show(title="WouoUI Font Preview")
        except Exception as e:
            print(f"预览显示失败: {e}")
    
    def run(self):
        """
        运行主程序
        """
        print("\n" + "="*60)
        print("  WouoUI 字体生成工具")
        print("="*60)
        
        # 扫描字体
        print("\n扫描字体文件夹...")
        fonts = self.scan_fonts()
        
        if not fonts:
            print("没有找到可用的字体")
            return
        
        # 显示字体列表
        self.display_fonts(fonts)
        
        # 选择字体
        result = self.select_font(fonts)
        if not result:
            return
        font_name, ttf_path = result
        
        # 获取字体大小
        size = self.get_font_size()
        if size is None:
            return
        font_width, font_height = size

        # 预览字体
        self.preview_font(ttf_path, font_width, font_height)
        
        # 生成字体
        self.generate_font(ttf_path, font_width, font_height)
        
        print("\n" + "="*60)
        print("  字体生成完成!")
        print("="*60 + "\n")


def main():
    """主函数"""
    try:
        tool = FontGeneratorTool()
        tool.run()
    except Exception as e:
        print(f"发生错误: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
