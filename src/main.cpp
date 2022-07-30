/*
    Title: French Pixmap
    Author: Le Juez Victor
    Thanks to: Jacques-Olivier Lapeyre
    Version file: 02
    Date: 30/07/2022
*/

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Pixmap/Pixmap.hpp"

#define WIN_W 864
#define WIN_H 486

void blit_sin (Pixmap const &src, Pixmap &target, int const target_x1, int const target_y1, int const amplitude_x, float const phase, float const ripple_rate)
{

   /* amplitude_x: ripple distance   */
   /* phase: ripple speed/frequency  */
   /* ripple_rate: ripple count/rate */

    int const src_height = src.get_height();

    for (int y = 0; y < src_height; y++)
    {
        int x = target_x1 - amplitude_x * sin (y * ripple_rate + phase);
        src.blit_line (target, y, x, target_y1 + y);
    }

}

void draw_french_flag (Pixmap &pix)
{
    int const third_of_the_flag = pix.get_width() / 3;
    int const pix_height = pix.get_height() - 1;

    for (int i = 0; i < 3; i++)
    {

        Rectbox rect (third_of_the_flag * i, 0, third_of_the_flag * (i+1), pix_height);

        pixel colour;
        if      (i == 0) colour = 0xFF0050A4;
        else if (i == 1) colour = 0xFFFFFFFF;
        else    {        colour = 0xFFEF4135; rect.x2 += 1;}

        pix.draw_rectbox (rect, colour);

    }
}

void draw_checkerboard (Pixmap &pix, int const side_length)
{
    int const pix_width  = pix.get_width()  / side_length;
    int const pix_height = pix.get_height() / side_length;

    for (int y = 0; y < pix_height; y++)
    {
        for (int x = 0; x < pix_width; x++)
        {

            pixel colour;
            if (x % 2 == y % 2) colour = 0xFFFFFFFF;
            else                colour = 0xFF000000;

            int x1 = x * side_length; int y1 = y * side_length;
            Rectbox rect (x1, y1, x1 + side_length - 1, y1 + side_length - 1);

            pix.draw_rectbox (rect, colour);

        }
    }
}

void draw_text (SDL_Renderer* renderer, const char* text,
               int const size, int const x, int const y, int const w, int const h,
               uint8_t const r, uint8_t const g, uint8_t const b, uint8_t const a)
{

    #ifdef __linux__
      const char* font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    #elif _WIN32
      const char* font_path = "C:\\Windows\\Fonts\\DejaVuSans.ttf";
    #endif

    TTF_Font* font = TTF_OpenFont (font_path, size);

    if (!font) { std::cerr << "ERROR: " << TTF_GetError() << std::endl; return; }

    const SDL_Color text_color    = {r, g, b, a};
    const SDL_Rect  text_location = {x, y, w, h};

    SDL_Surface* text_surface = TTF_RenderText_Blended (font, text, text_color);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface (renderer, text_surface);
    if (a!=255) SDL_SetTextureAlphaMod(text_texture, a); // Apply the alpha channel if necessary

    SDL_RenderCopy (renderer, text_texture, NULL, &text_location); // Render the created texture

    /* Destruction of work datas */

    SDL_DestroyTexture (text_texture);
    SDL_FreeSurface (text_surface);
    TTF_CloseFont (font);

}

