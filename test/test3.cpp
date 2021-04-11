#include "../graphics.h"
#include "../cheat.h"
#include "../joy.h"

#include<iostream>

// Keymenu test.  This one's interactive.  It should be possible to choose an option using up arrow, down arrow, and return/escape.

int main(){

    graphics::screen_ &scr=graphics::init();
    cheat::wait(10); //Necessary magic?

    const char *options[]={"Pugh", "Pugh", "Barney McGrew", "Cuthbert", "Dibble", "Grubb"};
    int c = keymenu(scr, 50, 50, 2, options, sizeof(options)/sizeof(*options));
    
    graphics::exit();

    std::cout<<"Option Chosen: "<<c<<" ("<<options[c]<<")\n";

    return 0;
}
