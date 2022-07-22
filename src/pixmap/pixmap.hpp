/*

    Author: Le Juez Victor
    Thanks to: Jacques-Olivier Lapeyre

    Version file: 01
    Date: 22/07/2022

*/

#ifndef __PIXMAP_HPP__
#define __PIXMAP_HPP__

typedef uint8_t  pixcmp; // cmp: component (of a pixel)
typedef uint32_t pixel;

void int_restrict (int* const value, int const vmin, int const vmax);

struct Rectbox {

  int x1; int y1;
  int x2; int y2;

  Rectbox ();
  Rectbox (int const x1, int const y1, int const x2, int const y2);
  Rectbox (Rectbox const &R); // Re-copy constructor
  void setter (int const x1, int const y1, int const x2, int const y2);

};

class Pixmap {

  private:
    int width;
    int height;
    pixel* datas;
    bool with_alpha;

    void init(int const width, int const height, bool const alpha);

    pixel get_average_of_neighbors (float const f_radius, int const cx, int const cy) const;

  public:
    Pixmap  ();
    Pixmap  (int const width, int const height);
    Pixmap  (int const width, int const height, int const background_color, bool const alpha);
    Pixmap  (Pixmap const & pix); // Re-copy constructor.
    ~Pixmap ();

    void blit_on_texture_centered (SDL_Texture* const texture, int const texture_w, int const texture_h) const;
    void blit_on_texture (SDL_Texture* const texture, int const x1, int const y1) const;

    void blit_line (Pixmap const &pix, int const line_number, int const x1, int const y1) const;

    void draw_rectbox_ns (Rectbox const &rect, pixel const color); // (non-secure) Does not test for overtaking but faster. ! (does not manage the alpha channel)!
    void draw_rectbox_ys (Rectbox const &rect, pixel const color); // (yes-secure) Test for pixmap overflows therefore slower.

    void fill (pixel const background_color); // Fill the pixmap with the desired color
    void grayscale (); // Converted to gray

    void vertical_gradient (Rectbox const &rr, pixel const c_width, pixel const c_height);

    int get_pixel_index (int const x, int const y) const;
    pixel* get_pixel_adress (int const x, int const y) const;

    void average_filter (float const radius); // Blur effect

    pixel read_pixel (int const x, int const y) const;
    void write_pixel (int const x, int const y, pixel const color);

    // NOTE: faire une seule fonction pour 'get_win_size() : ret -> w,h'

    int get_width  () const;
    int get_height () const;

    pixel* get_pixels () const;

};

inline void pixel_get_rgba (pixel const colour, pixcmp* const r, pixcmp* const g, pixcmp* const b, pixcmp* const a)
{
    *a = colour >> 24;
    *r = (colour >> 16) & 0xFF;
    *g = (colour >> 8) & 0xFF;
    *b = colour & 0xFF;
}

inline pixel make_pixel_rgb (pixcmp const r, pixcmp const g, pixcmp const b)
{
    return (0xFF000000) | (r << 16) | (g << 8) | b;
}

inline pixel make_pixel_rgba (pixcmp const r, pixcmp const g, pixcmp const b, pixcmp const a)
{
    return (a << 24) | (r << 16) | (g << 8) | b;
}

inline int Pixmap::get_pixel_index (int const x, int const y) const
{
    return y * width + x;
}

inline pixel* Pixmap::get_pixel_adress (int const x, int const y) const
{

    #ifdef DEBUG
        if ((x < 0) || (y < 0) || (x >= width) || (y >= height))
        {
            std::cout << "Pixmap::get_pixel_adress > Pixel ' " << x << " : " << y << " ' is out of limit." << std::endl;
            return NULL;
        }
    #endif

    return datas + y * width + x;

}

inline void pixel_put_alpha (pixel const front, pixel* const bottom)
{
    pixcmp r,g,b,a;
    pixel_get_rgba (front, &r, &g, &b, &a);

    pixcmp rr,gg,bb,aa;
    pixel_get_rgba (*bottom, &rr, &gg, &bb, &aa);

    float f1 = a / 255.0;
    float f2 = 1.0 - f1;

    r = rr * f2 + f1 * r;
    g = gg * f2 + f1 * g;
    b = bb * f2 + f1 * b;
    a = aa * f2 + f1 * a;

    *bottom = make_pixel_rgba (r, g, b, a);

}

#endif
