#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>

#include <cmath>
#include <iostream>
#include <format>
#include <map>
#include <format>

#include "atom.h"


class AtomRenderer
{
public:
    AtomRenderer(SDL_Renderer& renderer): renderer(renderer)
    {
        // Initialize color map
        color_map['a'] = {255,0,0,255};
        color_map['b'] = {0,255,0,255};
        color_map['c'] = {0,0,255,255};
        color_map['d'] = {0,255,255,255};
        color_map['e'] = {255,0,255,255};
        color_map['f'] = {255,255,0,255};

    }
    ~AtomRenderer() {
        for (auto& [key, texture] : texture_map) {
            SDL_DestroyTexture(texture);
        }
        texture_map.clear();
    };

    void draw(const Atom& atom) {
        
        TypeState ts {atom.type, atom.state};
        SDL_Texture* texture = nullptr;

        auto it = texture_map.find(ts);
        if (it != texture_map.end()) {
            texture = it->second;
        }
        else {
            std::cout << "creating texture for " << atom.type << atom.state << "\n";
            SDL_Surface* surface = create_surface(atom);
            texture = SDL_CreateTextureFromSurface(&renderer, surface);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_FreeSurface(surface);
            texture_map[ts] = texture;
        }

        SDL_Rect tgt_rect = {(int)(atom.x-radius), (int)(atom.y-radius), (int)(2*radius), (int)(2*radius)};  
        SDL_RenderCopy(&renderer, texture, nullptr, &tgt_rect); 
    }

    SDL_Surface* create_surface(const Atom& atom) {
        
        // set color from type
        SDL_Color color = color_map[atom.type];
                
        // -- create surface and renderer
        
        int width = radius*2;
        int height = radius*2;
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);      // needed?
        
        SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
        
        // -- draw disk geometry
        
        // a vertex for each segment on the circle + 1 center
        SDL_Vertex vertices[segments+1];
        int num_vertices = segments + 1;
        
        // each segment is a triangle with 3 points
        int indices[segments*3];
        int num_indices = segments*3;
        
        float x0 = radius+1;
        float y0 = radius+1;
        
        vertices[0].position.x = x0;
        vertices[0].position.y = y0;
        vertices[0].color = color;

        for (int i=0;i<segments;++i) {
            float angle=2*M_PI*i/segments;
            float dx = cos(angle)*radius;
            float dy = sin(angle)*radius;
            
            // add vertex for each segment
            vertices[i+1].position.x = x0+dx;
            vertices[i+1].position.y = y0+dy;
            vertices[i+1].color = color;
            
            // add 
            indices[i*3]=0;
            indices[i*3+1]=(i+1)%segments+1;
            indices[i*3+2]=(i+2)%segments+1;
                
        }
        
        SDL_RenderGeometry(renderer,nullptr,vertices,num_vertices,indices,num_indices); 
        
        // -- draw text
        
        TTF_Font* font = TTF_OpenFont("assets/FreeSans.ttf", 16);
        if (!font) { std::cout << "font not found\n"; return surface;}
        
        std::string txt = std::format("{}{}",atom.type,atom.state);
        
        SDL_Color fg_color {
            static_cast<Uint8>(255-color.r),
            static_cast<Uint8>(255-color.g),
            static_cast<Uint8>(255-color.b),
            255};
        
        SDL_Surface* text_surf = TTF_RenderText_Blended(font, txt.c_str(), fg_color);
        
        SDL_Rect src_rect = {0,0,text_surf->w, text_surf->h};
        SDL_Rect dst_rect = {surface->w/2 - text_surf->w/2, surface->h/2 - text_surf->h/2, 0,0};        // note: w and h unused
        
        SDL_BlitSurface(text_surf, &src_rect, surface, &dst_rect);
        
        SDL_FreeSurface(text_surf);
        TTF_CloseFont(font);
        
        SDL_DestroyRenderer(renderer);
        
        return surface;
        
    };


private:
    
    struct TypeState {
        char type;
        int state;
        
        bool operator<(const TypeState& other) const {
            return std::tie(type, state) < std::tie(other.type, other.state);
        }
    };

    const float radius = 16;
    static constexpr int segments = 16;
    SDL_Renderer& renderer;
    SDL_Surface* surface;
    std::map<char,SDL_Color> color_map;
    std::map<TypeState,SDL_Texture*> texture_map;

    
};
