
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "joy.h"

//controls
struct con
{
char *constr[9];
bool isk[9];
int  *b[9];
volatile char *k[9];
public:
int stick_ud;
JOYSTICK_AXIS_INFO *stick_udaxis;
int stick_lr;
JOYSTICK_AXIS_INFO *stick_lraxis;
void redefine();
bool get(int);
void info(int,int);
};


void joystick::load_data()
{
if (load_joystick_data(0))remove_joystick();
}

void joystick::poll()
{
  ::poll_joystick();
}

int calibrate_stick(void)
{

#ifdef JOYSTICK

int m;
const char *joy_type[15]={
"Guess","No Joystick","Standard","2 Sticks","4 Button","6 Button","8 Button",
"Flightstick Pro","Wingman Extreme","Sidewinder","Gamepad Pro",
"Snespad LPT1","Snespad LPT2","Snespad LPT3","Wingman Warrior"
};
int joy_arg[15]={
JOY_TYPE_AUTODETECT,JOY_TYPE_NONE,JOY_TYPE_STANDARD,JOY_TYPE_2PADS,
JOY_TYPE_4BUTTON,JOY_TYPE_6BUTTON,JOY_TYPE_8BUTTON,JOY_TYPE_FSPRO,
JOY_TYPE_WINGEX,JOY_TYPE_SIDEWINDER,JOY_TYPE_GAMEPAD_PRO,
JOY_TYPE_SNESPAD_LPT1,JOY_TYPE_SNESPAD_LPT2,JOY_TYPE_SNESPAD_LPT3,
JOY_TYPE_WINGWARRIOR
};

graphics::bitmap sparescreen(640,480);

l_top:
clear(screen);
m=keymenu(150,80,0,joy_type,15);
 while(keypressed())clear_keybuf();

clear(screen);
remove_joystick();
if(m!=1)
  {
  textout_ex(screen,font,"Move joystick to centre and press a key",50,50,15,-1);
  readkey();
  }
if(install_joystick(joy_arg[m]))
  {
  textout_ex(screen,font,"Error: ",100,100,4,-1);
  textout_ex(screen,font,allegro_error,156,100,4,-1);
  rest(1500);
  goto l_top;
  }
for (int i=0; i<num_joysticks; i++)
  {
  while (joy[i].flags &	JOYFLAG_CALIBRATE)
    {
    const char *msg = calibrate_joystick_name(i);
    rectfill(screen,50,50,450,58,0);
    textout_ex(screen,font,msg,50,50,15,-1);
    textout_ex(screen,font," and press a key",50+(strlen(msg))*8,50,15,-1);
    readkey();
    if (calibrate_joystick(i) != 0)return -1;
    }
  while(!keypressed())
    {
    poll_joystick();
    //plot_joystick_state(sparescreen,i);
    graphics::screen_ scr;
    blit(sparescreen,scr,0,0,0,0,640,480);
    }
  clear(screen);
  }

save_joystick_data(0);


#endif

return 0;
}


//This function largely copied from Allegro Setup 3.12, by Shawn Hargreaves

