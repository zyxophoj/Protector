/*
Baddies.cpp: various baddies derived from sbaddie

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

#include <math.h>
#include <string.h>
#include <iostream>

#include "prot.h"
#include "baddies.h"
#include "missiles.h"

extern int score[3];

graphics::bitmap bogon::bogpic;
dlring<bogon>bogon::bogons;

void bogon::misc()
{
if(life)life--;
if(m_y<-2||m_y>420)life=0;
}

void bogon::move()
{
fx+=fdx;
fy+=fdy;
m_dx=R(int(fx)-m_x);
m_dy=int(fy)-m_y;

m_x+=m_dx;
m_y+=m_dy;
m_x=RU(m_x);
}

bogon::bogon(int xstart,int ystart,float dxin,float dyin)
:sprite(xstart,ystart,0,0,bogpic,level_fg),life(150)
{
fx=xstart;
fy=ystart;
fdx=dxin;
fdy=dyin;
bogons.add(this);
}

bogon::~bogon()
{
bogons.remove(this);
}

int bogon::isdead(){return !life;}

void bogon::init()
{
int a;
make_bitmap(bogpic,3,3);
for(a=0;a<9;a++)
bogpic.putpixel(a/3,a%3,a%2?7:a==4?14:4);
}

void bogon::cleanup(void)
{
}



graphics::bitmap bigon::bpelpic;

void bigon::misc(void)
{
//Explode (silently) if life counter runs out, but just 
//dissappear if we go off the top or the bottom
if(life) life--;
if(life==0&&!deadflag) 
   for(int a=0;a<8;a++)
     (void)(new fader(RU(getxcentre()-1),getycentre()-1,a));
if(m_y<-h||m_y>BOTTOM) life=0;
if(life==0) deadflag=1;
radar::pip(m_x,m_y,34);
}


void bigon::move()
{
fx+=fdx;
fy+=fdy;
m_dx=R(static_cast<int>(fx)-m_x);
m_dy=static_cast<int>(fy)-m_y;
if(fdx<0)fdx+=XWIDTH;
if(fdx>XWIDTH)fdx-=XWIDTH;
m_x+=m_dx;
m_y+=m_dy;
m_x=RU(m_x);
}

bigon::bigon(int xin,int yin,float dxin,float dyin)
:sbaddie(xin,yin,0,0,bpelpic,1),life(250)
{
fx=xin;
fy=yin;
fdx=dxin;
fdy=dyin;
}

bigon::~bigon(){}

void bigon::init(void)
{
unsigned char spic[100]={
 0, 0, 0,33,33,33,33, 0, 0, 0,
 0, 0,33,33,33,33,33,33, 0, 0,
 0,33,33,34,34,34,34,33,33, 0,
33,33,34,34,35,35,34,34,33,33,
33,33,34,35,35,35,35,34,33,33,
33,33,34,35,35,35,35,34,33,33,
33,33,34,34,35,35,34,34,33,33,
 0,33,33,34,34,34,34,33,33, 0,
 0, 0,33,33,33,33,33,33, 0, 0,
 0, 0, 0,33,33,33,33, 0, 0, 0
};
make_bitmap(bpelpic,10,10);
for(int a=0;a<100;a++)bpelpic.putpixel(a%10,a/10,spic[a]);
}

void bigon::cleanup()
{
}

void bigon::become_evil()
{
//Let bigons be bigons... No effect
}



graphics::bitmap mine::minepic;

mine::mine(void)
:sbaddie(RANDX,rand()%400,0,0,minepic,100)
{
init_shoot();
}

//construct a minefield
mine::mine(int n)
:sbaddie(RANDX,50+rand()%300,0,0,minepic,100)
{
init_shoot();
int a;
float theta,r;

//Some sort of cone-shaped distribution
for(a=0;a<n;a++)
  {
  theta=2.0*M_PI*rand()/RAND_MAX;
  r=rand()%50;
  (void)(new mine(RU(static_cast<int>(m_x+r*cos(theta))),static_cast<int>(m_y+r*sin(theta))));
  }
}

mine::mine(int xin,int yin)
:sbaddie(xin,yin,0,0,minepic,100)
{
init_shoot();
}

void mine::init_shoot()
{
shoot_time=(rand()%4)*100;
salvo_length=(rand()%10)?1:30;
}

mine::~mine()
{
}

void mine::init()
{
int a;
unsigned char data[100]=
{
34,35,35,35,35,35,35,35,35,36,
33,34,35,35,35,35,35,35,36,37,
33,33,34,35,35,35,35,36,37,37,
33,33,33,34,35,35,36,37,37,37,
33,33,33,33,34,36,37,37,37,37,
33,33,33,33,34,36,37,37,37,37,
33,33,33,34,35,35,36,37,37,37,
33,33,34,35,35,35,35,36,37,37,
33,34,35,35,35,35,35,35,36,37,
34,35,35,35,35,35,35,35,35,36
};
make_bitmap(minepic,10,10);
for(a=0;a<100;a++)minepic.putpixel(a%10,a/10,data[a]);
}

void mine::cleanup()
{
}

void mine::misc()
{
if(++shoot_time==salvo_length*400)shoot_time=0;
if(shoot_time<salvo_length)shoot<bogon>(&target,4.0);
radar::pip(m_x,m_y,35);
}

graphics::bitmap red_tape::tapepic;
int red_tape::yarray[200];
int red_tape::dyarray[200];

red_tape::red_tape(int n)
:sbaddie(RANDX,rand()%300+50,2*(rand()%2)-1,0,tapepic,100)
{
int a;
phase=0;
ymean=m_y;
for(a=0;a<n;a++)(void)(new red_tape(m_x,ymean,m_dx,m_dx*a));
}

red_tape::red_tape(int xin,int yin,int dxin,int	xdisp)
:sbaddie(RU(xin+5*xdisp),0,dxin,0,tapepic,100)
{
ymean=yin;
phase=6*abs(xdisp);
m_y=ymean+yarray[phase];
}

red_tape::~red_tape()
{

}

void red_tape::init(void)
{
make_bitmap(tapepic,10,10);
//red(colour 4) circle, black background
tapepic.clear(4);
for(int a=0;a<10;a++)
  for(int b=0;b<10;b++)
    {
    if((2*a-9)*(2*a-9)+(2*b-9)*(2*b-9)>89)tapepic.putpixel(b,a,0);
    }

//lookup tables for y and dy.
for(int a=0;a<200;a++)
  yarray[a]=static_cast<int>(0.5+30*sin(2.0*M_PI*a/200));
for(int a=0;a<200;a++)
  dyarray[a]=yarray[a]-yarray[(a+199)%200];
}

void red_tape::cleanup(void)
{
}

void red_tape::move(void)
{
phase=(phase+1)%200;
m_y=ymean+yarray[phase];
m_dy=dyarray[phase];
m_x=RU(m_x+m_dx);
}

void red_tape::misc(void)
{
radar::pip(m_x,m_y,4);
}

void red_tape::takeshot(int n)
{
  shoot<bogon>(&target,4.0);
  sbaddie::takeshot(n);
}


graphics::bitmap firewall::wallpic[6];

firewall::firewall(int n)
:sbaddie(RANDX,0,4*(rand()%2)-2,1,wallpic[0],150)
{
int d,bratdy;
for(int a=1;a<n;a++)
  {
  d=780*a/n;
  if(d>390)
    {
    d=780-d;
    bratdy=-1;
    }
  else bratdy=1;
  (void)(new firewall(m_x,d,m_dx,bratdy));
  }
animflag=0;
}

firewall::firewall(int xin,int yin,int dxin,int	dyin)
:sbaddie(xin,yin,dxin,dyin,wallpic[0],150)
{
animflag=0;
}

firewall::~firewall(){}

void firewall::init()
{
unsigned char data[200]=
{
0, 0, 1, 1, 1, 1, 1, 1,	0, 0,
0, 1,95,94,93,92,91,96,	1, 0,
1,91,96, 1, 1, 1, 1,95,94, 1,
1,92, 1, 1, 0, 0, 1, 1,93, 1,
1,93, 1, 0, 0, 0, 0, 1,92, 1,
1,94, 1, 0, 0, 0, 0, 1,91, 1,
1,95, 1, 0, 0, 0, 0, 1,96, 1,
1,96, 1, 0, 0, 0, 0, 1,95, 1,
1,91, 1, 0, 0, 0, 0, 1,94, 1,
1,92, 1, 0, 0, 0, 0, 1,93, 1,
1,93, 1, 0, 0, 0, 0, 1,92, 1,
1,94, 1, 0, 0, 0, 0, 1,91, 1,
1,95, 1, 0, 0, 0, 0, 1,96, 1,
1,96, 1, 0, 0, 0, 0, 1,95, 1,
1,91, 1, 0, 0, 0, 0, 1,94, 1,
1,92, 1, 0, 0, 0, 0, 1,93, 1,
1,93, 1, 1, 0, 0, 1, 1,92, 1,
1,94,95, 1, 1, 1, 1,96,91, 1,
0, 1,96,91,92,93,94,95,	1, 0,
0, 0, 1, 1, 1, 1, 1, 1,	0, 0,
};
//six frames, with dots moving round 91-92-93-94-95-96 in the data
for(int a=0;a<6;a++)
  {
  make_bitmap(wallpic[a],10,20);
  for(int b=0;b<200;b++)
    wallpic[a].putpixel(b%10, b/10, data[b]>90?9+5*!((a+data[b])%6):data[b]);
  }
}

void firewall::cleanup()
{
}

void firewall::misc()
{
if(m_y<=0||m_y>=390)shoot<bigon>(&target,4);
radar::pip(m_x,m_y,9);
}

void firewall::move()
{
if(m_y<=0) m_dy=1;
if(m_y>=BOTTOM-DEFHEIGHT-h) m_dy=-1;

m_x=RU(m_x+m_dx);
m_y+=m_dy;
}

void firewall::animate()
{
animflag=(animflag+1)%48;
pic=&wallpic[animflag/8];
}


graphics::bitmap bundle::bunpic;

void bundle::misc()
{
radar::pip(m_x,   m_y,   11);
radar::pip(m_x+14,m_y,   11);
radar::pip(m_x,   m_y+14,11);
radar::pip(m_x+14,m_y+14,11);
}

void bundle::move()
{
if(target.islive())
  {
  m_dx=( R(target.getx()-getxcentre()) > 0 ? 1 : -1 );
  m_x=R(m_x+m_dx);
  }
else m_dx=0;
}

bundle::bundle(int n)
:sbaddie(RANDX,190,0,0,bunpic,1000)
{
int a;
for(a=0;a<n;a++)drones.add(new drone(this));
}

bundle::~bundle()
{
while(drones.size())delete(drones+0);
}


void bundle::init(void)
{
make_bitmap(bunpic,24,24);
bunpic.clear(11);
}

void bundle::cleanup(void)
{
}

void bundle::takeshot(int n)
{
if(drones.size()==0&&deadflag==0)
  {
  (void)(new powerup(m_x+4,m_y+4,rand()%7));
  sbaddie::takeshot(n);
  }
}

graphics::bitmap drone::dronepic;

void drone::move()
{
int ox,oy;
ox=m_x;oy=m_y;
phase=(phase+1)%150;
m_x=RU(parent->getxcentre()+static_cast<int>(cos(2.0*M_PI*phase/150)*xr)-w/2);
m_dx=R(m_x-ox);
m_y=parent->getycentre()+static_cast<int>(sin(2.0*M_PI*phase/150)*yr)-h/2;
m_dy=m_y-oy;
}

void drone::misc(void)
{
radar::pip(m_x,m_y,3);
}

drone::drone(bundle *b)
:sbaddie(b?b->m_x:0,b?b->m_y:0,0,0,dronepic,20)
{
xr=rand()%100+8;
yr=rand()%100+8;
phase=rand()%150;
parent=b;
}

drone::~drone(void)
{
if(parent)parent->drones.remove(this);
}

void drone::init(void)
{
make_bitmap(dronepic,10,10);
dronepic.clear(3);
}

void drone::cleanup()
{
}

graphics::bitmap dom_s_dos::dospic[64];

dom_s_dos::dom_s_dos()
:sbaddie(RANDX, rand()%300, -(5+rand()%5), 0, dospic[0], 15000)
{
animflag = 0;
energy = 50;
}

void dom_s_dos::init()
{
const int npoints = 9;
int point[npoints] = {0, 10, 38, 100, 105, 125, 130, 197, 200};
int val[npoints] =   {8,  8, 25,  25,  20,  20,  25,  25, 22};
char rad[200];

for(int b=0; b<npoints-1; b++)
  for(int a=point[b]; a<point[b+1];a++)
    rad[a]= (val[b+1]*(a-point[b]) + val[b]*(point[b+1]-a)) /
                (point[b+1]-point[b]);

const char *dsd = "DOM-S-DOS";
graphics::bitmap dsdpic(strlen(dsd)*8, 16);
clear(dsdpic);
dsdpic.drawtext(dsd, 0, 0, 1);
const int dsdstart(42);
const int dsdend(190);

for(int a=0; a<64; a++)
  {
  make_bitmap(dospic[a], 200, 50);
  clear(dospic[a]);
  for(int x=0; x<200; x++)
    {
    int ymin = 25-rad[x];
    int ymax = 24+rad[x];
    for(int y=ymin; y<=ymax; y++)
       {
       double angle = acos((24.5-y)/(ymax-ymin)*2) + M_PI*a/64;
       dospic[a].putpixel(x, y, (int)(angle/M_PI*32)%8?1:9);
       if(x>=dsdstart && x<dsdend)
          {
          if(dsdpic.getpixel( (x-dsdstart)*(dsdpic.width())/(dsdend-dsdstart),
             (int)((angle/M_PI)*dsdpic.height())%dsdpic.height()))
            {
            dospic[a].putpixel(x, y, 15);
            }
          }
       }
    }
  }
}

void dom_s_dos::cleanup()
{
}

void dom_s_dos::move()
{
  m_x = RU(m_x+m_dx);
}

void dom_s_dos::animate()
{
  animflag = (animflag+1)%64;
  pic = &dospic[animflag];
}

void dom_s_dos::misc()
{
  if (target.islive())
  {
    int xdiff = R(m_x-target.getx());
    //This one is quite nasty - if the target is somewhere in front of us,
    //we fire loads of bigons in a vaguely forward direction
    if (xdiff>0 && xdiff<500)
    {
      int r = (rand()%4)+4;
      double theta = M_PI*rand()/RAND_MAX;
      (void)(new bigon(R(m_x+5-5), getycentre()-5, m_dx-r*sin(theta),
                                      r*cos(theta)));
      sound::play(sound::bpop,255);
    }
  }
  radar::bigpip(m_x, m_y, *pic);
}


void dom_s_dos::takeshot(int n)
{
  energy--;
  if (!energy && !deadflag)
  {
    deadflag=1;
    for(int a=0; a<10; a++)
    {
      explode(m_x+rand()%w,m_y+rand()%h);
    }
    (void)(new powerup(m_x+rand()%w, m_y+rand()%h,powerup::XTRA_LIFE));
    (void)(new powerup(m_x+rand()%w, m_y+rand()%h,rand()%8));
    score[n]+=points;
  }
  else
  {
    sound::play(sound::ouch, 255);
  }
}

graphics::bitmap vip::vippic;
graphics::bitmap vip::edgeL;
graphics::bitmap vip::edgeR;

const char *vip::blurb[]={

" DID YOU KNOW..?  RESISTANCE IS FUTILE.  YOU WILL BE ASSIMILATED. ",  //Borg
" DID YOU KNOW..?  THE PLANET EXPLODER IS ENTIRELY INVINCIBLE IN EVERY RESPECT. ",
" DID YOU KNOW..?  SOON, PEACE AND ORDER WILL BE RESTORED THROUGHOUT THE GALAXY. ", //Tie fighter
" DID YOU KNOW..?  YOUR PITIFUL REBELLION WILL BE UTTERLY DESTROYED.  "
   "ONLY THE EMPIRE WILL REMAIN. ",
" DID YOU KNOW..?  THE JEDIX WILL NEVER RETURN. ",
" DID YOU KNOW..?  ALL YOUR BASE ARE BELONG TO US. ", //Sorry, couldn't resist it.
" TIP: DO NOT UNDERESTIMATE THE DARK SIDE OF THE SOURCE. ",  //Empire Strikes Back
" TIP: BELIEVE THE HYPE. ",
" TIP: READ THE LICENSE AGREEMENT.  YOU BELONG TO US. ",
" TIP: SURRENDER OR BE DESTROYED. ",
" TIP: DIE, REBEL SCUM! ",
" TIP: BE YE AFRAID.  BE YE GREATLY AFRAID. ", //Gospel of Tux
" PLANET EXPLODER: WHAT DO YOU WANT TO BLOW UP TODAY? ",
" ONE VISION.  ONE PURPOSE. ", //Tiberian Sun
" NICE PLANET...  WE'LL BLOW IT UP. ", //Mars Attacks! 
" C:  C:\\WINDOWS  C:\\WINDOWS\\CRAWL  CRAWL\\WINDOWS\\CRAWL ",  //As in: "See Spot run"
" OS/2 - HALF AN OPERATING SYSTEM.  MUHAHAHAHA!!!!! ",
" DID YOU READ THE LICENSE AGREEMENT?  IF WE DESTROY YOUR PLANET, IT'S NOT OUR FAULT. ",
" FREEDOM IS IRRELEVANT.  COMPETITION IS IRRELEVANT.  "
   "WHERE *YOU* WANT TO GO TODAY IS IRRELEVANT. "  //More Borg.

};

int vip::nblurbs(sizeof(vip::blurb)/sizeof(*vip::blurb));
int vip::messx1;
int vip::messx2;
int vip::messy;

void vip::animate(void)
{
myblurb.drawtext(blurb[curr_mess],mess_step-8*strlen(blurb[curr_mess]),0,4,0);
scrollflag++;
scrollflag%=3;
if(scrollflag)mess_step--;
if(mess_step==0)
  {
  curr_mess=rand()%nblurbs;
  mess_step=8*strlen(blurb[curr_mess])+messx2-messx1;
  }
blit(myblurb,vippic,0,0,messx1,messy,messx2-messx1+1,8);
masked_blit(edgeL,vippic,0,0,messx1,messy,4,8);
masked_blit(edgeR,vippic,0,0,messx2-3,messy,4,8);
}

void vip::misc()
{
if(neyes==0)
  {
  if(!isfalling)shoot<bogon>(&maintarget,3.0);
  }
else
  if(neyes==1)
    if(!(rand()%20))(void)(new mine(RU(m_x+rand()%30),m_y+rand()%30));
//secondtarget is only live if become_evil was called
if(!isfalling&&!(rand()%40))shoot<bogon>(&secondtarget,4.1);
if(!isfalling&&!(rand()%40))shoot<bigon>(&secondtarget,7.0);
}

void vip::move()
{
int xdiff;
if(secondtarget.islive())
  {
  xdiff=R(secondtarget.getx()-getxcentre());
  m_dx=(xdiff>0?1:-1);
  m_dx*=std::min(3,abs(xdiff));
  }
m_x=RU(m_x+m_dx);
if(isfalling)
  {
  dubdy+=GRAVITY;
  duby+=dubdy;
  m_dy=static_cast<int>(duby)-m_y;
  m_y=static_cast<int>(duby);
  if(!(rand()%10))explode(RU(m_x+rand()%140),m_y+rand()%40);
  if(m_y>420-h)
    {
    deadflag=1;
    colldet::addbomb(RU(getxcentre()));
    timerstuff::add_bomb();
    for(int a=0;a<100;a++)
      (void)(new clipbit(RU(m_x+rand()%w),m_y+rand()%h));
    for(int a=0;a<8;a++)
      sound::play(sound::kaboom,255);
    //Half of all sound channels.  This is loud!
    }
  }
}

void vip::clipart(int xc,int yc,int s)
{
if(s)
  vippic.putpixel(xc,yc,24);
else
  {
  vippic.putpixel(xc, yc-1, 28);
  vippic.putpixel(xc-1, yc, 28);
  vippic.putpixel(xc, yc+1, 20);
  vippic.putpixel(xc+1, yc, 20);
  }
}

vip::vip()
:sbaddie(RANDX,rand()%200+80,rand()%2*2-1,0,vippic,10000)
,secondtarget(0)
{
eyes[0]=new evil_eye(this,70-20-8);
eyes[1]=new evil_eye(this,70+20-8);
neyes=2;
energy=100;
scrollflag=0;
isfalling=0;
make_bitmap(myblurb,messx2-messx1+1,8);
myblurb.clear(0);
curr_mess=rand()%nblurbs;
mess_step=8*strlen(blurb[curr_mess])+messx2-messx1;
}

vip::~vip()
{
if(eyes[0])delete eyes[0];
if(eyes[1])delete eyes[1];
}

void vip::init(void)
{
int a,b;
double dubx,duby,theta;
make_bitmap(vippic,140,40);
clear(vippic);

for(b=0;b<2;b++)
  {
  for(a=20;a<=90;a++)
    clipart(a,35,b);
  for(a=50;a<=150;a++)
    {
    theta=2.0*M_PI*a/200;
    dubx=20.0+15.0*cos(theta)+0.5;
    duby=20.0+15.0*sin(theta)+0.5;
    clipart(static_cast<int>(dubx),static_cast<int>(duby),b);
  }
  for(a=20;a<=120;a++)
    clipart(a,5,b);
  for(a=150;a<250;a++)
    {
    theta=2.0*M_PI*a/200;
    dubx=120.0+13.0*cos(theta)+0.5;
    duby= 18.0+13.0*sin(theta)+0.5;
    clipart(static_cast<int>(dubx),static_cast<int>(duby),b);
    }
  for(a=50;a<=120;a++)
    clipart(a,31,b);
  for(a=50;a<=150;a++)
    {
    theta=2.0*M_PI*a/200;
    dubx=50.0+11.0*cos(theta)+0.5;
    duby=20.0+11.0*sin(theta)+0.5;
    clipart(static_cast<int>(dubx),static_cast<int>(duby),b);
    }
  for(a=50;a<=95;a++)
    clipart(a,9,b);
  }

messy=16;
a=90;
while(vippic.getpixel(a,19)==0)a--;
messx1=a;
make_bitmap(edgeL,4,8);
blit(vippic,edgeL,messx1,messy,0,0,4,8);

b=90;
while(vippic.getpixel(b,19)==0)b++;
messx2=b;
make_bitmap(edgeR,4,8);
blit(vippic,edgeR,messx2-3,messy,0,0,4,8);

}

void vip::cleanup()
{
}

void vip::takeshot(int n)
{
if(!neyes&&!isfalling)
  {
  energy--;
  sound::play(sound::ouch,255);
  if(!energy)
    {
    isfalling=1;
    duby=m_y;
    dubdy=0;
    //First 7 powerups only - no elephant gun!
    (void)(new powerup(RU(getxcentre()-8),getycentre()-8,rand()%7));
    score[n]+=points;
    }
  }
}

void vip::loseeye(evil_eye *eep)
{
neyes--;
if(eyes[0]==eep)eyes[0]=0;
if(eyes[1]==eep)eyes[1]=0;
}

void vip::become_evil(void)
{
secondtarget.settarget(defender::getdef());
}

void vip::teleport(int xin,int yin)
{
m_x=xin;
m_y=yin;
eyes[0]->teleport(m_x+70-20-8,m_y-20);
eyes[1]->teleport(m_x+70+20-8,m_y-20);
}


graphics::bitmap evil_eye::eyepic[32];

void evil_eye::animate()
{
int xdif,ydif,n;
if(target.islive())
  {
  ydif=target.gety()-getycentre();
  xdif=R(target.getx()-getxcentre());
  if(abs(xdif)+abs(ydif)>5)
    {
    if(ydif>0)
      {
      n=static_cast<int>(atan2(ydif,xdif)/2.0/M_PI*32.0+64.5);
      n=n%32;
      if(n>=0&&n<32)pic=&eyepic[n];
      }
    else
      {
      n=static_cast<int>(atan2(-ydif,-xdif)/2.0/M_PI*32.0+80.5);
      n=n%32;
      if(n>=0&&n<32)pic=&eyepic[n];
      }
    }
  }
else pic=&eyepic[8];
}
void evil_eye::move()
{
int oldx=m_x;
m_x=RU(parent->getx()+xoff);
m_dx=R(m_x-oldx);
}

void evil_eye::misc(void)
{
if(!(rand()%50))shoot<bogon>(&target,0.5*(rand()%5+6));
radar::pip(m_x,m_y,15);
}

evil_eye::evil_eye(vip *pin,int xoffin)
:sbaddie(pin->m_x+xoffin,pin->m_y-20,0,0,eyepic[0],2500)
{
parent=pin;
energy=80;
xoff=xoffin;
}

evil_eye::~evil_eye()
{
parent->loseeye(this);
}

void evil_eye::init(void)
{
int n,a,b;
for(n=0;n<32;n++)
  {
  make_bitmap(eyepic[n],16,16);
  eyepic[n].clear(15);
  for(a=0;a<16;a++)
    for(b=0;b<16;b++)
      if((2*a-15)*(2*a-15)+(2*b-15)*(2*b-15)>235)
	eyepic[n].putpixel(b,a,0);
  
  int x=static_cast<int>(8.0+5.0*cos(2.0*M_PI*n/32.0));
  int y=static_cast<int>(8.0+5.0*sin(2.0*M_PI*n/32.0));
  
  int col[]={16, 21, 21, 21, 21, 27, 27, 27, 27};
  int xx[]  ={ 0,  0,  0,  1, -1,  1,  1, -1, -1};
  int yy[]  ={ 0,  1, -1,  0,  0,  1, -1,  1, -1};
  for(a=0; a<9; a++)
    eyepic[n].putpixel(x+xx[a], y+yy[a], col[a]);
  }
}

void evil_eye::cleanup(void)
{
}

void evil_eye::takeshot(int n)
{
energy--;
sound::play(sound::ouch,255);
if(!energy&&!deadflag)
  {
  deadflag=1;
  explode(getxcentre(),getycentre());
  score[n]+=points;
  }
}

void evil_eye::become_evil(void)
{
//evil eyes are already evil
}

graphics::bitmap clipbit::bitpic[16];

void clipbit::animate()
{
animflag=(animflag+da+160)%160;
pic=&bitpic[animflag/10];
}

void clipbit::move()
{
int xx,yy;
dubx+=dubdx;
if(dubx<0)dubx+=XWIDTH;
if(dubx>XWIDTH)dubx-=XWIDTH;
xx=static_cast<int>(dubx);
m_dx=R(xx-m_x);
m_x=xx;
dubdy+=GRAVITY;
duby+=dubdy;
yy=static_cast<int>(duby);
m_dy=yy-m_y;
m_y=yy;
if(m_y+h>BOTTOM)explode(getxcentre(),getycentre());
}

clipbit::clipbit(int xin,int yin)
:sprite(xin,yin,0,0,bitpic[0],level_fg)
{
animflag=rand()%160;
da=rand()%5-2;
dubx=xin;
duby=yin;
dubdx=-0.7+0.05*(rand()%70);
dubdy=-1.0-0.1*(rand()%50);
}

clipbit::~clipbit(void)
{

}

void clipbit::init(void)
{
char data[100]={
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
24,20,20, 0, 0, 0, 0, 0, 0, 0,
 0,24,24,20,20, 0, 0, 0, 0, 0,
 0,28,28,24,24,20,20, 0, 0, 0,
 0, 0, 0,28,28,24,24,20,20, 0,
 0, 0, 0, 0, 0,28,28,24,24, 0,
 0, 0, 0, 0, 0, 0, 0,28,28,24,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

for(int a=0;a<16;a++)make_bitmap(bitpic[a],10,10);
for(int a=0;a<10;a++)
  for(int b=0;b<10;b++)
    {
    bitpic[0].putpixel(b,a, a>3?a<7?a*4+4:0:0);
    bitpic[1].putpixel(b,a, data[10*a+b]);
    bitpic[2].putpixel(b,a, (abs(a-b)<2&&abs(a+b-9)<8)?(a-b)*4+24:0);
    bitpic[3].putpixel(b,a, data[10*(9-b)+(9-a)]);
    }
for(int a=4;a<16;a++)
  for(int b=0;b<10;b++)
    for(int c=0;c<10;c++)
      bitpic[a].putpixel(c,b, bitpic[a-4].getpixel(9-b,c));
}

void clipbit::cleanup()
{
}

int clipbit::isdead(){return (m_y+h>BOTTOM);}

graphics::bitmap bootdisk::diskpics[64];

void bootdisk::animate()
{
animflag=(animflag+1)%256;
pic=&diskpics[animflag/4];
}

void bootdisk::misc()
{
if(!(rand()%5))(void)(new boot(RU(m_x+rand()%90),m_y+12));
}


void bootdisk::move()
{
moveinc=(moveinc+1)%3;
m_dx=!moveinc;
m_x=RU(m_x+m_dx);
}


bootdisk::bootdisk()
:sbaddie(RANDX,rand()%100,0,0,diskpics[0],500)
{
energy=100;
animflag=rand()%256;
}

bootdisk::~bootdisk()
{

}

void bootdisk::init()
{
  double dubx,duby,c,s;
  int rx,ry;

  graphics::bitmap tempdata(50,50);
  //Draw shaded grey background
  for(int xx=0;xx<50;xx++)
    for(int yy=0;yy<50;yy++)
      tempdata.putpixel(xx, yy, 19+xx/5+
       static_cast<int>(0.5+((xx-24.5)*(xx-24.5)+(yy-24.5)*(yy-24.5))*3/625));
  //Draw logo on top
  tempdata.rectfill(12,12,37,37,0);
  tempdata.rectfill(14,14,23,23,4);
  tempdata.rectfill(14,26,23,35,2);
  tempdata.rectfill(26,14,35,23,9);
  tempdata.rectfill(26,26,35,35,14);


  for(int a=0;a<64;a++)
  {
    make_bitmap(diskpics[a],100,30);
    clear(diskpics[a]);

    for(int xx=0;xx<100;xx++)
    {
      //draw the top of the disk
      for(int yy=0;yy<20;yy++)
      {
        //The disk is rotating.  dubx and duby are coords of this pixel on the
        //disk, measured from the centre
        dubx=xx-49.5;
        duby=49.5/9.5*(yy-9.5);
        if(dubx*dubx+duby*duby<2500)
        {
          c=cos(2.0*M_PI*a/64.0);
          s=sin(2.0*M_PI*a/64.0);
          rx=static_cast<int>(floor(0.5+0.5*(c*dubx+s*duby)+24.5));
          ry=static_cast<int>(floor(0.5+0.5*(s*dubx-c*duby)+24.5));
          if(rx<0||rx>50||ry<0||ry>50)
            //paranoia
	      diskpics[a].putpixel(xx,yy,7);
	    else
	      diskpics[a].putpixel(xx,yy,tempdata.getpixel(rx,ry));
        }
      }
      //draw the "skirt" around the disk.  The height of this depends on
      //the angle, which can be computed from x coord and frame number
      int yup=29;
      while(diskpics[a].getpixel(xx,yup)==0) yup--;
      double angle = acos((49.5-xx)/50)+2.0*M_PI*a/64.0;
      diskpics[a].vline(xx,yup+1,yup+(cos(4*angle)>0?8:4),
            static_cast<int>(0.5+3.0*cos(4*angle))+41);
      double realheight=4.0*(sqrt(2500-dubx*dubx)*9.5/49.5+9.5-yup);
      diskpics[a].putpixel(xx, yup+(cos(4*angle)>0?9:5), 45+static_cast<int>(realheight));

      yup=0;
      while(diskpics[a].getpixel(xx,yup)==0)yup++;
      if(yup)diskpics[a].putpixel(xx, yup-1,
        static_cast<int>((diskpics[a].getpixel(xx,yup)-16)*realheight/4.0)+16);
    }
  }
}

void bootdisk::cleanup()
{
}

void bootdisk::takeshot(int n)
{
energy--;
sound::play(sound::ouch,255);
if(!energy&&!deadflag)
  {
  deadflag=1;
  explode(getxcentre(),getycentre());
  if(rand()%4)
    {
    (void)(new powerup(m_x,   m_y,powerup::RAILGUN));
    (void)(new powerup(m_x+42,m_y,powerup::AUTOFIRE));
    (void)(new powerup(m_x+84,m_y,powerup::SHIELD));
    }
  else
    (void)(new powerup(m_x+42,m_y,powerup::ELEPHANTGUN));
  score[n]+=points;
  }
}


graphics::bitmap boot::bootpic;

void boot::move()
{
m_y+=2;
if(m_y>410)deadflag=1;
}

void boot::misc()
{
radar::pip(m_x,m_y,41);
}


boot::boot(int xin,int yin)
:sbaddie(xin,yin,0,2,bootpic,1)
{

}

boot::~boot(){}

void boot::init(void)
{
char data[100]={
40,38,44,42, 0, 0, 0, 0, 0, 0,
40,38,44,42, 0, 0, 0, 0, 0, 0,
40,41,41,42, 0, 0, 0, 0, 0, 0,
40,41,41,42, 0, 0, 0, 0, 0, 0,
40,41,41,42, 0, 0, 0, 0, 0, 0,
40,41,41,42, 0,39,40,41,42, 0,
40,41,41,41,40,40,41,41,41,42,
40,41,41,41,41,41,41,41,41,41,
40,41,41,41,41,41,41,41,41,41,
40,41,41,41, 0,41,41,41,41,41};

make_bitmap(bootpic,10,10);
for(int a=0;a<100;a++)bootpic.putpixel(a%10, a/10, data[a]);
}

void boot::cleanup()
{
}

void boot::become_evil()
{
//Do nothing... this is a "missile" baddie, not a "baddie" baddie
}



graphics::bitmap exploder::explpic[5];
int exploder::yarray[100];
int exploder::dyarray[100];

void exploder::animate()
{
animflag=(animflag+1)%50;
pic=&explpic[animflag/10];
}

void exploder::misc()
{
radar::pip(m_x+11,m_y   ,34);
radar::pip(m_x+22,m_y   ,34);
radar::pip(m_x   ,m_y+11,34);
radar::pip(m_x+33,m_y+11,34);
radar::pip(m_x   ,m_y+22,34);
radar::pip(m_x+33,m_y+22,34);
radar::pip(m_x+11,m_y+33,34);
radar::pip(m_x+22,m_y+33,34);
}

void exploder::move()
{
int xdiff;
if(target.islive())
  {
  xdiff=R(target.getx()-getxcentre());
  m_dx=2*(xdiff>0?1:-1);
  m_x=RU(m_x+m_dx);
  if(abs(xdiff)<4)
    {
    int r5=rand()%5;
    deadflag=1;
    for(int b=0;b<16;b++)
      {
      double dc=cos(2.0*M_PI*(b+0.2*r5)/16.0);
      double ds=sin(2.0*M_PI*(b+0.2*r5)/16.0);
      int xs=static_cast<int>(17.5+17.5*dc);
      int ys=static_cast<int>(17.5+17.5*ds);
      (void)(new bigon(m_x+xs,m_y+ys,2*ds,-2*dc));
      }
    sound::play(sound::bpop,255);
    sound::play(sound::bpop,255);
    sound::play(sound::bpop,255);
    sound::play(sound::bpop,255);
    }
  }
else m_dx=0;
m_y=ymean+yarray[phaseflag];
m_dy=dyarray[phaseflag];
phaseflag=(phaseflag+1)%100;

}

exploder::exploder()
:sbaddie(RANDX,rand()%200+100,0,0,explpic[0],1500)
{
ymean=m_y;
animflag=rand()%50;
phaseflag=rand()%100;
energy=16;
}

exploder::exploder(int x,int y)
:sbaddie(x,y,0,0,explpic[0],1500)
{
ymean=m_y;
animflag=rand()%50;
phaseflag=rand()%100;
energy=16;
}

exploder::~exploder(){}

void exploder::init(void)
{
unsigned char spic[100]={
 0, 0, 0,33,33,33,33, 0, 0, 0,
 0, 0,33,33,33,33,33,33, 0, 0,
 0,33,33,34,34,34,34,33,33, 0,
33,33,34,34,35,35,34,34,33,33,
33,33,34,35,35,35,35,34,33,33,
33,33,34,35,35,35,35,34,33,33,
33,33,34,34,35,35,34,34,33,33,
 0,33,33,34,34,34,34,33,33, 0,
 0, 0,33,33,33,33,33,33, 0, 0,
 0, 0, 0,33,33,33,33, 0, 0, 0
};
graphics::bitmap tempbmap(10,10);
graphics::bitmap hbmap(22,44);
for(int a=0;a<100;a++)tempbmap.putpixel(a%10,a/10,spic[a]);
for(int a=0;a<100;a++)yarray[a]=static_cast<int>(0.5+20*sin(2.0*a*M_PI/100));
for(int a=0;a<100;a++)dyarray[a]=yarray[a]-yarray[(a+99)%100];
for(int a=0;a<5;a++)
  {
  make_bitmap(explpic[a],44,44);
  clear(explpic[a]);
  for(int b=0;b<16;b++)
    {
    int xs=static_cast<int>(17.5+17.5*cos(2.0*M_PI*(b+0.2*a)/16.0));
    int ys=static_cast<int>(17.5+17.5*sin(2.0*M_PI*(b+0.2*a)/16.0));
    masked_blit(tempbmap,explpic[a],0,0,xs,ys,10,10);
    if(b==7)blit(explpic[a],hbmap,22,0,0,0,22,44);
    if(b==15)masked_blit(hbmap,explpic[a],0,0,22,0,22,44);
    }

  }
}

void exploder::cleanup(void)
{
}

void exploder::takeshot(int n)
{
if(energy)
  {
  energy--;
  sound::play(sound::ouch,255);
  }
if(!energy&&!deadflag)
  {
  deadflag=1;
  if(!(rand()%10))(void)(new powerup(RU(m_x+14),m_y+14,powerup::ITCURSE));
  for(int b=0;b<16;b++)
    {
    int xs=static_cast<int>(21.5+17.5*cos(2.0*M_PI*b/16.0));
    int ys=static_cast<int>(21.5+17.5*sin(2.0*M_PI*b/16.0));
    explode(m_x+xs,m_y+ys);
    }
  score[n]+=points;
  }
}







