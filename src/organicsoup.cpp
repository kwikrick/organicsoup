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
#include <unordered_map>
#include <set>

// my includes
#include "atom.h"
#include "atomrenderer.h"
#include "bond.h"
#include "rule.h"
#include "spacemap.h"
#include "physicsparameters.h"
#include "charge.h"



// Dear ImGui
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

using AtomPair = std::pair<const Atom*, const Atom*>;
template<>
struct std::hash<AtomPair>
{
    std::size_t operator()(const AtomPair& pair) const noexcept
    {
        std::size_t h1 = (std::size_t)pair.first;
        std::size_t h2 = (std::size_t)pair.second;
        return h1 ^ (h2 << 1);
    }
};

class Application {
public:


    Application() {
        
        //  fixed size for now
        const int window_width = 1600;
        const int window_height = 900;
        params.space_width = window_width;
        params.space_height = window_height;
        
        SDL_CreateWindowAndRenderer(window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL, &window, &renderer);

        SDL_SetWindowTitle(window, "Organic Soup");
  
        imgui_setup();

        atom_renderer = std::make_unique<AtomRenderer>(*renderer);

        // rules.push_back(std::make_unique<Rule>('a', 0, false, 'b', 0, 1, true, 0));
        // rules.push_back(std::make_unique<Rule>('b', 0, false, 'c', 0, 1, true, 0));
        // rules.push_back(std::make_unique<Rule>('c', 0, false, 'd', 0, 1, true, 0));
        // rules.push_back(std::make_unique<Rule>('d', 0, false, 'e', 0, 1, true, 0));
        // rules.push_back(std::make_unique<Rule>('e', 0, false, 'f', 0, 1, true, 0));
         
        restart();

    }

    void frame() {

        auto clock_start = std::chrono::high_resolution_clock::now();

        handle_events();
        if (!paused) {
            update_iterative();
        }
        draw();

        auto clock_finished = std::chrono::high_resolution_clock::now();


        // delay
        std::chrono::duration<float> busy_duration = clock_finished - clock_start;
        float busy_ms = busy_duration.count() * 1000; 
        if (busy_ms < minimum_frame_time_ms) {
            SDL_Delay(minimum_frame_time_ms - busy_ms);
        }

        // calculate FPS
        std::chrono::duration<float> frame_duration = clock_start - last_frame_clock;
        last_frame_clock = clock_start;
        float frame_time = frame_duration.count(); 
        float fps = 1.0f / frame_time;
        debug_average_fps = debug_average_fps * 0.9f + fps * 0.1f;
    }

    bool quit_requested() const {
        return quit;
    }

private:
    