/* joystick test display */
#if 0
void plot_joystick_state(graphics::bitmap &bmp, int i)
{
   char	buf[80];
   int j, x, y;
   int c = 0;

   clear(bmp);

   if (joystick_driver)	{
      if (num_joysticks	> 1)
	 textprintf(bmp, font, 64, 40+c*20, -1,	"%s (%d/%d)", joystick_driver->name, i+1, num_joysticks);
      else
	 textprintf(bmp, font, 64, 40+c*20, -1,	joystick_driver->name);
      c++;
   }

   for (j=0; j<joy[i].num_sticks; j++) {
      if (joy[i].stick[j].num_axis == 2) {
	 if (joy[i].stick[j].flags & JOYFLAG_ANALOGUE)
	   {
	   x = (joy[i].stick[j].axis[0].pos+128)*SCREEN_W/256;
	   y = (joy[i].stick[j].axis[1].pos+128)*(SCREEN_H-60)/256;
	   hline(bmp, x-12, y, x+12, 15);
	   vline(bmp, x, y-12, y+12, 15);
	   circle(bmp, x, y, 12, 1);
	   circlefill(bmp, x, y, 4, 31);
	   }
	 else {
	    sprintf(buf, "%s:",	joy[i].stick[j].name);
	    if (joy[i].stick[j].axis[1].d1) 
	       strcat(buf, " up");
	    if (joy[i].stick[j].axis[1].d2) 
	       strcat(buf, " down");
	    if (joy[i].stick[j].axis[0].d1) 
	       strcat(buf, " left");
	    if (joy[i].stick[j].axis[0].d2) 
	       strcat(buf, " right");
	    textout_ex(bmp, font, buf, 64,	40+c*20, 15,-1);
	    c++;
	 }
      }
      else {
	 sprintf(buf, "%s: %s %4d %s", 
		joy[i].stick[j].name, 
		(joy[i].stick[j].axis[0].d1) ? "<-" : "  ", 
		joy[i].stick[j].axis[0].pos, 
		(joy[i].stick[j].axis[0].d2) ? "->" : "  ");

	 textout_ex(bmp, font, buf, 64, 40+c*20, 15,-1);
	 c++;
      }
   }

   for (j=0; j<joy[i].num_buttons; j++)	{
      sprintf(buf, "%s: %s", joy[i].button[j].name, (joy[i].button[j].b) ? "*" : "");

      if (j&1) {
	 textout_ex(bmp, font, buf, 192, 40+c*20, 15,-1);
	 c++;
      }
      else
	 textout_ex(bmp, font, buf, 64, 40+c*20, 15,-1);
   }

   textout_centre_ex(bmp, font, "- press a key to accept -", SCREEN_W/2, SCREEN_H-76, 15,-1);
}
#endif



//Generate menu, return index of selected item.
int keymenu(int x,int y,int start,const char **textlist,int length)
{
int h(start);
while(key[KEY_SPACE]||key[KEY_ENTER]);
for(int a=0;a<length;a++)textout_ex(screen,font,(char *)textlist[a],x,y+8*a,15,-1);
while(!key[KEY_SPACE]&&!key[KEY_ENTER])
  {
  textout_ex(screen,font,(char *)textlist[h],x,y+8*h,2,-1);
  readkey();
  if(key[KEY_UP]&&h)
    {
    textout_ex(screen,font,(char *)textlist[h],x,y+8*h,15,-1);
    h--;
    }
  if(key[KEY_DOWN]&&h<length-1)
    {
    textout_ex(screen,font,(char *)textlist[h],x,y+8*h,15,-1);
    h++;
    }
  }
while(key[KEY_SPACE]||key[KEY_ENTER]);
return h;
}


void keys::clear_buffer()
{
  ::clear_keybuf();
}

int keys::readkey()
{
  return ::readkey();
}

bool keys::keypressed()
{
  return ::keypressed();
}

int keys::keycount()
{
int a;
int count;
for(a=count=0;a<128;a++)if(key[a])count++;
return count;
}

void keys::set_key_led_flag(int i)
{
key_led_flag = i;
}


//timer interrupt
volatile int counter(0);
void biginc(void){counter++;}
END_OF_FUNCTION(biginc);

void timer::wait(int i)
{
  ::rest(i);
}


void timer::init()
{
LOCK_VARIABLE(counter);
LOCK_FUNCTION(biginc);
install_int_ex(biginc,MSEC_TO_TIMER(10));//100 frames per second
}

int timer::get_counter()
{
  return counter;
}

void timer::set_counter(int n)
{
  counter = n;
}





