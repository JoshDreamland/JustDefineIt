/**
 * @file svg_simple.h
 * @brief A small library for creating simple SVG images.
 * 
 * @section License
 * 
 * Copyright (C) 2011 Josh Ventura
 * This file is part of JustDefineIt.
 * 
 * JustDefineIt is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License, or (at your option) any later version.
 * 
 * JustDefineIt is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * JustDefineIt. If not, see <http://www.gnu.org/licenses/>.
**/


#include <cstdio>
#include <string>

class SVG {
  FILE* f;
  public:
    static std::string escape(std::string e);
    static std::string tostring(int d);
    void write_header(int w, int h);
    void draw_circle(std::string id, int x,int y,int r,unsigned fill = 0xFFFFFFFF,unsigned stroke = 0xFF000000, double stroke_width = 2);
    void draw_line(std::string id, int x1, int y1, int x2, int y2, unsigned color = 0xFF000000, float stroke_width = 2);
    void draw_rectangle(std::string id, int x1, int y1, int x2, int y2, unsigned color = 0xFF000000, float stroke_width = 2);
    void draw_text(std::string id, int cx, int bly, std::string t, int sz=12, unsigned color=0xFF000000);
    void close();
    bool is_open();
    SVG(const char* fn);
    ~SVG();
};
