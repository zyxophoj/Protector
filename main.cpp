/*
Main.cpp: main loop, attract mode, options screens

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

#include <time.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <algorithm>
#include <functional>

#include "stuff.h"
#include "prot.h"
#include "missiles.h"
#include "baddies.h"
#include "back.h"
#include "graphics.h"
#include "joy.h"
#include "main.h"

void game(int nplayers, int craptop_mode);

//attract mode functions
void attract();
void blocktext(graphics::bitmap &,char *,int,int,int);
graphics:: bitmap planexppic(int);
void jools(graphics::bitmap &);
void drawit(int *,int *,int *,int,int,int);


void screenshot();

//hi score table
hst thst("prot.hst");

//Bodge: score[0] is the amount scored by "nobody"... some baddies are not
//considered to have been killed by a player, but they want to increment
//a score when they die anyway.  score[1] and [2] are player scores.
int score[3];


//Not really the main function - that's in main_fn.h
int main_()
{
graphics::init();

srand(time(NULL));

sound::init();
letter::init();
defender::init();
shot::init();
elephant::init();
fader::init();
pod::init();
portal::init();
bug::init();
lawyer::init();
bogon::init();
mine::init();
red_tape::init();
firewall::init();
bundle::init();
drone::init();
dom_s_dos::init();
vip::init();
evil_eye::init();
clipbit::init();
bootdisk::init();
boot::init();
exploder::init();
bigon::init();
powerup::init();
star::init();
spark::init();

exploderanim::init();

timer::init();

graphics::setcol(32,32,63,0);
graphics::setcol(33,63,32,0);
graphics::setcol(34,60,31,0);
graphics::setcol(35,57,30,0);
graphics::setcol(36,54,29,0);
graphics::setcol(37,51,28,0);

graphics::setcol(38,16,35,16);
graphics::setcol(39,16,36,16);
graphics::setcol(40,16,37,16);
graphics::setcol(41,16,38,16);
graphics::setcol(42,16,39,16);
graphics::setcol(43,16,40,16);
graphics::setcol(44,16,41,16);

graphics::setcol(45, 3, 8, 3);
graphics::setcol(46, 6,15, 6);
graphics::setcol(47, 9,23, 9);
graphics::setcol(48,12,30,12);

graphics::setcol(49, 0, 0, 0);
graphics::setcol(50,16, 0, 0);
graphics::setcol(51,32, 0, 0);
graphics::setcol(52,47, 0, 0);
graphics::setcol(53,63, 0, 0);

controls::standard();

joystick::load_data();

int cont=0; //"might be used uninitialised"
int craptop_mode=0;
do
  {
  keys::clear_buffer();
  attract();
  do
    {
    //main menu
      {
      graphics::screen_ scr;
      clear(scr);
      scr.drawtext("PROTECTOR",268,236,15);
    }
    const char *options[]={
      "1 player game",
      "2 player game",
      "Redefine player 1 controls",
      "Redefine player 2 controls",
      "Calibrate Joystick",
      "Edges",
      "Quit"};    

    int m=keymenu(220,300,0,options,7);
    switch(m)
      {

      case 0:
      game(1,craptop_mode);
      //while(keys::keycount());
      thst.add("Player 1",score[1]);
      //while(keys::keycount());
//      timer::set_counter(0);
//      while(!keys::keycount())if(timer::get_counter()>2500)break;
      cont=1;
      break;

      case 1:
      game(2,craptop_mode);
      //while(keys::keycount());
      //Do the lower score first - this means it is possible for the higher
      //score to push it off the bottom.
      if(score[1]<=score[2])
      {
        thst.add("Player 1",score[1]);
        keys::clear_buffer();
        while(!keys::keypressed());
        thst.add("Player 2",score[2]);
      }
      else
      {
        thst.add("Player 2",score[2]);
        keys::clear_buffer();
        while(!keys::keypressed());
        thst.add("Player 1",score[1]);
      }
    
      timer::set_counter(0);
      
      cont=1;
      break;

      case 2:
      controls::get(0).redefine();
      cont=2;
      break;

      case 3:
      controls::get(1).redefine();
      cont=2;
      break;

      case 4:
      calibrate_stick();
      cont=2;
      break;

      case 5:
      //This might one day be an options screen.  Currently there is only 1 option, so it
      //looks a bit odd.
      craptop_mode=get_craptop_mode();
      cont=2;
      break;

      case 6:
      cont=0;
      break;

      }
    }while(cont>1);
  }while(cont);

bogon::cleanup();
mine::cleanup();
red_tape::cleanup();
firewall::cleanup();
bundle::cleanup();
drone::cleanup();
dom_s_dos::cleanup();
vip::cleanup();
evil_eye::cleanup();
clipbit::cleanup();
bootdisk::cleanup();
boot::cleanup();
exploder::cleanup();
bigon::cleanup();
star::cleanup();
spark::cleanup();

sound::cleanup();

graphics::exit();
return 0;
}


//The game!
void game(int nplayers,int cm)
{

//Disable caps/num/scroll lock toggling.
class clearleds
{
public:
clearleds(){keys::set_key_led_flag(0);}
~clearleds(){keys::set_key_led_flag(1);}
}cl;

//Data for levels
//Columns represent Reinforcement Threshold, bugs, red tapes, firewalls,
//minefields, Bundles, dom-s-dosses, VIPs, bootdisks, exploders
const int cols=10;
const int nlevels(15);
int leveldata[cols*nlevels+1]={/*
RT  b rt fw mf  B  d  V bd  e  */
 0, 8, 0, 0, 0, 0, 0, 0, 0, 0,

 0,15, 1, 0, 0, 1, 0, 0, 0, 0,

 0,20, 0, 1, 0, 1, 0, 0, 0, 0,
