#include "../graphics.h"
#include "../cheat.h"

// Write "Hello, world!" in the first 16 colours.  Background is black and white.
// Colour 13 will not work at all because it is unlucky enough to be the internal transparency colour.
// (This is arguably a bug, but Protector never draws text in colour 13.)

int main(){

    graphics::screen_ &scr=graphics::init();
    
    scr.rect(25,0,100,200,15);
    for(int i=0; i<16; ++i){
        scr.drawtext("Hello, world!", 10, 10+10*i, i);
    }

    cheat::wait(10);
    scr.kick(); 
    cheat::wait_for_key();
    
    graphics::exit();

    return 0;
}
