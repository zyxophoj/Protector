/*
Colision.cpp: collisions and smart bombs

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

#include "prot.h"
#include "missiles.h"
#include "stuff.h"

//Smart bombs.  These are not exactly "collisions"
//addbomb makes a note of the smart bomb, detonate_bombs kills baddies

std::list<int>colldet::bombs;

void colldet::addbomb(int x)
{
bombs.push_back(x);
timerstuff::add_bomb();
}

void colldet::detonate_bombs()
{
while(bombs.size())
  {
  int bx = *bombs.begin();
  bombs.erase(bombs.begin());
  for(std::set<sbaddie *>::iterator i=sbaddie::sbaddies.begin(); i!=sbaddie::sbaddies.end(); i++)
    {
    sbaddie *sp = *i;
    if(abs(R(sp->getxcentre()-bx))<320)
      sp->takebomb();
    }
  }
}



//Collisions

//All the collision detection uses this algorithm, which
//just checks every single pair of sprites.  
template<class Acontainer, class Bcontainer, class Bin_Pred, class Fn>
void do_collisions(Acontainer &a, Bcontainer &b, Bin_Pred bin_pred, Fn fn)
{
  for(typename Acontainer::iterator iA = a.begin(); iA!=a.end(); ++iA)
    for(typename Bcontainer::iterator iB = b.begin(); iB!=b.end(); ++iB)
      if(bin_pred(*iA, *iB))
         fn(*iA, *iB);
}

void colldet::do_all_collisions()
{
    //Collisions which can kill baddies.  Do these first, to prevent players getting killed 
    //by "already dead" baddies.  Since no points are scored for baddies killed by smart bomb, 
    //try to kill them by all other means first.
    do_collisions(shot::shots,         sbaddie::sbaddies, rect_test, shot_sbaddie_fn);
    do_collisions(elephant::elephants, sbaddie::sbaddies, rect_test, elephant_sbaddie_fn);
    do_collisions(elephant::elephants, bogon::bogons,     rect_test, elephant_bogon_fn);
    detonate_bombs();
    
    //Collisions which can kill the player
    do_collisions(defender::defenders, sbaddie::sbaddies, pixel_test, defender_sbaddie_fn);
    do_collisions(defender::defenders, bogon::bogons,     pixel_test, defender_bogon_fn);
    
    //Others
    do_collisions(defender::defenders, powerup::powerups,   rect_test,              defender_powerup_fn);
    do_collisions(defender::defenders, defender::defenders, defender_defender_test, defender_defender_fn);
    do_collisions(defender::defenders, pod::pods,           defender_pod_test,      defender_pod_fn);
    do_collisions(defender::defenders, portal::portals,     defender_portal_test,   defender_portal_fn);
}


//Predicates to detect collisions.
//For most collisions, intersecting rectangles are sufficient.  But for
//important collisions (those that can kill the player), the
//slow pixel-by-pixel test must also be passed.

bool colldet::rect_test(sprite *a, sprite *b)
{
  int dy=a->gety()-b->gety();
  if(dy>=-a->h&&dy<b->h)
    {
    int dx=a->getx()-b->getx();
    dx=R(dx);
    if((dx>-a->w)&&(dx<b->w))
      {
      return true;
      }
    }
  return false;
}

bool colldet::pixel_test(sprite *a, sprite *b)
{
  return rect_test(a,b) && a->pixel_by_pixel(b);
}

//Attempt to detect movement through the portal, by comparing
//relative x-positions from this frame and the last one.
bool colldet::defender_portal_test(defender *pdef, portal *ppor)
{
  if(pdef->getpassenger())
  {
    int dx1=R(pdef->getxcentre()-ppor->getxcentre());
    int dx2=dx1-pdef->getdx();
    int dy=pdef->getycentre()-ppor->getycentre();
    return sign(dx1)!=sign(dx2) && abs(dx1+dx2)<30 && abs(dy)<45;
  }
  return false;
}

bool colldet::defender_pod_test(defender *pdef, pod *ppod)
{
    //rect_test may be a bit generous here - it's a bit too easy to "steal" pods from bugs
    return !pdef->getpassenger() && !ppod->rescuer && rect_test(pdef, ppod);
}

//bit of a bodge: avoid same defender collisions, prevent players getting stuck together
//(by checking colltime), make sure each collision only counts once!
bool colldet::defender_defender_test(defender *pdef1, defender *pdef2)
{
   return (pdef1->number > pdef2->number) && 
          pixel_test(pdef1, pdef2) && 
          !pdef1->colltime && !pdef2->colltime;
}


//Functions which get called when a collision occurs.

void colldet::shot_sbaddie_fn(shot *pshot, sbaddie *psbaddie)
{
  pshot->hitbaddie();
  psbaddie->takeshot(pshot->getplayernum());
}

void colldet::elephant_sbaddie_fn(elephant *pele, sbaddie *pbad)
{
    pbad->takeshot(pele->getplayernumber());
}

void colldet::elephant_bogon_fn(elephant *, bogon *pbog)
{
  pbog->die();
}

void colldet::defender_sbaddie_fn(defender *pdef, sbaddie *pbad)
{
  if(pdef->isshielded())
    pbad->takeshot(pdef->number);
  else
    if(!pdef->isinvuln())
      pdef->die();
}

void colldet::defender_bogon_fn(defender *pdef, bogon *pbog)
{
if(pdef->isshielded())
  pbog->die();
else
  if(!pdef->isinvuln())
    pdef->die();
}

void colldet::defender_portal_fn(defender *pdef, portal *)
{
    pod::rescued();
    delete pdef->getpassenger();
    pdef->setpassenger(0);
    sound::play(sound::putdown,255);
}

void colldet::defender_pod_fn(defender *pdef, pod *ppod)
{
   pdef->setpassenger(ppod);
   ppod->grabbed(pdef);
   sound::play(sound::pickup,255);
}

void colldet::defender_powerup_fn(defender *pdef, powerup *ppow)
{
  pdef->getpowerup(ppow->gettype());
  ppow->die();
  sound::play(sound::collect,255);
}

void colldet::defender_defender_fn(defender *pdef1, defender *pdef2)
{
   sound::play(sound::clang,255);
   if (pdef1->isshielded() == pdef2->isshielded())
   {
       //swap velocities
	 float spare=pdef1->fdx;
	 pdef1->fdx=pdef2->fdx;
	 pdef2->fdx=spare;
	 spare=pdef1->fdy;
	 pdef1->fdy=pdef2->fdy;
	 pdef2->fdy=spare;
   }
   else
   {
       //One ship is shielded, the other is not.
       //No effect on shielded ship, the other ship is sent flying!
       sound::play(sound::clang,255);
       sound::play(sound::clang,255);
       sound::play(sound::clang,255);
       defender *pshielded = pdef1->isshielded()?pdef1:pdef2;
       defender *pnaked    = pdef1->isshielded()?pdef2:pdef1;
       
       //An elastic collision with an immovable object would 
       //use 2 here instead of 3. This is a super-elastic collision, 
       //caused by naked player getting "zapped" by the shield
       //(perhaps there should be a zap sound instead of a louder clang)
       pnaked->fdx += 3*(pshielded->fdx-pnaked->fdx);
       pnaked->fdy += 3*(pshielded->fdy-pnaked->fdy);
   }
   
   //Avoid getting stuck together
   pdef1->colltime=pdef2->colltime=10;
}



