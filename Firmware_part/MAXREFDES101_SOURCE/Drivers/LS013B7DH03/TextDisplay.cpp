/* mbed TextDisplay Display Library Base Class
 * Copyright (c) 2007-2009 sford
 * Released under the MIT License: http://mbed.org/license/mit
 */
 
#include "TextDisplay.h"
#include <cstdarg>

TextDisplay::TextDisplay(const char *name){
    _row = 0;
    _column = 0;

    if (name == NULL) {
        _path = NULL;
    } else {
        _path = new char[strlen(name) + 2];
        sprintf(_path, "/%s", name);
    }
}
    
int TextDisplay::_putc(int value) {
    if(value == '\n') {
        _column = 0;
        _row++;
        if(_row >= rows()) {
            _row = 0;
        }
    } else {
        character(_column, _row, value);
        _column++;
        if(_column >= columns()) {
            _column = 0;
            _row++;
            if(_row >= rows()) {
                _row = 0;
            }
        }
    }
    return value;
}

// crude cls implementation, should generally be overwritten in derived class
void TextDisplay::cls() {
    locate(0, 0);
    for(int i=0; i<columns()*rows(); i++) {
        _putc(' ');
    }
}

void TextDisplay::set_font(const unsigned char * f) {
    font = f;
    if(font==NULL) {
    	externalfont = 0;  // set display.font
    	locate(0, 0);
    }    
    else{
    	externalfont = 1;
    	locate(0, 0);
    }
}

void TextDisplay::locate(int column, int row) {
    _column = column;
    _row = row;
    char_x = column;
    char_y = row;
}

int TextDisplay::_getc() {
    return -1;
}
        
void TextDisplay::foreground(uint16_t colour) {
    _foreground = colour;
}

void TextDisplay::background(uint16_t colour) {
    _background = colour;
}

void TextDisplay::printf(const char* format, ...) {
	char buffer[MAX_PRINTF_CHARS + 1] = { 0 };
	uint32_t iterator = 0;
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	while((buffer[iterator] != 0) && (iterator < MAX_PRINTF_CHARS)) {
		_putc(buffer[iterator++]);
	}
}
