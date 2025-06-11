#pragma once

#include <map>
#include <format>

#include "atom.h"
#include "physicsparameters.h"

struct BondRule 
{
    char atom_type1;
    int before_state1;
    
    char atom_type2;
    int before_state2;
    
    int after_state1;
    int after_state2;

    BondDistance bond_distance;

    BondRule(char atom_type1, int before_state1, char atom_type2, int before_state2, BondDistance distance)
        :atom_type1(atom_type1), before_state1(before_state1), 
         atom_type2(atom_type2), before_state2(before_state2),
         after_state1(after_state1), after_state2(after_state2), bond_distance(distance)
    {
        
    };

    std::string toText() const 
    {
        static const std::map<const BondDistance, std::string> map = { 
            {BondDistance::Near, "near"},
            {BondDistance::Middle, "middle"},
            {BondDistance::Far, "far"},
        };

        std::string bond_distance_string = map.at(bond_distance);
        return std::format("{}{}{}{}->{}",
            atom_type1,
            before_state1,
            atom_type2,
            before_state2,
            bond_distance_string
        );
    };

    bool match(const std::shared_ptr<const Atom>& atom1, const std::shared_ptr<const Atom>& atom2) const
    {
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

        if (atom1->state != before_state1 || atom2->state != before_state2) return false;
        
        return true;
    };
    
};