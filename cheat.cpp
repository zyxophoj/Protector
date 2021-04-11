#include "cheat.h"
#include "sdlinclude.h"

void cheat::wait_for_key(){
  SDL_Event event;   
  while(true){                                     
    while(SDL_PollEvent(&event)){
      if (event.type==SDL_KEYDOWN) {
        return;
      }
    }
  }
}

void cheat::wait(int i){
  SDL_Delay(i);
}
