#include "../sdlinclude.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480


int main(){
  SDL_Init(SDL_INIT_VIDEO);


  SDL_Window* window = SDL_CreateWindow(
			    "Test 0",
			    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			    SCREEN_WIDTH, SCREEN_HEIGHT,
			    SDL_WINDOW_SHOWN
			    );

  SDL_Surface* surface =  SDL_GetWindowSurface(window);

  SDL_FillRect(surface, NULL, 0);


  SDL_Delay(10); //Necessary magic!
  SDL_UpdateWindowSurface(window);

  SDL_Event event;   
  while(true){                                     
    while(SDL_PollEvent(&event)){
      if (event.type==SDL_KEYDOWN) {
        goto out;
      }
    }
  }
  out:


  SDL_DestroyWindow(window);
  SDL_Quit();
}
