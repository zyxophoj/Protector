/*
Back.cpp: pretty things which appear in the background
(but add nothing to the gameplay)

Copyright (C) 2002,2006 Mark Boyd

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

#include "back.h"
#include <cmath>
#include "stuff.h"

//I can't do without these!
using std::min;
using std::max;

graphics::bitmap star::starpics[6];

void star::move()
{
//
//Think:
//dx = pdisp->getcdx()-pdisp->getcdx()/depth;
//x=RU(x+dx);
//
//Unfortunately, this would create ikky rounding errors (try it!)
//so to get round this, "spare" remembers the rounding error
//(actually -depth*(rounding error), to avoid floating-point)
//which is then subtracted the next time round.
int spx = spare-pdisp->getcdx();
m_dx=spx/depth+pdisp->getcdx();
spare=spx-spx/depth*depth;
m_x=RU(m_x+m_dx);
}

star::star(display *d)
:sprite1(RANDX,rand()%400,0,0,starpics[0],d)
{
int r;
spare=0;
r=rand()%6;
depth=(2+r);
spare = rand()%depth;
pic=&starpics[5-r];
}

star::~star(){}

void star::init()
{
  for(int a=0;a<6;a++)
  {
    make_bitmap(starpics[a],1,1);
    starpics[a].putpixel(0,0,18+2*a);
  }
}

void star::cleanup()
{
}

//bitmap shenanigans here - a vile hack!
domed_city::domed_city(display *d)
:sprite1(RANDX, 0, 0, 0, *new graphics::bitmap(rand()%150+60, 15+rand()%15), d)
{ 
  pdisp=d;
  m_y=BOTTOM-h;

  for(int a=w/6;a;a--)
  {
    int width = rand()%4+1;
    int left  = rand()%(w-width);
    int max_height = min(2*left*h/w,2*(w-(left+width))*h/w);
    towers.push_back(tower(left, left+width, randint(max_height), 18+rand()%6));    
  }
  m_pic_is_valid = false;
  m_dome_integrity = 1000;
  m_alive = true;
  m_destroyed = false;

  spare=0;
  depth = 20;
  m_is_under_attack = false;

}

domed_city::~domed_city()
{
  delete pic;
}


void domed_city::animate()
{
  if (m_pic_is_valid) return;
  
  //It's not valid, so redraw it
  m_pic_is_valid = true;
  int alive = false;
  graphics::bitmap &pic_=const_cast<graphics::bitmap &>(*pic);
  clear(pic_);
  if(m_dome_integrity)
  {
    //Arc goes through bottom corners and centre of top edge
    int r = (w*w/4+h*h) / (2*h);
    pic_.circle(w/2, r ,r, 16+m_dome_integrity/100);
  }
  for (std::vector<tower>::iterator i=towers.begin(); i!=towers.end(); ++i)
  {
    if (!i->falling)
    {
      alive = true;
    }
    else if (i->height>0) 
    { 
      i->fdy += GRAVITY/depth;
      i->fheight -= i->fdy;
      int new_height = static_cast<int>(i->fheight);
      if (new_height != i->height)
      {    
        i->height = max(new_height, 0);
      }
      m_pic_is_valid = false;
      alive = true;
    }
    pic_.rectfill(i->left, h-i->height, i->right, h, i->colour);
  }
  m_alive = alive;
}

int domed_city::isdead(){return !m_alive;}
bool domed_city::is_destroyed(){return m_destroyed;}
void domed_city::move()
{
int spx = spare-pdisp->getcdx();
m_dx=spx/depth+pdisp->getcdx();
spare=spx-spx/depth*depth;
m_x=RU(m_x+m_dx);
}

void domed_city::misc()
{
if (!(rand()%10000) && !m_is_under_attack)
  {
  (void)(new lawyer_storm(this));
  m_is_under_attack = true;
  }
}


void domed_city::get_target(int &gx, int &gy, int &gn)
{
  if(m_dome_integrity > 0)
  {
    //Random point on the dome
    gx = rand()%w;
    double r = w*w/(8.0*h)+h;
    gy = static_cast<int>( r-std::sqrt(r*r-0.25*(2*gx-w)*(2*gx-w)) );
    gy=min(gy,h/2);
    gn=-1;
  }
  else
  {
    //Centre of a tower.  This could use some work.
    int surviving_towers=0;
    for (std::vector<tower>::iterator i=towers.begin(); i!=towers.end(); ++i)
      surviving_towers += !i->falling;
    if (surviving_towers == 0)
    {
      //hack: target somewhere above top of screen
      gx=rand()%w;
      gy=-BOTTOM; 
      gn=-2;
    }
    else
    {
      int target = rand()%surviving_towers;
      for(unsigned int i=0; i!=towers.size(); i++)
      {
        if(!towers[i].falling)
        {
          if(target)
          {
            target--;
          }
          else
          {
            gx=towers[i].left+rand()%(towers[i].right-towers[i].left);
            gy=h-towers[i].height/2;
            gn=i;
            return;
          }
        }
      }
    }
  }
}

bool domed_city::zap(int tower_num)
{  
  if(tower_num == -1)
  {
    //Dome
    m_dome_integrity--;
    m_pic_is_valid = false;
    return (m_dome_integrity <= 0);
  }
  else
  {
    towers[tower_num].integrity--;
    if (towers[tower_num].integrity <= 0)
    {
      towers[tower_num].falling = true;
      m_pic_is_valid = false;
      //Was this the last tower?
      m_destroyed = true;
      for (unsigned int i=0; i<towers.size(); i++)
        if (!towers[i].falling)
        {
          m_destroyed = false;
          goto stop;
        }
      stop:
      return true;
    }
  }
  return false;
}


lawyer_storm::lawyer_storm(domed_city *pdc_)
:sprite1(RU(pdc_->getx()-BOUNCE_HW), 0, -(pdc_->h+2*BOUNCE_HH), 0, 
         *(new graphics::bitmap(pdc_->w+2*BOUNCE_HW, pdc_->h+2*BOUNCE_HH)), pdc_->get_disp())
{

  pdc=pdc_;
  
  int n = min(40, rand()%(3*w));
  lawyers.reserve(n);
  for(;n;n--)
  {
    lawyer_pos l;
    l.x = rand()%w;
    l.y = rand()%h;
    l.dx = (rand()%2)?1:-1;
    l.dy = 1;  
    pdc->get_target(l.xtarget, l.ytarget, l.target_tower);

    lawyers.push_back(l); 
  }
  state = LS_DESCENDING;
  
  spare=0;
  depth = 20;
}

lawyer_storm::~lawyer_storm()
{
   delete pic;
}

void lawyer_storm::move()
{
  int spx = spare-pdisp->getcdx();
  m_dx=spx/depth+pdisp->getcdx();
  spare=spx-spx/depth*depth;
  m_x=RU(m_x+m_dx);

  switch(state)
  {
  case LS_DESCENDING:
    m_y++;
    m_dy=1;
    if (m_y+h >= BOTTOM+BOUNCE_HH) state = LS_ATTACKING;
    break;
  
  case LS_ASCENDING:
    m_y--;
    m_dy=-1;
    if (m_y+h<=0) state = LS_FINISHED;
    break;

  default:
    m_dy=0;
  }
  
  for(std::vector<lawyer_pos>::iterator i=lawyers.begin(); i!=lawyers.end(); i++)
  {
    //Position of target relative to lawyer
    int targ_x = i->xtarget + BOUNCE_HW - i->x;
    int targ_y;
    if (pdc) 
      targ_y = pdc->gety()+i->ytarget - (m_y+i->y);
    else
      targ_y = -2*BOUNCE_HH; //City already destroyed, go up!
    bool close = true;
    if(targ_x >  BOUNCE_HW) {i->dx =  1; close = false;}
    if(targ_x < -BOUNCE_HW) {i->dx = -1; close = false;}
    if(targ_y >  BOUNCE_HH) {i->dy =  1; close = false;}
    if(targ_y < -BOUNCE_HH) {i->dy = -1; close = false;}
    i->x += i->dx;
    i->y += i->dy-m_dy;
    if (close && !(rand()%5))
    {
      //visible zap effect
      if (!(rand()%20)) spark::make_sparks(pdc->getx()+i->xtarget, pdc->gety()+i->ytarget, pdisp, depth); 

      if(pdc && pdc->zap(i->target_tower))
      {
        int destroyed_tower = i->target_tower;
        for(std::vector<lawyer_pos>::iterator j=lawyers.begin(); j!=lawyers.end(); j++)
        {
          if (j->target_tower == destroyed_tower)
          {
            pdc->get_target(j->xtarget, j->ytarget, j->target_tower);
          }  
        }
        if(pdc->is_destroyed())
        {
          state = LS_ASCENDING;
          pdc=0;
        }
      }
    }
  }  
}

void lawyer_storm::animate()
{
   graphics::bitmap &b=const_cast<graphics::bitmap &>(*pic);
   clear(b);
   for (std::vector<lawyer_pos>::iterator i=lawyers.begin(); i!=lawyers.end(); ++i)
     b.putpixel(i->x, i->y, 5);
}

int lawyer_storm::isdead(){return state == LS_FINISHED;}


graphics::bitmap spark::spark_pic[16];

void spark::make_sparks(int x, int y, display *pd, int depth)
{
  for(int n=rand()%5+3; n;n--) (void)(new spark(x, y, pd, depth));
}

spark::spark(int x, int y, display *pd, int depth)
:sprite1(x, y, 0, 0, spark_pic[0], pd)
{
  float theta = 1.0*2*M_PI*rand()/RAND_MAX;
  float r = 0.15*rand()/RAND_MAX;
  m_fdx = r*cos(theta);
  m_fdy = r*sin(theta); 
  m_fx = m_x;
  m_fy = m_y;
  m_depth = depth;
  m_spare=0;
} 

void spark::move()
{
  m_fx+=m_fdx;
  m_fy+=m_fdy;
  m_fdy+=GRAVITY/m_depth;
  if(m_fx<0) m_fx += XWIDTH;
  if(m_fx>XWIDTH) m_fx -= XWIDTH;
  int oldx = m_x;
  int oldy = m_y;
  
  m_y = static_cast<int>(m_fy);
  m_dy = m_y-oldy;

  m_x = static_cast<int>(m_fx);
  m_dx = R(m_x-oldx);
  m_x = RU(m_x);
  
  //Now adjust for depth
  int spx = m_spare-pdisp->getcdx();
  int extra=spx/m_depth+pdisp->getcdx();
  m_spare=spx-spx/m_depth*m_depth;

  m_x=RU(m_x+extra);
  m_dx += extra;
  m_fx += extra;
}

void spark::animate()
{
  pic = &spark_pic[rand()%16];
}

void spark::init()
{
  for(int i=0; i<16; i++)
  {
    make_bitmap(spark_pic[i],1,1);
    spark_pic[i].clear(16+i);
  }
}

void spark::cleanup()
{
}


back_planet::back_planet(display *d)
:sprite1(RANDX, rand()%(BOTTOM/2), 0, 0, *make_bitmap(), d)
,m_depth(rand()%5+3),m_spare(0)
{
//Nothing left to do!
}

back_planet::~back_planet()
{
  delete pic;
}

void back_planet::move()
{
  int spx = m_spare-pdisp->getcdx();
  int extra=spx/m_depth+pdisp->getcdx();
  m_spare=spx-spx/m_depth*m_depth;

  m_dx = extra;
  m_x=RU(m_x+m_dx);
}

graphics::bitmap *back_planet::make_bitmap()
{
  //Direction the light is coming from.
  //z-cordinate goes "into the screen"
  float light[3]={-0.6, -0.3, -0.7};
  //Planet radius
  const int r (10+rand()%15);
  //There should be a smooth colour gradient between these two
  const int DARK_COLOUR(16);
  const int LIGHT_COLOUR(20);
  
  float light_size = std::sqrt(light[0]*light[0] + light[1]*light[1] + light[2]*light[2]);

  graphics::bitmap b(2*r, 2*r);
  b.clear(0);
  for(int x=0; x<2*r; x++)
    for(int y=0; y<2*r; y++)
    {
      //tx, ty - coords of pixel where 0 is at the centre
      float tx = 0.5+x-r;
      float ty = 0.5+y-r;
      if (tx*tx+ty*ty < r*r)
      {
        float tz = -std::sqrt(r*r-(tx*tx+ty*ty));
        //Light intensity = dot product of light vector and unit vector perpendicular to the surface
        //The unit vector is just (tx,ty,tz)/r
        float dot_product = (light[0]*tx + light[1]*ty + light[2]*tz)/(light_size*r);
        dot_product = max<float>(dot_product, 0);  //Let's not have negative light!
        b.putpixel(x,y,pftoi(DARK_COLOUR + dot_product*(LIGHT_COLOUR-DARK_COLOUR)));
      }
    }
  return new graphics::bitmap(b);
}
