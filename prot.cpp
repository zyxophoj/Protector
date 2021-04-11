/*
prot.cpp: game classes - display, sound, radar, player, baddies etc

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

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <algorithm>
#include <functional>
#include <iostream>

#include "stuff.h"
#include "dlring.h"
#include "missiles.h"
#include "prot.h"
#include "protdata.h"
#include "back.h"
#include "sound.h"
#include "joy.h"

//evil global variables
extern int score[3];

drect::drect(int xin,int yin,int win,int hin)
:x(xin),y(yin),w(win),h(hin){}

//Draw from a bitmap (usually the spare screen) to the real screen
void drect::draw(const graphics::bitmap &b, graphics::screen_ &scr, int xoffset, int yoffset)
{
  if (!graphics::is_graphics_mode())
  {
    //cout<<"help!!";
    exit(-1);
  }
  blit(b,scr,x,y,x+xoffset,y+yoffset,w,h);
}

void drect::cleardraw(graphics::bitmap &b)
{
  b.rectfill(x,y,x+w,y+h,0);
}


std::list<display *>display::displays;

display::display(int win, int hin, int topin,int xin,int defnum,radar *rin)
{
  w=win;
  h=std::min(BOTTOM,hin-51);
  yoff=topin;
  make_bitmap(vscreen,w,h);
  clear(vscreen);
  xoff=xin;
  displays.push_back(this);
  m_pradar = 0;
  m_pradar=rin;
  m_pradar->set(this);
  make_bitmap(ground,640+512,2);
  for(int a=0;a<512;a++)
  {
    ground.putpixel(a, 0, rand()%16+16);
    ground.putpixel(a, 1, rand()%16+16);
  }
  for(int a=512;a<640+512;a++)
  {
    ground.putpixel(a, 0, ground.getpixel(a-512, 0));
    ground.putpixel(a, 1, ground.getpixel(a-512, 1));
  }
  for(int a=0;a<200;a++)(void)(new star(this));
  //for(int a=0;a<5;a++)  (void)(new domed_city(this));
  for(int a=0;a<3;a++) (void)(new back_planet(this)); 
  centre=new defender(RANDX,200,defnum);
  halfw=(win-DEFWIDTH)/2;
  centre->disp=this;
}

display::~display()
{
  //TODO: this drawing stuff needs to happen somewhere, but NOT IN THE DESTRUCTOR FFS

  //Draw current frame (it's this frame that just killed the player)
  //graphics::screen_ scr;
  //drawall(scr);

  for_each(drects.begin(), drects.end(), kill<drect>());
  drects.empty();
  while(sprite1s.size())
  {
    delete *(sprite1s.begin());
  }
  if (m_pradar)
  {
    delete m_pradar;
    m_pradar = 0;
  }

  if(centre)
  {
    centre->disp=0;
    centre=0;
  }
  displays.remove(this);


  //scr.rectfill(xoff+halfw-53,195,xoff+halfw+51,212,8);
  //scr.drawtext("YOU'RE DEAD.",xoff+halfw-48,200,15);
}

int display::gethw(void)
{
return halfw;
}

int display::getcx(void)
{
return centre->getx();
}

int display::getcdx(void)
{
return centre->getdx();
}

void display::add_drect(drect *d)
{
drects.push_back(d);
}

void display::add_sprite1(sprite1 *s)
{
sprite1s.insert(s);
}

void display::remove_sprite1(sprite1 *s)
{
sprite1s.erase(s);
}

void display::drawall(graphics::screen_ &scr)
{
//Could just blit vscreen to screen, but this would be very slow.
//Only blit the bits which have changed - the rest is just black,
//so there is not a lot of point in drawing it.




//blit chunks of vscreen to real screen
for(std::list<drect *>::iterator i=drects.begin(); i!=drects.end(); i++)
  {
  (*i)->draw(vscreen,scr,xoff,yoff);
  }

//black out chunks of vscreen
while(drects.size())
  {
  (*drects.begin())->cleardraw(vscreen);
  delete *(drects.begin());
  drects.erase(drects.begin());
  }

//draw ground
for(std::list<display *>::iterator i=displays.begin(); i!=displays.end(); i++)
  {
  for(int g=0;g<5;g++)
    blit((*i)->ground,scr,((*i)->centre->getx())%512,g%2,(*i)->xoff,
                               BOTTOM+g+(*i)->yoff,(*i)->w,1);
  }

}

void display::drawsprites(graphics::screen_ &scr)
{
  for(std::list<display *>::iterator i = displays.begin(); i != displays.end(); i++)
    (*i)->drawall(scr);
}

void display::allmessage(const char *tm)
{
for(std::list<display *>::iterator i=displays.begin(); i!=displays.end(); ++i)
  (*i)->message(tm);
}

void display::message(const char *tm)
{
int l=strlen(tm);
int left=halfw+10-4*l;
for(int a=0;a<l;a++)
  if(tm[a]>32)
    (void)(new letter(left+8*a,200,tm[a],this));
}

void display::killall(void)
{
for_each(displays.begin(), displays.end(), kill<display>());
displays.empty();
}

//The all-important drawing function.  A drect is created which represents the
//smallest rectangle containing the sprite's current rectangle and the
//rectangle it had last frame.  The top and height of the drect are easy to
//calculate, left and width are a bit more difficult because the display
//scrolls horizontally to keep the player in the centre.
//We also blit the current picture to the spare screen.  The drects are stored
//in the display, which will blit them all to the real screen later.
void sprite::redraw()
{
int tx,ddx,left,width;
display *disp;
for(std::list<display *>::iterator i = display::displays.begin(); i!=display::displays.end(); i++)
  {
  disp=*i;
  ddx=R(m_dx-disp->centre->m_dx);
  tx=R(m_x-disp->centre->m_x+disp->halfw);
  left=R(tx-std::max(ddx,0));
  width=w+abs(ddx);
  if((left+width>=0&&left<disp->w))
    {
    disp->add_drect(new	drect(left,m_y-std::max(m_dy,0),width,h+abs(m_dy)));
    masked_blit(*pic,disp->vscreen,0,0,tx,m_y,w,h);
    }
  }
}

//drawoff is similar to draw, but does not blit the picture to the spare
//screen - wipes it instead.  It is called instead of draw when something is
//destroyed or otherwise made invisible.
void sprite::drawoff()
{
int tx,ddx,left,width;
for(std::list<display *>::iterator i = display::displays.begin(); i!=display::displays.end(); i++)
  {
  display *pdisp = *i;
  ddx=R(m_dx-pdisp->centre->m_dx);
  tx=R(m_x-pdisp->centre->m_x+pdisp->halfw);
  left=R(tx-std::max(ddx,0));
  width=w+abs(ddx);
  if(left+width>=0&&left<pdisp->w)
    {
    pdisp->add_drect(new drect(left,m_y-std::max(m_dy,0),width,h+abs(m_dy)));
    pdisp->vscreen.rectfill(tx,m_y,tx+w,m_y+h,0);
    }
  }
}

std::set<sprite *> sprite::sprites[sprite::levels];

sprite::sprite(int x, int y, int dx, int dy, const graphics::bitmap &picin, int level)
:w(picin.width()),h(picin.height()),m_x(x),m_y(y),m_dx(dx),m_dy(dy),pic(&picin),m_level(level)
{
sprites[m_level].insert(this);
}

sprite::~sprite()
{
sprites[m_level].erase(this);
}

void sprite::gameinc1(void)
{
move();
misc();
}

void sprite::gameinc2(void)
{
animate();
redraw();
}

void sprite::kill_all()
{
   for (int i=0; i< levels; i++) while(sprites[i].size()) delete *sprites[i].begin();
}
void sprite::process_all()
{
  for (int i=0; i< levels; i++) 
  {
     std::for_each(sprites[i].begin(), sprites[i].end(), std::mem_fun(&sprite::gameinc1));
     std::for_each(sprites[i].begin(), sprites[i].end(), std::mem_fun(&sprite::gameinc2));
  }
}

void sprite::demoinc(graphics::screen_ &scr)
{
animate();
blit(*pic,scr,0,0,m_x,m_y,w,h);
}

//Slow pixel check for collision detection
bool sprite::pixel_by_pixel(sprite *other)
{
  int ydiff=other->m_y-m_y;
  int xdiff=R(other->m_x-m_x);
  for(int xx=std::max(0,xdiff);xx<std::min(w,other->w+xdiff);xx++)
    for(int yy=std::max(0,ydiff);yy<std::min(h,other->h+ydiff);yy++)
      if(pic->getpixel(xx, yy) && other->pic->getpixel(xx-xdiff, yy-ydiff)) return true;

  return false;
}

void sprite1::redraw()
{
int tx,ddx,left,width;

ddx=R(m_dx-pdisp->centre->getdx());
tx=R(m_x-pdisp->centre->getx()+pdisp->halfw);
left=R(tx-std::max(ddx,0));
width=w+abs(ddx);
if((left+width>=0&&left<pdisp->w))
  {
  pdisp->add_drect(new drect(left,m_y-std::max(m_dy,0),width,h+abs(m_dy)));
  masked_blit(*pic,pdisp->vscreen,0,0,tx,m_y,w,h);
  }
}

void sprite1::drawoff()
{
int tx,ddx,left,width;

ddx=R(m_dx-pdisp->centre->getdx());
tx=R(m_x-pdisp->centre->getx()+pdisp->halfw + pdisp->centre->getdx());
left=R(tx-std::max(ddx,0));
width=w+abs(ddx);
if(left+width>=0&&left<pdisp->w)
  {
  pdisp->add_drect(new drect(left,m_y-std::max(m_dy,0),width,h+abs(m_dy)));
  pdisp->vscreen.rectfill(tx,m_y,tx+w-1,m_y+h-1,0);
  }
}

sprite1::sprite1(int xin,int yin,int dxin,int dyin,
		     const graphics::bitmap &picin,display *din)
:sprite(xin,yin,dxin,dyin,picin,level_bg)
{
pdisp=din;
pdisp->add_sprite1(this);
}

sprite1::~sprite1()
{
pdisp->remove_sprite1(this);
}

int sprite1::isdead(){return 0;}


std::set<letter *>letter::letters;
graphics::bitmap letter::lpic[256];

void letter::move()
{
m_dx=pdisp->getcdx();
m_x=RU(m_x+m_dx);
}

letter::letter(int xin,int yin,int n,display *dispin)
:sprite1(dispin->getcx()-dispin->gethw()+xin,yin,0,0,lpic[n],dispin)
{
deadflag=0;
letters.insert(this);
}

letter::~letter()
{
letters.erase(this);
}

void letter::init()
{
char strin[2];
strin[1]='\0';
for(int a=0;a<256;a++)
  {
  strin[0]=a;
  graphics::bitmap bmp(8,8);
  lpic[a]=bmp;
  lpic[a].drawtext(strin,0,0,15);
  }
}

void letter::message(const char *the_message,display *dp)
{
timerstuff::new_message();
if(dp)
  dp->message(the_message);
else
  display::allmessage(the_message);
}

void letter::kill1(void)
{
std::set<letter *>::iterator i = letters.begin();
for(int n=rand()%letters.size(); n; n--)i++;
letter *pl = *i;
explode(pl->getxcentre(),pl->getycentre());
pl->deadflag = 1;
}

int letter::isdead(void){return deadflag;}
int letter::nletters(void){return letters.size();}


graphics::bitmap defender::defpics[9];
dlring<defender>defender::defenders;
int defender::violent;

void defender::misc()
{
command &mycontrols = controls::get(type==2);

if(mycontrols.get_shoot()&&!shottime)
  {

  (void)(new shot(RU(m_x+20*lr),m_y+5,10*lr+m_dx,!!railshots+2*!!longshots,number));
  if(railshots)railshots--;
  if(autoshots)autoshots--;
  if(longshots)longshots--;
  if(elephants)
    {
    elephants--;
    (void)(new elephant(getxcentre()-3,m_y,fdx,fdy,lr,number));
    }
  shottime=autoshots?1:5;
  sound::play(sound::shoot,255);
  violent=1;
  }

if(mycontrols.get_bomb()&&bombs&&!bombpress)
  {
  colldet::addbomb(m_x);
  bombs--;
  bombpress=1;
  violent=1;
  }
if(!mycontrols.get_bomb())bombpress=0;

if(mycontrols.get_shield()&&!shieldpress)
  {
  shieldstate=shieldtime&&!shieldstate;
  shieldpress=1;
  }
if(!mycontrols.get_shield())shieldpress=0;

if(mycontrols.get_die())die();

if(shottime)shottime--;
if(invtime)invtime--;

if(shieldstate)
  {
  if(!invtime)shieldtime--;
  if(shieldtime==0)
    {
    shieldstate=0;
    invtime=500;
    }
  }
if(colltime)colltime--;
radar::pip(m_x,m_y,1);
update();
}

void defender::move()
{
  const float HTHRUST(0.16);
  const float VTHRUST(0.16);
  //Coefficient of restitution is:
  //(upward speed after bouncing)/(downward speed before bouncing)
  //Kinetic energy (0.5*mv^2) is converted to gravitational potential energy (mgh)
  //so setting restitution to sqrt(x) means we'll bounce to x*(previous bounce height)
  //(or we would if there was no air resistance, anyway)
  const float RESTITUTION(sqrt(0.2));

  command &com = controls::get(type==2);


  //up/down
  fdy += VTHRUST * com.get_ud()/128;

  
  //left/right
  //Player may be using a joystick.  Stick must move a certain distance before
  //the ship "flips" This prevents crazy flipping but makes it possible to
  //fly backwards :-/
  int xy = com.get_lr();
  fdx += HTHRUST*(xy)/128;
  if(abs(xy)>20) lr=sign(xy);

  
//Apply "Physics"
fdy += GRAVITY;
fdx -= AIR_RESISTANCE*fdx;
fdy -= AIR_RESISTANCE*fdy;

//don't go off the top
fdy = std::max<float>(fdy, -m_y);

//bounce when we hit the bottom
float bounce = std::max<float>(0,fy+fdy-(BOTTOM-h));
float oldfdy = fdy;
fdy=std::min<float>(fdy,BOTTOM-h-m_y);
fx+=fdx;
fy+=fdy;
if(bounce) fdy = -oldfdy*RESTITUTION;

//fdx, fdy, fx, fy are "real" values.
//m_dx, m_dy, etc are integers for pixel calculations
//In particular, m_dy must be (current y - old y) but fdy is
//the speed at which we are moving down.  These are very
//different if we've just bounced.

m_dx=R(static_cast<int>(fx)-m_x);
m_x=RU(m_x+m_dx);
m_dy=static_cast<int>(fy)-m_y;
m_y+=m_dy;

//there's no % operator for floats, so..
if(fx<0)fx+=XWIDTH;
if(fx>XWIDTH)fx-=XWIDTH;

pic=&defpics[(lr==1)+2*!!passenger+4*!!shieldstate];
//defpics[8] is all black, this makes the defender flash when invulnerable
if(invtime&&invtime/5%2)pic=&defpics[8];
}

void defender::update()
{
// TODO: call all this from something that legitimately has a screen!
/*
char strin[15];
int coords[24]={76,0,620,440,464,464,76,0,620,450,472,472,
		20,20,560,470,470,470,20,100,400,460,470,470};

graphics::screen_ scr;
sprintf(strin,"%d",lives);
strcat(strin," ");
scr.drawtext(strin,coords[type],coords[type+3],1,16);
sprintf(strin,"%d",bombs);
strcat(strin," ");
scr.drawtext(strin,coords[type+6],coords[type+9],4,16);
scr.drawtext("L",coords[type+12],coords[type+15],16+longshots/64,16);
scr.drawtext("A",coords[type+12]+10,coords[type+15],16+autoshots/64,16);
scr.drawtext("R",coords[type+12]+20,coords[type+15],16+railshots/64,16);
scr.drawtext("S",coords[type+12]+30,coords[type+15],16+shieldtime/128,16);
scr.drawtext("E",coords[type+12]+40,coords[type+15],16+elephants/64,16);

sprintf(strin,"%d",score[number]);
scr.drawtext(strin,coords[type+18],coords[type+21],15, 16);
*/
}

