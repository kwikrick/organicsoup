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
#include "bond.h"
#include "rule.h"

// Dear ImGui
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"


class Application {
public:
    Application() {
     
        SDL_CreateWindowAndRenderer(1600, 900, SDL_WINDOW_RESIZABLE, &window, &renderer);

        imgui_setup();

        atom_renderer = std::make_unique<AtomRenderer>(*renderer);
        
        int width, height;
        SDL_GetWindowSize(window, &width,&height);

        // rules.push_back(std::make_unique<Rule>('a', 0, false, 'b', 0, 1, true, 1));
        // rules.push_back(std::make_unique<Rule>('b', 0, false, 'c', 0, 1, true, 1));
        // rules.push_back(std::make_unique<Rule>('c', 0, false, 'd', 0, 1, true, 1));
        // rules.push_back(std::make_unique<Rule>('d', 0, false, 'e', 0, 1, true, 1));
        // rules.push_back(std::make_unique<Rule>('e', 0, false, 'f', 0, 1, true, 1));
         
        restart();

    }
               
    void update() {
        int width, height;
        SDL_GetWindowSize(window, &width,&height);

        //TODO: expensive pairwise search
        for (int i=0;i<atoms.size();++i) {
            for (int j=i+1;j<atoms.size();++j) {
                auto& atom1 = atoms[i];
                auto& atom2 = atoms[j];
                auto dist = sqrt(pow(atom1->x-atom2->x,2) + pow(atom1->y-atom2->y,2));
                if (dist < bonding_radius) {
                     for (auto& rule: rules) {
                        if (match_rule(*rule, atom1, atom2)) {
                            apply_rule(*rule, atom1, atom2);
                        }
                        if (match_rule(*rule, atom2, atom1)) {
                            apply_rule(*rule, atom2, atom1);
                        }
                    }
                }
            }
        }
        for (auto& bond: bonds) {
            bond->update();
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
        for (auto& bond: bonds) {
                bond->draw(*renderer);
        }

        SDL_RenderPresent(renderer);
        
        imgui_render_frame();
        
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
        auto bonds_it = std::remove_if(bonds.begin(), bonds.end(), [&](const std::shared_ptr<Bond>& bond) {
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
        // Forward event to imGui backend
        // TODO: should this be a loop? And also do my own event processing?
        SDL_Event event;
        SDL_PollEvent(&event);
        ImGui_ImplSDL2_ProcessEvent(&event); 

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow(); // Show demo window! :)

        // Creates a new window.
        if (ImGui::Begin("Test Window"))
        {
            if (ImGui::Button("Restart")) {
                restart();
            }
            ImGui::LabelText("Number of atoms", "%d", (int)atoms.size());
            ImGui::LabelText("Number of bonds", "%d", (int)bonds.size());
            ImGui::SeparatorText("Rules");
            for (auto& rule: rules) {
                ImGui::PushID(rule.get());
                ImGui::PushItemWidth(50);

                const char* atom_type_items[] = { "a","b","c","d","e","f"};
                const char* atom_state_items[] = { "0","1","2","3","4","5","6","7","8","9"}; 

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
        ImGui::End();
    }

    void imgui_render_frame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    
    void restart() {
        atoms.clear();
        bonds.clear();
        for (int i=0;i<100;++i) {
            int x=rand()%1600;
            int y=rand()%900;
            char type = 'a' + rand()%6;
            int state = 0;
            atoms.push_back(std::make_shared<Atom>(x,y,type,state));
        }
    }

    void add_rule() {
        rules.push_back(std::make_unique<Rule>('a', 0, false, 'b', 0, 0, true, 0));
    }
    // ----- variables ------
    const float bonding_radius = 50;

    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unique_ptr<AtomRenderer> atom_renderer;
    std::vector<std::shared_ptr<Atom>> atoms;
    std::vector<std::shared_ptr<Bond>> bonds;
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
 



