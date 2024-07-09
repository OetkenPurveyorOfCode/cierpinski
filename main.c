#define GB_IMPLEMENTATION
#define GB_PLATFORM
#define GB_STATIC
#include "gb.h"
#include "pcg_basic.h"
#include "pcg_basic.c"
#include <math.h>
#include <stdbool.h>
#include <assert.h>

void putpixel(gbPlatform* platform, isize x, isize y, u32 color) {
    assert(0 <= x && x < platform->window_width);
    assert(0 <= y && y < platform->window_height);
    ((u32*)platform->sw_framebuffer.memory)[platform->window_width*y+x] = color;
}

void clear(gbPlatform platform) {
    gb_memset(platform.sw_framebuffer.memory, 0, platform.sw_framebuffer.memory_size);
}

u32 get_color(isize i, bool enable_colors) {
    if (!enable_colors) {
        return 0xFFFFFFFF;
    } else {
        switch (i) {
            break;case 0: return 0xFFFF0000;
            break;case 1: return 0xFF00FF00;
            break;case 2: return 0xFF0000FF;
            break;case 3: return 0xFFFFFFFF;
        }
    }
    return 0xFFFFFFFF;
}

typedef enum {
    RM_LEFT_RIGHT,
    RM_SCALE,
    RM_SCALECOORD,
    RM_SCALEVERT,
    RM_SQUARE,
    RM_NODOUBLE,
    RM_LAST_PLUS_ONE,
    RM_QSCALE,
    RM_RSCALE,
    RM_ZSCALE,
    RM_PENTA,
} RenderMode;

typedef struct {
    RenderMode mode;
    bool enable_colors;
} App;

