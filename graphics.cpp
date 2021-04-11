/*
graphics.cpp - Graphics, oddly enough.

Copyright (C) 2001 Mark Boyd

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be fun to play,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Alternatively, try http://www.gnu.org, it's worth
a look even if you did get a copy of the GPL.
*/

// This includes "graphics" - which is good for getting into graphics mode
// ... and "bitmap", which is used for drawing
//
// We pretend to use a 256-colour pallette, which means colour arguments must be 0-255 (for convenience, they are ints)
//
//
// Internally, SDL has SDL_Surface-s - essentially bitmaps that live in normal memory,
// and SDL_Texture-s - eldritch entities which live in the graphics card, are hard to work with, and can
// be drawn incredibly quickly.  For the moment, we are just working with SDL_Surface-s. 
//
// TODO: some day, we might draw with textures, but we should still use surfaces for collision detection

#include <iostream>
#include <map>

#include "graphics.h"
#include "sdlinclude.h"

//Evil(?) global variables

//TODO: make these options or something?
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// g_pallette maps "colour" (int from 0 to 255) to RGB triples 
unsigned char g_pallette[256][3]={
    // RRR, GGG, BBB
    {  0,   0,   0}, //black
    {  0,   0, 128}, //1: blue
    {  0, 128,   0}, 
    {  0, 128, 128}, 
    {128,   0,   0}, //4: red 
    {128,   0, 128}, 
    {128, 128,   0}, 
    { 96,  96,  96}, //silver
    {128, 128, 128}, //grey
    {  0,   0, 255}, 
    {  0, 255,   0}, 
    {  0, 255, 255}, 
    {255,   0,  0}, 
    {255,   0, 255}, 
    {255, 255,   0},
    {255, 255, 255}, //white
};

Uint32 g_b_pallette[256]={};
std::map<Uint32, int> g_reverse_pallette_lookup;

bool g_is_graphics_mode=false;
SDL_Window* g_window=0;
graphics::bitmap g_font;
graphics::screen_ *the_screen=0;

class rect:public SDL_Rect{
		public:
		rect(int xx, int yy, int ww, int hh){x=xx;y=yy;w=ww;h=hh;}
		operator SDL_Rect *(){return static_cast<SDL_Rect *>(this);}
};

graphics::ssur::~ssur() {if (m_sur){SDL_FreeSurface(m_sur);}}


graphics::screen_ &graphics::init()
{

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
    return *the_screen; //TODO: This wil crash!
  }

  the_screen = new graphics::screen_();

  g_is_graphics_mode = true;

  return *the_screen;
}

void graphics::exit()
{
  delete the_screen;
  the_screen=0;

  IMG_Quit();
  SDL_Quit();

  g_is_graphics_mode = false;
}

bool graphics::is_graphics_mode()
{
  return g_is_graphics_mode;
}

void graphics::setcol(unsigned char n,unsigned char r,
	              unsigned char g,unsigned char b)
{
  g_pallette[n][0]=r;
  g_pallette[n][1]=g;
  g_pallette[n][2]=b;

  g_b_pallette[n]=SDL_MapRGB(SDL_GetWindowSurface(g_window)->format, r,g,b);
  if (g_reverse_pallette_lookup.find(g_b_pallette[n])==g_reverse_pallette_lookup.end())
    g_reverse_pallette_lookup[g_b_pallette[n]]=n;
}

using namespace graphics;

