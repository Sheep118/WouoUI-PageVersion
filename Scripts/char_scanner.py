"""
字符扫描脚本 - 第1部分：扫描 C/H 字符串中的中文字符

能力：
1) 读取 config.yaml 的 scan 配置
2) 自动推测文件编码（BOM + chardet）
3) 从 C 语言字符串字面量中提取中文字符并统计频次
4) 支持排除规则列表（如 printf("...")）
5) 输出 UTF-8 纯文本报告
"""

import os
import re
import argparse
from collections import Counter
from datetime import datetime

import chardet
import yaml


# ============================================================
# 编码检测
# ============================================================

def detect_file_encoding(filepath, bomless_preferred_encodings=None):
    """
    检测文件编码，优先检查 BOM，其次用 chardet 推断。

    Returns:
        tuple: (encoding: str, confidence: float)
    """
    with open(filepath, 'rb') as f:
        raw = f.read()

    if bomless_preferred_encodings is None:
        bomless_preferred_encodings = ['utf-8', 'gb2312']

    # BOM 优先
    if raw.startswith(b'\xef\xbb\xbf'):
        return 'utf-8-sig', 1.0
    if raw.startswith(b'\xff\xfe'):
        return 'utf-16-le', 1.0
    if raw.startswith(b'\xfe\xff'):
        return 'utf-16-be', 1.0

    # 无 BOM 时优先按配置做严格解码尝试
    for enc in bomless_preferred_encodings:
        if not isinstance(enc, str) or not enc.strip():
            continue
        name = enc.strip()
        try:
            raw.decode(name)
            return name, 1.0
        except (LookupError, UnicodeDecodeError):
            continue

    result = chardet.detect(raw)
    encoding = result.get('encoding') or 'utf-8'
    confidence = result.get('confidence', 0.0)

    # GB2312 / GBK 升级为 GB18030，向下兼容且支持更多字符
    if encoding.upper() in ('GB2312', 'GBK', 'GB-2312', 'GBK-EUC'):
        encoding = 'gb18030'

    return encoding, confidence


def read_file_auto(filepath, bomless_preferred_encodings=None):
    """
    自动检测编码后读文件。

    Returns:
        tuple: (content: str, encoding: str, confidence: float)
    """
    encoding, confidence = detect_file_encoding(filepath, bomless_preferred_encodings)
    try:
        with open(filepath, 'r', encoding=encoding, errors='replace') as f:
            content = f.read()
        return content, encoding, confidence
    except Exception:
        with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()
        return content, 'utf-8(fallback)', 0.0


# ============================================================
# C 语言字符串解析（状态机）
# ============================================================

def _guess_call_name_before_string(code, quote_index, lookback=256):
    """猜测字符串前的函数调用名（用于排除如 printf("...")）。"""
    start = max(0, quote_index - lookback)
    prefix = code[start:quote_index]
    tail = prefix.rstrip()

    # 典型场景：func("...")，此时 tail 末尾应为 "func("
    m = re.search(r'([A-Za-z_][A-Za-z0-9_]*)\s*\($', tail)
    return m.group(1) if m else None


def extract_c_string_literals_with_context(code):
    """
    解析 C 代码，提取字符串字面量并附带上下文。

    Returns:
        list[dict]: [{
            'value': str,
            'line': int,
            'start': int,
            'prefix': str,
            'call_name': str|None,
        }]
    """
    literals = []

    i = 0
    n = len(code)
    line = 1

    in_line_comment = False
    in_block_comment = False
    in_string = False
    in_char = False

    escape_next = False
    string_start = -1
    string_line = 1
    buf = []

    while i < n:
        ch = code[i]
        nxt = code[i + 1] if i + 1 < n else ''

        if ch == '\n':
            line += 1

        if in_line_comment:
            if ch == '\n':
                in_line_comment = False
            i += 1
            continue

        if in_block_comment:
            if ch == '*' and nxt == '/':
                in_block_comment = False
                i += 2
            else:
                i += 1
            continue

        if in_char:
            if escape_next:
                escape_next = False
            elif ch == '\\':
                escape_next = True
            elif ch == "'":
                in_char = False
            i += 1
            continue

        if in_string:
            if escape_next:
                buf.append(ch)
                escape_next = False
            elif ch == '\\':
                buf.append(ch)
                escape_next = True
            elif ch == '"':
                start = string_start
                prefix_start = max(0, start - 300)
                prefix = code[prefix_start:start]
                literals.append({
                    'value': ''.join(buf),
                    'line': string_line,
                    'start': start,
                    'prefix': prefix,
                    'call_name': _guess_call_name_before_string(code, start),
                })
                in_string = False
                buf = []
            else:
                buf.append(ch)
            i += 1
            continue

        # NORMAL 状态
        if ch == '/' and nxt == '/':
            in_line_comment = True
            i += 2
            continue
        if ch == '/' and nxt == '*':
            in_block_comment = True
            i += 2
            continue
        if ch == "'":
            in_char = True
            escape_next = False
            i += 1
            continue
        if ch == '"':
            in_string = True
            escape_next = False
            string_start = i
            string_line = line
            buf = []
            i += 1
            continue

        i += 1

    return literals


