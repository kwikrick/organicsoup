#pragma once

enum class BondDistance {
        Near,
        Middle,
        Far
};

struct PhysicsParameters
{
    float space_width = 1600;
    float space_height = 900;

    float temp = 0.1f; // Brownian motion temperature
    float friction = 0.01f; // friction coefficient
    
    float atom_radius = 16.0f; // radius of atoms
    float collision_elasticity = 0.9f; // elasticity of collisions

    //float bonding_distance = 32.0f;         // length of bonding spring between atoms
    float bonding_start_distance = 33.0f;   // radius to start bonding atoms
    float bonding_end_distance = 48.0f;     // radius to break bonding atoms
    float bonding_strength = 0.1f;       // strength of bonding spring between atoms

    int max_bonds_per_atom = 6;

    //----

    float bonding_distance(BondDistance distance) const {
        switch(distance) {
            case BondDistance::Near: return bonding_start_distance;
            case BondDistance::Middle: return (bonding_start_distance+bonding_end_distance)/2;
            case BondDistance::Far: return bonding_end_distance;
            default: return 0;
        }
    }
};
