#include "graphics.h"

namespace joystick
{
void load_data();
void poll();
}

namespace keys
{
void clear_buffer();
int readkey();
bool keypressed();
int keycount();
void set_key_led_flag(int);
}

namespace timer
{
void init();
void wait(int);
int get_counter();
void set_counter(int);
}




struct con;

class command
{
public:
  command (con *pcon):m_pcon(pcon){}
  ~command();

  int get_ud() const;
  int get_lr() const;

  bool get_shoot() const;
  bool get_bomb() const;
  bool get_shield() const;
  bool get_pause() const;
  bool get_die() const;

  void redefine();
  void info(int x, int y);

//todo: fix this!
  con *m_pcon;
};

class controls
{
public:
  friend class defender;
  static void standard(void);
  static void cleanup(void);

  static command &get(int player);
  static void redefine(int player);
  static void info(int player, int x, int y);
private:

};

//control functions
int calibrate_stick();
int get_craptop_mode();
void plot_joystick_state(const graphics::bitmap &, int);
int keymenu(int,int,int,const char **,int);
