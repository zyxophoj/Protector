/*
prot.cpp: game classes - display, sound, radar, player, baddies etc

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
#ifndef PROT_H
#define PROT_H

#include <list>
#include <set>
#include <vector>
#include <cmath>
#include "dlring.h"
#include "graphics.h"
#include "sound.h"

//R = R-educe modulo 4096
//RU "unsigned" version - used to calculate x co-ordinates which must
//be from 0 to 4096-1
//R is used to calculate the difference between two x co-ordinates.
//differences should be in the range -2048 to +2048

//x&4095 == x%4096 for good numbers

#define XWIDTH 4096
#define R(x) ((((x)+6144)&4095)-2048)
#define RU(x) (((x)+4096)&4095)
#define RANDX (rand()%4096)

//bottom of the game area 
const int BOTTOM = 420;

//Warning: Do not change the laws of physics unless you know what you are
//doing.

//The gravitational acceleration, in pixels per frame per frame
const float GRAVITY(0.03);
//Fraction of velocity lost per frame.  Note that air resistance is the only
//thing which stops players from achieving very high speeds.
const float AIR_RESISTANCE(0.01);

inline int randint(int n){return n?(rand()%n):0;}

//hack: size of the player
#define DEFHEIGHT 10
#define DEFWIDTH 20

namespace graphics{
class screen_;
}

class sprite;
class sprite1;
class defender;
class deftarget;
class pod;
class sbaddie;
class bug;
class lawyer;
class bogon;
class bundle;
class drone;
class evil_eye;
class boot;
class exploder;
class star;
class letter;
class drect;
class colldet;
class radar;


//drect = dirty rectangle
//When a sprite moves (or stands still for a frame), it generates
//a drect - the smallest rectangle containing the sprite's current
//position and its old position.  The drawing code blits the drects
//and not the whole screen.  This is faster when there's lots of
//empty space.
class drect
{
public:
  drect(int, int, int, int);
  void draw(const graphics::bitmap &, graphics::screen_ &, int, int);
  void cleardraw(graphics::bitmap &);

private:
  const int x,y,w,h;
};

//Display "owns" a rectangle somewhere on the screen,
//and displays stuff centred on a player
class display
{
friend class sprite;
friend class sprite1;
friend class pod;
friend class radar;
friend class defender;

public:
  display(int,int,int,int,int,radar *);
  ~display();
  int gethw(void);
  int getcx(void);
  int getcdx(void);
  void add_drect(drect *);
  void add_sprite1(sprite1 *);
  void remove_sprite1(sprite1 *);
  void drawall(graphics::screen_ &);
  static void drawsprites(graphics::screen_ &);
  static void allmessage(const char *);
  void message(const char *);
  static void killall(void);

private:
  static std::list<display *> displays;
  std::list<drect *> drects;
  std::set<sprite1 *> sprite1s;
  defender *centre;
  graphics::bitmap vscreen;
  int w, h, halfw;
  int xoff;
  int yoff;
  radar *m_pradar;
  graphics::bitmap ground;
};

class radar
{
public:
  radar(int,int,int,int);
  ~radar();
  void set(display *);
  static void wipe();
  static void pip(int,int,unsigned char);
  static void bigpip(int, int, const graphics::bitmap &);
  static void alert(int);
  static void drawall(graphics::screen_ &);

private:
  static std::list<radar *>radars;

  void draw(graphics::screen_ &);
  
  int top;
  int left;
  int width;
  int height;
  float xs;
  float ys;
  display *d;
  static int timer;
  graphics::bitmap pad;
};

//Base class for sprites.  Does the drect drawing stuff.
class sprite
{
  friend class colldet;

public:
  enum {level_bg = 0,
        level_fg = 1,
        levels = 2};
  static std::set<sprite *> sprites[levels];

  sprite(int,int,int,int,const graphics::bitmap &,int);
  virtual ~sprite();
  void gameinc1();
  void gameinc2();
  void demoinc(graphics::screen_ &);
  bool pixel_by_pixel(sprite *);
  virtual void teleport(int x,int y) {m_x=x;m_y=y;} //a heinous hack
  virtual int isdead() {return 0;}
  int getxcentre() {return m_x+w/2;}
  int getycentre() {return m_y+h/2;}
  int getx()       {return m_x;}
  int gety()       {return m_y;}
  virtual void drawoff();

  const int w,h;

  static void kill_all();
  static void process_all();

protected:
  int m_x,m_y;
  int m_dx,m_dy;
  const graphics::bitmap *pic;
  virtual void misc(){};
  virtual void move(){};
  virtual void animate(){};
  virtual void redraw();

private:
  int m_level;

};

//A special sprite which only appears in one display.
//used mainly for player messages, like "You're it!"
class sprite1:public sprite
{
public:
  sprite1(int,int,int,int,const graphics::bitmap &,display *);
  virtual ~sprite1();
  virtual int isdead();
  virtual void redraw();
  void drawoff();
  display *get_disp(){return pdisp;}

protected:
  display *pdisp;

};


//The player's ship
class defender:public sprite
{
friend class deftarget;
friend class display;
friend class colldet;

public:
  static int violent;
  defender(int,int,int);
  ~defender();
  static void init();
  static defender *getdef();
  void die();
  int isdead();
  static int alldead();
  static void make_all_invulnerable();
  void getpowerup(int);
  static void cheat();
  void setpassenger(pod *ps) {passenger=ps;}
  pod *getpassenger() {return passenger;}

  int getx() {return m_x;}
  int gety() {return m_y;}
  int getdx(){return m_dx;}
  float getfdx() {return fdx;}
  float getfdy() {return fdy;}
  bool isshielded() {return !!shieldstate;}
  bool isinvuln() {return !!invtime;}
  const int number;

private:
  void drawoff();
  static graphics::bitmap defpics[9];
  static dlring<defender> defenders;

  int shottime;
  int lr;
  float fdx,fdy,fx,fy;
  int colltime;
  pod *passenger;
  int lives;
  int bombs;
  char bombpress;
  int shieldstate;
  int shieldtime;
  int shieldpress;
  int invtime;
  
  int railshots;
  int autoshots;
  int longshots;
  int elephants;
  int type;
  display *disp;

  void misc(void);
  void move(void);
  void update(void);
  void make_invulnerable();
};

//Used by baddies which "target" a player (to shoot at, home in on, etc)
//Keeping a list of all deftargets means we can do fun things like
//it-curse a player
class deftarget
{
public:
  deftarget();
  deftarget(defender *);
  virtual ~deftarget();
  bool islive(){return !!dt;}
  virtual int getx(){return dt->getxcentre();}
  virtual int gety(){return dt->getycentre();}
  virtual float getfdx() {return dt->getfdx();}
  virtual float getfdy() {return dt->getfdy();}
  void settarget(defender *);
  static void defdie(defender *);
  static void itcurse(defender *);
  static void makerandom();

private:
  static dlring<deftarget>deftargets;
  defender *dt;
  void not_this_one(defender *);
};

//letter for displaying messages
class letter:public sprite1
{
public:
  static int nletters(void);

  letter(int,int,int,display *);
  ~letter();
  static void init(void);
  static void message(const char *,display *);
  static void kill1(void);
  int isdead(void);
  
private:
  static std::set<letter *>letters;
  static graphics::bitmap lpic[256];

  int deadflag;
  deftarget target;
  void move(void);

};

//Shot fired by player.  
class shot:public sprite
{
friend class colldet;

public:
  static void init();

  shot(int,int,int,int,int);
  ~shot();
  int isdead(void);
  void hitbaddie(){if(!railness) life=0;}
  int getplayernum(){return n;}
  enum //shot flags
  {
    SF_RAIL=1,
    SF_LONG=2
  };

private:
  static graphics::bitmap spic;
  static std::set<shot *> shots;

  int life;
  int railness;
  int n;
  void move();
};



//Fired by player with "elephant gun"
class elephant:public sprite
{
friend class colldet;

public:
  static void init(void);

  elephant(int,int,float,float,int,int);
  ~elephant();  
  int isdead(void);
  int getplayernumber() {return n;}

private:
  static graphics::bitmap epic[2];
  static std::set<elephant *>elephants;
  float fx,fy,fdx,fdy;
  int n;
  void move(void);

};

//Fading square, for explosion effect
class fader:public sprite
{
public:
  fader(int,int,int);
  ~fader();
  static void init();
  int isdead();

private:
  int life;
  static graphics::bitmap fpics[16];
  void animate();
  void misc();
  void move();
};

//The things you must save from the Evil Empire!
class pod:public sprite
{
friend class colldet;
friend class bug;

public:
  static void init();
  static pod *getvictim();
  static void make8();
  static int in();
  static int out();
  static void rescued() {passengers--; safe++;}

  pod();
  pod(int, int);
  ~pod();
  int isdead(void);
  void grabbed(defender *);
  void drop(int, int);

private:
  static graphics::bitmap podpic[3];
  static std::vector<pod *>pods;
  static int safe;
  static int passengers;

  int toggle;
  int bdx;
  int grabflag;
  int animflag;
  int deadflag;
  int altime;
  bug *attacker;
  defender *rescuer;
  void animate(void);
  void misc(void);
  void move(void);
  void redraw(void);
};

//Base class for agents of the Evil Empire
class sbaddie:public sprite
{
friend class colldet;

public:
  static void all_become_evil();
  static int population();
  static int alldead();

  sbaddie(int,int,int,int,const graphics::bitmap &,int);
  virtual ~sbaddie();
  int isdead();
  virtual void takeshot(int);
  virtual void takebomb();
  template<class missile> void shoot(deftarget *target, float speed);
  virtual void become_evil();

protected:
  int deadflag;
  int points;

private:
  static std::set<sbaddie *> sbaddies;
};

//Try to shoot a <missile> at the target.  
//Missile class must have a missile(x, y, dx, dy) constructor

template<class missile>
void sbaddie::shoot(deftarget *target, float speed)
{
  //How to shoot down a target moving at constant velocity:
  //The target is at displacement r from us, moving at velocity v
  //
  //v = (v - r * (v.r)/(r.r))  +  (r * (v.r)/(r.r))
  //The first component is perpendicular to r:
  //(v-r*(v.r)/(r.r)).r = v.r - (r.r)*(v.r)/(r.r) = 0
  //The second is a multiple of r.
  //
  //Must match the perpendicular component, or the shot won't
  //be on target, so fire the bogon with this velocity:
  //(v - r * (v.r)/(r.r)) - r*k
  //where k is a positive scalar and is as large as possible
  //(there is an upper limit on how fast we can shoot.

  if(target->islive())
  {
    int rx = R(target->getx()-getxcentre());
    if(abs(rx) < 300)
    {
      int ry = target->gety()-getycentre();
      int r2 = rx*rx + ry*ry;       //r.r
      if(r2)
      {
        float vx = target->getfdx()-m_dx;      //v = relative velocity of target
        float vy = target->getfdy()-m_dy;
        float dot = vx*rx + vy*ry;  //v.r
        float fdx = vx - dot*rx/r2; //fd = v - (v.r)/(r.r)
        float fdy = vy - dot*ry/r2;
        //Use speed*sqrt(r2)/300 instead of speed.  So the maximum speed depends on
        //speed arg and distance from target.  This means the player has(very approximately) 
        //the same amount of time to get out of the way at any distance 
        float extra = speed*speed*r2/90000 - (fdx*fdx + fdy*fdy);
        if(extra>0)
	  {
	    extra = sqrt(extra/r2);
          (void)(new missile(RU(m_x+w/2-1),m_y+h/2-1,m_dx+fdx+extra*rx,m_dy+fdy+extra*ry));
	    sound::play(sound::pshoot,255);
	  }
      }
    }
  }
}


//The classic defender baddie.  It's main form of attack is to get in the
//player's way.
class bug:public sbaddie
{
friend class colldet;
friend class pod;

public:
  static void init();

  bug(); 
  ~bug();

private:
  static graphics::bitmap landerpic[10];
  
  int bdx,bdy;
  pod *victim;
  int landstage;
  int animflag;
  void animate();
  void misc();
  void move();

};

//The most evil creature imaginable.
//It moves close to the defender and shoots at it.
class lawyer:public sbaddie
{
public:
  lawyer(int,int);
  ~lawyer();
  virtual void takebomb();
  void become_evil();
  static void init();

private:
  static graphics::bitmap lawyerpic[10];
  int xdev,ydev;
  int animflag;
  deftarget target;
  void misc(void);
  void move(void);
  void animate(void);
};

//Thing to put rescued pods in.
//Some sort of stable wormhole, I suppose.  
class portal:public sprite
{
friend class colldet;

public:
portal(void);
~portal();
static void init(void);

private:
static graphics::bitmap porpic[8];
static std::list<portal *>portals;
int animflag;
void animate(void);
void misc(void);

};

//Improved weapons.  Baddies beware!
class powerup:public sprite
{
friend class colldet;

public:
  enum
  {
  RAILGUN     =0,
  AUTOFIRE    =1,
  LONGRANGE   =2,
  BOMB        =3,
  SHIELD      =4,
  XTRA_LIFE   =5,
  ITCURSE     =6,
  ELEPHANTGUN =7
  };

  static void init();


  powerup(int,int,int);
  ~powerup();
  int isdead(void);
  void die() {deadness = 1;}
  int gettype(){return type;}

private:
  static std::set<powerup *>powerups;
  static graphics::bitmap pupics[8];

  int type;
  int deadness;  
  void misc(void);

};

//collision detection
class colldet
{

public:
  static void addbomb(int);
  static void do_all_collisions();

private:
  static bool rect_test(sprite *, sprite *);
  static bool pixel_test(sprite *, sprite *);
  static bool defender_pod_test(defender *, pod *);
  static bool defender_portal_test(defender *, portal *);
  static bool defender_defender_test(defender *, defender *);

  static void shot_sbaddie_fn(shot *, sbaddie *);
  static void elephant_sbaddie_fn(elephant *, sbaddie *);
  static void elephant_bogon_fn(elephant *, bogon *);
  static void defender_sbaddie_fn(defender *, sbaddie *);
  static void defender_bogon_fn(defender *, bogon *);
  static void defender_pod_fn(defender *, pod *);
  static void defender_powerup_fn(defender *, powerup *);
  static void defender_portal_fn(defender *, portal *);
  static void defender_defender_fn(defender *, defender *);

  static void detonate_bombs();
  static std::list<int> bombs;

};

//Things the game sometimes needs to wait for - messages, smart bombs, etc
class timerstuff
{
  static int bombstate;
  static int messtimer;

public:
  static void add_bomb();
  static void tick();
  static void new_message();
  static int ok_to_continue();
};



//Hi score table
class hst
{
  char *location;
  char *names[10];
  int scores[10];

public:
  hst(const char *);
  void add(graphics::screen_ &scr, const char *, int);
  void show(graphics::screen_ &scr);
  ~hst();
};

//colour cycling helper thingy
class exploderanim
{  
public:
  static void init();
  static void cycle();

private:
  static unsigned char colours[32][3];
  static int n;
};


int keycount();
void setcol(unsigned char n, unsigned char r, unsigned char g,unsigned char b);
void explode(int x,int y);


#endif

