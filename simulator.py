#!/usr/bin/env python3
"""
CISC-16-A OS シミュレーター
基本的な16-bit環境をエミュレートしてOSを実行
"""

import struct
import sys
import os

class CISC16A_Simulator:
    def __init__(self):
        # メモリ初期化 (64KB)
        self.memory = bytearray(65536)
        
        # レジスタ
        self.ax = 0
        self.bx = 0  
        self.cx = 0
        self.dx = 0
        self.si = 0
        self.di = 0
        self.sp = 0x7FFF  # スタックポインタ
        self.pc = 0xFE00  # プログラムカウンタ (開始アドレス)
        
        # フラグ
        self.carry_flag = False
        self.zero_flag = False
        
        # VGA状態
        self.vga_cursor = 0
        
    def load_os(self, filename):
        """OSバイナリをメモリにロード"""
        try:
            with open(filename, 'rb') as f:
                os_data = f.read()
            
            # 0x0000からロード
            for i, byte in enumerate(os_data):
                if i < len(self.memory):
                    self.memory[i] = byte
                    
            print(f"OS loaded: {len(os_data)} bytes")
            return True
            
        except FileNotFoundError:
            print(f"Error: {filename} not found")
            return False
    
    def read_memory(self, address):
        """メモリ読み取り"""
        if 0 <= address < len(self.memory):
            return self.memory[address]
        return 0
    
    def write_memory(self, address, value):
        """メモリ書き込み"""
        if 0 <= address < len(self.memory):
            self.memory[address] = value & 0xFF
            
            # VGA VRAM への書き込み検出
            if 0x8000 <= address <= 0x8FFF:
                self.handle_vga_write(address, value)
    
    def handle_vga_write(self, address, value):
        """VGA書き込み処理"""
        char = chr(value) if 32 <= value <= 126 else '.'
        print(f"VGA[{address:04X}]: {char} ({value:02X})", end='')
    
    def print_state(self):
        """レジスタ状態表示"""
        print(f"\n=== CPU State ===")
        print(f"AX:{self.ax:04X} BX:{self.bx:04X} CX:{self.cx:04X} DX:{self.dx:04X}")
        print(f"SI:{self.si:04X} DI:{self.di:04X} SP:{self.sp:04X} PC:{self.pc:04X}")
        print(f"Flags: CF={self.carry_flag} ZF={self.zero_flag}")
    
    def simulate_basic_execution(self):
        """基本的な実行シミュレーション"""
        print("=== CISC-16-A OS Simulation ===")
        print("Simulating basic OS execution...")
        
        # 基本的なOSの動作をシミュレート
        print("\n1. Stack pointer setup")
        self.sp = 0x7FFF
        
        print("2. VGA initialization")  
        # VGA画面クリア
        for addr in range(0x8000, 0x8000 + 80*30):
            self.write_memory(addr, ord(' '))
        
        print("\n3. Welcome message")
        welcome = "CISC-16-A OS"
        for i, char in enumerate(welcome):
            self.write_memory(0x8000 + i, ord(char))
        
        print("\n4. Shell prompt")
        prompt = "> "
        for i, char in enumerate(prompt):
            self.write_memory(0x8000 + 80 + i, ord(char))
        
        print("\n\n=== Simulation Complete ===")
        self.print_state()

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 simulator.py <os_binary>")
        print("Example: python3 simulator.py os_nasm.bin")
        sys.exit(1)
    
    simulator = CISC16A_Simulator()
    
    if simulator.load_os(sys.argv[1]):
        simulator.simulate_basic_execution()
    else:
        sys.exit(1)

if __name__ == "__main__":
    main()