def _compile_exclude_context_patterns(patterns):
    compiled = []
    for p in patterns:
        if not isinstance(p, str):
            continue
        if not p.strip():
            continue
        try:
            compiled.append(re.compile(p, flags=re.MULTILINE))
        except re.error as ex:
            print(f"[!] 无效 exclude_context_patterns 正则，已跳过: {p} ({ex})")
    return compiled


def should_exclude_literal(literal, exclude_functions, exclude_context_patterns):
    """判断单个字符串是否应排除。"""
    call_name = (literal.get('call_name') or '').lower()
    if call_name and call_name in exclude_functions:
        return True

    prefix = literal.get('prefix', '')
    tail = prefix[-300:]
    for pattern in exclude_context_patterns:
        if pattern.search(tail):
            return True

    return False


def extract_chinese_chars(text):
    """
    从文本中提取中文字符（CJK 统一汉字基本区 + 扩展A + 兼容汉字）。

    Returns:
        list[str]
    """
    pattern = re.compile(
        r'[\u4e00-\u9fff'   # CJK 统一汉字（基本区，约 20000 字）
        r'\u3400-\u4dbf'    # CJK 扩展 A
        r'\uf900-\ufaff'    # CJK 兼容汉字
        r']'
    )
    return pattern.findall(text)


# ============================================================
# 文件 / 目录扫描
# ============================================================

def scan_file(filepath, exclude_functions=None, exclude_context_patterns=None, bomless_preferred_encodings=None):
    """
    扫描单个 C/H 文件，返回其中字符串字面量里的中文字符。

    Returns:
        dict: {
            'filepath': str,
            'encoding': str,
            'confidence': float,
            'string_count': int,      # 找到的字符串字面量数量
            'chinese_chars': list[str]
        }
    """
    if exclude_functions is None:
        exclude_functions = set()
    if exclude_context_patterns is None:
        exclude_context_patterns = []

    content, encoding, confidence = read_file_auto(filepath, bomless_preferred_encodings)

    literals = extract_c_string_literals_with_context(content)
    strings = []
    excluded_count = 0
    for literal in literals:
        if should_exclude_literal(literal, exclude_functions, exclude_context_patterns):
            excluded_count += 1
            continue
        strings.append(literal['value'])

    chinese_chars = []
    for s in strings:
        chinese_chars.extend(extract_chinese_chars(s))

    return {
        'filepath': filepath,
        'encoding': encoding,
        'confidence': confidence,
        'literal_count': len(literals),
        'excluded_literal_count': excluded_count,
        'string_count': len(strings),
        'chinese_chars': chinese_chars,
    }


def scan_directory(
    dir_path,
    include_exts,
    exclude_abs_paths,
    exclude_functions,
    exclude_context_patterns,
    bomless_preferred_encodings,
):
    """
    递归扫描目录，对每个符合条件的文件调用 scan_file。

    Args:
        dir_path (str): 要扫描的目录绝对路径
        include_exts (set[str]): 允许的扩展名，如 {'.c', '.h'}
        exclude_abs_paths (set[str]): 要排除的绝对路径（文件或目录）
        exclude_functions (list[str]): 传递给 scan_file

    Returns:
        list[dict]
    """
    results = []

    for root, dirs, files in os.walk(dir_path):
        # 原地过滤掉要排除的子目录，阻止 os.walk 递归进入
        dirs[:] = [
            d for d in dirs
            if os.path.abspath(os.path.join(root, d)) not in exclude_abs_paths
        ]

        for filename in files:
            ext = os.path.splitext(filename)[1].lower()
            if ext not in include_exts:
                continue
            filepath = os.path.join(root, filename)
            if os.path.abspath(filepath) in exclude_abs_paths:
                continue
            result = scan_file(
                filepath,
                exclude_functions,
                exclude_context_patterns,
                bomless_preferred_encodings,
            )
            results.append(result)

    return results


