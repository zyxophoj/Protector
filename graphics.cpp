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

#include <iostream>
#include<allegro.h>
#include "graphics.h"

using namespace graphics;

class bitmap_impl
{
public:
  BITMAP *pb;
}; 

bitmap::bitmap()
{
  m_pimpl = 0;
}

bitmap::bitmap(int w, int h)
{
  m_pimpl = new bitmap_impl;
  m_pimpl->pb = create_bitmap(w,h);
  ::clear(m_pimpl->pb);
}

bitmap::~bitmap()
{
  if (m_pimpl)
  {
    //This is a really horrible hack...
    //There are static bitmaps which get destroyed after graphics_exit is called.
    //destroy_bitmap sometimes crashes if it's called when we're not in graphics mode
    //so don't call it!  Techically we're leaking stuff here, but since it only happens
    //when the program exits, who cares?  It's better than crashing on exit.
    //(I care.  todo: come up with a better way of doing this)
    if (is_graphics_mode())
    {
      destroy_bitmap(m_pimpl->pb);
    }
    delete m_pimpl;
  }
}

bitmap::bitmap(bitmap &other)
{
  m_pimpl = other.m_pimpl;
  
  other.m_pimpl=0;
}

const bitmap &bitmap::operator =(bitmap &other)
{
  if (m_pimpl)
  {
    destroy_bitmap(m_pimpl->pb);
    delete m_pimpl;
  }

  m_pimpl = other.m_pimpl;
  
  other.m_pimpl=0;

  return *this;
}

void bitmap::clear(int col)
{
  clear_to_color(m_pimpl->pb, col);
}

int bitmap::height() const
{
  return m_pimpl->pb->h;
}

int bitmap::width() const
{
  return m_pimpl->pb->w;
}

int bitmap::getpixel(int x, int y) const
{
  return ::getpixel(m_pimpl->pb, x, y);
}

//todo: think about which of htese need to be members

void bitmap::putpixel(int x, int y, int col)
{
  return ::putpixel(m_pimpl->pb, x, y, col);
}

void bitmap::drawtext(const char *str, int left, int top, int colour, int bg)
{
  textout_ex(m_pimpl->pb, font, str, left, top, colour,bg); 
}

void bitmap::drawtext_justify(const char *str, int x1, int x2, int y, int diff, int colour, int bg) 
{
  textout_justify_ex(m_pimpl->pb, font, str, x1, x2, y, diff, colour,bg); 
}

void bitmap::rect(int x1, int y1, int x2, int y2, int col)
{
  ::rect(m_pimpl->pb, x1, y1, x2, y2, col);
}

void bitmap::circle(int x, int y, int r, int col)
{
  ::circle(m_pimpl->pb, x, y, r, col);
}


void bitmap::rectfill(int x1, int y1, int x2, int y2, int col)
{
  ::rectfill(m_pimpl->pb, x1, y1, x2, y2, col);
}

void bitmap::circlefill(int x, int y, int r, int col)
{
  ::circlefill(m_pimpl->pb, x, y, r, col);
}
 
void bitmap::vline(int x, int y1, int y2, int col)
{
  ::vline(m_pimpl->pb, x, y1, y2, col);
}

void bitmap::hline(int x1, int y, int x2, int col)
{
  ::hline(m_pimpl->pb, x1, y, x2, col);

}

void graphics::blit(const bitmap &from, bitmap &to, int x1, int y1, int x2, int y2, int w, int h)
{
  blit(from.m_pimpl->pb, to.m_pimpl->pb, x1, y1, x2, y2, w, h);
}

void graphics::masked_blit(const bitmap &from, bitmap &to, int x1, int y1, int x2, int y2, int w, int h)
{
  masked_blit(from.m_pimpl->pb, to.m_pimpl->pb, x1, y1, x2, y2, w, h);
}

void graphics::clear(bitmap &b) 
{
  b.clear(0);
}


//todo: remove this!
//BITMAP *bitmap::get_BITMAP() const {return m_pimpl->pb;}

void graphics::make_bitmap(graphics::bitmap &b, int w, int h)
{
  bitmap temp(w,h);
  b=temp;
}

bool graphics_mode(false);

screen_::screen_()
{
  acquire_screen();
  m_pimpl = new bitmap_impl;
  m_pimpl->pb = ::screen; 
}

screen_::~screen_()
{
  delete m_pimpl;
  m_pimpl = 0;
  release_screen();
}

//A bit of a hack - Allegro allows the "screen" bitmap to be larger than the actual screen, 
//due to virtual screens.  Protector does not need or understand this, so we override
//width() and height()

int screen_::width() const
{
  return SCREEN_W;
}

int screen_::height() const
{
  return SCREEN_H;
}

//text_mode is deprecated in allegro.  Luckily, we never used this!
/*void graphics::set_text_mode(int mode)
{
  ::text_mode(mode);
}*/

void graphics::setcol(unsigned char n,unsigned char r,
	              unsigned char g,unsigned char b)
{
RGB col;
col.r=r;col.g=g;col.b=b;
set_color(n,&col);
}

void graphics::init()
{
  allegro_init();
  set_config_file("prot.cfg");
  install_keyboard();
  install_timer();
  if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN,640,480,0,0) < 0 &&
      set_gfx_mode(GFX_AUTODETECT,640,480,0,0))
  {
    std::cout << "No graphics mode!\n";
    ::exit(-1);
  }

  graphics_mode = true;
}

void graphics::exit()
{
  allegro_exit();

  graphics_mode = false;
}

bool graphics::is_graphics_mode()
{
  return graphics_mode;
}


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