int main (int argc, char** argv)
{

    /* SDL window initialization */

    if (SDL_Init (SDL_INIT_VIDEO) < 0) { std::cerr << "ERROR: " << SDL_GetError() << std::endl; return 1; };
    if (TTF_Init() < 0) { std::cerr << "ERROR: " << TTF_GetError() << std::endl; return 1; }

    SDL_Window* win = SDL_CreateWindow ("French Pixmap", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, 0);
    if (!win) { std::cerr << "ERROR: Failed to create SDL window. " << SDL_GetError() << std::endl; return 1; }

    SDL_Renderer* ren = SDL_CreateRenderer (win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture*  tex = SDL_CreateTexture (ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WIN_W, WIN_H);

    SDL_Event event;

    /* Program initialization */

    Pixmap render (WIN_W, WIN_H, 0xFF000000, false);  // Render Pixmap (image is blitted in)
    Pixmap image  (320,   240,   0xFFFFFFFF, false);  // Pixmap that will be animated

    //draw_checkerboard (image, 40) // Instead of 'draw_french_flag' if you wish it.
    draw_french_flag (image); // Draw the pixmap as a parameter
    image.average_filter (6); // Average filter, adds blur effect

    int render_w = render.get_width(), render_h = render.get_height();
    Rectbox render_rect (0, 0, render_w-1, render_h-1); // For gradient of background

    int const image_x1 = (render.get_width()  - image.get_width())  / 2;
    int const image_y1 = (render.get_height() - image.get_height()) / 2;

    int tex_w, tex_h; // Query on texture for get its dimension on 'tex_w && tex_h'
    SDL_QueryTexture (tex, NULL, NULL, &tex_w, &tex_h);

    float phase_factor, ripple_rate;
    if (argc > 1) phase_factor = -atoi(argv[1]) * 0.001;
    else          phase_factor = -0.05;
    if (argc > 2) ripple_rate = atoi(argv[2]) * 0.001;
    else          ripple_rate  =  0.01;

    int  loop_nb = 0;   // Number of iterations of the execution loop (is used to calculate the phase of 'blit_sin')
    bool running = true;

    /* Program execution */

    while (running)
    {

        /* SDL Event Management */

        while (SDL_PollEvent(&event))
        {

            running = event.type != SDL_QUIT;

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_UP:
                        phase_factor -= 0.001; break;
                    case SDLK_DOWN:
                        phase_factor += 0.001; break;
                    case SDLK_LEFT:
                        ripple_rate  -= 0.001; break;
                    case SDLK_RIGHT:
                        ripple_rate  += 0.001; break;
                    default: break;
                }
            }

        }

        /* Animating the pixmap */

        //render.fill (0xFF000000); // for black background, instead of 'vertical_gradient' if you wish it
        render.vertical_gradient (render_rect, 0xFF000000, 0xFFFFFFFF);

        blit_sin (image, render, image_x1, image_y1, 50, (loop_nb * phase_factor), ripple_rate);

        render.blit_on_texture_centered (tex, tex_w, tex_h);

        SDL_RenderCopy (ren, tex, NULL, NULL);

        /* Calculation and display of an understandable speed */

        const int speed_info_val = (int) (-phase_factor * 1000);
        const std::string speed_info_str = "SPEED: " + std::to_string (speed_info_val);
        const char* speed_info_txt = speed_info_str.c_str();

        draw_text (ren, speed_info_txt, 32, 25, 25, 175, 75, 255, 255, 255, 255);
        draw_text (ren, "UP / DOWN", 32, 25, (WIN_H - 90), 175, 75, 0, 0, 0, 255);

        /* Calculation and display of an understandable ripple rate */

        const int ripple_rate_info_val  = (int) (ripple_rate  * 1000);
        const std::string ripple_rate_info_str = "RIPPLE RATE: " + std::to_string (ripple_rate_info_val);
        const char* ripple_rate_info_txt = ripple_rate_info_str.c_str();

        draw_text (ren, ripple_rate_info_txt, 32, (WIN_W - 200), 25, 175, 75, 255, 255, 255, 255);
        draw_text (ren, "LEFT / RIGHT", 32, (WIN_W - 200), (WIN_H - 90), 175, 75, 0, 0, 0, 255);

        /* Render/wait/increment */

        SDL_RenderPresent (ren);
        SDL_Delay (16); // 17ms - 1ms = ~60fps
        ++loop_nb;

    }

    /* Closing the program */

    SDL_DestroyTexture  (tex);
    SDL_DestroyRenderer (ren);
    SDL_DestroyWindow   (win);

    TTF_Quit();
    SDL_Quit ();

    return 0;

}
