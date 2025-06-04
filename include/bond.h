#pragma once

#include <memory>
#include <SDL2/SDL.h>

#include "atom.h"

struct Bond {

    std::shared_ptr<Atom> atom1;
    std::shared_ptr<Atom> atom2;
    const PhysicsParameters& params;
    
    Bond(const PhysicsParameters& params,std::shared_ptr<Atom> atom1, std::shared_ptr<Atom> atom2)
        :params(params),atom1(atom1),atom2(atom2)
    {
        atom1->num_bonds+=1;
        atom2->num_bonds+=1;
    };

    ~Bond() {
        atom1->num_bonds-=1;
        atom2->num_bonds-=1;
    }

    void update() {
        float dx = atom2->x - atom1->x;
        float dy = atom2->y - atom1->y;
        float dist = sqrt(dx*dx + dy*dy);
        float force = (dist-params.bonding_distance) * params.bonding_strength;
        atom1->vx += force * dx / dist;
        atom1->vy += force * dy / dist;
        atom2->vx -= force * dx / dist;
        atom2->vy -= force * dy / dist;
    };

    void draw(SDL_Renderer& renderer, float scale, float offset_x, float offset_y) const {
        SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
        float dx = atom2->x - atom1->x;
        float dy = atom2->y - atom1->y;
        float d2 = dx * dx + dy * dy;
        float d = sqrt(d2) + 0.0001f; // avoid division by zero
        float nx = dx /d;
        float ny = dy /d; 
        float x1 = atom1->x + (params.atom_radius/2) * nx;
        float y1 = atom1->y + (params.atom_radius/2) * ny;
        float x2 = atom2->x - (params.atom_radius/2) * nx;
        float y2 = atom2->y - (params.atom_radius/2) * ny;
        x1 = offset_x + x1*scale;
        y1 = offset_y + y1*scale;
        x2 = offset_x + x2*scale;
        y2 = offset_y + y2*scale;
        SDL_RenderDrawLineF(&renderer, x1,y1,x2,y2);
    };
};