defender::defender(int xin,int yin,int typein)
:sprite(xin,yin,0,0,defpics[0],level_fg)
,number(1+(typein==2))
{
shottime=0;
fdx=fdy=0;
passenger=0;
fx=xin;fy=yin;
lr=1;
lives=3;
bombs=3;
bombpress=0;
shieldstate=0;
shieldtime=0;
invtime=500;
colltime=0;
railshots=0;
autoshots=0;
longshots=0;
elephants=0;
type=typein;

/* TODO: when should this be called?
if(type==0)
  {
  graphics::screen_ scr;
  scr.drawtext("LIVES:",20,440,1);
  scr.drawtext("BOMBS:",20,450,4);
  }
*/
disp=0;
defenders.add(this);
}

defender::~defender()
{
defenders.remove(this);
deftarget::defdie(this);
if(disp)
  {
  delete disp;//"You" defender in attract mode has no display!
  disp=0;
  }
if(passenger)
  {
  passenger->drop(m_x+5,m_y);
  passenger=0;
  }
}


void defender::init()
{
static bool is_already_initialised(false);
if (is_already_initialised) return;
is_already_initialised = true;

unsigned char defdata[400]={

0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,
0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,
0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,
0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,

0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,
0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
0,0,0,0,0,2,4,0,4,4,4,4,0,4,2,0,0,0,0,0,
0,0,0,0,0,2,4,4,0,0,0,0,4,4,2,0,0,0,0,0,
0,0,0,0,0,0,2,4,4,4,4,4,4,2,0,0,0,0,0,0

};
for(int a=0;a<9;a++)
{
  graphics::bitmap b(20,10);
  defpics[a]=b;
}

for(int a=0;a<200;a++)
  {
  defpics[0].putpixel(a%20,a/20,defdata[a]);
  defpics[1].putpixel(19-a%20,a/20,defdata[a]);
  defpics[2].putpixel(a%20,a/20,defdata[a+200]);
  defpics[3].putpixel(19-a%20,a/20,defdata[a+200]);
  }

for(int a=4;a<8;a++)
  {
  blit(defpics[a-4],defpics[a],0,0,0,0,20,10);
  for(int b=0;b<20;b++)
    {
    int c=0;
    while(defpics[a].getpixel(b,c)==0)c++;
    defpics[a].putpixel(b,c,15);
    c=9;
    while(defpics[a].getpixel(b,c)==0)c--;
    defpics[a].putpixel(b,c,15);
    }
  for(int b=0;b<10;b++)
    {
    int c=0;
    while(defpics[a].getpixel(c,b)==0)c++;
    defpics[a].putpixel(c,b,15);
    c=19;
    while(defpics[a].getpixel(c,b)==0)c--;
    defpics[a].putpixel(c,b,15);
    }
  }
clear(defpics[8]);

}

