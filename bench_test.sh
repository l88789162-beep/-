#!/bin/bash
#Pikafish patch benchmark test
ORIG_BIN="./pikafish-orig"
PATCH_BIN="./pikafish-patch"
echo "===== Benchmark Original Version ====="
$ORIG_BIN bench
echo "===== Benchmark Patched Version ====="
$PATCH_BIN bench
#批量对弈elo测试，使用cutechess-cli
cutechess-cli -engine cmd=$ORIG_BIN name=orig -engine cmd=$PATCH_BIN name=patch -each tc=4+0.05 -rounds 200 -games 2 -concurrency 4 -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05
