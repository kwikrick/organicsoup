#pragma once
#include <cassert>

#include "util.h" 
#include "physicsparameters.h"
#include "charge.h"

class Atom 
{
public:
    Atom(const PhysicsParameters& params, float x, float y, char type, int state)
        :params(params),x(x),y(y),type(type),state(state)
    {
        //vx += randf(-10,10);
        //vy += randf(-10,10);      
    };
    
    void update() {        // TODO: add delta parameter
        // Brownian motion
        vx += randf(-params.temp,params.temp);
        vy += randf(-params.temp,params.temp);      
        vx -= (vx * params.friction);
        vy -= (vy * params.friction);
        x += vx;
        y += vy;
        if (correction_n>0) {
            x += correction_x / correction_n;
            y += correction_y / correction_n;
            correction_x = 0;
            correction_y = 0;
            correction_n = 0;
        }
        if (x<params.atom_radius && vx<0) {vx=-vx; x=params.atom_radius+vx/2;}
        if (y<params.atom_radius && vy<0) {vy=-vy; y=params.atom_radius+vy/2;}
        if (x>params.space_width-params.atom_radius && vx>0) {vx=-vx; x=params.space_width-params.atom_radius+vx/2;}
        if (y>params.space_height-params.atom_radius && vy>0) {vy=-vy; y=params.space_height-params.atom_radius+vy/2;}
    }

    void attract(Atom& other, const std::set<Charge>& charges) {

        // TODO: a lot of code is in common with collide. Combine functions or pre-compute common vectors. 

        auto proto_charge = Charge(type,state,0);
        const auto found_charge = charges.find(proto_charge);
        if (found_charge==charges.cend()) return;
        float charge = (*found_charge).charge;

        auto proto_charge2 = Charge(other.type,other.state,0);
        const auto found_charge2 = charges.find(proto_charge2);
        if (found_charge2==charges.cend()) return;
        float other_charge = (*found_charge2).charge;

        float dx = other.x - x;
        float dy = other.y - y;
        float d2 = dx*dx + dy*dy;
        float diameter = 2* params.atom_radius;
        if (d2 > diameter * diameter) {
            float d = sqrt(d2)+0.0001f; // avoid division by zero
            float nx = dx/d;
            float ny = dy/d;
            float fraction = (d-diameter) / (params.charge_distance - diameter);
            if (fraction>1) fraction=1;
            if (fraction<0) fraction=0;
            float force = charge * other_charge * (1-fraction) * params.charge_strength; 
            vx -= force * nx;
            vy -= force * ny;
            other.vx += force * nx;
            other.vy += force * ny;
        };
    }

    bool collide(Atom& other) {
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
            correction_n += 1;
            correction_x -= correct_x;
            correction_y -= correct_y;
            other.correction_n += 1;
            other.correction_x += correct_x;
            other.correction_y += correct_y;
            return true;
        }
        return false;
    };

    bool off_world() { 
            return x < params.atom_radius 
                || x > params.space_width-params.atom_radius 
                || y < params.atom_radius 
                || y > params.space_height - params.atom_radius;
    };
    

    // variables
    
    char type;
    int state;

    const PhysicsParameters& params;
    float x;
    float y;
    float vx = 0;
    float vy = 0;
    float correction_x = 0;
    float correction_y = 0;
    int correction_n = 0;
    
    int spacemap_index = -1; // index in the spacemap, -1 if not in spacemap
    int num_bonds = 0;
    
};

