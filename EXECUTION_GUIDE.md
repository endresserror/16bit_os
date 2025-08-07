# CISC-16-A OS 実行ガイド

CISC-16-A OSを実際に動作させるための手順書です。

##  実行環境の選択肢

### 1. QEMU での実行 (推奨)

#### インストール
```bash
# Ubuntu/Debian
sudo apt-get install qemu-system-x86

# macOS  
brew install qemu

# Windows
# https://www.qemu.org/download/ からダウンロード
```

#### 実行
```bash
# ビルド
make

# QEMU実行
./run_qemu.sh
```

#### 操作方法
- 起動後、シェルプロンプト `>` が表示される
- `help` コマンドで利用可能コマンドを確認
- 終了: `Ctrl+A` → `X`

### 2. Python シミュレーター (開発・デバッグ用)

#### 実行
```bash
# ビルド
make

# シミュレーター実行  
python3 simulator.py os_nasm.bin
```

#### 特徴
- 基本的なCPU・メモリ動作をシミュレート
- VGA出力の確認が可能
- デバッグ情報表示

### 3. DOSBox での実行

#### インストール
```bash
# Ubuntu/Debian
sudo apt-get install dosbox

# macOS
brew install dosbox
```

#### 実行
```bash
# ビルド
make

# DOSBox準備・実行
./run_dosbox.sh
dosbox -conf dosbox.conf
```

### 4. 実機・FPGA での実行

#### FPGA実装の場合
1. CISC-16-A CPUコアをFPGAに実装
2. ROM領域に `os_nasm.bin` をロード
3. エントリーポイント `0xFE00` から実行開始

#### 必要な周辺機能
- VGAテキストコントローラ (80x30)
- PS/2キーボードコントローラ
- 32KB RAM + ROM領域

##  動作確認項目

### 起動シーケンス
1. "CISC-16-A OS" ウェルカムメッセージ表示
2. シェルプロンプト `>` 表示
3. キーボード入力受付開始

### コマンドテスト
```
> help                    # コマンド一覧表示
> peek 8000              # VRAMアドレス読み取り  
> poke 8000 41           # 'A'文字を画面に表示
> dump 0                 # メモリダンプ表示
> clear                  # 画面クリア
```

##  トラブルシューティング

### QEMU起動しない
- QEMU がインストールされているか確認
- `qemu-system-i386 --version` でバージョン確認

### 画面に何も表示されない  
- `-nographic -serial stdio` オプションでコンソール出力に切り替え
- VGA出力は現在未対応

### キーボード入力できない
- PS/2エミュレーションの制限
- シミュレーターでの動作確認を推奨

##  開発ノート

現在の実装は基本的なOSカーネルです。実際のハードウェア上での動作には以下が必要：

1. **CISC-16-A CPU実装**: 独自16-bitアーキテクチャ
2. **メモリマップ対応**: 指定されたアドレス配置
3. **周辺機器**: VGA、PS/2の実装

将来的には動画再生機能の追加を目指しています。
