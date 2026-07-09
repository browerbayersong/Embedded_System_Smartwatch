#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
import subprocess

MD_FILE = r"d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\说明.md"
PDF_FILE = r"d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\说明.pdf"
HTML_FILE = r"d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\说明.html"


def install_packages():
    packages = ['markdown']
    for pkg in packages:
        try:
            __import__(pkg)
            print(f"✓ {pkg} 已安装")
        except ImportError:
            print(f"正在安装 {pkg}...")
            subprocess.run([sys.executable, '-m', 'pip', 'install', pkg],
                          capture_output=True, text=True)


def generate_html(md_path, html_path):
    import markdown as md_lib

    with open(md_path, 'r', encoding='utf-8') as f:
        md_content = f.read()

    html_content = md_lib.markdown(
        md_content,
        extensions=['tables', 'fenced_code', 'sane_lists', 'toc', 'nl2br']
    )

    html_template = """<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<title>嵌入式项目硬件配置说明</title>
<style>
  @page {{ size: A4; margin: 20mm; }}
  body {{
    font-family: "Microsoft YaHei", "PingFang SC", "Hiragino Sans GB", "SimHei", "SimSun", sans-serif;
    font-size: 14px;
    line-height: 1.8;
    color: #333;
    max-width: 800px;
    margin: 0 auto;
    padding: 20px;
  }}
  h1 {{ font-size: 28px; color: #1a5490; border-bottom: 3px solid #3498db; padding-bottom: 12px; margin-top: 40px; }}
  h2 {{ font-size: 22px; color: #2058a5; margin-top: 35px; border-bottom: 2px solid #e0e0e0; padding-bottom: 8px; }}
  h3 {{ font-size: 18px; color: #2c3e50; margin-top: 25px; }}
  p {{ margin: 12px 0; text-align: justify; }}
  ul, ol {{ margin: 12px 0; padding-left: 28px; }}
  li {{ margin: 6px 0; }}
  table {{
    border-collapse: collapse;
    width: 100%;
    margin: 18px 0;
    font-size: 13px;
    page-break-inside: avoid;
  }}
  th, td {{
    border: 1px solid #bbb;
    padding: 10px 14px;
    text-align: left;
    vertical-align: top;
  }}
  th {{
    background: linear-gradient(to bottom, #4a90d9, #357abd);
    color: white;
    font-weight: bold;
  }}
  tr:nth-child(even) td {{ background-color: #f7fafc; }}
  tr:hover td {{ background-color: #e8f4ff; }}
  code {{
    font-family: "Consolas", "Courier New", monospace;
    background-color: #fff3cd;
    padding: 2px 8px;
    border-radius: 4px;
    color: #856404;
    font-size: 13px;
  }}
  pre {{
    background-color: #2d2d2d;
    color: #f8f8f2;
    padding: 16px;
    border-radius: 6px;
    overflow-x: auto;
    font-size: 12px;
    page-break-inside: avoid;
  }}
  pre code {{
    background-color: transparent;
    color: #f8f8f2;
    padding: 0;
  }}
  blockquote {{
    border-left: 5px solid #3498db;
    background: #f0f8ff;
    padding: 12px 20px;
    margin: 16px 0;
    border-radius: 0 6px 6px 0;
    color: #2c3e50;
  }}
  hr {{
    border: none;
    border-top: 2px dashed #3498db;
    margin: 30px 0;
  }}
  strong {{ color: #1a5490; font-weight: bold; }}
  em {{ color: #7f8c8d; font-style: italic; }}
  ul li::marker {{ color: #3498db; }}
  ol li::marker {{ color: #3498db; font-weight: bold; }}
  @media print {{
    body {{ padding: 0; max-width: none; }}
    h1 {{ page-break-before: always; }}
    h1:first-child {{ page-break-before: auto; }}
  }}
</style>
</head>
<body>
{}
</body>
</html>
"""

    full_html = html_template.format(html_content)

    with open(html_path, 'w', encoding='utf-8') as f:
        f.write(full_html)

    print(f"✓ HTML 已生成: {html_path}")
    return html_path


