#pragma once

#include "util.h" 

class Atom 
{
public:
    Atom(float x, float y, char type, int state)
        :x(x),y(y),type(type),state(state)
    {
   
    };
    
    void update(int width, int height) {        // TODO: add delta parameter
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

    void collide(Atom& other) {
        float dx = other.x - x;
        float dy = other.y - y;
        float d2 = dx*dx + dy*dy;
        if (d2 < radius * radius) {
            auto d = sqrt(d2 + 1);  // never zero
            vx -= dx / d * collide_strength;
            vy -= dy / d * collide_strength;
            other.vx += dx /d * collide_strength;
            other.vy += dy /d * collide_strength;
        }
    };
    
    float x;
    float y;
    float vx = 0;
    float vy = 0;
    char type;
    int state;
    
    static constexpr float radius = 16;
    
    const float friction = 0.1;
    const float temp = 1;
    const float collide_strength = 1;
    
};