20,10, 2, 0, 0, 1, 0, 0, 0, 0,

 0,25, 1, 1, 0, 0, 0, 0, 1, 0,
50,20, 0, 2, 1, 0, 0, 1, 0, 0,

 0,30, 0, 1, 1, 1, 0, 0, 0, 0,
40,20, 0, 0, 0, 0, 0, 0, 0,10,
10,10, 0, 0, 0, 2, 0, 0, 0,15,

 0,50, 0, 5, 0, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0, 0, 2, 0, 1, 0,

 0,10, 0, 0, 0, 0, 0, 10, 0, 0,

 0, 0, 0, 0, 0, 0, 0, 0, 0,30,
 8,99, 0, 1, 0, 0, 0, 0, 1, 0,
20, 0, 3, 0, 0, 0, 5,10, 0,10,

 0 //Last 0 is for sensible wrap behaviour


};

//force boolean value
cm=!!cm;

int level=0;
int stage=0;

int screen_w, screen_h;
  {
  graphics::screen_ scr;
  clear(scr);
  //Smart bomb changes colour 0, but 16 is always black
  scr.rectfill(0, 420, scr.width()-1, scr.height()-1, 16);
  //Don't assume 640x480.
  screen_w = scr.width();
  screen_h = scr.height();

  score[0]=score[1]=score[2]=0;

  //In craptop_mode the top, left and right sides of the screen are marked by
  //a white line.  Useful for laptops which have a "tunnel vision"
  //implementation of various graphics modes.  Not all laptops do this - there
  //exist laptops which are not craptops.
  int width1=screen_w-2*cm;
  int width2=screen_w/2-1-cm;

  switch(nplayers)
    {
    case 1:
      (void)(new display(width1, cm, cm, 0,
                     new radar(screen_w/2-150,screen_h-50,300,30)));
    break;

    case 2: // A bit cramped!;
      (void)(new display(width2,cm,          cm,1,new radar( 10,430,300,30)));
      (void)(new display(width2,cm,screen_w/2+1,2,new radar(330,430,300,30)));
      scr.vline(screen_w/2-1, 0, 425, 15);
      scr.vline(screen_w/2  , 0, 425, 15);
    }
    if(cm)
    {
      scr.vline(0, 0, 425, 15);
      scr.vline(screen_w-1, 0, 425, 15);
      scr.hline(0, 0, screen_w-1, 15);
    }
    scr.drawtext( "IN:    OUT:", screen_w/2-52, screen_h-10, 4);
  }
(void)new portal();