defender *defender::getdef(void)
{
if(defenders.size()==0)return 0;
return (defenders.getnext());
}

void defender::die(void)
{
lives--;
sound::play(sound::die,255);
invtime=1000;
}

int defender::isdead(){return lives<=0;}

int defender::alldead(){return (!defenders.size());}

void defender::make_invulnerable()
{
invtime=500;
}
void defender::make_all_invulnerable()
{
for (dlring<defender>::iterator i = defenders.begin(); i!=defenders.end(); ++i)
  (*i)->make_invulnerable();
}

void defender::getpowerup(int type)
{
switch(type)
  {
  case powerup::RAILGUN:
  railshots=1024;
  break;
  case powerup::AUTOFIRE:
  autoshots=1024;
  break;
  case powerup::LONGRANGE:
  longshots=1024;
  break;
  case powerup::BOMB:
  bombs++;
  break;
  case powerup::SHIELD:
  shieldtime=2048;
  break;
  case powerup::XTRA_LIFE:
  lives++;
  break;
  case powerup::ITCURSE:
  deftarget::itcurse(this);
  break;
  case powerup::ELEPHANTGUN:
  elephants=1024;
  break;
  }
}

//Modified version of drawoff:  a defender doesn't erase its image from its own display
//So when the display renders the final scene, it is clear where and why the poor sod died.
void defender::drawoff()
{
int tx,ddx,left,width;
for(std::list<display *>::iterator i = display::displays.begin(); i!=display::displays.end(); i++)
  if (*i != disp)
  {
  display *pdisp = *i;
  ddx=R(m_dx-pdisp->centre->m_dx);
  tx=R(m_x-pdisp->centre->m_x+pdisp->halfw);
  left=R(tx-std::max(ddx,0));
  width=w+abs(ddx);
  if(left+width>=0&&left<pdisp->w)
    {
    pdisp->add_drect(new drect(left,m_y-std::max(m_dy,0),width,h+abs(m_dy)));
    pdisp->vscreen.rectfill(tx,m_y,tx+w-1,m_y+h-1,0);
    }
  }
}

