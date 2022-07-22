/*

    Author: Le Juez Victor
    Thanks to: Jacques-Olivier Lapeyre

    Version file: 01
    Date: 22/07/2022

*/

#include <iostream>
#include <SDL2/SDL.h>

#include "pixmap.hpp"

/* VARIOUS MATHEMATICAL TOOLS */

void int_restrict (int* const value, int const vmin, int const vmax)
{
    if (*value < vmin) *value = vmin;
    if (*value > vmax) *value = vmax;
}

/* RECTBOX STRUCTURE */

void Rectbox::setter (int const x1, int const y1, int const x2, int const y2)
{
    this -> x1 = x1;
    this -> y1 = y1;
    this -> x2 = x2;
    this -> y2 = y2;
}

Rectbox::Rectbox()
{
    x1 = y1 = x2 = y2 = -1; // all equal to -1
}

Rectbox::Rectbox (Rectbox const &r)
{
    x1 = r.x1;
    y1 = r.y1;
    x2 = r.x2;
    y2 = r.y2;
}

Rectbox::Rectbox (int const x1, int const y1, int const x2, int const y2)
{
    this -> setter (x1, y1, x2, y2);
}


/* CLASS PIXMAP */

Pixmap::Pixmap ()
{
    width  = -1;
    height = -1;
    datas  = nullptr;
    with_alpha = false;
}

void Pixmap::init (int const width, int const height, bool const alpha)
{
    this -> width  = width;
    this -> height = height;
    this -> datas  = new pixel[width*height];
    this -> with_alpha = alpha;
}

Pixmap::Pixmap (int const width, int const height)
{
    init (width, height, true);
}

Pixmap::Pixmap (int const width, int const height, int const background_color, bool const alpha)
{
    init (width, height, alpha);
    fill (background_color);
}

Pixmap::Pixmap (Pixmap const &pix) // Re-copy constructor
{
    width  = pix.width;
    height = pix.height;
    with_alpha  = pix.with_alpha;
    datas  = new pixel [width*height];
    for (int i = 0; i < width * height; i++)
      datas[i] = pix.datas[i];
}

Pixmap::~Pixmap ()
{

    #ifdef DEBUG
        std::cout << "Destructor of pixmap ( " << width << " x " << height << " ) is called." << std::endl;
    #endif

    delete[] datas;

}

void Pixmap::blit_on_texture_centered (SDL_Texture* const texture, const int texture_w, const int texture_h) const
{
    int x1 = (texture_w - width)  / 2;
    int y1 = (texture_h - height) / 2;
    blit_on_texture (texture, x1, y1);
}

void Pixmap::blit_on_texture (SDL_Texture* const texture, int const x1, int const y1) const
{
    SDL_Rect rect = {x1, y1, width, height};
    SDL_UpdateTexture (texture, &rect, datas, width * sizeof (Uint32));
}

void Pixmap::blit_line (Pixmap const &pix, int const line_number, int const x1, int const y1) const
{

    pixel* src_ptr    = datas + line_number * width;
    pixel* target_ptr = pix.datas + y1 * pix.width + x1;

    if (!with_alpha)
    {
        for (int x = 0; x < width; x++, src_ptr++, target_ptr++)
            *target_ptr = *src_ptr;
    }
    else
    {
        for (int x = 0; x < width; x++, src_ptr++, target_ptr++)
            pixel_put_alpha (*src_ptr, target_ptr);
    }

}

void Pixmap::draw_rectbox_ns (Rectbox const &rect, pixel const color)
{
    for (int y = rect.y1; y <= rect.y2; y++)
    {
        for (int x = rect.x1; x <= rect.x2; x++)
        {
            datas[y * width + x] = color;
        }
    }
}

void Pixmap::draw_rectbox_ys (Rectbox const &rect, pixel const color)
{

    Rectbox r (rect.x1, rect.y1, rect.x2, rect.y2);

    int_restrict (&r.x1, 0, width  - 1);
    int_restrict (&r.y1, 0, height - 1);
    int_restrict (&r.x2, 0, width  - 1);
    int_restrict (&r.y2, 0, height - 1);

    if ((r.x1 >= r.x2) || (r.y1 >= r.y2))
    {

        #ifdef DEBUG
            std::cout << "Pixmap::draw_rectbox > Empty rect." << std::endl;
        #endif

        return;

    }

    if (with_alpha)
    {
        for (int y = r.y1; y <= r.y2; y++)
        {

            pixel* p_ptr = get_pixel_adress (r.x1, y);

            for (int x = r.x1; x <= r.x2; x++)
            {
                pixel_put_alpha (color, p_ptr); ++p_ptr;
            }

        }
    }
    else
    {
        for (int y = r.y1; y <= r.y2; y++)
        {

            pixel* p_ptr = get_pixel_adress (r.x1, y);

            for (int x = r.x1; x <= r.x2; x++)
            {
                *p_ptr = color; ++p_ptr;
            }
        }
    }

}

void Pixmap::fill (pixel const background_color)
{
    for (int i = 0; i < width * height; i++)
        datas[i] = background_color;
}