defender::violent=0;

  //The main loop

  while(!defender::alldead())
  {
    radar::wipe();
    joystick::poll();
    sprite::process_all();
    colldet::do_all_collisions();


    //handle dead sprites.
    {
      std::list<sprite *> morgue;
      copy_if(sprite::sprites[sprite::level_bg].begin(), sprite::sprites[sprite::level_bg].end(), back_inserter(morgue), std::mem_fun(&sprite::isdead));
      copy_if(sprite::sprites[sprite::level_fg].begin(), sprite::sprites[sprite::level_fg].end(), back_inserter(morgue), std::mem_fun(&sprite::isdead));
      for(std::list<sprite *> ::iterator i=morgue.begin(); i!=morgue.end(); i++)
      {
        (*i)->drawoff();
        delete *i;
      }
    }

    {
      graphics::screen_ the_screen;
  
      display::drawsprites(the_screen);
      radar::drawall(the_screen);

      //Update the "IN" and "OUT" display
      {
        char str[2];
        sprintf(str, "%d", pod::in());        
        the_screen.drawtext( str, the_screen.width()/2-20,
                                       the_screen.height()-10,4,16);
        sprintf(str, "%d", pod::out());
        the_screen.drawtext( str, the_screen.width()/2+44,
                                     the_screen.height()-10,4,16);
      }
    }

  switch(stage)
    {
    case 0:
    //level is initially 0, and becomes 1 when the first level starts.
    if( (sbaddie::population()+pod::out() <= leveldata[level*cols]) &&
         timerstuff::ok_to_continue() )
      {
      int base = level*cols;
      deftarget::makerandom();
      sound::play(sound::tadaa,255);
      //If leveldata[base] is 0, this is a new level, otherwise it's another
      //stage of the same level
      if(!leveldata[base])pod::make8();
      for(int a=0;a<leveldata[base+1];a++)(void)(new bug);
      for(int a=0;a<leveldata[base+2];a++)(void)(new red_tape(50));
      for(int a=0;a<leveldata[base+3];a++)(void)(new firewall(39));
      for(int a=0;a<leveldata[base+4];a++)(void)(new mine(50));
      for(int a=0;a<leveldata[base+5];a++)(void)(new bundle(30));
      for(int a=0;a<leveldata[base+6];a++)(void)(new dom_s_dos());
      for(int a=0;a<leveldata[base+7];a++)(void)(new vip());
      for(int a=0;a<leveldata[base+8];a++)(void)(new bootdisk());
      for(int a=0;a<leveldata[base+9];a++)(void)(new exploder());

      //Nasty:  All pods have been eaten before reinforcements
      //arrive, so they become evil immediately
      if(pod::in()+pod::out()==0) sbaddie::all_become_evil();
      //Temporary invulnerability, avoids nasty insta-death from new baddies
      defender::make_all_invulnerable();
      level++;
      level%=nlevels;
      //detect cheat condition after level 1
      if(level==2)defender::cheat();
      if(!leveldata[level*cols])stage=1;
      }
    break;

    case 1:
    //Is the level complete?
    if(sbaddie::population()+pod::out()==0)
      {
      char buf[80];
      int s=pod::in()*100;
      if(s==800)s=1600;
      sprintf(buf,"Pods rescued: %d.  Bonus: %d",pod::in(),s);
      score[1]+=s;
      score[2]+=s;
      letter::message(buf,0);
      stage=0;
      }
    break;
    }

  //Have we  been paused?
  if(controls::get(0).get_pause())
    {
    //screenshot();
    //Unlikely, but a stick may be used to pause, so must poll it.
    while(controls::get(0).get_pause()) joystick::poll();
    keys::clear_buffer();
    keys::readkey();
    }
  timerstuff::tick();
  while(timer::get_counter()==0);timer::set_counter(0);
}

//get rid of all the sprites
sprite::kill_all();

//Give player some time to read the "You're dead" message
timer::wait(1000);

//force all pending special effects (smart bombs) to finish.
while(!timerstuff::ok_to_continue())timerstuff::tick();

}

