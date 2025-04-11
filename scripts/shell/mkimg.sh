#!/bin/bash

# 检查参数
#if [ "$#" -ne 2 ]; then
#  echo "Usage: $0 <input.elf> <output.img>"
#  exit 1
#fi

PHYS_KERNEL=target/build/Clang-18-x86_64-pc-linux-gnu/kernel_phys #"$1"
MAIN_KERNEL=target/build/Clang-18-x86_64-pc-linux-gnu/ours.obi
OUTPUT_IMG=ours.iso #"$2"
GRUB_CFG="grub.cfg"

# 创建一个临时目录
TEMP_DIR=$(mktemp -d)

# 创建一个 GRUB 文件系统
mkdir -p "$TEMP_DIR/boot/grub"

# 创建 grub.cfg 配置文件
cat > "$TEMP_DIR/boot/grub/$GRUB_CFG" <<EOF
set timeout=0
set default=0
menuentry "ours" {
    multiboot2 /boot/$(basename "$PHYS_KERNEL")
    module2 /boot/$(basename "$MAIN_KERNEL")
    boot
}
EOF

cat $TEMP_DIR/boot/grub/$GRUB_CFG

if [ -e "$PHYS_KERNEL" ]; then
    echo "文件存在"
else
    echo "文件不存在"
fi

# 拷贝 ELF 文件到引导目录
cp "$PHYS_KERNEL" "$TEMP_DIR/boot/$(basename "$PHYS_KERNEL")"
cp "$MAIN_KERNEL" "$TEMP_DIR/boot/$(basename "$MAIN_KERNEL")"
tree $TEMP_DIR

# 创建 ISO 镜像
grub-mkrescue -o "$OUTPUT_IMG" "$TEMP_DIR"

# 清理临时目录
rm -rf "$TEMP_DIR"

echo "GRUB boot image created: $OUTPUT_IMG"