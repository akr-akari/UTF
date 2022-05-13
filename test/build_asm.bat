%1 "main.cc" -o"./out/main%1%2.o" -Wall -Wextra -std="c++2b" -g -c %2
objdump -d -Mintel -C -S "./out/main%1%2.o" 1> "./out/main%1%2.asm"
del out\main%1%2.o