//Attract mode - explain the plot(such as it is), show hi scores,
//do some flashy display hacks
void attract()
{
int dx,dy,dn;
char *waffle1=
"   After much delay, the Evil Empire completed construction of "
"a gigantic battle station - Planet Exploder 5.0!  "
"The Planet Exploder was used to destroy the planet "
"Penetsca, home of the Rebellion's main fleet, in a "
"sneak attack.  Faced with overwhelming opposition, the "
"remnants of the Rebellion were forced to retreat to a "
"secret base in the Gnu system, using all remaining "
"ships in the evacuation.";
char *waffle2=
"   Your mission is simple.  Destroy all Imperial forces at "
"each planet and rescue as many lifepods as you can.  Pick up the pods in "
"your ship and carry them through the portal.";
char *waffle3=
"   Bugs will also pick up lifepods and carry them off, but their mission is "
"not entirely humanitarian.  If a bug gets to the top of the screen with a "
"lifepod, it will eat the pod and turn into a lawyer.  If all the pods get "
"eaten, something very unpleasant will happen.";
char *puinfo=
"  The railgun, autofire and long range last for a limited number of "
"shots.  The shield lasts for a limited amount of time, but at least you can "
"turn it on and off.  In one player mode, the I powerup makes you invisible "
"to the baddies (so they can't shoot at you or home in on you).  In two "
"player mode, it still has this effect -because it makes all the baddies "
"target the other player.  Including some baddies which by default have no "
"target!  The elephant gun should be self-explanatory.";
char *disclaim=
"  All Evil Empires, especially those based on real software companies, "
"are entirely fictional.  Any resemblance between anything and anything "
"else is coincidental.  If anything goes wrong, it's not my fault.  So "
"there.";
char *creditees[5]={"DJ Delorie and a lot of other people","Shawn "
"Hargreaves and a lot of other people","The Temple Ov Thee Lemur","Timothy Logvinenko",
"Hugh Robinson"};
char *fors[5]={"For djgpp","For Allegro","For the plot.  (Hope they don't "
"mind me using it!)","For saying \"You can't get good scrolling on a PC\"",
"For explaining the principles of stellar parallax"};
char *pitch=
"More levels than SWIV!!  More AI than Tetris!!  More equations than A Brief "
"History Of Time!!  More exclamation marks than are really necessary!!  It's "
"PROTECTOR, the only game where the player is expected to get rid of all the "
"bugs.";
char *pitch2="There's nothing like a drug-induced sales pitch for getting "
"up people's noses, complete with exaggerations, lies and gang-bangs.  "
"That's a technical term for multiple exclamation marks, by the way.  But "
"some people will be reading this, just in case I've hidden some useful "
"information in here.  Even though I'm writing in a silly colour and "
"the background is nasty.  However, unless you are a very fast reader, you "
"won't be able to read all this in one go, so you'll have to wait for it to "
"come round again.";
char *pitch3="If you're not part of the solution, you're part of the "
"precipitate.  Every silver lining has a cloud.  More pot, less speed.  "
"You might want to try completing the first level without firing a shot or "
"using the smart bomb.   He who laughs last doesn't get the joke.  Give a "
"man a fire and he's warm for a day.  Set fire to him and he's warm for the "
"rest of his life.  Better to light a candle than to eat it.";
char *gpl = "Protector is free software, released under version 2 of the GPL,"
" or any later version.  Share and enjoy!";

int screen_w, screen_h;
{
  graphics::screen_ scr;
  screen_w = scr.width();
  screen_h = scr.height();
}

int sw2=screen_w/2;
int sh2=screen_h/2;

graphics::bitmap sparescreen(screen_w,screen_h);



  while(1)
  {
    for(int stage=0;stage<17;stage++)
    {
      struct sprite_splatter
      {
      ~sprite_splatter(){sprite::kill_all();}
      }_2;
      (void)_2; //Avoids "unused variable" warning!

 {     graphics::screen_ scr;

      //Don't assume screen is 640x480 - mot of these work OK on larger screens
      switch(stage)
      {

      case 0: //Title screen
        for(int a=0;a<screen_w;a++)
	  for(int b=0;b<screen_h;b++)
	    sparescreen.putpixel(a,b,128+static_cast<int>(
	                         sqrt((0.5-sw2+a)*(0.5-sw2+a)+
	                              (0.5-sh2+b)*(0.5-sh2+b)))%128);

        sparescreen.circlefill(sw2,sh2,100,0);
        sparescreen.drawtext("PROTECTOR",sw2-36,sh2-30,15);
        sparescreen.drawtext("Press any key to start",sw2-88,sh2+20,15);
        blit(sparescreen,scr,0,0,0,0,screen_w,screen_h);
      break;

      case 1: //Show hi scores
        thst.show();
      break;

      case 2:
        for(int a=0;a<screen_w;a++)
	  for(int b=0;b<screen_h;b++)
	    sparescreen.putpixel(a,b,128+(1280+static_cast<int>
                       (atan2(0.5-sw2+a,0.5-sh2+b)/2/M_PI*128*10))%128);
        sparescreen.circlefill(sw2,sh2,120,0);
        sparescreen.drawtext("Player 1 Controls:",sw2-74,sh2-60,15);
        blit(sparescreen,scr,0,0,0,0,screen_w,screen_h);
        controls::get(0).info(sw2-75,sh2-30);
      break;

      case 3:
        for(int a=0;a<screen_w;a++)
	  for(int b=0;b<screen_h;b++)
	    sparescreen.putpixel(a,b,sparescreen.getpixel(a,b)^127);
        sparescreen.circlefill(sw2,sh2,120,0);
        sparescreen.drawtext("Player 2 Controls:",sw2-74,sh2-60,15);
        blit(sparescreen,scr,0,0,0,0,screen_w,screen_h);
        controls::get(1).info(sw2-75,sh2-30);
      break;

      case 4:
        clear(scr);
        scr.drawtext("Not a long, long time ago...",180,10,15);
        blocktext(scr,waffle1,40,40,70);
        blocktext(scr,waffle2,40,100,70);
        (new pod())->teleport(80,200);
        scr.drawtext("Lifepod",100,201,15);
        (new bug())->teleport(180,201);
        scr.drawtext("Bug",200,201,15);
        (new lawyer(0,0))->teleport(80,220);
        scr.drawtext("Lawyer",100,221,15);
        (new defender(0,0,0))->teleport(175,221);
        scr.drawtext("You",200,221,15);
        (new portal())->teleport(300,165);
        scr.drawtext("Portal",320,211,15);
        blocktext(scr,waffle3,40,300,70);
        //HACK:  This is necessary because the defender writes "LIVES" and "BOMBS"
        //to the screen...
        scr.rectfill(0,440,100,479,0);
      break;

      case 5:
        clear(scr);
        scr.drawtext("Baddies",288,10,15);
        (new bug())->teleport(130,80);
        scr.drawtext("Bug                     50 Points",190,81,15);
        (new lawyer(0,0))->teleport(130,100);
        scr.drawtext("Lawyer                 500 Points",190,101,15);
        (new mine(0,0))->teleport(130,120);
        scr.drawtext("Mine                   100 Points",190,121,15);
        (new red_tape(0,0,0,0))->teleport(130,140);
        scr.drawtext("Red tape segment       100 Points",190,141,15);
        (new firewall(0,0,0,0))->teleport(130,160-5);
        scr.drawtext("Firewall segment       150 Points",190,161,15);

        (new bundle(0))->teleport(123,200);
        scr.drawtext("Bundle centre         1000 Points",190,209,15);
        (new drone(0))->teleport(130,240);
        scr.drawtext("Bundle drone            20 Points",190,241,15);

        (new exploder())->teleport(113,275);
        scr.drawtext("Exploder              1500 Points",190,296,15);

        (new boot(0,0))->teleport(130,340);
        scr.drawtext("Boot                     1 Point",190,341,15);
        (new bigon(0,0,0.0,0.0))->teleport(130,360);
        scr.drawtext("Projectile               1 Point",190,361,15);
      break;

      case 6:
        clear(scr);
        scr.drawtext("More Baddies",268,10,15);
        (new bootdisk())->teleport(70,50);
        scr.drawtext("Boot Disk           500 Points",250,60,15);
        (new vip())->teleport(50,200);
        scr.drawtext("Eyes               2500 Points each",250,185,15);
        scr.drawtext("V.I.P.            10000 Points",250,216,15);
        scr.drawtext("(V.I.P. = Very Irritating Paperclip)",150,270,15);
        (new dom_s_dos())->teleport(20,350);
        scr.drawtext("DOM-S-DOS         15000 Points",250,372,15);
      break;

      case 7:
        clear(scr);
        scr.drawtext("Powerups",288,10,15);
        for(int p=0;p<8;p++)(new powerup(0,0,p))->teleport(220,100+20*p);
        scr.drawtext("Rail gun",300,104,15);
        scr.drawtext("Autofire",300,124,15);
        scr.drawtext("Long Range",300,144,15);
        scr.drawtext("S.M.A.R.T. bomb",300,164,15);
        scr.drawtext("Shield",300,184,15);
        scr.drawtext("Extra life",300,204,15);
        scr.drawtext("Invisibility/It-curse",300,224,15);
        scr.drawtext("Elephant gun",300,244,15);
        blocktext(scr,puinfo,32,270,72);
        scr.drawtext("(S.M.A.R.T. = Selective Mortality Atomic "
                            "Radiation Therapy)",80,400,15);
      break;

      case 8:
        {
          graphics::bitmap bmp=planexppic(80);
          clear(scr);
          scr.drawtext("Etc",308,10,15);
          scr.drawtext("Programming by Mark Boyd",224,50,15);
          scr.drawtext("Thanks to:",20,70,15);
          for(int a=0;a<5;a++)
	    {
	      scr.drawtext(creditees[a],30,90+25*a,9);
	      scr.drawtext(fors[a],30,100+25*a,8);
	  }
          blocktext(scr, gpl, 96, 350, 54);
          blocktext(scr,disclaim,112,400,50);
          blit(bmp,scr,0,0,  0,380,100,100);
          blit(bmp,scr,0,0,540,380,100,100);
        }
        break;

      case 9:
      {
        int pesize=std::min(screen_h, screen_w)*5/6; //like h is ever going to be < w
        graphics::bitmap bmp=planexppic(pesize);
        clear(scr);
        blit(bmp,scr,0,0,sw2-pesize/2,sh2-pesize/2,pesize,pesize);
        scr.drawtext("PLANET EXPLODER",60,60,15);
        scr.drawtext("What do you want",screen_w-150,screen_h-150,15);
        scr.drawtext("to blow up today?",screen_w-150,screen_h-140,15);
      }
      break;

      case 10:  //Here come the display hacks...
        clear(scr);
        scr.drawtext("Still reading this nonsense?",208,10,15);
        scr.drawtext("Sure you don't want to play the game?",170,20,15);
        scr.drawtext("Perhaps you'll play if this demo gets \
sufficiently irritating.",64,30,128);
      break;

      case 11:  //spiral cycling thingy
        for(int a=0;a<screen_w;a++)
	  for(int b=0;b<screen_h;b++)
	    sparescreen.putpixel(a,b,static_cast<int>(
	     sqrt((0.5-sw2+a)*(0.5-sw2+a)+(0.5-sh2+b)*(0.5-sh2+b))+
	     1280+atan2(0.5-sw2+a,0.5-sh2+b)/2/M_PI*128*10 )%128+128);
        blit(sparescreen,scr,0,0,0,0,screen_w,screen_h);
        scr.rectfill(sw2-220,10,sw2+220,60,0);
        scr.drawtext("Irritation level 1",sw2-72,20,15);
        scr.drawtext("Look! Colour cycling! Isn't this clever?!",
	        sw2-164,30,15);
        scr.drawtext("No.",sw2-210,40,15);
      break;

      case 12: //Julia set
        clear(scr);
        jools(scr);
        scr.drawtext("Irritation level 2: Fractals",sw2-120,20,15);
        scr.drawtext("Didn't that go out last century?",sw2-136,470,15);
      break;

      case 13:  //Hilbert curve
        dx=0;dy=0;dn=128;
        drawit(&dx,&dy,&dn,1,0,10);
        scr.drawtext("Irritation level 3: Severe ikkyness",sw2-160,20,15);
      break;

      case 14:
        scr.drawtext("Irritation level 4: Too much to read",sw2-164,20,15);
        clear(sparescreen);
        blocktext(sparescreen,pitch,32,80,72);
        blocktext(sparescreen,pitch2,32,150,72);
        blocktext(sparescreen,pitch3,32,270,72);
        for(int a=0;a<640;a++)
	  for(int b=0;b<480;b++)
	    if(sparescreen.getpixel(a,b)) scr.putpixel(a,b,12);
      break;

      case 15: //Sierpinski triangles (colour cycling optimised for 640x480
               //(well 512x512 actually))
        clear(scr);
        for(int a=0;a<screen_w;a++)
          for(int b=0; b<screen_h;b++)
            if(a&b) scr.putpixel(a,b,((a&b)%512)/4+128);
      break;

      case 16: //Blue Screen Of Death(tm) - a screensaver which comes
               //free with M$ Windows
        scr.clear(1);
        scr.drawtext("PROT.EXE has caused a segmentation "
                             "fault in module WINDOZE.DLL",sw2-248,100,15);
        scr.drawtext("It may be posible to continue normally.  "
                             "On the other hand, it may not.", sw2-284,200,15);
        //(Turing in particular had something to say about that)
        scr.drawtext("Reflect, repent, reboot and reinstall.", sw2-152,300,15);
        break;
      }
}

    for(int n=0;n<1500;n++)
      {
      if(keys::keypressed()) return;

      while(timer::get_counter()==0);timer::set_counter(0);

      //Animate sprites
      {
        graphics::screen_ the_screen;
        std::set<sprite *>::iterator b = sprite::sprites[sprite::level_fg].begin();
        std::set<sprite *>::iterator e = sprite::sprites[sprite::level_fg].end();
        for(std::set<sprite *>::iterator i=b; i!=e; i++)
          (*i)->demoinc(the_screen);
      }
	
      //cycle colours.  If we can't do this with hardware, Allegro makes a valiant attempt
      //to simulate it, but it may not be fast enough.      
#ifdef COLOUR_CYCLING 
      exploderanim::cycle();
#endif      
      }
    }
  }
}

