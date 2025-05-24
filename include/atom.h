#pragma once

#include "util.h" 

class Atom 
{
public:
    Atom(float x, float y, char type, int state)
        :x(x),y(y),type(type),state(state)
    {
        //vx += randf(-10,10);
        //vy += randf(-10,10);      
    };
    
    void update(int width, int height) {        // TODO: add delta parameter
        // Brownian motion
        vx += randf(-temp,temp);
        vy += randf(-temp,temp);      
        vx -= (vx * friction);
        vy -= (vy * friction);
        x += vx;
        y += vy;
        if (x<0 && vx<0) {vx=-vx; x+=vx/2;}
        if (y<0 && vy<0) {vy=-vy; y+=vy/2;}
        if (x>width && vx>0) {vx=-vx; x+=vx/2;}
        if (y>height && vy>0) {vy=-vy; y+=vy/2;}
    }

    void collide(Atom& other) {
        float dx = other.x - x;
        float dy = other.y - y;
        float d2 = dx*dx + dy*dy;
        float diameter = radius + other.radius;
        if (d2 < diameter * diameter) {
            float d = sqrt(d2)+0.0001f; // avoid division by zero
            float nx = abs(dx/d); // normal vector
            float ny = abs(dy/d);
            // delta velocity
            float dvx_self = nx * (vx + ny * -vy);
            float dvy_self = ny * (vy + nx * vx);
            float dvx_other = nx * (other.vx + ny * -other.vy);
            float dvy_other = ny * (other.vy + nx * other.vx);
            // elastic collision
            float dvx = dvx_self - dvx_other;
            float dvy = dvy_self - dvy_other;
            vx -= dvx;
            vy -= dvy;
            other.vx += dvx;
            other.vy += dvy;
            // move apart
            float correct_x = dx/d * (diameter - d)/2; 
            float correct_y = dy/d * (diameter - d)/2;
            x -= correct_x;
            y -= correct_y;
            other.x += correct_x;
            other.y += correct_y;
        }
    };
    
    float x;
    float y;
    float vx = 0;
    float vy = 0;
    char type;
    int state;
    
    static constexpr float radius = 16;
    
    const float friction = 0.01;
    const float temp = 0.1;
    const float collide_strength = 1;
    
};

