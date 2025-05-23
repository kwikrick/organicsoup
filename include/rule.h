#pragma once

struct Rule 
{

    char atom_type1;
    int before_state1;
    
    bool before_bonded;
    
    char atom_type2;
    int before_state2;
    
    int after_state1;
    bool after_bonded;
    int after_state2;

    Rule(char atom_type1, int before_state1, bool before_linked,
         char atom_type2, int before_state2,
         int after_state1, bool after_linked, int after_state2)
        :atom_type1(atom_type1), before_state1(before_state1), before_bonded(before_linked),
         atom_type2(atom_type2), before_state2(before_state2),
         after_state1(after_state1), after_bonded(after_linked), after_state2(after_state2)
    {
        
    };


};