//Draw a block of text.  Try to justify it, but
//Don't                do                 this.
void blocktext(graphics::bitmap &bmp,char *strin,int x,int y,int l)
{
int a,b,c,sl,p;
sl=strlen(strin);
char *tempstrin;
tempstrin=new char[l+1];
p=0;
b=0;
c=0;
while(sl>p)
  {
  if(p+l>=sl)
    a=sl-p;
  else
    {
    a=l;
    while(!isspace(strin[p+a]))a--;
    }
  strncpy(tempstrin,strin+p,a);
  tempstrin[a]=0;
  p+=a+1;
  bmp.drawtext_justify(tempstrin,x,x+8*l,y+8*c,l,15);
  c+=1;
  }
delete[]tempstrin;
}



int get_craptop_mode()
{
//todo: perhaps craptop_mode should modify the pallete to cope with
//LCD abominations
graphics::screen_ scr;
clear(scr);
const char *options[2]={"On","Off"};
return !keymenu(150,80,0,options,2);
}

graphics::bitmap planexppic(int size)
{
  //Draw the planet exploder.
  //
  //The planet exploder is essentially a sphere with radius size/3
  //The light source is at direction (-0.6, 0, 0.8)
  //(to the left of and in front of the screen)
  //The brightness at a point is computed by taking the dot product
  //of the light source vector and the unit normal vector to the surface
  //at that point.  This is easy for a sphere because the normal vector
  //at a point is a multiple of the coordinates of that point

  graphics::bitmap out(size,size);
  clear(out);

  //draw ring behind planet
  for(int a=0;a<4*size;a++)
  {
    float fc=cos(2.0*M_PI*(0.125+1.0*a/(8.0*size)));
    float fs=sin(2.0*M_PI*(0.125+1.0*a/(8.0*size)));

    float r1=0.23*size;
    float r2=0.28*size;
    float r;
    int bmax=(size+5)/9;
    for(int b=0;b<=bmax;b++)
    {
      r=r1+(r2-r1)*b/bmax;
      float x=0.5*size+r*fc;
      float y=0.5*size-r*fs;
      float fudge=0.5*(y-x);
      x-=fudge;y+=fudge;
      out.putpixel(static_cast<int>(0.5+x),
                   static_cast<int>(0.5+y),128+64*a/(4*size));
    }
  }

  //mark where planet is going to be
  out.circlefill(size/2,size/2,size/3,16);

  //draw ring in front of planet
  for(int a=4*size;a<8*size;a++)
  {
    float fc=cos(2.0*M_PI*(0.125+1.0*a/(8.0*size)));
    float fs=sin(2.0*M_PI*(0.125+1.0*a/(8.0*size)));

    float r1=0.23*size;
    float r2=0.28*size;
    float	r;
    int bmax=size+30/9;
    for(int b=0;b<bmax;b++)
    {
      r=r1+(r2-r1)*b/bmax;
      float x=0.5*size+r*fc;
      float y=0.5*size-r*fs;
      float fudge=0.5*(y-x);
      x-=fudge;y+=fudge;
      out.putpixel(static_cast<int>(0.5+x),
               static_cast<int>(0.5+y),128+64*a/(4*size));
    }
  }

  //draw planet
  const float O9s3  (0.90*sqrt(3));
  const float O87s3 (0.87*sqrt(3));
  const float O84s3 (0.84*sqrt(3));
  float subtractme;
  for(int a=0;a<size;a++)
    for(int b=0;b<size;b++)
      {
      if(out.getpixel(b,a)==16)
        {
        float tx=3.0*(b-size/2)/size;
        float ty=3.0*(a-size/2)/size;
        if(tx*tx+ty*ty<1)
	{
	float tz=sqrt(1.0-tx*tx-ty*ty);
	//Hack: crater
      if(tz-tx-ty>O9s3)
	  {
	  float vx=-(-tx-ty+tz)/3.0;
	  float vy=-(-tx-ty+tz)/3.0;
	  float vz= (-tx-ty+tz)/3.0;
	  tx=2*vx-tx;
	  ty=2*vy-ty;
	  tz=2*vz-tz;
	  }
	float dp=tx*-0.6+tz*0.8;//3,4,5 triangle!
	subtractme=0;
	if((a>0.3283*size&&a<0.3383*size)||
	   (a>0.49*size&&a<0.51*size)||
	   (a>0.6617*size&&a<0.6717*size))
	  subtractme=std::max(0.0,(b-0.4*size)/size*2);
	out.putpixel(b,a,pftoi(18.0+std::max(0.0,8.0*dp-subtractme)));

	if(tz-tx-ty>O84s3&&tz-tx-ty<O87s3)
	  out.putpixel(b,a,out.getpixel(b,a)+1);
	}
    }
  }

  return out;
}

