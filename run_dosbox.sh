#!/bin/bash
# DOSBoxでの実行スクリプト

echo "=== DOSBox実行準備 ==="

# バイナリをフロッピーイメージに変換
if [ ! -f "os_nasm.bin" ]; then
    echo "エラー: os_nasm.bin が見つかりません"
    echo "先に 'make' を実行してください"
    exit 1
fi

# 1.44MBフロッピーイメージ作成
echo "フロッピーイメージ作成中..."
dd if=/dev/zero of=floppy.img bs=512 count=2880 2>/dev/null

# OSをフロッピーの先頭に書き込み
dd if=os_nasm.bin of=floppy.img conv=notrunc 2>/dev/null

echo "DOSBox設定ファイル作成中..."
cat > dosbox.conf << EOF
[cpu]
core=normal
cputype=auto
cycles=3000

[dos]
boot=floppy.img

[autoexec]
# 自動起動設定
EOF

echo "DOSBoxで実行するには:"
echo "dosbox -conf dosbox.conf"