    void handle_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
           
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            ImGui_ImplSDL2_ProcessEvent(&event);
            auto io = ImGui::GetIO();
            if (!io.WantCaptureMouse)
            {
                float old_scale = scale;
                if (event.type == SDL_MOUSEWHEEL) {
                    if (event.wheel.y > 0) {
                        scale *= 1.1f;
                    } else if (event.wheel.y < 0) {
                        scale /= 1.1f;
                    }
                    int mouse_x, mouse_y;
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    offset_x -= (mouse_x - offset_x) * (scale / old_scale - 1);
                    offset_y -= (mouse_y - offset_y) * (scale / old_scale - 1);
                }
                if (event.type == SDL_MOUSEMOTION) {
                    if (event.motion.state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
                        offset_x += event.motion.xrel;
                        offset_y += event.motion.yrel;
                    }
                }
            }
        }
     }

   
    void update_iterative() {
        auto clock_start = std::chrono::high_resolution_clock::now();

        for (int i=0;i<iterations_per_frame;++i) {
            update();
        }
        
        auto clock_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = clock_end - clock_start;
        debug_update_duration = duration.count();
    }
               
    void update() {
        debug_num_pairs_tested = 0;
        debug_num_rules_tested = 0;
        debug_num_rules_applied = 0;

        float pair_distance = params.atom_radius*2;
        pair_distance = fmax(pair_distance, params.bonding_start_distance);
        pair_distance = fmax(pair_distance, params.charge_distance);
        auto pairs = spacemap->get_pairs(pair_distance);

        // try rules 
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

        using AtomPairBondPair = std::pair<const AtomPair,std::shared_ptr<Bond>>;
        auto broken = [&](AtomPairBondPair& item) {
            auto& bond = item.second;
            float dx = bond->atom2->x - bond->atom1->x;
            float dy = bond->atom2->y - bond->atom1->y;
            float dist = sqrt(dx*dx + dy*dy);
            return dist > params.bonding_end_distance;
        };
        std::vector<AtomPair> to_remove;
        for (auto item: atompair2bond) {
            if (broken(item)) {
                item.second->atom1->state = 0;
                item.second->atom2->state = 0;
                to_remove.push_back(item.first);
            }
        };
        for (auto& pair: to_remove) {
            atompair2bond.erase(pair);
        }

        // enfore bonds
        for (auto& item: atompair2bond) {
            item.second->update();
        }
        
        // attract/repulse and collide
        for (auto& pair: pairs) {
            auto& atom1 = pair.first;
            auto& atom2 = pair.second;
            atom1->attract(*atom2, charges);
            atom1->collide(*atom2);
        }
        
        // move atoms
        for (auto& atom: atoms) {
            atom->update();
            spacemap->update_atom(atom);
        }
    }

    void draw() {
    
        auto clock_start = std::chrono::high_resolution_clock::now();

        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        
        imgui_start_frame();
    
        draw_world();
        
        SDL_RenderFlush(renderer);
        
        imgui_render_frame();

        // log time
        auto clock_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> duration = clock_end - clock_start;
        debug_draw_duration = duration.count();

    }
    
    void draw_world() {

        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        SDL_SetRenderDrawColor(renderer, 64,64,64,255);
        SDL_FRect window_rect = {0, 0, (float)window_width, (float)window_height};
        SDL_RenderFillRectF(renderer, &window_rect);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_FRect space_rect = {offset_x, offset_y, params.space_width*scale, params.space_height*scale};
        SDL_RenderFillRectF(renderer, &space_rect);

        for (auto& atom: atoms) {
                atom_renderer->draw(*atom, scale, offset_x, offset_y);
        }

        for (auto& item: atompair2bond) {
                item.second->draw(*renderer, scale, offset_x, offset_y);
        }

    }

    bool match_rule(const Rule& rule, const std::shared_ptr<const Atom>& atom1, const std::shared_ptr<const Atom>& atom2)
    {
        bool bonded = atompair2bond.contains(make_atom_pair(atom1.get(),atom2.get()));
        return rule.match(atom1, atom2, bonded);
    };
    
    void apply_rule(const Rule& rule, std::shared_ptr<Atom>& atom1, std::shared_ptr<Atom>& atom2)
    {
        atom1->state = rule.after_state1;
        atom2->state = rule.after_state2;
        bool bonded = atompair2bond.contains(make_atom_pair(atom1.get(),atom2.get()));
        if (rule.after_bonded != bonded) {
            if (rule.after_bonded) {
                if (atom1->num_bonds >= params.max_bonds_per_atom) return;
                if (atom2->num_bonds >= params.max_bonds_per_atom) return;
                add_bond(atom1, atom2);
            } else {
                // 
                atompair2bond.erase(make_atom_pair(atom1.get(),atom2.get()));
            }
        }
    };
    
    AtomPair make_atom_pair(const Atom* atom1, const Atom* atom2)
    {
        const Atom* left = (atom1<atom2)?atom1:atom2;
        const Atom* right = (atom1<atom2)?atom2:atom1;
        return AtomPair(left,right);
    }

    std::shared_ptr<Bond> add_bond( std::shared_ptr<Atom>& atom1, std::shared_ptr<Atom>& atom2) 
    {
        auto pair = make_atom_pair(atom1.get(),atom2.get());
        auto bond = std::make_shared<Bond>(params,atom1,atom2);        
        atompair2bond[pair]=bond;
        return bond;
    }

    void imgui_setup() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

        ImGui_ImplSDL2_InitForOpenGL(window, SDL_GL_GetCurrentContext());
        ImGui_ImplOpenGL3_Init();
        
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

            int old_width = params.space_width;
            int old_height = params.space_height;
            
            ImGui::SetNextItemWidth(100);    
            ImGui::InputFloat("width", &params.space_width, 100, 1000.0f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);    
            ImGui::InputFloat("height", &params.space_height, 100, 1000.0f);

            if (old_width != params.space_width || old_height != params.space_height) {
                resize();
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

            ImGui::SeparatorText("Bonding Rules");

            static const char* atom_type_items[] = { "a","b","c","d","e","f", "X", "Y"};
            static const char* atom_state_items[] = { "0","1","2","3","4","5","6","7","8","9"}; 
            auto atom_type_from_index = [&](int index)->char {return atom_type_items[index][0];};
            auto atom_type_to_index = [&](char type)->int {
                 for (int i=0;i<IM_ARRAYSIZE(atom_type_items);++i) {
                    if (atom_type_items[i][0]==type) return i;
                 }
                 return 0;
            };

            // new rule
            ImGui::PushItemWidth(50);
            static int atom_type1 = 0;
            ImGui::Combo("##type1", &atom_type1, atom_type_items, IM_ARRAYSIZE(atom_type_items));     
            ImGui::SameLine();
            static int before_state_1 = 0;
            ImGui::Combo("##before1", &before_state_1, atom_state_items, IM_ARRAYSIZE(atom_state_items));
            ImGui::SameLine();
            static bool bonded_before = false;
            ImGui::Checkbox("##bonded_before", &bonded_before);
            ImGui::SameLine();
            static int atom_type2 = 0;
            ImGui::Combo("##type2", &atom_type2, atom_type_items, IM_ARRAYSIZE(atom_type_items));    
            ImGui::SameLine();      
            static int before_state_2 = 0;
            ImGui::Combo("##before2", &before_state_2, atom_state_items, IM_ARRAYSIZE(atom_state_items));
            ImGui::SameLine(); 
            ImGui::Text("->");
            ImGui::SameLine();  
            static int after_state_1 = 0;
            ImGui::Combo("##after1", &after_state_1, atom_state_items, IM_ARRAYSIZE(atom_state_items));
            ImGui::SameLine();
            static bool bonded_after = true;
            ImGui::Checkbox("##bonded_after", &bonded_after);
            ImGui::SameLine();
            static int after_state_2 = 0;
            ImGui::Combo("##after2", &after_state_2, atom_state_items, IM_ARRAYSIZE(atom_state_items));
            ImGui::PopItemWidth();


            if (ImGui::Button("Add Rule")) {
                rules.push_back(std::make_unique<Rule>(atom_type_from_index(atom_type1), before_state_1, bonded_before,
                                                       atom_type_from_index(atom_type2), before_state_2, 
                                                       after_state_1, bonded_after, after_state_2));
            }

            for (auto& rule: rules) {
                ImGui::PushID(rule.get());
                ImGui::PushItemWidth(50);

                // delete button
                if (ImGui::Button("X")) {
                    atom_type1 = atom_type_to_index(rule->atom_type1);
                    atom_type2 = atom_type_to_index(rule->atom_type2);
                    before_state_1 = rule->before_state1;
                    before_state_2 = rule->before_state2;
                    after_state_1 = rule->after_state1;
                    after_state_2 = rule->after_state2;
                    bonded_before = rule->before_bonded;
                    bonded_after = rule->after_bonded;
                    rules.erase(std::remove(rules.begin(), rules.end(), rule), rules.end());
                    ImGui::PopID();
                    ImGui::PopItemWidth();
                    break;
                }
                ImGui::SameLine();
                ImGui::Text("%s",rule->toText().c_str());                  
                ImGui::PopItemWidth();
                ImGui::PopID();
            }

            ImGui::SeparatorText("Charges");

            // for (int atom_number = 0; atom_number<params.num_atom_types;atom_number++) {
            //     char atom_type = atom_number + 'a';
            //     ImGui::PushID(atom_type);
            //     ImGui::PushItemWidth(100);
            //     int value = params.atom_charges[atom_number];
            //     ImGui::InputInt(std::format("charge {}",atom_type).c_str(), &value);
            //     params.atom_charges[atom_number] = value;
            //     ImGui::PopID();
            //     if (atom_number != 2 && atom_number != 5) {
            //         ImGui::SameLine();
            //     }
            // }

             // new rule
            static int charge_type_number = 0;
            ImGui::SetNextItemWidth(50);
            ImGui::Combo("##charge_type", &charge_type_number, atom_type_items, IM_ARRAYSIZE(atom_type_items));     
            ImGui::SameLine();
            char charge_type = charge_type_number + 'a';
            static int charge_state = 0;
            ImGui::SetNextItemWidth(50);
            ImGui::Combo("##charge_state", &charge_state, atom_state_items, IM_ARRAYSIZE(atom_state_items));
            ImGui::SameLine(); 
            ImGui::Text(":");
            ImGui::SameLine();  
            static int charge_value = 0;
            ImGui::SetNextItemWidth(100);
            ImGui::InputInt("##charge_value", &charge_value);
            
            if (ImGui::Button("Add Charge")) {
                auto new_charge = Charge(charge_type, charge_state, (float)charge_value);
                if (charges.contains(new_charge)) charges.erase(new_charge);
                charges.insert(new_charge);
            }

            int i=0;
            for (const auto& charge: charges) {
                ImGui::PushID(&charge);
                ImGui::PushItemWidth(50);

                // delete button
                if (ImGui::Button("X")) {
                    charges.erase(charge);
                    ImGui::PopID();
                    ImGui::PopItemWidth();
                    break;
                }
                ImGui::SameLine();
                ImGui::Text("%s",charge.to_text().c_str());                  
                ImGui::PopItemWidth();
                ImGui::PopID();
            }


            if (ImGui::CollapsingHeader("Physics Parameters")) {
                ImGui::SliderFloat("Temperature", &params.temp, 0.0f, 1.0f);
                ImGui::SliderFloat("Friction", &params.friction, 0.0f, 1.0f);
                ImGui::SliderFloat("Collision Elasticity", &params.collision_elasticity, 0.0f, 1.0f);
                //ImGui::SliderFloat("Atom Radius", &params.atom_radius, 1.0f, 100.0f);
                ImGui::SliderFloat("Charge Distance", &params.charge_distance, 1.0f, 128.0f);
                ImGui::SliderFloat("Charge Strength", &params.charge_strength, 0.0f, 1.0f);
                ImGui::SliderFloat("Bonding Distance", &params.bonding_distance, 1.0f, 128.0f);
                ImGui::SliderFloat("Bonding Start Distance", &params.bonding_start_distance, 1.0f, 128.0f);
                ImGui::SliderFloat("Bonding End Distance", &params.bonding_end_distance, 1.0f, 128.0f);
                ImGui::SliderFloat("Bonding Strength", &params.bonding_strength, 0.0f, 1.0f);
                ImGui::SliderInt("Max bonds per atom", &params.max_bonds_per_atom, 0,16);
                
            }
        
            if (ImGui::CollapsingHeader("Statistics")) {
                ImGui::LabelText("Number of atoms", "%d", (int)atoms.size());
                ImGui::LabelText("Number of bonds", "%d", (int)atompair2bond.size());
                ImGui::LabelText("Number of pairs tested", "%d", debug_num_pairs_tested);
                ImGui::LabelText("Number of rules tested", "%d", debug_num_rules_tested);
                ImGui::LabelText("Number of rules applied", "%d", debug_num_rules_applied);
                ImGui::LabelText("Update duration (ms)", "%f", debug_update_duration * 1000);
                ImGui::LabelText("Draw duration (ms)", "%f", debug_draw_duration * 1000);
                ImGui::LabelText("Average FPS", "%f", debug_average_fps);
                ImGui::SliderInt("Minimum Frame Time (ms)", &minimum_frame_time_ms, 0, 16, "%d ms");
            }
        }
        ImGui::End();
    }

    void imgui_render_frame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    void resize() {
        spacemap = std::make_unique<SpaceMap>(params.space_width, params.space_height, params.atom_radius*2, params.atom_radius*2);
        std::vector<AtomPair> to_remove;

        for (auto& item: atompair2bond) {
            auto& bond = item.second;
            if (bond->atom1->off_world() || bond->atom2->off_world()) {
                to_remove.push_back(item.first);        
            }
        }
        for (auto& pair: to_remove) {
            atompair2bond.erase(pair);
        }
        
        auto off_world = [&](std::shared_ptr<Atom> atom){return atom->off_world();};
            
        atoms.erase(std::remove_if(atoms.begin(),atoms.end(),off_world),atoms.end());
        for (auto& atom: atoms) {
            atom->spacemap_index = -1;
            spacemap->update_atom(atom);
        }
    }
    
    void restart() {
        atoms.clear();
        atompair2bond.clear();

        // new spacemap for atom size and world size
        spacemap = std::make_unique<SpaceMap>(params.space_width, params.space_height, params.atom_radius*2, params.atom_radius*2);
        // create random atoms
        for (int color=0;color<6;++color) {
            for (int i=0;i<start_atoms[color];++i) {
                float x=randf(0,params.space_width);
                float y=randf(0,params.space_height);
                char type = 'a' + color;
                atoms.push_back(std::make_shared<Atom>(params,x,y,type,0));
                spacemap->update_atom(atoms.back());
            }
        }
    }

    // ----- variables ------

    bool quit = false;
    bool paused = false;
    int minimum_frame_time_ms = 16; // ms, ~60 fps
    int iterations_per_frame = 1; // how many physics iterations per frame

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    float scale = 1.0f;
    float offset_x = 0.0f;
    float offset_y = 0.0f;

    // parameters 
    std::array<int,6> start_atoms = {16,16,16,16,16,16};
    PhysicsParameters params;

    // data
    std::unique_ptr<SpaceMap> spacemap;
    std::unique_ptr<AtomRenderer> atom_renderer;
    std::vector<std::shared_ptr<Atom>> atoms;
    //std::vector<std::shared_ptr<Bond>> bonds;
    std::vector<std::unique_ptr<Rule>> rules;
    std::set<Charge> charges;

    // TODO: instead of this map, we could use an unordered set of bonds with a proper hash and compare for bonds...
    std::unordered_map<AtomPair,std::shared_ptr<Bond>> atompair2bond;

    
   
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
 