#ifdef MAIN 
int main(void) {
#else
int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR     lpCmdLine,int       nShowCmd) { 
#endif
    App app = {
        .mode=RM_LEFT_RIGHT, .enable_colors=true,
    };
    gbPlatform platform = {0};
    gb_platform_init_with_software(&platform, "Cierpinski", 800, 600, gbWindow_Resizable);
    GB_ASSERT(platform.sw_framebuffer.bits_per_pixel == 32);
    GB_ASSERT(platform.sw_framebuffer.memory_size == platform.window_height * platform.window_width * platform.sw_framebuffer.bits_per_pixel / 8);
    gb_platform_display(&platform);
    i32 old_window_height = platform.window_height;
    i32 old_window_width = platform.window_width;
    double xs[] = {0, platform.window_width/2, platform.window_width-1};
    double ys[] = {0, platform.window_height-1, 0};
    double square_xs[] = {0, 0,                      platform.window_width, platform.window_width };
    double square_ys[] = {0, platform.window_height, 0,                     platform.window_height};
    double r = gb_min(platform.window_width/2, platform.window_height/2);
    double penta_xs[5];
    double penta_ys[5];
    for (int i = 0; i < 5; i++) {
        penta_xs[i] = r*cosf(6.28/5*i)+ platform.window_width/2;
        penta_ys[i] = r*sinf(6.28/5*i)+ platform.window_height/2;
    }
    double x = 0;
    double y = 0;
    double counter = 0;
    while (!platform.window_is_closed) {
        b32 was_resized = (platform.window_width != old_window_width) || (platform.window_height != old_window_height);
        if (was_resized) {
            old_window_height = platform.window_height;
            old_window_width = platform.window_width;
            xs[1] = platform.window_width / 2;
            xs[2] = platform.window_width - 1;
            ys[1] = platform.window_height - 1;
            square_xs[2] = square_xs[3] = platform.window_width;
            square_ys[1] = square_ys[3] = platform.window_height;
            x = 0;
            y = 0;

            double r = gb_min(platform.window_width/2, platform.window_height/2);
            for (int i = 0; i < 5; i++) {
                penta_xs[i] = r*cosf(6.28/5*i)+ platform.window_width/2;
                penta_ys[i] = r*sinf(6.28/5*i)+ platform.window_height/2;
            }
            clear(platform);
        }
        //input
        if (platform.keys['S'] == gbKeyState_Down) {
            app.mode = RM_SCALE;
        }
        if (platform.keys['L'] == gbKeyState_Down) {
            app.mode = RM_LEFT_RIGHT;
        }
        if (platform.keys['T'] == gbKeyState_Down) {
            app.mode = RM_SCALECOORD;
        }
        if (platform.keys['V'] == gbKeyState_Down) {
            app.mode = RM_SCALEVERT;
        }
        if (platform.keys['C'] == gbKeyState_Released) {
            app.enable_colors = !app.enable_colors;
        }
        if (platform.keys['Q'] == gbKeyState_Released) {
            app.mode = RM_SQUARE;
        }
        if (platform.keys['N'] == gbKeyState_Released) {
            app.mode = RM_NODOUBLE;
        }
        if (platform.keys['W'] == gbKeyState_Released) {
            app.mode = RM_LAST_PLUS_ONE;
        }
        if (platform.keys['E'] == gbKeyState_Released) {
            app.mode = RM_QSCALE;
        }
        if (platform.keys['R'] == gbKeyState_Released) {
            app.mode = RM_RSCALE;
        }
        if (platform.keys['U'] == gbKeyState_Released) {
            app.mode = RM_ZSCALE;
        }
        if (platform.keys['P'] == gbKeyState_Released) {
            app.mode = RM_PENTA;
        }

        //render

        switch (app.mode) {
            break;case RM_SCALE: {
                double scale = (sin((double)counter) + 3);
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(3);
                    x = (x + xs[i])/scale;
                    y = (y + ys[i])/scale;
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_LEFT_RIGHT: {
                xs[1] = (sin((double)counter)*(platform.window_width-1))/2+platform.window_width/2;
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(3);
                    x = (x + xs[i])/2;
                    y = (y + ys[i])/2;
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_SCALECOORD: {
                double scale = 4*(sin((double)counter) + 1);
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(3);
                    x = (scale*x + xs[i])/(1+scale);
                    y = (scale*y + ys[i])/(1+scale);
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_SCALEVERT: {
                double scale = 4*(sin((double)counter) + 1);
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(3);
                    x = (x + scale*xs[i])/(1+scale);
                    y = (y + scale*ys[i])/(1+scale);
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_SQUARE: {
                square_xs[1] = (sin((double)counter)*(platform.window_width-1))/2+platform.window_width/2;
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(4);
                    x = (x + square_xs[i])/2;
                    y = (y + square_ys[i])/2;
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_NODOUBLE: {
                isize last_i = 0;
                square_xs[1] = (sin((double)counter)*(platform.window_width-1))/2+platform.window_width/2;
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(4);
                    if (i != last_i) {
                        x = (x + square_xs[i])/2;
                        y = (y + square_ys[i])/2;
                    }
                    last_i = i;
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_LAST_PLUS_ONE: {
                isize last_i = 0;
                square_xs[1] = (sin((double)counter)*(platform.window_width-1))/2+platform.window_width/2;
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(4);
                    if (i == (last_i + 2) % 4) {
                        x = (x + square_xs[i])/2;
                        y = (y + square_ys[i])/2;
                    }
                    last_i = i;
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_QSCALE: {
                isize last_i = 0;
                double scale = 4*(sin((double)counter) + 1);
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(4);
                    if (i == (last_i + 2) % 4) {
                        x = (x + scale*square_xs[i])/(scale+1);
                        y = (y + square_ys[i])/2;
                    }
                    last_i = i;
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_RSCALE: {
                isize last_i = 0;
                double scale = 4*(sin((double)counter) + 1.1);
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(4);
                    if (i != last_i && i != last_i + 1) {
                        x = (scale*x + square_xs[i])/(scale+1);
                        y = (scale*y + square_ys[i])/(scale+1);
                    }
                    last_i = i;
                    putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_ZSCALE: {
                double scale = 4*(sin((double)counter) + 1);
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(4);
                    x = (x + scale*square_xs[i])/(scale+1);
                    y = (y + scale*square_ys[i])/(scale+1);
                    if (0 <= x && x < platform.window_width && 0 <= y && y < platform.window_height) {
                        putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    }
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
            break;case RM_PENTA: {
                double scale = 4*(sin((double)counter) + 1);
                for(isize it = 0; it < 10000; it++) {
                    isize i = pcg32_boundedrand(5);
                    x = (x + scale*penta_xs[i])/(scale+1);
                    y = (y + scale*penta_ys[i])/(scale+1);
                    if (0 <= x && x < platform.window_width && 0 <= y && y < platform.window_height) {
                        putpixel(&platform, x, y, get_color(i, app.enable_colors));
                    }
                    putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
                };
                counter += 1e-3;
                gb_platform_update(&platform);
                gb_platform_display(&platform);
            }
        }

    };
    gb_platform_destroy(&platform);
    return 0;
}
