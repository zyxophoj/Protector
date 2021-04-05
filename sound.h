#ifndef SOUND_H
#define SOUND_H
//Sound effects

class sample;

class sound
{
static bool is_ok;
public:
static int init();
static void cleanup();
enum foo
  {
  bang,
  die,
  shoot,
  pickup,
  putdown,
  alert,
  pshoot,
  ouch,
  tadaa,
  collect,
  kaboom,
  bpop,
  clang
  };
static void play(const foo &, int);
};

#endif
