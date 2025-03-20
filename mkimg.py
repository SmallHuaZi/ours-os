import os
import shutil
import subprocess
import tempfile

# 定义参数
PHYS_KERNEL = "target/build/Clang-18-x86_64-pc-linux-gnu/kernel_phys"
MAIN_KERNEL = "target/build/Clang-18-x86_64-pc-linux-gnu/kernel_main_image.bin"
OUTPUT_IMG = "ours.iso"
GRUB_CFG = "grub.cfg"

# 创建一个临时目录
temp_dir = tempfile.mkdtemp()

try:
    # 创建 GRUB 文件系统目录结构
    boot_grub_dir = os.path.join(temp_dir, "boot", "grub")
    os.makedirs(boot_grub_dir)

    # 创建 grub.cfg 配置文件
    grub_cfg_path = os.path.join(boot_grub_dir, GRUB_CFG)
    with open(grub_cfg_path, "w") as grub_cfg_file:
        grub_cfg_file.write(f"""set timeout=0
set default=0
menuentry "ours" {{
    multiboot2 /boot/{os.path.basename(PHYS_KERNEL)}
    module2 /boot/{os.path.basename(MAIN_KERNEL)}
    boot
}}
""")
    print(f"Generated GRUB config:\n{open(grub_cfg_path).read()}")

    # 检查文件是否存在
    if os.path.exists(PHYS_KERNEL):
        print("文件存在")
    else:
        print("文件不存在")
        exit(1)

    # 拷贝 ELF 文件到引导目录
    shutil.copy(PHYS_KERNEL, os.path.join(temp_dir, "boot", os.path.basename(PHYS_KERNEL)))
    shutil.copy(MAIN_KERNEL, os.path.join(temp_dir, "boot", os.path.basename(MAIN_KERNEL)))
    print(f"File structure in temporary directory:\n{subprocess.getoutput(f'tree {temp_dir}')}")

    # 创建 ISO 镜像
    subprocess.run(["grub-mkrescue", "-o", OUTPUT_IMG, temp_dir], check=True)
    print(f"GRUB boot image created: {OUTPUT_IMG}")

finally:
    # 清理临时目录
    shutil.rmtree(temp_dir)
