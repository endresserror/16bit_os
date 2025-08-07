#!/bin/bash
# CISC-16-A OS QEMU実行スクリプト

echo "=== CISC-16-A OS QEMU実行 ==="

# バイナリが存在するかチェック
if [ ! -f "os_nasm.bin" ]; then
    echo "os_nasm.bin が見つかりません。先にビルドしてください:"
    echo "make"
    exit 1
fi

# QEMUで実行
echo "QEMUでCISC-16-A OSを起動します..."
echo "終了するには Ctrl+A → X を押してください"
echo

qemu-system-i386 \
    -cpu 486 \
    -m 1M \
    -drive format=raw,file=os_nasm.bin,if=floppy \
    -boot a \
    -nographic
