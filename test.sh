#!/bin/bash
echo "=== CISC-16-A OS Build Test ==="
make
if [ $? -eq 0 ]; then
    echo "✓ Build successful" 
    ls -la os_nasm.bin
else
    echo "✗ Build failed"
fi
