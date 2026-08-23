import subprocess
import csv
import os
import re
from tabulate import tabulate

def run_adb(cmd):
    """修复 Windows GBK 编码问题 + 空值容错"""
    try:
        r = subprocess.run(
            cmd, shell=True, capture_output=True, text=True,
            encoding='utf-8', errors='replace'
        )
        return r.stdout.strip() if r.stdout else ""
    except Exception as e:
        print(f"\n⚠️ 命令执行失败: {cmd[:60]}... | 错误: {e}")
        return ""

def get_packages():
    """获取所有第三方应用包名"""
    out = run_adb("adb shell pm list packages -3")
    return [l.replace("package:", "") for l in out.splitlines() if l.startswith("package:")]

def get_desktop_name(pkg):
    """【核心】通过 aapt 获取应用在桌面显示的名称 (application-label)"""
    # 1. 获取 APK 路径
    path_out = run_adb(f'adb shell pm path {pkg}')
    apk_path = ""
    for line in path_out.splitlines():
        if line.startswith("package:"):
            apk_path = line.replace("package:", "").strip()
            break
    
    if not apk_path:
        return "N/A"

    # 2. 使用 aapt dump badging 提取 application-label
    badging_out = run_adb(f'adb shell aapt dump badging "{apk_path}"')
    for line in badging_out.splitlines():
        if "application-label:" in line:
            # 提取单引号内的名称，例如: application-label:'微信'
            match = re.search(r"application-label:'(.+?)'", line)
            if match:
                return match.group(1)
    return "N/A"

def get_package_info(pkg):
    """提取安装时间"""
    first, last = "N/A", "N/A"
    dump_out = run_adb(f'adb shell dumpsys package {pkg}')
    
    for line in dump_out.splitlines():
        stripped = line.strip()
        if "firstInstallTime=" in stripped:
            match = re.search(r'firstInstallTime=(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2})', stripped)
            if match: 
                first = match.group(1)
        elif "lastUpdateTime=" in stripped:
            match = re.search(r'lastUpdateTime=(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2})', stripped)
            if match: 
                last = match.group(1)
                
    return first, last

def save_to_csv(data, headers, filename="adb_app_list.csv"):
    """将数据保存为 CSV 文件"""
    try:
        with open(filename, 'w', newline='', encoding='utf-8-sig') as f:
            writer = csv.writer(f)
            writer.writerow(headers)
            writer.writerows(data)
        print(f"💾 数据已成功保存至: {os.path.abspath(filename)}")
    except Exception as e:
        print(f"⚠️ 保存 CSV 失败: {e}")

def main():
    print("⏳ 正在获取第三方应用列表...")
    pkgs = get_packages()
    data = []
    total = len(pkgs)
    
    for i, pkg in enumerate(pkgs, 1):
        print(f"🔍 ({i}/{total}) 正在解析: {pkg} ...", end="\r")
        
        # 修复了这里的语法错误，补全了括号
        desktop_name = get_desktop_name(pkg)
        first, last = get_package_info(pkg)
        
        data.append([desktop_name, pkg, first, last])
    
    print("\n✅ 解析完成！\n")
    
    headers = ["桌面应用名称", "包名", "首次安装时间", "最后更新时间"]
    print(tabulate(data, headers=headers, tablefmt="grid", stralign="left"))
    save_to_csv(data, headers)

if __name__ == "__main__":
    main()