int pftoi(float f)
{
//Probabilistic Float to Int.  Returns one of the two ints either side to f,
//in proportions which make the average return value equal to f
int i=(int)(floor(f));
if(rand()<(f-i)*RAND_MAX) i++;
return i;
}

void jools(graphics::bitmap &bmp)
{
  //Draw (the boundary of) a Julia set.
  //Sorry, no essay this time.  Figure it out yourself!

  struct link
  {
  float real;
  float imag;
  int col;
  struct link *next;
  };

  int length,px,py,inscreen,c,bw2,bh2;
  link *start1,*start2,*end;
  float m,x,y,tx,ty,xfactor,yfactor;

  bw2=bmp.width()/2;
  bh2=bmp.height()/2;
  xfactor=(float)(bmp.width())/3.1;
  yfactor=(float)(bmp.height())/3.1;

  (start1=start2=end=new link)->col=128;
  m=float(rand())/RAND_MAX*2*M_PI;
  x=(cos(m)/2-cos(2*m)/4);
  y=(sin(m)/2-sin(2*m)/4);
  m=float(rand())/RAND_MAX*2*M_PI;
  x=x+0.01*cos(m);
  y=y+0.01*sin(m);
  tx=1-4*x;
  ty=-4*y;
  m=sqrt(tx*tx+ty*ty);
  end->real=0.5+sqrt((m+tx)/8);
  end->imag=sqrt((m-tx)/8)*(ty>0?1:-1);

  int width=bmp.width();
  int height = bmp.height();
  length=1;
  while(length--)
  {
    px=int(((start1->real)*xfactor))+bw2;
    py=int(((start1->imag)*yfactor))+bh2;
    inscreen=px>=0&&py>=0&&px<width&&py<height;
    if(!(inscreen&&bmp.getpixel(px,py)))
    {
      c=start1->col;
      if(inscreen)bmp.putpixel(px,py,c);
      tx=start1->real-x;
      ty=start1->imag-y;
      m=sqrt(tx*tx+ty*ty);
      ty=sqrt((m-tx)/2)*(ty>0?1:-1);
      tx=sqrt((m+tx)/2);
      if(++c==256)c=128;
      end=end->next=new link;
      end->real=tx;
      end->imag=ty;
      end->col=c;
      end=end->next=new link;
      end->real=-tx;
      end->imag=-ty;
      end->col=c;
      length+=2;
    }
    start1=start1->next;
    delete start2;
    start2=start1;
  }
}

