/*
Baddies.h: various baddies derived from sbaddie

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

#ifndef BADDIES_H
#define BADDIES_H

#include "prot.h"

//Stationary baddie which shoots at the player
class mine:public sbaddie
{
public:
  mine();
  mine(int);
  mine(int x,int y);
  ~mine();
  static void init();
  static void cleanup();

private:
  void misc();
  void init_shoot();
  
  static graphics::bitmap minepic;
  deftarget target;
  int shoot_time;
  int salvo_length;
  
};

//Snakey thing, fires bogons when killed.
class red_tape:public sbaddie
{
public:
  red_tape(int);
  red_tape(int,int,int,int);
  ~red_tape();
  static void init();
  static void cleanup();
  void takeshot(int);

private:
  static graphics::bitmap tapepic;
  static int yarray[200];
  static int dyarray[200];

  deftarget target;
  int phase;
  int ymean;
  void move();
  void misc();
};

//A wall which fires at the player
class firewall:public sbaddie
{
public:
  firewall(int);
  firewall(int,int,int,int);
  ~firewall();
  static void init(void);
  static void cleanup(void);

private:
  static graphics::bitmap wallpic[6];

  deftarget target;
  int animflag;
  void misc();
  void move();
  void animate();
};

//The bundle centre is surrounded by drones.  
//It can't be killed until all drones are killed
class bundle:public sbaddie
{
friend class drone;

public:
  bundle(int);
  ~bundle();
  static void init(void);
  static void cleanup(void);
  void takeshot(int);

private:
  static graphics::bitmap bunpic;
  dlring<drone> drones;
  deftarget target;
  void misc(void);
  void move(void);
};

//bundle drone.  Orbits the bundle centre
class drone:public sbaddie
{
public:
  drone(bundle *);
  ~drone();
  static void init(void);
  static void cleanup(void);

private:
  static graphics::bitmap dronepic;
  bundle *parent;
  int xr;
  int yr;
  int phase;
  void move(void);
  void misc(void);
};

//Note for Americans and other aliens:
//It might help to know that Domestos is a brand of toilet cleaner found
//in the UK in blue bottles.
class dom_s_dos: public sbaddie
{
public:
  static void init(void);
  static void cleanup(void);

  dom_s_dos();
  void takeshot(int);

private:
  static graphics::bitmap dospic[64];

  void animate();
  void misc();
  void move();
  deftarget target;

  int animflag;
  int energy;
};

//Very Irritating Paperclip
//Tells you things you don't want to know, and is hard to get rid of.
class vip:public sbaddie
{
friend class evil_eye;

public:
  vip();
  ~vip();
  void takeshot(int);
  void loseeye(evil_eye *);
  void become_evil();
  void teleport(int,int);
  static void init();
  static void cleanup();

private:
  static graphics::bitmap vippic;
  static graphics::bitmap edgeL, edgeR;
  static char *blurb[];
  static int nblurbs;
  static int messx1;
  static int messx2;
  static int messy;

  graphics::bitmap myblurb;
  evil_eye *eyes[2];
  int curr_mess;
  int mess_step;
  int neyes;
  int energy;
  int scrollflag;
  int isfalling;
  double duby;
  double dubdy;
  deftarget maintarget;
  deftarget secondtarget;
  static void clipart(int,int,int);
  void animate(void);
  void move(void);
  void misc(void);

};

//Eyes of the paperclip.
class evil_eye:public sbaddie
{
public:
  static void init();
  static void cleanup();

  evil_eye(vip *,int);
  ~evil_eye();  
  void takeshot(int);
  void become_evil();

private:
  static graphics::bitmap eyepic[32];

  vip *parent;
  deftarget target;
  int energy;
  int xoff;
  void animate(void);
  void move(void);
  void misc(void);
};

//Special effect created by exploding paperclip
class clipbit:public sprite
{
public:
  static void init(void);
  static void cleanup(void);

  clipbit(int,int);
  ~clipbit();  
  int isdead(void);

private:
  static graphics::bitmap bitpic[16];
  
  int animflag,da;
  double dubx,duby;
  double dubdx,dubdy;
  void animate(void);
  void move(void);
};

//A disk which drops boots on you
class bootdisk:public sbaddie
{
public:
  static void init(void);
  static void cleanup(void);
  
  bootdisk(void);
  ~bootdisk();
  void takeshot(int);

private:
  static graphics::bitmap diskpics[64];
  
  int energy;
  int moveinc;
  int animflag;
  void animate();
  void move();
  void misc();
};

//Dropped by bootdisk
class boot:public sbaddie
{
public:
  static void init();
  static void cleanup();

  boot(int,int);
  ~boot();
  void become_evil();

private:
  static graphics::bitmap bootpic;
  
  void move();
  void misc();
};

//Moves near you and explodes
class exploder:public sbaddie
{
public:
  static void init();
  static void cleanup();

  exploder();
  exploder(int,int);
  ~exploder();
  void takeshot(int);

private:
  static graphics::bitmap explpic[5];
  static int yarray[100];
  static int dyarray[100];
  
  deftarget target;
  int ymean;
  int animflag;
  int phaseflag;
  int energy;
  void animate();
  void misc();
  void move();
};

#endif
