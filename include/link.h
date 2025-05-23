#pragma once

#include <memory>

#include "atom.h"
#include <SDL2/SDL.h>

struct Link {

    std::shared_ptr<Atom> atom1;
    std::shared_ptr<Atom> atom2;
    
    Link(std::shared_ptr<Atom> atom1, std::shared_ptr<Atom> atom2)
        :atom1(atom1), atom2(atom2)
    {
      
    };

    void update() {
        float dx = atom2->x - atom1->x;
        float dy = atom2->y - atom1->y;
        float dist = sqrt(dx*dx + dy*dy);
        float force = (dist-length) * strength;
        atom1->vx += force * dx / dist;
        atom1->vy += force * dy / dist;
        atom2->vx -= force * dx / dist;
        atom2->vy -= force * dy / dist;
    };

    void draw(SDL_Renderer& renderer) {
        //SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(&renderer, atom1->x, atom1->y, atom2->x, atom2->y);
    };

    const float length = 50;
    const float strength = 0.01;
};

