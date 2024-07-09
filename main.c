#define GB_IMPLEMENTATION
#define GB_PLATFORM
#define GB_STATIC
#include "gb.h"
#include "pcg_basic.h"
#include "pcg_basic.c"
#include <math.h>
#include <stdbool.h>

void putpixel(gbPlatform* platform, isize x, isize y, u32 color) {
    GB_ASSERT(0 <= x && x < platform->window_width);
    GB_ASSERT(0 <= y && y < platform->window_height);
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
        }
    }
    return 0xFFFFFFFF;
}

typedef enum {
    RM_LEFT_RIGHT,
    RM_SCALE,
    RM_SCALECOORD,
    RM_SCALEVERT,
    RM_PLASMA,
} RenderMode;

typedef struct {
    RenderMode mode;
    bool enable_colors;
} App;

int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR     lpCmdLine,int       nShowCmd) { 
    App app = {
        .mode=RM_LEFT_RIGHT,
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
            x = 0;
            y = 0;
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
        }

    };
    gb_platform_destroy(&platform);
    return 0;
}