void defender::cheat()
{
if(!violent)
  for(int a=0;a<defenders.size();a++)
    defenders.getnext()->lives+=100;
}


dlring<deftarget>deftarget::deftargets;

void deftarget::not_this_one(defender *dp)
{
if(defender::defenders.size()==0)
  {
  dt=0;
  return;
  }
if(defender::defenders.size()==1&&(defender::defenders+0)==dp)
  {
  dt=0;
  return;
  }
while((defender::defenders+0)==dp)defender::defenders.getnext();
dt=(defender::defenders+0);
}

void deftarget::settarget(defender *dp)
{
dt=dp;
}

deftarget::deftarget(void)
{
deftargets.add(this);
dt=defender::getdef();
}

deftarget::deftarget(defender *this_one)
{
deftargets.add(this);
dt=this_one;
}

deftarget::~deftarget()
{
deftargets.remove(this);
}

void deftarget::defdie(defender *dp)
{
for(int a=0;a<deftargets.size();a++)
  {
  if(deftargets[0].dt)deftargets[0].not_this_one(dp);
  deftargets.getnext();
  }
}

void deftarget::itcurse(defender *dp)
{
defender *itcursee;
if(defender::defenders.size()==0)
  itcursee=0;
else if(defender::defenders.size()==1&&(defender::defenders+0)==dp)
  itcursee=0;
else
  {
  while((defender::defenders+0)==dp)defender::defenders.getnext();
  itcursee=(defender::defenders+0);
  }

if(itcursee)letter::message("You're It!",itcursee->disp);

for(int a=0;a<deftargets.size();a++)
  {
  deftargets[0].settarget(itcursee);
  deftargets.getnext();
  }
}

void deftarget::makerandom()
{
int l=defender::defenders.size();
if(l)
  {
  int r=rand()%l;
  for(;r;r--)defender::defenders.getnext();
  }
}


graphics::bitmap shot::spic;
std::set<shot *> shot::shots;

void shot::move()
{
if(life) life--;
m_x=RU(m_x+m_dx);
}

shot::shot(int xin,int yin,int dxin,int type,int number)
:sprite(xin,yin,dxin,0,spic,level_fg),life(30+30*(type&SF_LONG)),
 railness(!!(type&SF_RAIL)),n(number)
{
shots.insert(this);
}

shot::~shot()
{
shots.erase(this);
}

void shot::init()
{
  make_bitmap(spic,20,1);
  spic.clear(15);
}

int shot::isdead(){return !life;}

graphics::bitmap elephant::epic[2];
std::set<elephant *>elephant::elephants;

void elephant::move()
{
fdy+=GRAVITY;
fx+=fdx;
fy+=fdy;

m_dx=R(int(fx)-m_x);
m_dy=int(fy)-m_y;

m_x=RU(m_x+m_dx);
m_y+=m_dy;
}

