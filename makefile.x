
LIBS = `allegro-config --libs`
CC = g++
ARGS = 
 

HEADERS = back.h baddies.h graphics.h missiles.h prot.h stuff.h joy.h
COPYFILES = prot.txt gpl.txt changes.txt prot.cfg
PROTCOPYFILES = prot/prot.txt prot/gpl.txt prot/changes.txt prot/prot.cfg
OBJS = prot.o baddies.o main.o colision.o back.o graphics.o sound.o joy.o main_fn.o

#The final executable
TARGET = prot/prot

#Options available here are:
#-DCOLOUR_CYCLING (usable in full screen mode, not so hot in a window)
#-DJOYSTICK (DOS only atm :( )
DEFINES =

#Directory where exedat is
ALLEGRODIR = 




all : prot copy

prot: $(TARGET)
	
$(TARGET) : main_fn.o joy.o main.o prot.o baddies.o colision.o back.o graphics.o sound.o prot.dat
	$(CC) $(ARGS) $(OBJS) -o $(TARGET) $(LIBS)
	$(ALLEGRODIR)exedat $(TARGET) prot.dat



#compile object files
#Depends on all headers - could do better here...
.cpp.o: $(HEADERS)
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $< 

#I'm just too lazy to copy these by hand
copy :
	cp $(COPYFILES) prot

clean :
	rm -f $(OBJS) $(PROTCOPYFILES) $(TARGET) 