#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
从 D:/ct/vcpkg 按 FILES_TO_COPY 列表复制文件到 PATCH_ROOT，保持相对路径。
"""

import shutil
from pathlib import Path

# 源根目录（本机完整 vcpkg）
SOURCE_ROOT = Path(r"D:/ct/vcpkg")
# 补丁输出目录（你指定的目标目录）
PATCH_ROOT = Path(r"E:/vcpkg_patch")

# 从截图整理出的 45 个报错文件（相对路径）
FILES_TO_COPY = [
    r"vcpkg\buildtrees\qtsvg\x64-windows-dbg\bin\Qt6Svgd.dll",
    r"vcpkg\buildtrees\qttools\x64-windows-rel\src\designer\src\lib\.qt\rcc\qrc_ClamshellPhone.cpp",
    r"vcpkg\buildtrees\qttools\x64-windows-rel\src\designer\src\lib\.qt\rcc\qrc_S60-nHD-Touchscreen.cpp",
    r"vcpkg\buildtrees\seacas\src\76a825bc51-50a3a4f350.clean\packages\zoltan\doc\Zoltan_html\ug_html\Structural_MATVEC_Avg_Time.jpg",
    r"vcpkg\downloads\msys2-perl-5.38.4-2-x86_64.pkg.tar.zst",
    r"vcpkg\downloads\PowerShell-7.2.24-win-x64.zip",
    r"vcpkg\downloads\python-3.12.7-embed-amd64.zip",
    r"vcpkg\downloads\sandialabs-seacas-47120843900fd7ef845688fa145ebf76a825bc51.tar.gz",
    r"vcpkg\packages\libwebp_x64-windows\bin\libsharpyuv.pdb",
    r"vcpkg\packages\qtsvg_x64-windows\Qt6\plugins\iconengines\qsvgicon.pdb",
    r"vcpkg\packages\sqlite3_x64-windows\bin\sqlite3.dll",
]

def main():
    copied = missing = 0
    for rel in FILES_TO_COPY:
        src = SOURCE_ROOT / rel
        dst = PATCH_ROOT / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        if src.exists():
            shutil.copy2(src, dst)
            print(f"[COPY] {rel}")
            copied += 1
        else:
            print(f"[MISS] {rel}")
            missing += 1
    print(f"\n完成：复制 {copied} 个，缺失 {missing} 个。")

if __name__ == "__main__":
    main()
