#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nonCanonical.h"

struct termios saved_attributes;

void reset_input_mode (void){
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}


void set_input_mode (void){
  struct termios tattr;

  if (!isatty (STDIN_FILENO))
    {
      fprintf (stderr, "Not a terminal.\n");
      exit (EXIT_FAILURE);
    }

  tcgetattr (STDIN_FILENO, &saved_attributes);
  atexit (reset_input_mode);

  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO);
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}
