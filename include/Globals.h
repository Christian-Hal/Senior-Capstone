
#pragma once
#include "Canvas.h"

class Globals {

public:
    // setter methods
    void set_fileOpen (bool state);
    void set_canvas_x(int val);
    void set_canvas_y(int val);

    void set_scr_width(int val);
    void set_scr_height(int val);

    // getter methods
    bool is_file_open();
    int get_canvas_x();
    int get_canvas_y();

    int get_scr_width();
    int get_scr_height();

private:
    ////// all of the actual variables
    // window settings
    int SCR_WIDTH = 1280;
    int SCR_HEIGHT = 720;

    // canvas settings
    bool fileOpen = false;
    int canvas_x = 0;
    int canvas_y = 0;
};