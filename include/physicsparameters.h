#pragma once

struct PhysicsParameters
{
    float space_width = 1600;
    float space_height = 900;

    float temp = 0.1f; // Brownian motion temperature
    float friction = 0.01f; // friction coefficient
    
    float atom_radius = 16.0f; // radius of atoms
    float collision_elasticity = 0.9f; // elasticity of collisions

    float bonding_start_radius = 40.0f;   // radius to start bonding atoms
    float bonding_end_radius = 48.0f;     // radius to break bonding atoms
    float bonding_length = 32.0f;         // length of bonding spring between atoms
    float bonding_strength = 0.01f;       // strength of bonding spring between atoms
};