screen_::screen_()
{
  g_window = SDL_CreateWindow(
    "Protector",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    SCREEN_WIDTH, SCREEN_HEIGHT,
    SDL_WINDOW_SHOWN
  );

  
  if (g_window == NULL) {
    fprintf(stderr, "could not create window: %s\n", SDL_GetError());

    //TODO: throw an exception here.  Ugh.
    //return 1;
  }else{
    printf("screen ok\n");
  }

  for(int i=0; i<16; ++i)
    setcol(i, g_pallette[i][0], g_pallette[i][1], g_pallette[i][2]);
  for(int i=16; i<32; ++i)
    setcol(i, (i-16)*16, (i-16)*16, (i-16)*16);
  // Load our "font"
  // This, of course, violates the "purity" of Protector's original design, which hand-drew all bitmaps
  // But we were probably relying on Allegro providing this data somehow, which we can no longer do.
  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG){
    fprintf(stderr, "IMG_Init(IMG_INIT_PNG) failed");
    //TODO: ??
  }
  bitmap font(IMG_Load("system_font.png"));
  // This has white for the letters and transparent background, but that's 
  // actually the wrong way round.  We want to be able to write text in any colour.  We'll do this by
  // blitting the background onto a temporary bmp filled with the right colour, then blitting that
  // to the target.

  g_font=bitmap(font.width(), font.height());
  g_font.rect(0,0,font.width(), font.height(), 13); //Fill with "magic pink"
  blit(font, g_font, 0,0,0,0, font.width(), font.height());
  g_font.set_transparency(15);
  // g_font now has a transparent (secretly white) foreground and a magic pink background
  
  m_surp=boost::shared_ptr<ssur>(new ssur(SDL_GetWindowSurface(g_window)));
}

screen_::~screen_()
{
  // We didn't create this window, so we shouldn't be deleting it 
  m_surp->disarm();

  if (g_window){
    SDL_DestroyWindow(g_window);
    g_window=0;
  }
}

int screen_::width() const
{
  return SCREEN_WIDTH;
}

int screen_::height() const
{
  return SCREEN_HEIGHT;
}

void screen_::kick(){
  SDL_UpdateWindowSurface(g_window);
}

boost::shared_ptr<graphics::ssur> i_make_surface(int w, int h){

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
    Uint32 rmask, gmask, bmask, amask;
if (SDL_BYTEORDER == SDL_BIG_ENDIAN){
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
}else{
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
}
SDL_Surface *sur = SDL_CreateRGBSurface(0, w,h, 32, rmask, gmask, bmask, amask);
SDL_SetSurfaceBlendMode(sur, SDL_BLENDMODE_NONE);
                           
return boost::shared_ptr<graphics::ssur>(new graphics::ssur(sur));
}


bitmap::bitmap()
{
  
}

bitmap::bitmap(int w, int h)
:m_surp(i_make_surface(w,h)){
  ::clear(*this);
}

bitmap::bitmap(SDL_Surface *s)
:m_surp(new graphics::ssur(s))
{
}

void bitmap::set_transparency(int col){
  SDL_SetColorKey((*m_surp)(), SDL_TRUE, g_b_pallette[col]);
}

void bitmap::clear(int col)
{
  rect(0,0,width(),height(), col);
}

int bitmap::height() const
{
  return (*m_surp)()->h;
}

int bitmap::width() const
{
  return (*m_surp)()->w;
}

// These 2 functions are from the SDL documentation

Uint32 sdl_getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void sdl_putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

//std::cout<<"bpp is "<<bpp<<"\n";

    switch(bpp) {    
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
   //std::cout<<"pixel: "<<x<<","<<y<<": "<<pixel<<"\n";
        *(Uint32 *)p = pixel;
        break;
    }
}

int bitmap::getpixel(int x, int y) const
{
  //std::cout<<"mustlock: "<<SDL_MUSTLOCK((*m_surp)())<<"\n";

  // This function is very evil.
  // If possible, use haspixel instead
  //std::cout<<"Looking at ("<<x<<", "<<y<<") w:"<<width()<<" h= "<<height()<<"\n";
  if (x>=width()) throw x;
  if (y>=height()) throw y;


  Uint8 r, g, b;
  SDL_GetRGB(sdl_getpixel((*m_surp)(), x, y), (*m_surp)()->format,  &r, &g, &b);
  for (int i=0; i<156; ++i){
    if (g_pallette[i][0]==r && g_pallette[i][1]==g && g_pallette[i][2]==b) return i;

  }

    std::cout<<"Found nothing!\n";
    return 0; //This is bad
}