def html_to_pdf_weasyprint(html_path, pdf_path):
    try:
        from weasyprint import HTML
        print("正在使用 WeasyPrint 生成 PDF...")
        HTML(filename=html_path).write_pdf(pdf_path)
        print(f"✓ PDF 已生成: {pdf_path}")
        return True
    except ImportError:
        print("正在安装 weasyprint...")
        subprocess.run([sys.executable, '-m', 'pip', 'install', 'weasyprint'],
                      capture_output=True, text=True)
        try:
            from weasyprint import HTML
            HTML(filename=html_path).write_pdf(pdf_path)
            print(f"✓ PDF 已生成: {pdf_path}")
            return True
        except Exception as e:
            print(f"WeasyPrint 失败: {e}")
            return False
    except Exception as e:
        print(f"WeasyPrint 失败: {e}")
        return False


def html_to_pdf_via_browser(html_path, pdf_path):
    """使用 Windows 的打印功能，通过浏览器生成 PDF"""
    try:
        # 尝试使用 win32com
        print("正在尝试通过 Windows 打印功能生成 PDF...")

        # 方案: 尝试使用 Edge/Chrome 的命令行打印
        browsers = [
            r"C:\Program Files\Google\Chrome\Application\chrome.exe",
            r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
            r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
        ]

        for browser_path in browsers:
            if os.path.exists(browser_path):
                print(f"  使用浏览器: {browser_path}")
                cmd = [
                    browser_path,
                    '--headless',
                    '--disable-gpu',
                    '--no-margins',
                    f'--print-to-pdf={pdf_path}',
                    html_path
                ]
                result = subprocess.run(cmd, capture_output=True, timeout=30)
                if result.returncode == 0 and os.path.exists(pdf_path):
                    print(f"✓ PDF 已生成: {pdf_path}")
                    return True

        print("  未找到可用浏览器进行自动打印")
        return False
    except Exception as e:
        print(f"  浏览器打印失败: {e}")
        return False


def main():
    print("=" * 60)
    print("Markdown → PDF 转换工具")
    print("=" * 60)

    if not os.path.exists(MD_FILE):
        print(f"✗ 找不到源文件: {MD_FILE}")
        sys.exit(1)

    print(f"\n源文件: {MD_FILE}")
    print(f"输出文件: {PDF_FILE}")
    print("-" * 60)

    # Step 1: 安装依赖
    print("\n[1/3] 检查依赖...")
    install_packages()

    # Step 2: 生成 HTML
    print("\n[2/3] 生成 HTML...")
    html_path = generate_html(MD_FILE, HTML_FILE)

    # Step 3: HTML → PDF
    print("\n[3/3] 转换为 PDF...")

    # 尝试方案 1: WeasyPrint
    if html_to_pdf_weasyprint(html_path, PDF_FILE):
        print("\n" + "=" * 60)
        print("转换完成！")
        print("=" * 60)
        return

    # 尝试方案 2: 浏览器打印
    if html_to_pdf_via_browser(html_path, PDF_FILE):
        print("\n" + "=" * 60)
        print("转换完成！")
        print("=" * 60)
        return

    # 方案 3: 手动说明
    print("\n" + "=" * 60)
    print("自动 PDF 生成失败，但已生成 HTML 文件！")
    print("=" * 60)
    print(f"\n请按以下步骤操作：")
    print(f"  1. 在文件管理器中双击打开: {HTML_FILE}")
    print(f"  2. 使用浏览器 (Chrome/Edge) 打开")
    print(f"  3. 按 Ctrl+P 打开打印对话框")
    print(f"  4. 选择 '另存为 PDF'")
    print(f"  5. 保存到: {PDF_FILE}")
    print("\n提示: HTML 文件已包含完整的样式和中文字体设置。")
    print("=" * 60)


if __name__ == '__main__':
    main()