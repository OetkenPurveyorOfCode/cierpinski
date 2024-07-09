call clang -Wall -Wno-unused-function -Wextra -Wno-missing-field-initializers main.c -o main.exe -O3 -lgdi32 -luser32 -lopengl32 -Wl,/subsystem:windows && main.exe