void bitmap::putpixel(int x, int y, int col)
{
  //std::cout<<"putting "<<col<<"\n";
  return sdl_putpixel((*m_surp)(), x, y, SDL_MapRGB((*m_surp)()->format, g_pallette[col][0], g_pallette[col][1], g_pallette[col][2]));
}

void bitmap::drawtext(const char *str, int left, int top, int colour, int bg)
{
  // Our magic font bitmap has 8*8 charactes separated by 1 pixel of space
  int ROWS=16;
  int SPACE=1;
  int W=8,H=8;

  int WW=W+SPACE, HH=H+SPACE;

  bitmap temp(8,8);
  temp.set_transparency(13);

  for(int i=0; str[i]; ++i){
    char s=str[i];

    temp.clear(colour);
    blit(g_font, temp, SPACE+(s%ROWS)*WW, SPACE+(s/ROWS)*HH, 0,0,W,H);
    blit(temp, *this,0,0, left+i*W, top, W, H);
  }

  //TODO: colour?  bg?
}

void bitmap::drawtext_justify(const char *str, int x1, int x2, int y, int diff, int colour, int bg) 
{
  //textout_justify_ex(m_pimpl->pb, font, str, x1, x2, y, diff, colour,bg); 
}

void bitmap::rect(int x1, int y1, int x2, int y2, int col)
{
  SDL_FillRect((*m_surp)(), ::rect(x1,y1,x2-x1, y2-y1), SDL_MapRGB((*m_surp)()->format, g_pallette[col][0], g_pallette[col][1], g_pallette[col][2])); 
}

void bitmap::circle(int x, int y, int r, int col)
{
  //::circle(m_pimpl->pb, x, y, r, col);
}


void bitmap::rectfill(int x1, int y1, int x2, int y2, int col)
{
  SDL_FillRect((*m_surp)(), ::rect(x1,y1,x2-x1, y2-y1), SDL_MapRGB((*m_surp)()->format, g_pallette[col][0], g_pallette[col][1], g_pallette[col][2])); 
}

void bitmap::circlefill(int x, int y, int r, int col)
{
  //::circlefill(m_pimpl->pb, x, y, r, col);
}
 
void bitmap::vline(int x, int y1, int y2, int col)
{
  //::vline(m_pimpl->pb, x, y1, y2, col);
}

void bitmap::hline(int x1, int y, int x2, int col)
{
  //::hline(m_pimpl->pb, x1, y, x2, col);

}




void graphics::blit(const bitmap &from, bitmap &to, int x1, int y1, int x2, int y2, int w, int h)
{
  //blit(from.m_pimpl->pb, to.m_pimpl->pb, x1, y1, x2, y2, w, h);
  SDL_BlitSurface((*from.m_surp)(), rect(x1,y1,w,h), (*to.m_surp)(), rect(x2,y2,w,h));
}



void graphics::masked_blit(const bitmap &from, bitmap &to, int x1, int y1, int x2, int y2, int w, int h)
{
  //masked_blit(from.m_pimpl->pb, to.m_pimpl->pb, x1, y1, x2, y2, w, h);
  SDL_BlitSurface((*from.m_surp)(), rect(x1,y1,w,h), (*to.m_surp)(), rect(x2,y2,w,h));
}

void graphics::clear(bitmap &b) 
{
  b.clear(0);
}


void graphics::make_bitmap(graphics::bitmap &b, int w, int h)
{
  bitmap temp(w,h);
  b=temp;
}




/*
void graphics::set_colours(int base, unsigned char (*colours)[3], int n)
{
PALETTE pal;
get_palette(pal);
for(int a=0;a<32;a++)
  {
  pal[128+(a+n)%128].r=colours[a][0];
  pal[128+(a+n)%128].g=colours[a][1];
  pal[128+(a+n)%128].b=colours[a][2];
  }
set_palette(pal);
}
*/