elephant::elephant(int xin,int yin,float dxin,float dyin,int d,int nin)
:sprite(xin,yin,0,0,epic[d==1],level_fg)
{
float theta=2.0*M_PI*((float)(rand())/RAND_MAX+1.0)/12;
fx=m_x;
fy=m_y;
fdx=dxin+3.0*cos(theta)*d;
fdy=dyin-3.0*sin(theta);
n=nin;
elephants.insert(this);
}

elephant::~elephant()
{
elephants.erase(this);
}

void elephant::init()
{
int data[36]={
0,7,7,0,7,0,
7,7,7,7,9,7,
7,7,7,7,7,7,
7,7,7,7,0,7,
0,7,0,7,0,7,
0,7,0,7,0,0};
make_bitmap(epic[0],6,6);
make_bitmap(epic[1],6,6);
for(int a=0;a<36;a++)
  {
  epic[0].putpixel(a%6, a/6, data[a+5-2*(a%6)]);
  epic[1].putpixel(a%6, a/6, data[a]);
  }
}

int elephant::isdead(void)
{
return m_y>BOTTOM;
}


graphics::bitmap fader::fpics[16];

void fader::animate(void)
{
pic=&fpics[life/2];
}

void fader::misc(void)
{
if(life)life--;
}

void fader::move(void)
{
m_x=RU(m_x+m_dx);
m_y+=m_dy;
}

fader::fader(int xin,int yin,int d)
:sprite(xin,yin,0,0,fpics[15],level_fg),life(31)
{
if(d>=4)d++;       //012
m_dx=2*(d/3-1);    //3 4
m_dy=2*(d%3-1);    //567 
}

fader::~fader()
{

}

void fader::init()
{
for(int a=0;a<16;a++)
  {
  make_bitmap(fpics[a],2,2);
  fpics[a].clear(16+a);
  }
}

int fader::isdead(void){return !life;}


graphics::bitmap pod::podpic[3];
std::vector<pod *>pod::pods;
int pod::safe(0);
int pod::passengers(0);

void pod::animate(void)
{
animflag=(animflag-m_dx+12)%12;
pic=&podpic[animflag/4];
}

void pod::misc()
{
if(grabflag&&attacker&&!(altime++%150))
  sound::play(sound::alert,255-m_y/3);
if(!rescuer)radar::pip(m_x,m_y,4);
}

void pod::move()
{
if(grabflag)
  {
  if(attacker)
    {
    m_dx=0;
    m_dy=attacker->m_y+8-m_y;
    m_y+=m_dy;
    radar::alert(m_x);
    }
  else return;
  }
else
  {
  toggle++;
  toggle%=2;
  m_dx=toggle?0:bdx;
  m_x=RU(m_x+m_dx);
  m_dy=std::min(2,BOTTOM-h-m_y);
  m_y+=m_dy;
  }
}

void pod::redraw()
{
if(!rescuer)sprite::redraw();
}

pod::pod()
:sprite(RANDX,410,0,0,podpic[0],level_fg),
 toggle(0),bdx(rand()%2*2-1),grabflag(0),attacker(0),rescuer(0)
{
m_dx=bdx;
animflag=0;
deadflag=0;
pods.push_back(this);
}

pod::~pod()
{
pods.erase(find(pods.begin(), pods.end(), this));
if(pods.size()+safe+passengers==0) sbaddie::all_become_evil();
}

void pod::init()
{
unsigned char data[100]=
{
 0, 0, 0,14,12,13,14, 0, 0, 0,
 0, 0,13, 4, 4, 4, 4,12, 0, 0,
 0,12, 4, 4, 4, 4, 4, 4,13, 0,
14, 4, 4, 0, 4, 4, 0, 4, 4,14,
13, 4, 4, 4, 4, 4, 4, 4, 4,12,
12, 4, 4, 4, 4, 4, 4, 4, 4,13,
14, 4, 0, 4, 4, 4, 4, 0, 4,14,
 0,13, 4, 0, 0, 0, 0, 4,12, 0,
 0, 0,12, 4, 4, 4, 4,13, 0, 0,
 0, 0, 0,14,13,12,14, 0, 0, 0

};
for(int a=0;a<3;a++)make_bitmap(podpic[a],10,10);
for(int a=0;a<100;a++)
  {
  int b=data[a];
  for(int c=0;c<3;c++)
    podpic[c].putpixel(a%10, a/10, b>10?12*!!((c+b)%3):b);
  }
}

pod *pod::getvictim(void)
{
if(pods.size()<=unsigned(rand()%8))return 0;
int i=rand()%pods.size();

if(!(pods[i]->attacker||
     pods[i]->rescuer||
     pods[i]->m_dy)) return(pods[i]);

return 0;
}


int pod::isdead(){return deadflag;}

void pod::grabbed(defender *dp)
{
if(attacker)
  {
  if(attacker->landstage==3)
    {
    attacker->landstage=4;
    score[dp->number]+=250;
    }
  else
    {
    attacker->landstage=0;
    }
  attacker->victim=0;
  attacker=0;
  }
  passengers++;
  rescuer = dp;
  //erase old position to avoid leaving droppings
  drawoff();
  rescuer->setpassenger(this);
}

//Called when a carrying defender is killed
void pod::drop(int x, int y)
{
  m_x=x;m_y=y;
  rescuer=0;
  passengers--;
}

void pod::make8()
{
while(pods.size()<8)(void)(new pod);
safe=0;
passengers=0;
}

int pod::in(void){return safe;}
int pod::out(void){return pods.size();}

graphics::bitmap portal::porpic[8];
std::list<portal *>portal::portals;

void portal::animate()
{
animflag=(animflag+1)%24;
pic=&porpic[animflag/3];
}

void portal::misc(void)
{
radar::pip(m_x,m_y,15);
radar::pip(m_x,m_y+h,15);
for(int a=2;a<9;a++)radar::pip(m_x,m_y+h*a/10,1);
}


portal::portal(void)
:sprite(RANDX,rand()%100+50,0,0,porpic[0],level_fg)
{
animflag=0;
portals.push_back(this);
}

portal::~portal()
{
portals.remove(this);
}

