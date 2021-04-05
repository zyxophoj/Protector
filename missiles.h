/*
Missiles.h: Definitions of missile classes

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


#ifndef MISSILES_H
#define MISSILES_H

#include "prot.h"

//Basic missile
class bogon:public sprite
{
friend class colldet;

public:
  bogon(int,int,float,float);
  ~bogon();
  int isdead(void);
  void die() {life=0;}
  static void init(void);
  static void cleanup(void);

private:
  static graphics::bitmap bogpic;
  static dlring<bogon>bogons;
  float fx,fy,fdx,fdy;
  int life;
  void misc(void);
  void move(void);
};

//Similar to a bogon, but it's bigger
//and it can be shot, so it's less nasty
class bigon:public sbaddie
{
public:
  bigon(int x, int y, float dx, float dy);
  ~bigon();
  static void init();
  static void cleanup();
  void become_evil();

private:
  static graphics::bitmap bpelpic;

  float fx,fy,fdx,fdy;
  int life;
  void misc();
  void move();
};

#endif
