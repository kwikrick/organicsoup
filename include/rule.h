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

    std::string toText() const {
        return std::format("{}{}{}{}{}->{}{}{}{}{}",
            atom_type1,
            before_state1,
            before_bonded?"":"+",
            atom_type2,
            before_state2,
            atom_type1,
            after_state1,
            after_bonded?"":"+",
            atom_type2,
            after_state2
        );
    }

    bool match(const std::shared_ptr<const Atom>& atom1, const std::shared_ptr<const Atom>& atom2, bool bonded) const
    {
        if (bonded != before_bonded) return false;
        if (atom1->state != before_state1 || atom2->state != before_state2) return false;

        char match_x = 0;
        char match_y = 0;

        if (atom_type1 == 'X') {
            if (match_x == 0 || match_x == atom1->type)
                match_x = atom1->type;
            else {
                return false;
            }
        }
        else if (atom_type1 == 'Y') {
            if (match_y == 0 || match_y == atom1->type)
                match_y = atom1->type;
            else {
                return false;
            }
        }
        else {
             if (atom1->type != atom_type1) return false;
        }
        if (atom_type2 == 'X') {
            if (match_x == 0 || match_x == atom2->type)
                match_x = atom2->type;
            else {
                return false;
            }
        }
        else if (atom_type2 == 'Y') {
            if (match_y == 0 || match_y == atom2->type)
                match_y = atom2->type;
            else {
                return false;
            }
        }
        else {
              if (atom2->type != atom_type2) return false;
        }

        return true;
    };
    
};