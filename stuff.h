/*
Stuff.h:  Random useful things

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

#ifndef STUFF_H
#define STUFF_H

//Function which deletes its argument
template<class T>
class kill
{
  public: void operator()(const T *pt){delete pt;}
};

//Copied from Bjarne Stroustrup's book
template<class In, class Out, class Pred>
Out copy_if(In first, In last, Out res, Pred p)
{
   while(first != last)
   {
       if(p(*first)) *res++ = *first;
       ++first;
   }
return res;
}

//sign function
//returns 1 if argument>1, and -1 if argument <1
template<class T>
int sign(const T &t) {return t>0?1:-1;}

//Probability float to int
int pftoi(float);

#endif

