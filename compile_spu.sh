gcc -g -O0 -Wall -Wextra -fsanitize=address src/spu/main.c src/file_utils.c src/dump_utils.c src/spu/spu.c secstack/src/secstack.c secstack/src/dump_funcs.c secstack/src/canary_utils.c -o spu