# ============================================================
# 主扫描器类
# ============================================================

class CharScanner:
    """字符扫描器：读取 config.yaml，扫描文件，生成报告。"""

    def __init__(self, config_path):
        self.config_path = config_path
        self.script_dir = os.path.dirname(os.path.abspath(config_path))
        self.config = self._load_config()

    def _load_config(self):
        with open(self.config_path, 'r', encoding='utf-8') as f:
            return yaml.safe_load(f)

    def _resolve(self, relative_path):
        return os.path.abspath(os.path.join(self.script_dir, relative_path))

    def scan(self):
        """
        执行扫描，返回每个文件的扫描结果列表。

        Returns:
            list[dict]
        """
        scan_cfg = self.config.get('scan', {})

        scan_paths       = scan_cfg.get('scan_paths', [])
        include_exts      = {e.lower() for e in scan_cfg.get('include_exts', ['.c', '.h'])}
        exclude_rels      = scan_cfg.get('exclude_paths', [])
        exclude_funcs     = {str(f).lower() for f in scan_cfg.get('exclude_functions', [])}
        context_patterns  = scan_cfg.get('exclude_context_patterns', [])
        preferred_encs    = scan_cfg.get('bomless_preferred_encodings', ['utf-8', 'gb2312'])

        # 兼容旧配置别名
        if not context_patterns:
            context_patterns = scan_cfg.get('exclude_string_patterns', [])

        compiled_patterns = _compile_exclude_context_patterns(context_patterns)

        exclude_abs = {self._resolve(p) for p in exclude_rels}

        all_results = []

        for path_spec in scan_paths:
            full_path = self._resolve(path_spec)

            if os.path.isfile(full_path):
                ext = os.path.splitext(full_path)[1].lower()
                if ext in include_exts and full_path not in exclude_abs:
                    all_results.append(
                        scan_file(full_path, exclude_funcs, compiled_patterns, preferred_encs)
                    )
                else:
                    print(f"[!] 跳过: {full_path}")

            elif os.path.isdir(full_path):
                dir_results = scan_directory(
                    full_path,
                    include_exts,
                    exclude_abs,
                    exclude_funcs,
                    compiled_patterns,
                    preferred_encs,
                )
                all_results.extend(dir_results)
                print(f"[+] 目录 {full_path} 扫描完毕，共 {len(dir_results)} 个文件")

            else:
                print(f"[!] 路径不存在: {full_path}")

        return all_results

    def save_report(self, results, output_path=None):
        """
        将扫描结果保存为 UTF-8 纯文本报告。

        Returns:
            str: 报告文件的绝对路径
        """
        scan_cfg = self.config.get('scan', {})

        if output_path is None:
            output_path = self._resolve(scan_cfg.get('output_report', './scan_result.txt'))

        total_counter = Counter()
        for r in results:
            total_counter.update(r['chinese_chars'])

        now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        lines = []

        lines.append('WouoUI 中文字符扫描报告')
        lines.append(f'生成时间: {now}')
        lines.append('=' * 60)
        lines.append(f'扫描文件数      : {len(results)}')
        lines.append(f'中文字符种类    : {len(total_counter)}')
        lines.append(f'中文字符出现总次: {sum(total_counter.values())}')
        lines.append('=' * 60)
        lines.append('')

        # ---- 各文件详情 ----
        lines.append('【各文件扫描详情】')
        lines.append('')
        for r in results:
            file_counter = Counter(r['chinese_chars'])
            rel = os.path.relpath(r['filepath'], self.script_dir)
            lines.append(f"  文件    : {rel}")
            lines.append(f"  编码    : {r['encoding']}  (置信度 {r['confidence']:.0%})")
            lines.append(f"  字符串数: {r['string_count']} (原始 {r['literal_count']}，排除 {r['excluded_literal_count']})")
            lines.append(f"  中文种类: {len(file_counter)}")
            if file_counter:
                char_preview = ''.join(ch for ch, _ in file_counter.most_common())
                lines.append(f"  字符集  : {char_preview}")
            lines.append('')

        # ---- 频次表 ----
        lines.append('=' * 60)
        lines.append('【全部中文字符（按频次排序）】')
        lines.append('')
        lines.append(f"{'字符':<4}{'频次':>6}")
        lines.append('-' * 12)
        for ch, cnt in total_counter.most_common():
            lines.append(f"{ch:<4}{cnt:>6}")

        # ---- 字符集合（可直接复制用于字体生成脚本）----
        lines.append('')
        lines.append('=' * 60)
        lines.append('【所有中文字符集合（可直接用于字体生成）】')
        lines.append('')
        lines.append(''.join(ch for ch, _ in total_counter.most_common()))
        lines.append('')

        out_dir = os.path.dirname(os.path.abspath(output_path))
        if out_dir:
            os.makedirs(out_dir, exist_ok=True)

        with open(output_path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))

        return output_path

    def run(self):
        """
        完整流程：扫描 -> 打印摘要 -> 保存报告。

        Returns:
            tuple: (results: list[dict], total_counter: Counter)
        """
        print('开始扫描中文字符...')
        results = self.scan()

        if not results:
            print('[-] 未扫描到任何文件')
            return [], Counter()

        total_counter = Counter()
        for r in results:
            total_counter.update(r['chinese_chars'])

        print(f'[+] 共扫描 {len(results)} 个文件')
        print(f'[+] 发现 {len(total_counter)} 种中文字符，共出现 {sum(total_counter.values())} 次')

        output_path = self.save_report(results)
        print(f'[+] 报告已保存: {output_path}')

        return results, total_counter


