
			      PROTECTOR

How to compile Protector:
Unzip the zip file (You probably did that already, or you wouldn't be reading this!) 


You should have these files in one directory, with a subdirectory called "prot", which is where the compiled game will go.

back.h
baddies.h
dlring.h
graphics.h
joy.h
main.h
missiles.h
prot.h
protdata.h
sound.h
stuff.h
back.cpp
baddies.cpp
colision.cpp
graphics.cpp
joy.cpp
main.cpp
main_fn.cpp
prot.cpp
sound.cpp
prot.dat
makefile.x
makefile.dos
prot.cfg
prot.txt
readme.txt
changes.txt
gpl.txt

If anything is missing, you should be able to get it from here:
http://www.markboyd.me.co.uk/games/prot/prot.html


DOS (including windows 95, 98, ME, XP) instructions:

You need the DJGPP compiler and the Allegro library, which you can get here:
http://www.delorie.com/djgpp/
or here:
http://www.simtel.net/simtel.net/
or you could always try something like this:
http://www.google.com/

If you can install those successfully, then Protector should not give you too much trouble - hopefully all you need to do is type this at the DOS prompt:

make -f makefile.dos

A note about the DJGPP license:  If you want to distribute a version of Protector compiled with DJGPP, you have to tell the recipients how to get DJGPP.   


LINUX:

You need the Allegro library.  You can get it here
http://www.simtel.net/simtel.net/

1. Edit the makefile:  ALLEGRODIR needs to be set to the directory allegro is in.
   If you are very brave, add "-DCOLOUR_CYCLING" to the defines line.  This sort of works, but there are some problems with detecting a keypress while cycling.
2. dos2unix *.cpp *.h  (This should only be necessary if I've left some DOS returns in the source instead of UNIX returns.  But it won't do any harm)
3. make -f makefile.x
4. Go into the prot directory and run the game.  Quit the game.  The hi-score table file prot.hst should have been generated.  Adjust the permissions on this file so that other users can write to it.  If you don't want other people to beat your scores, you don't need to bother with this step.



OTHER OPERATING SYSTEMS:
Allegro has been ported to MacOS, BeOS, DirectS, and other operating systems, so it should be possible to get Protector to work.  I haven't tried it, though, so I'm not making any promises.


What's prot.dat?

This is an Allegro data file.  It can be edited with the grabber tool, which comes with Allegro


Licensing and distribution:

Protector is free software, released under the version 2 of the GPL, or any later version.  Share and enjoy!


Disclaimer:

If anything goes wrong, it's not my fault.