#pragma once

// Utility functions

float randf(float min, float max) {

  return ((float)rand() * (max-min) / (float)RAND_MAX) + min;

}



// util

template<typename T> std::vector<T> randomize(std::vector<T> in) {
    // note: input by value, makes a copy, we can safely erase from
    std::vector<T> out;
    while( !in.empty()) {
        int i = rand() % in.size();
        out.push_back(in[i]);       // copy
        in.erase(in.begin() + i);
    }
    return out;
}
