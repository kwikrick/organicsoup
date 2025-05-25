// Organic Builder Clone
// by kwikrick

// Define WEBAPP when compiling a web applicaion with Emscripten (cpp -> html+js+wasm)
// Set from the Makefile with with a preprocessor flag, e.g. -DWEBAPP
// #define WEBAPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// C++ includes
#include <stdlib.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

// my includes
#include "atom.h"
#include "atomrenderer.h"
#include "bond.h"
#include "rule.h"
#include "spacemap.h"
#include "physicsparameters.h"

// Dear ImGui
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"


class Application {
public:
    Application() {
        
        //  fixed size for now
        const int window_width = 1600;
        const int window_height = 900;
        params.space_width = window_width;
        params.space_height = window_height;
        
        SDL_CreateWindowAndRenderer(window_width, window_height, SDL_WINDOW_RESIZABLE, &window, &renderer);
        SDL_SetWindowTitle(window, "Organic Soup");
  
        imgui_setup();

        atom_renderer = std::make_unique<AtomRenderer>(*renderer);
        spacemap = std::make_unique<SpaceMap>(window_width, window_height, params.atom_radius*2, params.atom_radius*2);

        // rules.push_back(std::make_unique<Rule>('a', 0, false, 'b', 0, 1, true, 0));
        // rules.push_back(std::make_unique<Rule>('b', 0, false, 'c', 0, 1, true, 0));
        // rules.push_back(std::make_unique<Rule>('c', 0, false, 'd', 0, 1, true, 0));
        // rules.push_back(std::make_unique<Rule>('d', 0, false, 'e', 0, 1, true, 0));
        // rules.push_back(std::make_unique<Rule>('e', 0, false, 'f', 0, 1, true, 0));
         
        restart();

    }

    void frame() {
        handle_events();
        if (!paused) {
            for (int i=0;i<iterations_per_frame;++i) {
                update();
            }
        } else { 
            debug_update_duration = 0;
        }
        draw();
        SDL_Delay(frame_delay);
    }

    void handle_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            ImGui_ImplSDL2_ProcessEvent(&event); 
        }
    }

    bool quit_requested() const {
        return quit;
    }
               
    void update() {

        auto clock_start = std::chrono::high_resolution_clock::now();

        debug_num_pairs_tested = 0;
        debug_num_rules_tested = 0;
        debug_num_rules_applied = 0;

        auto pairs = spacemap->get_pairs(params.bonding_start_radius);
        for (auto& pair: pairs) {
            debug_num_pairs_tested++;
            auto& atom1 = pair.first;
            auto& atom2 = pair.second;
            for (auto& rule: rules) {
                debug_num_rules_tested ++;
                if (match_rule(*rule, atom1, atom2)) {
                    apply_rule(*rule, atom1, atom2);
                    debug_num_rules_applied++;
                
                }
                else if (match_rule(*rule, atom2, atom1)) {
                    apply_rule(*rule, atom2, atom1);
                    debug_num_rules_applied++;
                }
            }
        }

        // break bonds
        auto break_bonds = std::remove_if(bonds.begin(), bonds.end(), [&](const std::shared_ptr<Bond>& bond) {
            float dx = bond->atom2->x - bond->atom1->x;
            float dy = bond->atom2->y - bond->atom1->y;
            float dist = sqrt(dx*dx + dy*dy);
            return dist > params.bonding_end_radius;
        });
        bonds.erase(break_bonds, bonds.end());

        // collide
        for (auto& pair: pairs) {
            auto& atom1 = pair.first;
            auto& atom2 = pair.second;
            atom1->collide(*atom2,params);      // Bug: collide can move atom; spacemap is not updated
        }

        // enfore bonds
        for (auto& bond: bonds) {
            bond->update(params);
        }

        // move atoms
        for (auto& atom: atoms) {
            atom->update(params);
            spacemap->update_atom(atom);
        }

        // log time
        auto clock_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = clock_end - clock_start;
        debug_update_duration = duration.count();
    }

    void draw() {
    
        auto clock_start = std::chrono::high_resolution_clock::now();

        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        
        imgui_start_frame();
    
        for (auto& atom: atoms) {
                atom_renderer->draw(*atom);
        }

        SDL_SetRenderDrawColor(renderer, 255,55,255,255);
        for (auto& bond: bonds) {
                bond->draw(*renderer);
        }

        SDL_RenderPresent(renderer);
        
        imgui_render_frame();

        // log time
        auto clock_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = clock_end - clock_start;
        debug_draw_duration = duration.count();

        // calculate FPS
        std::chrono::duration<float> frame_duration = clock_start- last_frame_clock;
        float frame_time = frame_duration.count(); 
        float fps = 1.0f / frame_time;
        debug_average_fps = debug_average_fps * 0.9f + fps * 0.1f;
        last_frame_clock = clock_start;

        
    }
    
