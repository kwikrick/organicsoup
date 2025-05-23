#pragma once

#include "util.h" 

class Atom 
{
public:
    Atom(float x, float y, char type, int state)
        :x(x),y(y),type(type),state(state)
    {
   
    };
    
    void update(int width, int height) {
        // Brownian motion
        vx += randf(-temp,temp);
        vy += randf(-temp,temp);      
        vx -= (vx * friction);
        vy -= (vy * friction);
        x += vx;
        y += vy;
        if (x<0) {vx=-vx; x+=vx/2;}
        if (y<0) {vy=-vy; y+=vy/2;}
        if (x>width) {vx=-vy; x+=vx/2;}
        if (y>height) {vy=-vy; y+=vy/2;}
    }
    
    float x;
    float y;
    float vx = 0;
    float vy = 0;
    char type;
    int state;
    
    const float friction = 0.1;
    const float temp = 1;
    
};

