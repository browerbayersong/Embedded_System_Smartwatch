#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
将 Markdown 文件转换为 PDF
"""

import sys
import os

try:
    import markdown
except ImportError:
    print("正在安装 markdown 库...")
    os.system("pip install markdown")
    import markdown

try:
    import pdfkit
except ImportError:
    print("正在安装 pdfkit 库...")
    os.system("pip install pdfkit")
    import pdfkit


def convert_md_to_pdf(md_path, pdf_path):
    # 读取 Markdown 文件 (UTF-8 编码)
    with open(md_path, 'r', encoding='utf-8') as f:
        md_content = f.read()

    # 转换 Markdown 为 HTML
    html_content = markdown.markdown(
        md_content,
        extensions=['tables', 'fenced_code', 'sane_lists', 'toc']
    )

    # 添加 CSS 样式 (支持中文)
    css_style = """
    <!DOCTYPE html>
    <html lang="zh-CN">
    <head>
        <meta charset="UTF-8">
        <title>嵌入式项目硬件配置说明</title>
        <style>
            @page {
                size: A4;
                margin: 2cm;
            }
            body {
                font-family: "Microsoft YaHei", "SimHei", "SimSun", "Segoe UI", Arial, sans-serif;
                font-size: 14px;
                line-height: 1.6;
                color: #333;
            }
            h1 {
                font-size: 24px;
                color: #2c3e50;
                border-bottom: 2px solid #3498db;
                padding-bottom: 10px;
                margin-top: 30px;
            }
            h2 {
                font-size: 20px;
                color: #2c3e50;
                margin-top: 25px;
            }
            h3 {
                font-size: 16px;
                color: #34495e;
                margin-top: 20px;
            }
            p {
                margin: 10px 0;
            }
            ul, ol {
                margin: 10px 0;
                padding-left: 25px;
            }
            li {
                margin: 5px 0;
            }
            table {
                border-collapse: collapse;
                width: 100%;
                margin: 15px 0;
                font-size: 13px;
            }
            th, td {
                border: 1px solid #ddd;
                padding: 8px 12px;
                text-align: left;
            }
            th {
                background-color: #f8f9fa;
                font-weight: bold;
                color: #2c3e50;
            }
            tr:nth-child(even) {
                background-color: #fafafa;
            }
            tr:hover {
                background-color: #f0f8ff;
            }
            code {
                font-family: "Consolas", "Courier New", monospace;
                background-color: #f4f4f4;
                padding: 2px 6px;
                border-radius: 3px;
                font-size: 13px;
                color: #e74c3c;
            }
            pre {
                background-color: #f4f4f4;
                border: 1px solid #ddd;
                padding: 10px;
                border-radius: 5px;
                overflow-x: auto;
                font-size: 12px;
            }
            pre code {
                background-color: transparent;
                padding: 0;
                color: #333;
            }
            blockquote {
                border-left: 4px solid #3498db;
                padding-left: 15px;
                margin: 15px 0;
                color: #555;
                background-color: #f8f9fa;
                padding: 10px 15px;
            }
            hr {
                border: none;
                border-top: 1px solid #ddd;
                margin: 20px 0;
            }
            strong {
                color: #2c3e50;
                font-weight: bold;
            }
            em {
                color: #7f8c8d;
            }
        </style>
    </head>
    <body>
    """

    html_full = css_style + html_content + "</body></html>"

    # 保存 HTML 为临时文件 (用于调试)
    html_path = md_path.replace('.md', '.html')
    with open(html_path, 'w', encoding='utf-8') as f:
        f.write(html_full)
    print(f"已生成 HTML: {html_path}")

    # 检查 wkhtmltopdf
    try:
        import pdfkit
        # 尝试直接转换
        options = {
            'page-size': 'A4',
            'margin-top': '2cm',
            'margin-right': '2cm',
            'margin-bottom': '2cm',
            'margin-left': '2cm',
            'encoding': 'UTF-8',
            'no-outline': None,
            'enable-local-file-access': None,
        }
        pdfkit.from_string(html_full, pdf_path, options=options)
        print(f"✓ 成功生成 PDF: {pdf_path}")
    except Exception as e:
        print(f"pdfkit 转换失败: {e}")
        print("\n请安装 wkhtmltopdf:")
        print("  下载地址: https://wkhtmltopdf.org/downloads.html")
        print(f"\n或者，你可以用浏览器打开 {html_path}，然后打印为 PDF。")
        return False

    return True


if __name__ == '__main__':
    md_file = r"d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\说明.md"
    pdf_file = r"d:\Project\Embedded_System_Design\Class_Project\Embedded_System_Smartwatch\说明.pdf"

    if not os.path.exists(md_file):
        print(f"错误: 找不到文件 {md_file}")
        sys.exit(1)

    print(f"正在转换: {md_file}")
    print(f"输出文件: {pdf_file}")
    print("-" * 50)

    success = convert_md_to_pdf(md_file, pdf_file)

    if not success:
        print("\n已生成 HTML 文件，可以用浏览器打开后打印为 PDF。")