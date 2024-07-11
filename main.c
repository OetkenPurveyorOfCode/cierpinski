#define TINYCOLORMAP_IMPLEMENTATION
#include "tinycolormap.h"

#define GB_IMPLEMENTATION
#define GB_PLATFORM
#define GB_STATIC
#include "gb.h"
#include "pcg_basic.h"
#include "pcg_basic.c"


#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#define MAXVERTICES 100

void putpixel(gbPlatform* platform, isize x, isize y, u32 color) {
    if (0 <= x && x < platform->window_width && 0 <= y && y < platform->window_height) {
        ((u32*)platform->sw_framebuffer.memory)[platform->window_width*y+x] = color;
    }
}

void clear(gbPlatform platform) {
    gb_memset(platform.sw_framebuffer.memory, 0, platform.sw_framebuffer.memory_size);
}

u32 get_color(isize i, bool enable_colors, isize count) {
    if (!enable_colors) {
        return 0x00FFFFFF;
    } else {
        if (count < 6) {
            switch(i) {
                break;case 0: return 0xFFFF0000;
                break;case 1: return 0xFF00FF00;
                break;case 2: return 0xFF0000FF;
                break;case 3: return 0xFFFFFFFF;
                break;default: return 0xFFFF00FF;
            }
        } else {
            return tcm_to_argb(tcm_heat((double)i / count));
        }
    }
}

typedef enum {
    RM_STATIC,
    RM_LEFT_RIGHT,
    RM_JUSTSCALE,
    RM_SCALE_COORD,
    RM_SCALE_VERT,
    RM_ROT_CLOCKWISE,
    RM_ROT_COUNTERCLOCKWISE,
    RM_SCALE_X
} RenderMode;

typedef enum {
    FM_NONE,
    FM_NODOUBLE,
    FM_LAST_PLUS_TWO,
} FilterMode;

typedef struct {
    RenderMode render_mode;
    FilterMode filter_mode;
    bool enable_colors;
    bool clearbg;
    float ifactor;
    isize vertices;
    double rot;
    double speed;
} App;

void init_vertices(gbPlatform* platform, double* xs, double* ys, double rot, isize count, RenderMode render_mode) {
    assert(count < MAXVERTICES);
    if (count == 3 && (render_mode != RM_ROT_CLOCKWISE && render_mode != RM_ROT_COUNTERCLOCKWISE)) {
        xs[0] = 0;
        xs[1] = platform->window_width/2;
        xs[2] = platform->window_width;
        ys[0] = 0;
        ys[1] = platform->window_height;
        ys[2] = 0;
        return;
    }
    else if (count == 4 && (render_mode != RM_ROT_CLOCKWISE && render_mode != RM_ROT_COUNTERCLOCKWISE)) {
        xs[0] = xs[3] = 0;
        ys[0] = ys[1] = platform->window_height;
        xs[1] = xs[2] = platform->window_width;
        ys[2] = ys[3] = 0;
        return;
    }
    else {
        double r = gb_min(platform->window_width/2, platform->window_height/2);
        for (int i = 0; i < count; i++) {
            xs[i] = r*cosf(6.28/count*(i+rot))+ platform->window_width/2;
            ys[i] = r*sinf(6.28/count*(i+rot))+ platform->window_height/2;
        }
        return;
    }
}