void con::redefine()
{
char *desc[9]={"fire","go up","go down","go left","go right","detonate a bomb",
	       "activate the shield","commit suicide","pause"};

char *kdesc[128]={
"??1","Escape","1","2","3","4","5","6","7","8","9","0","-","=","Backspace",
"Tab","Q","W","E","R","T","Y","U","I","O","P","[","]","Enter",
"Left Control","A","S","D","F","G","H","J","K","L",";",
"'","`","Left Shift","#","Z","X","C","V","B","N","M",",",".","/","Right Shift","*",
"Alt","(Space)","Caps Lock","F1","F2","F3","F4","F5","F6","F7","F8","F9",
"F10","Num Lock","Scroll Lock","Home","Up","Page Up","- (Pad)","Left",
"5 (Pad)","Right","+ (Pad)","End","Down","Insert","Delete","Print Screen",
"F11","F12","\\","??2","??3","??4","??5","??6","??7","??8","??9","??10",
"??11","??12","??13","??14","??15","??16","??17","??18","??19","??20","??21",
"??22","??23","??24","??25","??26","??27","??28","??29","??30","??31","??32",
"??33","??34","??35","??36","??37","??38","??39","??40","??41","??42"

};


char blurb[40]="";
clear(screen);
stick_ud=0;
stick_lr=0;

for(int a=0;a<9;a++)
  {
  textout_ex(screen,font,"Press the key you want to use to ",20,100+10*a,15,-1);
  textout_ex(screen,font,desc[a],284,100+10*a,15,-1);
  int count;
  volatile char *ck;
  int *cb;
  JOYSTICK_AXIS_INFO *ca;
  JOYSTICK_STICK_INFO *cs;
  int ad;
  int key_base[128];
  for(int i=0; i<128;i++) key_base[i]=key[i];
  do
    {
    count=0;
    ck=0;
    cb=0;
    ca=0;
    cs=0;
    for(int b=0;b<128;b++)
      if(key[b] && !key_base[b])
	{
	count++;
	ck=key+b;
	strcpy(blurb,kdesc[b]);
	}
    poll_joystick();
    for(int b1=0;b1<num_joysticks;b1++)
      for(int b2=0;b2<joy[b1].num_buttons;b2++)
	if(joy[b1].button[b2].b)
	  {
	  count++;
	  cb=&joy[b1].button[b2].b;
	  strcpy(blurb,joy[b1].button[b2].name);
	  }
    for(int a1=0;a1<num_joysticks;a1++)
      for(int a2=0;a2<joy[a1].num_sticks;a2++)
	for(int	a3=0;a3<joy[a1].stick[a2].num_axis;a3++)
	  {
	  if(joy[a1].stick[a2].axis[a3].d1)
	    {
	    count++;
	    cs=&(joy[a1].stick[a2]);
	    ca=&(joy[a1].stick[a2].axis[a3]);
	    ad=-1;
	    strcpy(blurb,joy[a1].stick[a2].name);
	    strcat(blurb,a3?" Up":" Left");
	    cb=&(joy[a1].stick[a2].axis[a3].d1);
	    }
	  if(joy[a1].stick[a2].axis[a3].d2)
	    {
	    count++;
	    cs=&(joy[a1].stick[a2]);
	    ca=&(joy[a1].stick[a2].axis[a3]);
	    ad=1;
	    strcpy(blurb,joy[a1].stick[a2].name);
	    strcat(blurb,a3?" Down":" Right");
	    cb=&(joy[a1].stick[a2].axis[a3].d2);
	    }
	  }
    }
  while(count!=1);
  isk[a]=ck;
  k[a]=ck;
  b[a]=cb;
  textout_ex(screen,font,blurb,440,100+10*a,4,-1);
  delete[]constr[a];
  constr[a]=new	char[strlen(blurb)+1];
  strcpy(constr[a],blurb);
  rest(500);
  if(ca&&(cs->flags&JOYFLAG_ANALOGUE))
    {
    if(a==2&&cb==&(ca->d2)&&b[1]==&(ca->d1))
      {
      stick_ud=1;
      stick_udaxis=ca;
      textout_ex(screen,font,"Analogue up/down",50,300,2,-1);
      }
    if(a==2&&cb==&(ca->d1)&&b[1]==&(ca->d2))
      {
      stick_ud=-1;
      stick_udaxis=ca;
      textout_ex(screen,font,"Analogue up/down (reversed)",50,300,2,-1);
      }
    if(a==4&&cb==&(ca->d2)&&b[3]==&(ca->d1))
      {
      stick_lr=1;
      stick_lraxis=ca;
      textout_ex(screen,font,"Analogue left/right",50,310,2,-1);
      }
    if(a==4&&cb==&(ca->d1)&&b[3]==&(ca->d2))
      {
      stick_lr=-1;
      stick_lraxis=ca;
      textout_ex(screen,font,"Analogue left/right (reversed)",50,310,2,-1);
      }
    }

  do
    {
    count=0;
    for(int b=0;b<128;b++)if(key[b] && !key_base[b])count++;
    poll_joystick();
    for(int b1=0;b1<num_joysticks;b1++)
      for(int b2=0;b2<joy[b1].num_buttons;b2++)
	if(joy[b1].button[b2].b)count++;
    for(int a1=0;a1<num_joysticks;a1++)
      for(int a2=0;a2<joy[a1].num_sticks;a2++)
	for(int	a3=0;a3<joy[a1].stick[a2].num_axis;a3++)
	  {
	  if(joy[a1].stick[a2].axis[a3].d1)count++;
	  if(joy[a1].stick[a2].axis[a3].d2)count++;
	  }
    }
  while(count);
  }
rest(1000);
}