void Pixmap::grayscale () // convert to gray
{
    for (int i = 0; i < width * height; i++)
    {
        pixcmp r, g, b, a;
        pixel_get_rgba (datas[i], &r, &g, &b, &a);
        pixcmp grey = 0.59 * g + 0.3 * r + 0.11 * b;
        datas[i] = make_pixel_rgba (grey, grey, grey, a);
    }
}

void Pixmap::vertical_gradient (Rectbox const &rr, pixel const c_up, pixel const c_down)
{

    Rectbox R (rr);

    int_restrict (&R.x1, 0, width  - 1);
    int_restrict (&R.y1, 0, height - 1);
    int_restrict (&R.x2, 0, width  - 1);
    int_restrict (&R.y2, 0, height - 1);

    pixcmp r_up, g_up, b_up, a_up;
    pixcmp r_down, g_down, b_down, a_down;

    pixel_get_rgba (c_up, &r_up, &g_up, &b_up, &a_up);
    pixel_get_rgba (c_down, &r_down, &g_down, &b_down, &a_down);

    int pseudo_height = R.y2 - R.y1;

    if (!with_alpha)
    {
        for (int y = R.y1; y <= R.y2; y++)
        {

            float f1 = (y - R.y1) / (pseudo_height + 0.0);
            float f2 = 1.0 - f1;

            pixcmp r = r_up * f2 + f1 * r_down;
            pixcmp g = r_up * f2 + f1 * g_down;
            pixcmp b = r_up * f2 + f1 * b_down;
            pixcmp a = r_up * f2 + f1 * a_down;

            pixel color = make_pixel_rgba (r, g, b, a);
            int index = get_pixel_index (R.x1, y);

            for (int x = R.x1; x <= R.x2; x++)
            {
                datas[index] = color; ++index;
            }

        }
    }
    else
    {
        for (int y = R.y1; y <= R.y2; y++)
        {

            float f1 = (y - R.y1) / (pseudo_height + 0.0);
            float f2 = 1.0 - f1;

            pixcmp r = r_up * f2 + f1 * r_down;
            pixcmp g = r_up * f2 + f1 * g_down;
            pixcmp b = r_up * f2 + f1 * b_down;
            pixcmp a = r_up * f2 + f1 * a_down;

            pixel color = make_pixel_rgba (r, g, b, a);
            int index = get_pixel_index (R.x1, y);

            for (int x = R.x1; x <= R.x2; x++)
            {
                pixel_put_alpha (color, datas + index); ++index;
            }

        }
    }
}

pixel Pixmap::get_average_of_neighbors (float const f_radius, int const cx, int const cy) const
{

    int sum_r = 0, sum_g = 0, sum_b = 0, sum_a = 0;

    int radius = (int) f_radius;

    int x1 = cx - radius;
    int y1 = cy - radius;
    int x2 = cx + radius;
    int y2 = cy + radius;

    int_restrict (&x1, 0, width  - 1);
    int_restrict (&y1, 0, height - 1);
    int_restrict (&x2, 0, width  - 1);
    int_restrict (&y2, 0, height - 1);

    int rect_w = x2 - x1 + 1;
    int rect_h = y2 - y1 + 1;
    int area = rect_w * rect_h;

    for (int y = y1; y <= y2; y++)
    {

        int index = get_pixel_index (x1, y);

        for (int x = x1; x <= x2; x++)
        {

            pixcmp r,g,b,a;
            pixel_get_rgba (datas[index], &r, &g, &b, &a);

            sum_r += r;
            sum_g += g;
            sum_b += b;
            sum_a += a;

            ++index;

        }
    }

    #ifdef DEBUG
        if (area == 0)
        {

            std::cout << "Pixmap::get_average_of_neighbors > Null area, non-computable mean." << std::endl;

            return 0xFFFF00FF;

        }
    #endif

    sum_r /= area;
    sum_g /= area;
    sum_b /= area;
    sum_a /= area;

    return make_pixel_rgba ((pixcmp) sum_r, (pixcmp) sum_g, (pixcmp) sum_b, (pixcmp) sum_a);

}

void Pixmap::average_filter (float const radius)
{

    Pixmap clone = Pixmap (*this);

    int i = 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            datas[i] = clone.get_average_of_neighbors (radius, x, y); ++i;
        }
    }

}

pixel Pixmap::read_pixel (int const x, int const y) const
{

    if ((x < 0) || (y < 0) || (x >= width) || (y >= height))
    {

        #ifdef DEBUG
            std::cout << "Pixmap::read_pixel > Pixel ' " << x << " : " << y << " ' is out of limit." << std::endl;
        #endif

        return 0;

    }

    return datas[y * width + x];

}

void Pixmap::write_pixel (int const x, int const y, pixel const color)
{
    if ((x < 0) || (y < 0) || (x >= width) || (y >= height))
    {

        #ifdef DEBUG
            std::cout << "Pixmap::write_pixel > Pixel ' " << x << " : " << y << " ' is out of limit." << std::endl;
        #endif

        return;
    }

    datas[y * width + x] = color;

}

int Pixmap::get_width () const
{
    return width;
}

int Pixmap::get_height () const
{
    return height;
}

pixel* Pixmap::get_pixels () const
{
    return datas;
}