#ifdef MAIN 
int main(void) {
#else
int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR     lpCmdLine,int       nShowCmd) { 
    (void)hInstance, (void)hPrevInstance, (void)lpCmdLine, (void)nShowCmd;
#endif
    App app = {
        .render_mode=RM_LEFT_RIGHT,
        .filter_mode=FM_NONE,
        .clearbg = false,
        .enable_colors=true,
        .vertices = 3,
        .rot = 0,
        .ifactor = 1,
        .speed = 1,
    };
    gbPlatform platform = {0};
    gb_platform_init_with_software(&platform, "Cierpinski", 800, 600, gbWindow_Resizable);
    assert(platform.sw_framebuffer.bits_per_pixel == 32);
    assert(platform.sw_framebuffer.memory_size == platform.window_height * platform.window_width * platform.sw_framebuffer.bits_per_pixel / 8);
    gb_platform_display(&platform);
    i32 old_window_height = platform.window_height;
    i32 old_window_width = platform.window_width;

    double xs[MAXVERTICES] = {0};
    double ys[MAXVERTICES] = {0};
    init_vertices(&platform, xs, ys, app.rot, app.vertices, app.render_mode);
    double x = 0;
    double y = 0;
    double counter = 0;
    while (!platform.window_is_closed) {
        b32 was_resized = (platform.window_width != old_window_width) || (platform.window_height != old_window_height);
        if (was_resized) {
            old_window_height = platform.window_height;
            old_window_width = platform.window_width;
            clear(platform);
        }

        //input
        // config
        if (platform.keys['C'] == gbKeyState_Released) {
            app.enable_colors = !app.enable_colors;
        }
        if (platform.keys['R'] == gbKeyState_Released) {
            app.clearbg = !app.clearbg;
        }
        if (platform.keys['W'] == gbKeyState_Released) {
            app.speed *= 1.2;
        }
        if (platform.keys['S'] == gbKeyState_Released) {
            app.speed /= 1.2;
        }
        if (platform.keys[gbKey_Up] == gbKeyState_Released) {
            app.ifactor *= 1.2;
        }
        if (platform.keys[gbKey_Down] == gbKeyState_Released) {
            app.ifactor /= 1.2;
        }
        if (platform.keys[gbKey_Left] == gbKeyState_Released) {
            if (3 <= app.vertices && app.vertices < MAXVERTICES) {
                app.vertices -= 1;
            }  
        }
        if (platform.keys[gbKey_Right] == gbKeyState_Released) {
            if (2 <= app.vertices && app.vertices+1 < MAXVERTICES) {
                app.vertices += 1;
            }  
        }

        // mode
        if (platform.keys[gbKey_Lshift] != gbKeyState_Down && platform.keys[gbKey_Rshift] != gbKeyState_Down) {
            //render mode
            if (platform.keys[gbKey_0] == gbKeyState_Down) {
                app.render_mode = RM_STATIC;
            }
            if (platform.keys[gbKey_1] == gbKeyState_Down) {
                app.render_mode = RM_LEFT_RIGHT;
            }
            if (platform.keys[gbKey_2] == gbKeyState_Down) {
                app.render_mode = RM_JUSTSCALE;
            }
            if (platform.keys[gbKey_3] == gbKeyState_Down) {
                app.render_mode = RM_SCALE_COORD;
            }
            if (platform.keys[gbKey_4] == gbKeyState_Down) {
                app.render_mode = RM_SCALE_VERT;
            }
            if (platform.keys[gbKey_5] == gbKeyState_Down) {
                app.render_mode = RM_ROT_CLOCKWISE;
            }
            if (platform.keys[gbKey_6] == gbKeyState_Down) {
                app.render_mode = RM_ROT_COUNTERCLOCKWISE;
            }
            if (platform.keys[gbKey_7] == gbKeyState_Down) {
                app.render_mode = RM_SCALE_X;
            }
        }
        else {
            // filter mode
            if (platform.keys[gbKey_1] == gbKeyState_Down) {
                app.filter_mode = FM_NONE;
            }
            if (platform.keys[gbKey_2] == gbKeyState_Down) {
                app.filter_mode = FM_NODOUBLE;
            }
            if (platform.keys[gbKey_3] == gbKeyState_Down) {
                app.filter_mode = FM_LAST_PLUS_TWO;
            }
        }

        //rendering

        isize iter = platform.window_width * app.ifactor *  platform.window_height / 70;
        if (app.clearbg) {
            clear(platform);
        }
        else {
            for(isize it = 0; it < iter; it++) {
                putpixel(&platform, pcg32_boundedrand(platform.window_width), pcg32_boundedrand(platform.window_height), 0);
            }
        }
        double scale;
        if (app.render_mode == RM_ROT_CLOCKWISE) {
            app.rot = counter;
        }
        if (app.render_mode == RM_ROT_COUNTERCLOCKWISE) {
            app.rot = -counter;
        }
        if (app.render_mode == RM_JUSTSCALE) {
            scale = (sin((double)counter) + 3);
            app.rot = 0;
        }
        if (app.render_mode == RM_SCALE_COORD || app.render_mode == RM_SCALE_VERT) {
            scale = 4*(sin((double)counter) + 1);
            app.rot = 0;
        }
        if (app.render_mode == RM_SCALE_X) {
            scale = 4*(sin((double)counter) + 1);
            app.rot = 0;
        }
        init_vertices(&platform, xs, ys, app.rot, app.vertices, app.render_mode);
        if (app.render_mode == RM_LEFT_RIGHT) {
            xs[1] = (sin((double)counter)*(platform.window_width-1))/2+platform.window_width/2;
            app.rot = 0;
        }
        isize last_i = 0;
        for(isize it = 0; it < iter; it++) {
            isize i = pcg32_boundedrand(app.vertices);
            bool ok = true;
            switch (app.filter_mode) {
                break;case FM_NONE: {
                    ok = true;
                }
                break;case FM_NODOUBLE: {
                    ok = (i != last_i);
                }
                break;case FM_LAST_PLUS_TWO: {
                    ok = (i == (last_i + 2) % 4);
                }
            }
            last_i = i;
            if (ok) {
                if (app.render_mode == RM_JUSTSCALE) {
                    x = (x + xs[i])/scale;
                    y = (y + ys[i])/scale;
                }
                else if (app.render_mode == RM_SCALE_COORD) {
                    x = (scale*x + xs[i])/(1+scale);
                    y = (scale*y + ys[i])/(1+scale);
                }
                else if (app.render_mode == RM_SCALE_VERT) {
                    x = (x + scale*xs[i])/(1+scale);
                    y = (y + scale*ys[i])/(1+scale);
                }
                else if (app.render_mode == RM_SCALE_X) {
                    x = (x + scale*xs[i])/(scale+1);
                    y = (y + ys[i])/2;
                }
                else { // left right, rotate
                    x = (x + xs[i])/2;
                    y = (y + ys[i])/2;
                }
            }
            putpixel(&platform, x, y, get_color(i, app.enable_colors, app.vertices));
        };
        counter += 1e-3 *app.speed;
        gb_platform_update(&platform);
        gb_platform_display(&platform);
    };
    gb_platform_destroy(&platform);
    return 0;
}
