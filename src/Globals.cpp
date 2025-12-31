
#include "Globals.h"

// setter methods
void Globals::set_fileOpen(bool state) { fileOpen = state; }
void Globals::set_canvas_x(int new_x) { canvas_x = new_x; }
void Globals::set_canvas_y(int new_y) { canvas_y = new_y; }

void Globals::set_scr_width(int new_w) { SCR_WIDTH = new_w; }
void Globals::set_scr_height(int new_h) { SCR_HEIGHT = new_h; }

// getter methods
bool Globals::is_file_open() { return fileOpen; }
int Globals::get_canvas_x() { return canvas_x; }
int Globals::get_canvas_y() { return canvas_y; }

int Globals::get_scr_width() { return SCR_WIDTH; }
int Globals::get_scr_height() { return SCR_HEIGHT; }