void drawit(int *x,int *y,int *n,int dx,int dy,int level)
{
//Recursively draw Hilbert's space-filling curve
//
//   *2* *2* *2* *2*  (Actually, we only draw the corners in this
//   2 2 2 2 2 2 2 2   picture, but the comment "********
//   * *1* * * *1* *                             ********
//   1     1 1     1                             ********
//   *2* *2* *2* *2*                             ********
//     2 2     2 2                               ********
//   *2* *2*0*2* *2*                             ********
//   0             0                             ********
//   * *1*2* *2*1* *                             ********" would
//   2 2   2 2   2 2   not be very helpful...)
//   *2* *2* *2* *2*
//       1     1
//   *2* *2* *2* *2*
//   2 2   2 2   2 2
//   * *1*2* *2*1* *
//
if(level--)
  for(int a=0;a<4;a++)
    {
    switch(a)
      {
      case 1: *x+=dy;*y+=dx;break;
      case 2: *x+=dx;*y+=dy;break;
      case 3: *x-=dy;*y-=dx;break;
      }
    if(a){if(++*n==256)*n=128;graphics::screen_().putpixel(*x,*y,*n);}
    switch(a)
      {
      case 0:  drawit(x,y,n, dy, dx,level);break;
      case 3:  drawit(x,y,n,-dy,-dx,level);break;
      default: drawit(x,y,n, dx, dy,level);break;
      }
    }
}


#if 0
//Only used for marketing purposes, not called in normal builds
void screenshot()
{
//Dumps screen to bitmap file.  Some problems with pallette
BITMAP *ssbmp=create_bitmap(640,480);
PALETTE pal;

get_palette(pal);
blit(screen, ssbmp,0,0,0,0,640,480);
save_bmp("prot.bmp",ssbmp,pal);
destroy_bitmap(ssbmp);
}
#endif
