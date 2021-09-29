/* mbed GraphicsDisplay Display Library Base Class
 * Copyright (c) 2007-2009 sford
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * A library for providing a common base class for Graphics displays
 * To port a new display, derive from this class and implement
 * the constructor (setup the display), pixel (put a pixel
 * at a location), width and height functions. Everything else
 * (locate, printf, putc, cls, window, putp, fill, blit, blitbit) 
 * will come for free. You can also provide a specialised implementation
 * of window and putp to speed up the results
 */

#ifndef MBED_GRAPHICSDISPLAY_H
#define MBED_GRAPHICSDISPLAY_H

#include "TextDisplay.h"

class GraphicsDisplay : public TextDisplay {

public:         
          
    GraphicsDisplay(const char* name);
     
    virtual void pixel(int x, int y, int colour) = 0;
    virtual int width() = 0;
    virtual int height() = 0;
        
    virtual void window(int x, int y, int w, int h);
    virtual void putp(int colour);
    
    virtual void cls();
    virtual void rect(int x0, int y0, int x1, int y1, int colour);
    virtual void fillrect(int x0, int y0, int w, int h, int colour);
    // fill equals fillrect, name has been kept to not break compatibility
    virtual void fill(int x, int y, int w, int h, int colour);
    
    // To draw circle using ellipse, set a and b to the same values
    virtual void ellipse(int xc, int yc, int a, int b, unsigned int colour);
    virtual void fillellipse(int xc, int yc, int a, int b, unsigned int colour);
    virtual void circle(int x, int y, int r, int colour);
    
    virtual void hline(int x0, int x1, int y, int colour);
    virtual void vline(int x0, int y0, int y1, int colour);
    virtual void line(int x0, int y0, int x1, int y1, int colour);
    
    virtual void blit(int x, int y, int w, int h, const int *colour);    
    virtual void blitbit(int x, int y, int w, int h, const char* colour);
    
    virtual void character(int column, int row, int value);
    virtual int columns();
    virtual int rows();
    
protected:

    // pixel location
    short _x;
    short _y;
    
    // window location
    short _x1;
    short _x2;
    short _y1;
    short _y2;

};

#endif