private:
    
    bool match_rule(const Rule& rule, const std::shared_ptr<const Atom>& atom1, const std::shared_ptr<const Atom>& atom2) const
    {
        if (atom1->type != rule.atom_type1 || atom2->type != rule.atom_type2) return false;
        if (atom1->state != rule.before_state1 || atom2->state != rule.before_state2) return false;
        auto bonds_it = std::find_if(bonds.begin(), bonds.end(), [&](const std::shared_ptr<Bond>& bond) {
            return (bond->atom1 == atom1 && bond->atom2 == atom2) 
                || (bond->atom1 == atom2 && bond->atom2 == atom1);
        });
        bool bonded = (bonds_it != bonds.end());
        if (rule.before_bonded != bonded) return false;
        return true;
    };
    
    void apply_rule(const Rule& rule, std::shared_ptr<Atom>& atom1, std::shared_ptr<Atom>& atom2)
    {
        atom1->state = rule.after_state1;
        atom2->state = rule.after_state2;
        auto bonds_it = std::find_if(bonds.begin(), bonds.end(), [&](const std::shared_ptr<Bond>& bond) {
            return (bond->atom1 == atom1 && bond->atom2 == atom2) 
                || (bond->atom1 == atom2 && bond->atom2 == atom1);
        });
        bool bonded = (bonds_it != bonds.end());
        if (rule.after_bonded != bonded) {
            if (rule.after_bonded) {
                bonds.push_back(std::make_shared<Bond>(atom1, atom2));
            } else {
                bonds.erase(bonds_it);
            }
        }
    };
    
    void imgui_setup() {
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
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow(); // Show demo window! :)

        // Creates a new window.
        if (ImGui::Begin("Organic Soup")) {

            ImGui::SeparatorText("Time");
           
            ImGui::Checkbox("Pause", &paused);

            ImGui::SliderInt("Iterations per frame", &iterations_per_frame, 1, 100);

            ImGui::SeparatorText("World");

            if (ImGui::Button("Restart")) {
                restart();
            }
            ImGui::PushItemWidth(100);
            for (int color=0;color<start_atoms.size();++color) {
                std::string label = std::format("{:c}",'a' + color);
                ImGui::SliderInt(label.c_str(), &start_atoms[color], 0, 1000);
                if (color != 2 and color != 5) {
                    ImGui::SameLine();
                }
            }
            ImGui::PopItemWidth(); 

    
            ImGui::SeparatorText("Rules");
            for (auto& rule: rules) {
                ImGui::PushID(rule.get());
                ImGui::PushItemWidth(50);

                const char* atom_type_items[] = { "a","b","c","d","e","f"};
                const char* atom_state_items[] = { "0","1","2","3","4","5","6","7","8","9"}; 
                // delete button
                if (ImGui::Button("X")) {
                    rules.erase(std::remove(rules.begin(), rules.end(), rule), rules.end());
                    ImGui::PopID();
                    ImGui::PopItemWidth();
                    break;
                }
                ImGui::SameLine();
                // atom 1 type
                {
                    static int current_item_atom1 = 0;
                    current_item_atom1 = rule->atom_type1 - 'a';
                    ImGui::Combo("##type1", &current_item_atom1, atom_type_items, IM_ARRAYSIZE(atom_type_items));
                    rule->atom_type1 = 'a' + current_item_atom1;
                }
                ImGui::SameLine();
                // atom 1 before state 
                {
                    static int current_item_before_state_1 = 0;
                    current_item_before_state_1 = rule->before_state1;
                    ImGui::Combo("##before1", &current_item_before_state_1, atom_state_items, IM_ARRAYSIZE(atom_state_items));
                    rule->before_state1 = current_item_before_state_1;
                }
                ImGui::SameLine();
                {
                    static bool current_bonded_before = false;
                    current_bonded_before = rule->before_bonded;
                    ImGui::Checkbox("##bonded_before", &current_bonded_before);
                    rule->before_bonded = current_bonded_before;
                }
                ImGui::SameLine();
                // atom 2 type
                {
                    static int current_item_atom2 = 0;
                    current_item_atom2 = rule->atom_type2 - 'a';
                    ImGui::Combo("##type2", &current_item_atom2, atom_type_items, IM_ARRAYSIZE(atom_type_items));
                    rule->atom_type2 = 'a' + current_item_atom2;
                }
                ImGui::SameLine();
                // atom 2 before state 
                {
                    static int current_item_before_state_2 = 0;
                    current_item_before_state_2 = rule->before_state2;
                    ImGui::Combo("##before2", &current_item_before_state_2, atom_state_items, IM_ARRAYSIZE(atom_state_items));
                    rule->before_state2 = current_item_before_state_2;
                }
                ImGui::SameLine();
                ImGui::Text("->");
                ImGui::SameLine();
                // atom 1 after state 
                {
                    static int current_item_after_state_1 = 0;
                    current_item_after_state_1 = rule->after_state1;
                    ImGui::Combo("##after1", &current_item_after_state_1, atom_state_items, IM_ARRAYSIZE(atom_state_items));
                    rule->after_state1 = current_item_after_state_1;
                }
                ImGui::SameLine();
                {
                    static bool current_bonded_after = false;
                    current_bonded_after = rule->after_bonded;
                    ImGui::Checkbox("##bonded_after", &current_bonded_after);
                    rule->after_bonded = current_bonded_after;
                }
                ImGui::SameLine();
                // atom 2 after state 
                {
                    static int current_item_after_state_2 = 0;
                    current_item_after_state_2 = rule->after_state2;
                    ImGui::Combo("##after2", &current_item_after_state_2, atom_state_items, IM_ARRAYSIZE(atom_state_items));
                    rule->after_state2 = current_item_after_state_2;
                }
               
                ImGui::PopItemWidth();
                ImGui::PopID();
            }
        }
        if (ImGui::Button("Add Rule")) {
            add_rule();
        }
        if (ImGui::CollapsingHeader("Physics Parameters")) {
            ImGui::SliderFloat("Temperature", &params.temp, 0.0f, 1.0f);
            ImGui::SliderFloat("Friction", &params.friction, 0.0f, 1.0f);
            ImGui::SliderFloat("Collision Elasticity", &params.collision_elasticity, 0.0f, 1.0f);
            //ImGui::SliderFloat("Atom Radius", &params.atom_radius, 1.0f, 100.0f);
            ImGui::SliderFloat("Bonding Start Radius", &params.bonding_start_radius, 1.0f, 100.0f);
            ImGui::SliderFloat("Bonding End Radius", &params.bonding_end_radius, 1.0f, 100.0f);
            ImGui::SliderFloat("Bonding Length", &params.bonding_length, 1.0f, 100.0f);
            ImGui::SliderFloat("Bonding Strength", &params.bonding_strength, 0.0f, 1.0f);
        }
      
        if (ImGui::CollapsingHeader("Statistics")) {
            ImGui::LabelText("Number of atoms", "%d", (int)atoms.size());
            ImGui::LabelText("Number of bonds", "%d", (int)bonds.size());
            ImGui::LabelText("Number of pairs tested", "%d", debug_num_pairs_tested);
            ImGui::LabelText("Number of rules tested", "%d", debug_num_rules_tested);
            ImGui::LabelText("Number of rules applied", "%d", debug_num_rules_applied);
            ImGui::LabelText("Update duration (ms)", "%f", debug_update_duration * 1000);
            ImGui::LabelText("Draw duration (ms)", "%f", debug_draw_duration * 1000);
            ImGui::LabelText("Average FPS", "%f", debug_average_fps);
            ImGui::SliderInt("Frame Delay (ms)", &frame_delay, 0, 16, "%d ms");
        }
        ImGui::End();
    }

    void imgui_render_frame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #ifndef WEBAPP
            SDL_GL_SwapWindow(window);      // needed in Native, not in webApp
        #endif
    }
    
    void restart() {
        atoms.clear();
        bonds.clear();
        spacemap->clear();
        // create random atoms
        for (int color=0;color<6;++color) {
            for (int i=0;i<start_atoms[color];++i) {
                int x=rand()%1600;
                int y=rand()%900;
                char type = 'a' + color;
                atoms.push_back(std::make_shared<Atom>(x,y,type,0));
                spacemap->update_atom(atoms.back());
            }
        }
    }

    void add_rule() {
        rules.push_back(std::make_unique<Rule>('a', 0, false, 'b', 0, 0, true, 0));
    }

    // ----- variables ------

    bool quit = false;
    bool paused = false;
    int frame_delay = 16; // ms, ~60 fps
    int iterations_per_frame = 1; // how many physics iterations per frame

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // parameters 
    std::array<int,6> start_atoms = {16,16,16,16,16,16};
    PhysicsParameters params;

    // data
    std::unique_ptr<SpaceMap> spacemap;
    std::unique_ptr<AtomRenderer> atom_renderer;
    std::vector<std::shared_ptr<Atom>> atoms;
    std::vector<std::shared_ptr<Bond>> bonds;
    std::vector<std::unique_ptr<Rule>> rules;

    // Performance variables
    // TODO: rename debug->performance; or put in a struct
    int debug_num_pairs_tested = 0;
    int debug_num_rules_tested = 0;
    int debug_num_rules_applied = 0;
    float debug_draw_duration = 0;
    float debug_update_duration = 0;
    float debug_average_fps = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_frame_clock;
};

#ifdef WEBAPP
#include <emscripten/emscripten.h>

// global update function 
void update(void* app_ptr) {        // ugly void* is only was Emscripten will pass arguments
    Application* application = static_cast<Application*>(app_ptr);
    application->frame();
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
#else
int main(int argc, char* argv[]) {
    
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    Application app;
   
    while(!app.quit_requested()) {
        app.frame();
    }
   
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    TTF_Quit();
    SDL_Quit();
}
#endif
 



