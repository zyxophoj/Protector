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

//#include "sdlinclude.h"

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

struct SDL_Surface;

namespace graphics
{

//Stupid surface
//Adapter for SDL_Surface that knows how to clean it up
class ssur:public boost::noncopyable
{
    public:
	ssur(SDL_Surface *sur):m_sur(sur){}
	SDL_Surface * operator()(){return m_sur;}
	~ssur(); //{if (m_sur){SDL_FreeSurface(m_sur);}}
        void disarm(){m_sur=0;} //Evil function of evil, do not use this.
    private:
        SDL_Surface *m_sur;
};

class bitmap
{
friend void blit(const bitmap &, bitmap &, int, int, int, int, int, int);
friend void masked_blit(const bitmap &, bitmap &, int, int, int, int, int, int);

public:
  bitmap();
  bitmap(SDL_Surface *);
  bitmap(int w, int h);
  virtual ~bitmap(){};

  virtual int height() const;
  virtual int width() const;  
  
  int getpixel(int x, int y) const;
 
  void set_transparency(int);

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
   boost::shared_ptr<ssur> m_surp;

};

class screen_:public bitmap
{
  //This can be copied, but you are just copying a tiny reference to a single underlying resource.

private:
  // Call graphics::init to construct a screen
  screen_();
  friend screen_ &graphics::init();

public:
  ~screen_();

  virtual int height() const;
  virtual int width() const;  

  void kick(); //Force queued writes to actually happen
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

screen_ &init();
void exit();

bool is_graphics_mode();

}



#endif