void portal::init()
{
int a,b,c;
int v[8]={0,1,2,3,3,2,1,0};
unsigned char end[16]={
 0,15,15, 0,
15,13,13,15,
15,13,13,15,
 0,15,15, 0};
for(a=0;a<8;a++)
  {
  make_bitmap(porpic[a],4,100);
  clear(porpic[a]);
  for(b=0;b<4;b++)
    for(c=0;c<4;c++)
      {
      porpic[a].putpixel(c,b,end[4*b+c]);
      porpic[a].putpixel(c,b+96,end[4*b+c]);
      }

  }
for(a=5;a<96;a++)
  {
  b=rand()%8;
  for(c=0;c<8;c++)
    porpic[(b+c)%8].putpixel(v[c],a,1);
  }

}

std::set<sbaddie *> sbaddie::sbaddies;

sbaddie::sbaddie(int xin,int yin,int dxin,int dyin,
		 const graphics::bitmap &picin,int pfkin)
:sprite(xin,yin,dxin,dyin,picin,level_fg),deadflag(0),points(pfkin)
{
sbaddies.insert(this);
}

sbaddie::~sbaddie()
{
sbaddies.erase(this);
}

int sbaddie::isdead(){return deadflag;}
int sbaddie::alldead(){return !sbaddies.size();}

void sbaddie::takeshot(int n)
{
if(!deadflag)
  {
  deadflag=1;
  explode(getxcentre(), getycentre());
  score[n]+=points;
  }
}

void sbaddie::takebomb()
{
    takeshot(0);
}

void sbaddie::become_evil()
{
deadflag=1;
(void)(new lawyer(m_x,m_y));
}

void sbaddie::all_become_evil()
{
for_each(sbaddies.begin(), sbaddies.end(), std::mem_fun(&sbaddie::become_evil));
}

int sbaddie::population(){return sbaddies.size();}


//The "special" baddies
//Bugs are special because pods know about them (definition of bug must be visible to pod code)
//Lawyers are special because of sbaddie::become_evil - default behaviour is to turn into a lawyer.
//
//As many baddies as possible should be non-special and go in baddies.h/cpp 

graphics::bitmap bug::landerpic[10];

bug::bug()
:sbaddie(RANDX,rand()%400,0,0,landerpic[0],50)
,victim(0),landstage(0),animflag(0)
{
  bdx=rand()%3-1;
  bdy=rand()%3-1;
}

bug::~bug()
{
  if(victim)
  {
    victim->attacker=0;
    victim->grabflag=0;
  }
}

void bug::init()
{
  int a,b,c1,c2,d;
  unsigned char data[100]={
     0,28,27,26,25,25,24,23,23,0,
    28,27,26,25,24,24,23,22,22,22,
    28,27,26,25,24,24,23,22,22,22,
    28,27,26,25,24,24,23,22,22,22,
    28,27,26,25,24,24,23,22,22,22,
    28,27,26,25,24,24,23,22,22,22,
     0,26,25,24,23,23,22,21,21, 0,
     2,10, 0, 0,26,22, 0, 0,10, 2,
    10, 2, 0, 0,26,22, 0, 0, 2,10,
     2, 0, 0, 0,26,22, 0, 0, 0, 2
    };

  for(a=0;a<10;a++)
  {
    make_bitmap(landerpic[a],10,10);
    for(b=0;b<100;b++) landerpic[a].putpixel(b%10, b/10, data[b]);
    c1=static_cast<int>(floor(4.5+5*cos(2.0*M_PI*a/40)));
    c2=static_cast<int>(floor(4.5+5*cos(2.0*M_PI*(a+10)/40)));
    for(d=1;d<6;d++)
    {
      if(landerpic[a].getpixel(c1,d))
        landerpic[a].putpixel(c1,d, landerpic[a].getpixel(c1,d)-5);
      if(landerpic[a].getpixel(c2,d))
        landerpic[a].putpixel(c2,d, landerpic[a].getpixel(c2,d)-5);
    }
  }
}

void bug::animate()
{
animflag=(animflag+1)%40;
pic=&landerpic[animflag/4];
}

void bug::misc()
{
radar::pip(m_x,m_y,2);
}

void bug::move()
{
switch(landstage)
  {
  case 0: //Normal behaviour: wandering around randomly
    //sometines change direction
    if(!(rand()%20))
    {
      bdx=rand()%3-1;
      bdy=rand()%3-1;
    }
    if(m_y==0)bdy=1;
    //10 is height of potential victim
    if(m_y==BOTTOM-h-10)bdy=-1;

    //move erratically
    if(rand()%3)
    {
      m_x+=(m_dx=bdx);
      m_y+=(m_dy=bdy);
    }
    else m_dx=m_dy=0;

  //occasionally choose a victim
  if(!(rand()%1500)&&!victim)
    {
    victim=pod::getvictim();
    if(victim)
      {
      victim->attacker=this;
      landstage=1;
      }
    }
  break;

  case 1: //horizontal run
    if(abs(victim->m_x-m_x)<=3)
    {
      landstage++;
      m_dx=m_dy=0;
      break;
    }
    m_dx=-victim->bdx;
    m_x+=m_dx;
    m_dy=0;
  break;

  case 2://Somewhere above the victim, moving down
    m_dx=R(victim->m_x-m_x);
    m_x+=m_dx;
    if(m_y == BOTTOM-h-victim->h)
    {
      m_dy=0;
      landstage++;
      victim->grabflag=1;
      victim->altime=0;
      break;
    }
    m_dy=1;
    m_y+=m_dy;
  break;

  case 3://pull up
    m_dx=0;
    if(m_y==0)
    {
      deadflag=1;
      victim->deadflag=1;
      (void)(new lawyer(m_x,m_y));
    }
    else m_dy = -!(rand()%4);
    m_y+=m_dy;
  break;

  case 4://Confused.  Victim was rescued by player in the lifting stage.(3)
         //fly up quickly and die when we get to the top - the engines are stuck on "up"
    if(m_y<=0)
    {
      m_dx=m_dy=0;
      takeshot(0);
    }
    else
    {
      m_dx=0;
      m_dy=-2;
      m_y+=m_dy;
    }
  break;
  }
m_x=RU(m_x);
}


