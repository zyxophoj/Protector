/*
Dlring.h - doubly linked ring container class

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


#ifndef DLRING_H
#define DLRING_H

//doubly linked ring.
//This type of container is hard to provide stl iterators for.  The problem is that end() should return an iterator 
//at position "one past the end".  But since this is a loop, begin() and end() point to the same value!
//We want loops like this:
//for(dlring<T>::iterator i = ring.begin(); i!=ring.end(); i++){wibble(*i)} 
//to work properly - that is, to wibble each value exactly once.
//We also want iterators to be indefinitely incrementable.
//The solution used here is to make begin() return a "clean" iterator and end() returns a "dirty" one.
//incrementing an iterator makes it dirty.  An additional bodge is required when the list is empty - 
//in this case, both iterators are dirty (and invalid!) so they are equal.
//This is not exactly ideal, and if you try to do soemthing clever, it may not work :(

template<class T> struct dllink
{
dllink<T>* prev;
dllink<T>* next;
T* elt;
};

template<class T>
class dlring_iterator
{
    bool clean;
    dllink<T> *plink;

    public:

    dlring_iterator(dllink<T> *p,  bool c): clean(c), plink(p){}
    dlring_iterator<T> operator++() {plink=plink->next; clean=false; return *this;}
    dlring_iterator<T> operator++(int) {dlring_iterator<T> oldvalue(this->plink, this->clean);
                                        plink=plink->next; clean=false; 
                                        return oldvalue;}
    bool operator ==(const dlring_iterator<T> &i) const
    {return clean == i.clean &&
            plink == i.plink;}
    bool operator !=(const dlring_iterator<T> &i) const
    {return !(*this==i);}
    T* operator *() {return plink->elt;}
};

template<class T> class dlring
{
dllink<T> *start;
int len;

public:
typedef dlring_iterator<T> iterator;
class empty{};
dlring(void);
~dlring();
T* add(T*);
void remove(T*);
T* getelt(int);
T* getnext(void);
int size(void);
int contains(T*);
T* operator +(const int&);
T& operator [](const int&);

//mega-hack: when ring is empty, begin() == end()
iterator begin(){return iterator(start, !!size());}
iterator end()  {return iterator(start, false);}
};

template<class T> dlring<T>::dlring()
{
start=0;
len=0;
}

template<class T> dlring<T>::~dlring()
{
int a;
dllink<T>* sp;
for(a=0;a<len;a++)
  {
  delete start->elt;
  sp=start;
  start=start->next;
  delete sp;
  }
}

template<class T> T* dlring<T>::add(T* newelt)
{
if(!len)
  {
  start=new dllink<T>;
  start->prev=start;
  start->next=start;
  }
else
  {
  (start->prev->next=new dllink<T>)->prev=start->prev;
  (start->prev=start->prev->next)->next=start;
  }
len++;
start->prev->elt=newelt;
return newelt;
}

template<class T> void dlring<T>::remove(T* victim)
{
dllink<T>* scan=start;
while(1)
  {
  if(scan->elt==victim)
	 {
	 scan->prev->next=scan->next;
	 scan->next->prev=scan->prev;
    if(start==scan)start=scan->next;
	 delete scan;
    if(!--len)start=0;
	 return;
	 }
  scan=scan->next;
  }
}

template<class T> T* dlring<T>::getelt(int a)
{
dllink<T>* it;
if(!(it=start))throw empty();
for(int c=0;c<a;c++)it=it->next;
return (it->elt);
}

template<class T> T* dlring<T>::getnext(void)
{
if(!start)throw empty();
start=start->next;
return start->elt;
}

template<class T> int dlring<T>::size(void)
{
return len;
}

template<class T> int dlring<T>::contains(T *cand)
{
int a,out;
for(a=out=0;a<len;a++)if((start=start->next)->elt==cand)out=1;
return out;
}

template<class T> T* dlring<T>::operator+(const int& a)
{
int c;
dllink<T>* it;
if(!start)throw empty();
it=start;
for(c=0;c<a;c++)it=it->next;
return it->elt;
}

template<class T> T& dlring<T>::operator[](const int& index)
{
return *(*this+index);//Just like in real life.
}

#endif
