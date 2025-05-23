// Organic Builder Clone
// by kwikrick

// runs as a web-app (cpp -> html+js+wasm)

// C includes
#include <emscripten/emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// C++ includes
#include <stdlib.h>
#include <string>
#include <vector>
#include <memory>

// my includes
#include "atom.h"
#include "atomrenderer.h"
#include "link.h"
#include "rule.h"

// Dear ImGui
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"


class Application {
public:
    Application() {
     
        SDL_CreateWindowAndRenderer(1600, 900, SDL_WINDOW_RESIZABLE, &window, &renderer);

        setup_imgui();

        atom_renderer = std::make_unique<AtomRenderer>(*renderer);
        
        int width, height;
        SDL_GetWindowSize(window, &width,&height);

        for (int i=0;i<100;++i) {
            int x=rand()%width;
            int y=rand()%height;
            char type = 'a' + rand()%6;
            int state = 0;
            atoms.push_back(std::make_shared<Atom>(x,y,type,state));
        }

        // for (int i=0;i<100;++i) {
        //     auto atom1=atoms[rand()%atoms.size()];
        //     auto atom2=atoms[rand()%atoms.size()];
        //     if (atom1 != atom2) {
        //         links.push_back(std::make_shared<Link>(atom1, atom2));
        //     }
        // }

        rules.push_back(std::make_unique<Rule>(Rule::Type::BOND, 'a', 'b', 0, 0, 1, 1));
        rules.push_back(std::make_unique<Rule>(Rule::Type::BOND, 'b', 'c', 1, 0, 2, 2));
        rules.push_back(std::make_unique<Rule>(Rule::Type::BOND, 'c', 'd', 2, 0, 3, 3));
        rules.push_back(std::make_unique<Rule>(Rule::Type::BOND, 'd', 'e', 3, 0, 4, 4));
        rules.push_back(std::make_unique<Rule>(Rule::Type::BOND, 'e', 'f', 4, 0, 5, 5));
        
        
   }
               
    void update() {
        int width, height;
        SDL_GetWindowSize(window, &width,&height);

        // TODO: expensive pairwise search
        for (int i=0;i<atoms.size();++i) {
            for (int j=i+1;j<atoms.size();++j) {
                auto& atom1 = atoms[i];
                auto& atom2 = atoms[j];
                auto dist = sqrt(pow(atom1->x-atom2->x,2) + pow(atom1->y-atom2->y,2));
                if (dist < bonding_radius) {
                    // TODO: expensive trying all rules                        
                    for (auto& rule: rules) {
                        try_rule(*rule, atom1, atom2);
                        try_rule(*rule, atom2, atom1);
                    }
                }
            }
        }
        for (auto& link: links) {
            link->update();
        }
        for (auto& atom: atoms) {
            atom->update(width,height);
        }
    }

    void draw() {
    
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        
        imgui_start_frame();
    
        for (auto& atom: atoms) {
                atom_renderer->draw(*atom);
        }

        SDL_SetRenderDrawColor(renderer, 255,55,255,255);
        for (auto& link: links) {
                link->draw(*renderer);
        }

        SDL_RenderPresent(renderer);
        
        imgui_render_frame();

        SDL_GL_SwapWindow(window);
        
    }
    
private:
    bool try_rule(const Rule& rule, std::shared_ptr<Atom>& atom1, std::shared_ptr<Atom>& atom2) {

        if (rule.match(*atom1, *atom2)) {
            rule.apply_after_state(*atom1, *atom2);
            if (rule.type == Rule::Type::BOND) {
                links.push_back(std::make_shared<Link>(atom1, atom2));
            }
            else if (rule.type == Rule::Type::BREAK) {
                // TODO: expensive iterating over links
                auto it = std::remove_if(links.begin(), links.end(), [&](const std::shared_ptr<Link>& link) {
                    return (link->atom1 == atom1 && link->atom2 == atom2) || (link->atom1 == atom2 && link->atom2 == atom1);
                });
                links.erase(it, links.end());
            }
            return true;
        }
        return false;
    }
    
    void setup_imgui() {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

        ImGui_ImplOpenGL3_Init("#version 100");
        ImGui_ImplSDL2_InitForOpenGL(window, SDL_GL_GetCurrentContext());

    }

    void imgui_start_frame() {
        // (Where your code calls SDL_PollEvent())
        SDL_Event event;
        SDL_PollEvent(&event);
        ImGui_ImplSDL2_ProcessEvent(&event); // Forward your event to backend

        // (After event loop)
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); // Show demo window! :)
    }

    void imgui_render_frame() {
        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // (Your code calls SDL_GL_SwapWindow() etc.)
    }
       
    // ----- variables ------
    const float bonding_radius = 50;

    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unique_ptr<AtomRenderer> atom_renderer;
    std::vector<std::shared_ptr<Atom>> atoms;
    std::vector<std::shared_ptr<Link>> links;
    std::vector<std::unique_ptr<Rule>> rules;
    
};


// global update function 
void update(void* app_ptr) {        // ugly void* is only was Emscripten will pass arguments
    Application* application = static_cast<Application*>(app_ptr);
    application->update();
    application->draw();
}

int main(int argc, char* argv[]) {
    
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    Application app;
   
    emscripten_set_main_loop_arg(update, &app, 
        -1, // fps=-1 recommended, determined by browser.
        1   // simulate infinite loop, this call will not return
    );
   
    // never reached?
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    TTF_Quit();
    SDL_Quit();

}
 