bool con::get(int a)
{
return ( isk[a]?*(k[a]):*(b[a])	);
}

void con::info(int x,int y)
{
char *function[9]={"Fire:","Up:","Down:","Left:","Right:","Smart bomb:",
	       "Shield:","Die:","Pause:"};
for(int	a=0;a<9;a++)
  {
  textout_ex(screen,font,function[a],x,y+10*a,15,-1);
  textout_ex(screen,font,constr[a],x+100,y+10*a,4,-1);
  }
}



command &controls::get(int player)
{
  static con con0;
  static con con1;
  static command commands[] = {&con0, &con1};

  return commands[player];
}

void controls::standard()
{
char *names1[9]={"Caps Lock","A","Z","X","C","B","S","Escape","P"};
char *names2[9]={"Enter","Up","Down","Left","Right","[","]","Escape","P"};

con *con0 = controls::get(0).m_pcon;
con *con1 = controls::get(1).m_pcon;

for(int a=0;a<9;a++)
  {  
  con0->isk[a]=true;
  con1->isk[a]=true;
  con0->constr[a]=new char[strlen(names1[a])+1];
  strcpy(con0->constr[a],names1[a]);

  con1->constr[a]=new char[strlen(names2[a])+1];
  strcpy(con1->constr[a],names2[a]);

  }
con0->k[0]=key+KEY_CAPSLOCK;
con0->k[1]=key+KEY_A;
con0->k[2]=key+KEY_Z;
con0->k[3]=key+KEY_X;
con0->k[4]=key+KEY_C;
con0->k[5]=key+KEY_B;
con0->k[6]=key+KEY_S;
con0->k[7]=key+KEY_ESC;
con0->k[8]=key+KEY_P;

con1->k[0]=key+KEY_ENTER;
con1->k[1]=key+KEY_UP;
con1->k[2]=key+KEY_DOWN;
con1->k[3]=key+KEY_LEFT;
con1->k[4]=key+KEY_RIGHT;
con1->k[5]=key+KEY_OPENBRACE;
con1->k[6]=key+KEY_CLOSEBRACE;
con1->k[7]=key+KEY_ESC;
con1->k[8]=key+KEY_P;

con0->stick_ud=0;
con0->stick_lr=0;

con1->stick_ud=0;
con1->stick_lr=0;  
}

void controls::cleanup(void)
{
}

command::~command()
{
  if (m_pcon)
   for (int i=0; i<9; i++)
     if(m_pcon->constr[i])
       delete[] m_pcon->constr[i];
}
   
int command::get_ud() const
{
  if (m_pcon->stick_ud)
  {
    return m_pcon->stick_udaxis->pos * m_pcon->stick_ud;
  }
  else
  {
    return 128*(m_pcon->get(2)-m_pcon->get(1));
  }
}

int command::get_lr() const
{
  if (m_pcon->stick_lr)
  {
    return m_pcon->stick_lraxis->pos * m_pcon->stick_lr;
  }
  else
  {
    return 128*(m_pcon->get(4)-m_pcon->get(3));
  }
}

bool command::get_shoot() const  {return m_pcon->get(0);}
bool command::get_bomb() const   {return m_pcon->get(5);}
bool command::get_shield() const {return m_pcon->get(6);}
bool command::get_die() const    {return m_pcon->get(7);}
bool command::get_pause() const  {return m_pcon->get(8);}


//These probably shouldn't be members, and shouldnt be here.
void command::redefine() {m_pcon->redefine();}
void command::info(int x, int y) {m_pcon->info(x,y);}
