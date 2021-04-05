/*
Back.h: pretty things which appear in the background
(but add nothing to the gameplay)

Copyright (C) 2002 Mark Boyd

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

#include "prot.h"

//Parallax scrolling star
class star:public sprite1
{

public:
  static void init(void);
  static void cleanup(void);
  star(display *);
  ~star();

private:
  static graphics::bitmap starpics[6];
  
  void move(void);
  
  int depth;
  int spare;
};

class lawyer_storm;

class domed_city:public sprite1
{
public:
  domed_city(display *);
  ~domed_city();
  void get_target(int &,int &, int &);
  bool zap(int tower);
  int isdead();
  bool is_destroyed();

private:
  void move();
  void animate();
  void misc();
  
  struct tower
  {
    int left;
    int right;
    int height;
    int colour;
    int integrity;
    bool falling;
    float fheight;
    float fdy;
    tower(int l, int r, int h, int c)
      :left(l),right(r),height(h),colour(c),integrity((r-l)*h*10),falling(false),fheight(h),fdy(0.0){}
  };
  std::vector<tower> towers;
  int m_dome_integrity;
  bool m_pic_is_valid;
  bool m_alive;
  bool m_destroyed;
  int depth;
  int spare;
  bool m_is_under_attack;
};

class spark:public sprite1
{
public:
  static void make_sparks(int x, int y, display *, int depth);
  static void init();
  static void cleanup();
  spark(int x, int y, display *, int depth);

private:
  void move();
  void animate();
  int isdead() {return m_y>BOTTOM;}

  float m_fx, m_fy;
  float m_fdx, m_fdy;
  int m_spare;
  int m_depth;
  static graphics::bitmap spark_pic[16];
};

class lawyer_storm:public sprite1
{
public:
  lawyer_storm(domed_city *);
  ~lawyer_storm();
  int isdead();

private:
  static const int BOUNCE_HW = 10;
  static const int BOUNCE_HH = 6;
  enum
  {
  LS_DESCENDING,
  LS_ATTACKING,
  LS_ASCENDING,
  LS_FINISHED
  } state;

  void move();
  void animate();

  struct lawyer_pos
  {
    int x;
    int y;
    int dx;
    int dy;
    int xtarget;
    int ytarget;
    int target_tower;  
  };
  std::vector<lawyer_pos> lawyers;

  domed_city *pdc;
  int depth;
  int spare;
};

class back_planet:public sprite1
{
public:
  back_planet(display *);
  ~back_planet();
private:
  void move();

  int m_depth;
  int m_spare;
  graphics::bitmap m_pic;
  static graphics::bitmap *make_bitmap();
};
