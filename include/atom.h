#pragma once

#include "util.h" 
#include "physicsparameters.h"

class Atom 
{
public:
    Atom(float x, float y, char type, int state)
        :x(x),y(y),type(type),state(state)
    {
        //vx += randf(-10,10);
        //vy += randf(-10,10);      
    };
    
    void update(const PhysicsParameters& params) {        // TODO: add delta parameter
        // Brownian motion
        vx += randf(-params.temp,params.temp);
        vy += randf(-params.temp,params.temp);      
        vx -= (vx * params.friction);
        vy -= (vy * params.friction);
        x += vx;
        y += vy;
        if (x<0 && vx<0) {vx=-vx; x+=vx/2;}
        if (y<0 && vy<0) {vy=-vy; y+=vy/2;}
        if (x>params.space_width && vx>0) {vx=-vx; x+=vx/2;}
        if (y>params.space_height && vy>0) {vy=-vy; y+=vy/2;}
    }

    void collide(Atom& other, const PhysicsParameters& params) {
        float dx = other.x - x;
        float dy = other.y - y;
        float d2 = dx*dx + dy*dy;
        float diameter = 2* params.atom_radius;
        if (d2 < diameter * diameter) {
            float d = sqrt(d2)+0.0001f; // avoid division by zero
            float nx = dx/d;
            float ny = dy/d;
            // elastic collision
            float dvx_elastic = -nx * ((vx-other.vx)*(x-other.x) + (vy-other.vy)*(y-other.y)) / d;
            float dvy_elastic = -ny * ((vx-other.vx)*(x-other.x) + (vy-other.vy)*(y-other.y)) / d;
            // inelastic collision
            float dvx_inelastic = vx - (vx + other.vx) / 2;
            float dvy_inelastic = vy - (vy + other.vy) / 2;
            // apply collision
            float dvx = dvx_elastic * params.collision_elasticity + dvx_inelastic * (1-params.collision_elasticity);
            float dvy = dvy_elastic * params.collision_elasticity + dvy_inelastic * (1-params.collision_elasticity);
            vx -= dvx;
            vy -= dvy;
            other.vx += dvx;
            other.vy += dvy;
            // move apart
            float correct_x = nx * (diameter - d)/2; 
            float correct_y = ny * (diameter - d)/2;
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
    
    int spacemap_index = -1; // index in the spacemap, -1 if not in spacemap
    
};

