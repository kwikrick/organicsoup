#pragma once

struct Rule 
{

    enum class Type {BOND, BREAK};
    
    Type type;  
    char atom_type1;
    char atom_type2;
    int before_state1;
    int before_state2;
    int after_state1;
    int after_state2;

    Rule(Type rule_type, char before_type1, char before_type2, int before_state1, int before_state2, int after_state1, int after_state2)
        :type(rule_type), atom_type1(before_type1), atom_type2(before_type2), before_state1(before_state1), before_state2(before_state2), after_state1(after_state1), after_state2(after_state2)
    {
    };

    bool match(const Atom& atom1, const Atom& atom2) const
    {
        if (atom1.type != atom_type1 || atom2.type != atom_type2) return false;
        if (atom1.state != before_state1 || atom2.state != before_state2) return false;
        return true;
    };
    
    void apply_after_state(Atom& atom1, Atom& atom2) const
    {
        atom1.state = after_state1;
        atom2.state = after_state2;
    };
};