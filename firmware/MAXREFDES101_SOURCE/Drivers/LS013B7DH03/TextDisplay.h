/* mbed TextDisplay Library Base Class
 * Copyright (c) 2007-2009 sford
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * A common base class for Text displays
 * To port a new display, derive from this class and implement
 * the constructor (setup the display), character (put a character
 * at a location), rows and columns (number of rows/cols) functions.
 * Everything else (locate, printf, putc, cls) will come for free
 *
 * The model is the display will wrap at the right and bottom, so you can
 * keep writing and will always get valid characters. The location is 
 * maintained internally to the class to make this easy
 */

#ifndef MBED_TEXTDISPLAY_H
#define MBED_TEXTDISPLAY_H

#include "mbed.h"
#include "LCDSettings.h"

class TextDisplay {
public:

  // functions needing implementation in derived implementation class
  /** Create a TextDisplay interface
     *
     * @param name The name used in the path to access the strean through the filesystem
     */
    TextDisplay(const char *name = NULL);

    /** output a character at the given position
     *
     * @param column column where charater must be written
     * @param  row where character must be written
     * @param c the character to be written to the TextDisplay
     */
    virtual void character(int column, int row, int c) = 0;

    /** return number if rows on TextDisplay
     * @result number of rows
     */
    virtual int rows() = 0;

    /** return number if columns on TextDisplay
    * @result number of rows
    */
    virtual int columns() = 0;
    
    // Sets external font usage, eg. dispaly.set_font(Arial12x12);
    // This uses pixel positioning.
    // display.set_font(NULL); returns to internal default font.
    void set_font(const unsigned char * f);
    
    // set position of the next character or string print.
    // External font, set pixel x(column),y(row) position.
    // internal(default) font, set character column and row position 
    virtual void locate(int column, int row);
    
    // functions that come for free, but can be overwritten

    /** clear screen
    */
    virtual void cls();
    virtual void foreground(uint16_t colour);
    virtual void background(uint16_t colour);
    // putc (from Stream)
    // printf (from Stream)
    virtual void printf(const char* format, ...);
    
protected:

    virtual int _putc(int value);
    virtual int _getc();
    
    // external font functions
    const unsigned char* font;
    int externalfont;
 
    // character location
    uint16_t _column;
    uint16_t _row;
    unsigned int char_x;
    unsigned int char_y;

    // colours
    uint16_t _foreground;
    uint16_t _background;
    char *_path;
};

#endif
