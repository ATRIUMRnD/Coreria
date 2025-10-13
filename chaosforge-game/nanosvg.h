/*
 * NanoSVG v0.9.5 - Simple stupid SVG parser
 *
 * Copyright (c) 2013-2017 Mikko Mononen memon@inside.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Mikko Mononen memon@inside.org
 */

#ifndef NANOSVG_H
#define NANOSVG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Basic SVG structures for simple graphics
typedef struct NSVGshape {
    float* pts;     // Path points
    int npts;       // Number of points
    unsigned int fill;  // Fill color (RGBA)
    unsigned int stroke; // Stroke color (RGBA)
    float strokeWidth;
    struct NSVGshape* next;
} NSVGshape;

typedef struct NSVGimage {
    float width, height;
    NSVGshape* shapes;
} NSVGimage;

// Simple SVG parsing and rendering functions
NSVGimage* nsvgParseFromFile(const char* filename, const char* units, float dpi);
NSVGimage* nsvgParse(char* input, const char* units, float dpi);
void nsvgDelete(NSVGimage* image);

// Basic shape creation functions for procedural graphics
NSVGimage* nsvgCreateImage(float width, float height);
NSVGshape* nsvgCreateRect(float x, float y, float w, float h, unsigned int fill);
NSVGshape* nsvgCreateCircle(float cx, float cy, float r, unsigned int fill);
NSVGshape* nsvgCreateLine(float x1, float y1, float x2, float y2, unsigned int stroke, float strokeWidth);
void nsvgAddShape(NSVGimage* image, NSVGshape* shape);

// OpenGL rendering helper
void nsvgRenderGL(NSVGimage* image, float tx, float ty, float scale);

#endif // NANOSVG_H