def self_test():
    """内置快速自测：验证字符串提取与排除逻辑。"""
    sample = r'''
    // 注释里中文不应命中：注释中文 "注释中的字符串"
    /* 这个是一个多行注释 "多行注释中的字符串测试" */
    const char* a = "你好世界Sheep";
    const char* b = "ASCII only";
    printf("打印中文应该被排除");
    LOG_I("日志中文可选排除");
    const char* c = "中文对吧";
    const char* d = "含\\\"引号";  // 字符串里包含 \" 转义
    char ch = '\'';
    /* block comment "注释中的字符串" */
    '''

    literals = extract_c_string_literals_with_context(sample)
    exclude_functions = {'printf'}
    exclude_patterns = _compile_exclude_context_patterns([r'LOG_I\s*\($'])

    kept = []
    for literal in literals:
        if not should_exclude_literal(literal, exclude_functions, exclude_patterns):
            kept.append(literal['value'])

    chars = []
    for item in kept:
        chars.extend(extract_chinese_chars(item))

    counter = Counter(chars)
    print('[self-test] literals=', len(literals))
    print('[self-test] kept=', len(kept))
    print('[self-test] chinese_kinds=', len(counter), 'total=', sum(counter.values()))
    print('[self-test] chars=', ''.join(ch for ch, _ in counter.most_common()))

    # 预期：排除 printf/LOG_I 后，仅来自 "你好世界"、"中文"、"含\\\"引号"
    assert '你' in counter and '好' in counter and '世' in counter and '界' in counter
    assert '中' in counter and '文' in counter
    assert '含' in counter and '引' in counter and '号' in counter
    print('[self-test] PASS')


# ============================================================
# 独立运行入口
# ============================================================

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='WouoUI 中文字符串扫描器（第1部分）')
    parser.add_argument('--config', default=None, help='config.yaml 路径，默认使用脚本目录下 config.yaml')
    parser.add_argument('--self-test', action='store_true', help='运行内置自测')
    args = parser.parse_args()

    if args.self_test:
        self_test()
        raise SystemExit(0)

    _script_dir = os.path.dirname(os.path.abspath(__file__))
    _config_path = args.config or os.path.join(_script_dir, 'config.yaml')

    if not os.path.exists(_config_path):
        print(f'[-] 找不到配置文件: {_config_path}')
        raise SystemExit(1)

    scanner = CharScanner(_config_path)
    scanner.run()