graphics::bitmap lawyer::lawyerpic[10];

lawyer::lawyer(int x,int y)
:sbaddie(x,y,1,1,lawyerpic[0],500)
{
  xdev=rand()%300-150;
  ydev=rand()%(BOTTOM-100)+50;
  animflag=0;
}

lawyer::~lawyer()
{

}

void lawyer::init()
{
  int rs[10]={49,50,51,52,53,53,52,51,50,49};
  unsigned char data[100]={
     2, 0, 0, 0,26,22, 0, 0, 0, 2,
    10, 2, 0, 0,26,22, 0, 0, 2,10,
     2,10, 0, 0,26,22, 0, 0,10, 2,
     0, 5,13,13,13,13,13,13, 5, 0,
     0,13,13,49,13,13,49,13,13, 0,
     0, 5,13,15,49,49,15,13, 5, 0,
     0, 0,13,49,13,13,49,13, 0, 0,
     0, 0, 5,13,13,13,13, 5, 0, 0,
     0, 0, 0,13,13,13,13, 0, 0, 0,
     0, 0, 0, 0, 5, 5, 0, 0, 0, 0};

  for(int a=0;a<10;a++)
  {
    make_bitmap(lawyerpic[a],10,10);
    for(int b=0;b<100;b++)
      lawyerpic[a].putpixel(b%10,b/10, data[b]==49?rs[a]:data[b]);
  }
}

void lawyer::move()
{
  int rx,ry;

  //Choose a target position (offset from the player)
  //xdev from -150 to 150, ydev is random height but not within 50
  //of zero.  Player shoots horizontally so it's best to avoid the
  //same approximate height  
  if(!(rand()%5000))
  {
    xdev = (rand()%300)-150;
    ydev = (rand()%(BOTTOM-100))+50;
  }

  if(target.islive())
  {
    //relative position of target plus offset
    rx = R(xdev+target.getx()-getxcentre());
    ry = (target.gety()+ydev)%BOTTOM-DEFHEIGHT-getycentre();

    //"bounce" around in a rectangle (160x100) 
    if(m_y<2 || ry>50) m_dy=2;
    if(m_y+m_dy>BOTTOM-h-DEFHEIGHT||ry<-50) m_dy=-2;  
    if(rx>80)  m_dx=5;
    if(rx<-80) m_dx=-5;
    m_x=RU(m_x+m_dx);
    m_y+=m_dy;
  }
  else m_dx=m_dy=0;
}

void lawyer::animate()
{
  animflag=(animflag+1)%80;
  pic=&lawyerpic[animflag/8];
}

void lawyer::misc(void)
{
  if(!(rand()%150))shoot<bogon>(&target,6.0);
  radar::pip(m_x,m_y,13);
}

void lawyer::become_evil()
{
  //A lawyer can not become more evil than it already is...
  //There is not a lot of point in replacing a lawyer with another lawyer.
}

void lawyer::takebomb()
{
  //smart bombs don't always work on lawyers!
  if(13<(rand()%100))
    sbaddie::takebomb();
}

std::set<powerup *>powerup::powerups;
graphics::bitmap powerup::pupics[8];

void powerup::misc(void)
{
radar::pip(m_x,m_y,15);
}

powerup::powerup(int xin,int yin,int tin)
:sprite(xin,yin,0,0,pupics[tin],level_fg)
{
type=tin;
deadness=0;
powerups.insert(this);
}

powerup::~powerup()
{
powerups.erase(this);
}

void powerup::init()
{
const char *letter[8]={"R","A","L","B","S","X","I","E"};
for(int a=0;a<8;a++)
  {
  make_bitmap(pupics[a],15,15);
  pupics[a].circle(7,7,7,15);
  pupics[a].drawtext(letter[a],4,4,15);
  }
}

int powerup::isdead(){return deadness;}

std::list<radar *>radar::radars;

int radar::timer(0);

radar::radar(int l,int t,int w,int h)
:pad(w,h)
{
radars.push_back(this);
top=t;left=l;width=w;height=h;
xs=1.0*width/XWIDTH;
ys=1.0*height/410;

//TODO: when should this be called?
//graphics::screen_ scr;
//scr.hline(left,top-1     ,left+width,8);
//scr.hline(left,top+height,left+width,8);
}

radar::~radar()
{
radars.remove(this);
}

void radar::set(display	*din)
{
int sc_left,sc_right;
d=din;
sc_left=static_cast<int>((XWIDTH/2-d->w/2)*xs);
sc_right=static_cast<int>((XWIDTH/2+d->w/2)*xs);

//TODO: when should this be called?
//graphics::screen_ scr;
//scr.hline(left+sc_left,top-1,     left+sc_right,24);
//scr.hline(left+sc_left,top+height,left+sc_right,24);
}

void radar::wipe()
{
radar *rp;

for(std::list<radar *>::iterator i =radars.begin(); i!=radars.end(); i++)
  {
  rp=(*i);
  //Clear the radar to mostly colour 16(always black), with "fuzz" in colour
  //0 - only visible during smart bomb.
  rp->pad.clear(16);
  for(int b=(rp->width*rp->height)/50; b; b--)
    rp->pad.putpixel( rand()%rp->width, rand()%rp->height, 0);
  }
}

void radar::pip(int x,int y,unsigned char c)
{

radar *rp;
for(std::list<radar *>::iterator i =radars.begin(); i!=radars.end(); i++)
  {
  rp=(*i);
  rp->pad.putpixel(
    static_cast<int>((XWIDTH/2+R(x-rp->d->centre->getx()))*rp->xs),
    static_cast<int>(y*rp->ys),c);
  }
  
}

