#include "../graphics.h"

#include "../cheat.h"
#include <stdio.h>

//This should display the 16-colour default pallette

int main()
{
  graphics::screen_ scr = graphics::init();

  for(int i=0; i<16; ++i){
    int w=scr.width()/4, h=scr.height()/4;
    scr.rect(w*(i%4), h*(i/4), w*(i%4+1), h*(i/4+1), i);
  }

  cheat::wait(10);
  scr.kick(); 

  cheat::wait_for_key();
    
  graphics::exit();

  return 0;
}
