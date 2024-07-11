call clang -g3 -DMAIN -O3 -fsanitize=address -fsanitize=undefined -Wall -Wno-unused-function -Wextra -Wno-missing-field-initializers main.c -o main.exe -O3 -lgdi32 -luser32 -lopengl32 && main.exe
