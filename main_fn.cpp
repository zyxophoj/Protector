#include "main.h"
#include "allegro.h"

//A slight irritation - allegro demands we use the END_OF_MAIN macro
//but main.cpp shouldn't include allegro.h.  So put the real main
//function here.

int main()
{
  return main_();
}
END_OF_MAIN();
