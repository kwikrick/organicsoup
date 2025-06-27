#pragma once

#include <tuple>

struct Charge {

    char type;
    int state;
    float charge;

    Charge(char type, int state, float charge):type(type),state(state),charge(charge) {};

    bool operator<(const Charge& other) const {return std::tie(type,state) < std::tie(other.type,other.state); }

    std::string to_text() const {
        return std::format("{}{}:{}",type,state,charge);
    }
};

