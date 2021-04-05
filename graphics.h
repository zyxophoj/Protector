/*
Graphics.h - interface for the graphics library

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

#ifndef GRAPHICS_H
#define GRAPHICS_H


class bitmap_impl;

namespace graphics
{


//Caveat programmer:  bitmap has the same nasty copy semantics as auto_ptr!
class bitmap
{
friend void blit(const bitmap &, bitmap &, int, int, int, int, int, int);
friend void masked_blit(const bitmap &, bitmap &, int, int, int, int, int, int);

public:
  bitmap();
  bitmap(int w, int h);
  virtual ~bitmap();

  bitmap(bitmap &other);
  const bitmap &operator =(bitmap &);

  virtual int height() const;
  virtual int width() const;  
  
  int getpixel(int x, int y) const;
 

  void putpixel(int x, int y, int col);
  void clear(int col);
  void drawtext(const char *, int, int, int, int bg=-1);
  void drawtext_justify(const char *, int, int, int, int, int, int bg=-1);
  void rect(int x1, int y1, int x2, int y2, int col);
  void circle(int x, int y, int r, int col);
  void rectfill(int x1, int y1, int x2, int y2, int col);
  void circlefill(int x, int y, int r, int col);
  void vline(int x, int y1, int y2, int col);
  void hline(int x1, int y, int x2, int col);


protected:
  bitmap_impl *m_pimpl;

//The infamous auto_ptr hack
private:
  struct bitmap_ref
  {
    bitmap_impl *m_bi;
  bitmap_ref(bitmap_impl *bi):m_bi(bi){}
  };
public:
  operator bitmap_ref(){bitmap_impl *bi=m_pimpl; m_pimpl=0; return bitmap_ref(bi);}
  bitmap(bitmap_ref br):m_pimpl(br.m_bi){}
};

class screen_:public bitmap
{
public:
  screen_();
  ~screen_();

  virtual int height() const;
  virtual int width() const;  
private:
  //Not implemented - copying this just doesn't make sense.
  screen_(const screen_ &);
  const screen_ &operator=(const screen_ &);
};



void blit(const bitmap &, bitmap &, int, int, int, int, int, int);
void masked_blit(const bitmap &, bitmap &, int, int, int, int, int, int);

//nonmember nonfriends
void clear(bitmap &);
void make_bitmap(graphics::bitmap &, int w, int h);

//void set_text_mode(int);
void setcol(unsigned char n,unsigned char r,
            unsigned char g,unsigned char b);

void set_colours(int base, unsigned char (*colours)[3], int n);

void init();
void exit();

bool is_graphics_mode();

}



#endif