void radar::bigpip(int xsprite, int ysprite, const graphics::bitmap &b)
{
radar *rp;
for(std::list<radar *>::iterator i =radars.begin(); i!=radars.end(); i++)
  {
  rp=(*i);
  for(int x=0; x<b.width()*rp->xs; x++)
    {
    int xx = static_cast<int>(x/rp->xs);
    for(int y=0; y<b.height()*rp->ys; y++)
      {
        int yy = static_cast<int>(y/rp->ys);
        rp->pad.putpixel(
           static_cast<int>((XWIDTH/2+R(xsprite+xx-rp->d->centre->getx()))*rp->xs),
           static_cast<int>(ysprite*rp->ys+y),
           b.getpixel(xx, yy));
      }
    }
  }
}

void radar::alert(int x)
{
if(timer<25)
  {
  pip(x,0,14);
  pip(x,410,14);
  }
}

void radar::drawall(graphics::screen_ &s)
{
for(std::list<radar *>::iterator i=radars.begin(); i!=radars.end(); i++)
 (*i)->draw(s);
timer++;
timer%=50;
}

void radar::draw(graphics::screen_ &s)
{
    blit(pad, s, 0, 0, left, top, width, height);
}

int timerstuff::bombstate(0);
int timerstuff::messtimer(0);

void timerstuff::add_bomb(void)
{
bombstate=63;
}

void timerstuff::tick()
{
if(bombstate)
  {
  bombstate--;
  
  //TODO: find a way top make this work without breaking drects
  //graphics::setcol(0,bombstate,bombstate,bombstate);
  }
if(messtimer)
  {
  messtimer--;
  }
else
  {
  if(letter::nletters())
    if(!(rand()%4))letter::kill1();
  }
}

int timerstuff::ok_to_continue()
{
return !letter::nletters() && !bombstate;
}

void timerstuff::new_message()
{
messtimer=200;
}


hst::hst(const char *arg)
{
int a,b;
int scstart[10]={500000,400000,300000,200000,100000,50000,20000,10000,5000,1000};
FILE *f;
location=new char[strlen(arg)+1];
strcpy(location,arg);
f=fopen(location,"rb");
if(!f||feof(f))
  {
  if(f)fclose(f);
  for(a=0;a<10;a++)
    {
    names[a]=new char[6];
    strcpy(names[a],"Mark");
    scores[a]=scstart[a];
    }
  }
else
  {
  for(a=0;a<10;a++)
    {
    fread(&b,1,sizeof(int),f);
    names[a]=new char[b];
    fread(names[a],b,sizeof(char),f);
    }
  fread(scores,10,sizeof(int),f);
  fclose(f);
  }
}

void hst::show(graphics::screen_ &scr)
{
int a;
char strin[10];

clear(scr);
for(a=0;a<10;a++)
  {
  scr.drawtext(names[a],220,150+15*a,15);
  sprintf(strin,"%d",scores[a]);
  scr.drawtext(strin,400-8*strlen(strin),150+15*a,15);
  }
scr.drawtext("Top Scores",270,110,15);
scr.rect(200,130,425,310,15);
}

void hst::add(graphics::screen_ &scr, const char *who,int sin)
{
int a,b,g,r;
char strin[11];
char buf[80];
if(sin>scores[9])
  {
  delete[]names[9];
  a=9;
  while(a&&sin>scores[a-1])
    {
    scores[a]=scores[a-1];
    names[a]=names[a-1];
    a--;
    }
  scores[a]=sin;
  names[a][0]=0;
  show(scr);
  strcpy(buf,"Enter your name, ");
  strcat(buf,who);
  scr.drawtext(buf,320-4*strlen(buf),400,15);
  keys::clear_buffer();
  for(b=11;b>0;b--) strin[b-1]=0;
  //TODO : restore name-typing loop
  /*do
    {
    while(!keys::keypressed())
    scr.drawtext(" ",20,10,15);
    g=(r=keys::readkey())&0xff;
    switch(g)
      {
      case 0:
      if(r==0x5300)
	if(b)
	  {
	  strin[--b]=0;
	  sound::play(sound::pshoot,255);
	  }
      break;
      case 8:
      if(b)
	{
	strin[--b]=0;
	sound::play(sound::pshoot,255);
	}
      break;
      default:
      if(g>=32&&b<10)
	{
	strin[b++]=g;
	sound::play(sound::bpop,255);
	}
      break;
      }

    scr.rectfill(220,150+15*a,319,160+15*a,0);
    scr.drawtext(strin,220,150+15*a,15);
    }while(g!=13);*/
  for(int nn=0;nn<8;nn++)sound::play(sound::kaboom,255);
  strin[b++]=0;
  //Apologies to any Ms Returns out there
  if(b==1) {strcpy(strin,"Mr Return");b=7;}
  names[a]=new char[b];
  strcpy(names[a],strin);
  show(scr);
  }
else
  {
  show(scr);
  sprintf(strin,"%d",sin);
  strcpy(buf,who);
  strcat(buf," scored ");
  strcat(buf,strin);
  scr.drawtext(buf,320-4*strlen(buf),400,15);
  }
  keys::clear_buffer();
  //while(!keys::keypressed());
}

hst::~hst()
{
int a,b;
FILE *f;
if((f=fopen(location,"wb")))
  {
  for(a=0;a<10;a++)
    {
    b=strlen(names[a])+1;
    fwrite(&b,1,sizeof(int),f);
    fwrite(names[a],b,sizeof(char),f);
    delete[]names[a];
    }
  fwrite(scores,10,sizeof(int),f);
  fclose(f);
  }
  else printf("Could not save high score table!\n");
delete[]location;
}

unsigned char exploderanim::colours[32][3];
int exploderanim::n;

void exploderanim::init()
{
n=0;
for(int a=0;a<128;a++)graphics::setcol(128+a,0,0,63);
for(int a=0;a<32;a++)
  {
  int b=abs(a-16);
  colours[a][0]=32*(16-b)/16;
  colours[a][1]=32*(16-b)/16;
  colours[a][2]=63;
  }
}

//This needs cleaning up!
void exploderanim::cycle()
{
  n = (n+1)%128;
  //graphics::set_colours(128, colours, n);
}


//standard exploding baddie effect x,y = centre of explosion
void explode(int x,int y)
{
for(int a=0;a<8;a++)(void)(new fader(RU(x-1),y-1,a));
sound::play(sound::bang,255);
}


