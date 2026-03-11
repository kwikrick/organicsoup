#pragma once

struct PhysicsParameters
{
    float space_width = 1600;
    float space_height = 900;

    float temp = 0.1f; // Brownian motion temperature
    float friction = 0.01f; // friction coefficient
    
    float atom_radius = 16.0f; // radius of atoms
    float collision_elasticity = 0.9f; // elasticity of collisions

    float charge_distance = 128.0f; 
    float charge_strength = 0.1f; 

    static constexpr int num_atom_types = 6;
};
