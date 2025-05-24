#pragma once

#include <vector>
#include <memory>

#include "atom.h"

struct SpaceMap
{
    // types
    using Cell = std::vector<std::shared_ptr<Atom>>;
    
    // vars
    std::vector<Cell> cells;
    float xsize = 0;
    float ysize = 0; 
    float xstep = 1;
    float ystep = 1;
    int nx = 0;
    int ny = 0;

    SpaceMap(float xsize, float ysize, float xstep, float ystep)
        :xsize(xsize), ysize(ysize), xstep(xstep), ystep(ystep)
    {
        nx = static_cast<int>(xsize / xstep);
        ny = static_cast<int>(ysize / ystep);
        cells.resize(nx * ny);
    }

    int position_to_index(float x, float y) const {
        int ix = static_cast<int>(x / xstep);
        int iy = static_cast<int>(y / ystep);
        return grid_coord_to_index(ix, iy);
    }

    int grid_coord_to_index(int ix, int iy) const {
        if (ix < 0 || ix >= nx || iy < 0 || iy >= ny) {
            return -1;
        }
        return iy*nx + ix;
    }

    void add_atom(std::shared_ptr<Atom> atom) {
        int i = position_to_index(atom->x, atom->y);
        if (i < 0) return;
        auto& cell = cells[i];
        cell.emplace_back(atom);
    }

    void remove_atom_old(std::shared_ptr<Atom> atom, float old_x, float old_y) {    
        int i = position_to_index(old_x, old_y);
        if (i < 0) return;
        auto& cell = cells[i];
        cell.erase(std::remove(cell.begin(), cell.end(), atom), cell.end());
    }

    void update_atom(std::shared_ptr<Atom> atom, float old_x, float old_y) {
        int old_i = position_to_index(old_x, old_y);
        int new_i = position_to_index(atom->x, atom->y);
        if (old_i != new_i) {
            remove_atom_old(atom, old_x, old_y);
            add_atom(atom);
        }
    }
   
    void clear() {
        for (auto& cell : cells) {
            cell.clear();
        }
    }

    using AtomPair = std::pair<std::shared_ptr<Atom>, std::shared_ptr<Atom>>;
    std::vector<AtomPair> get_pairs(float radius) const {
        std::vector<AtomPair> pairs;
        int rx = static_cast<int>(radius / xstep)+1;
        int ry = static_cast<int>(radius / ystep)+1;
        for (int ix1 = 0; ix1 < nx; ++ix1) {
            for (int iy1 = 0; iy1 < ny; ++iy1) {
                int index1 = grid_coord_to_index(ix1, iy1);
                if (cells[index1].empty()) continue;
                auto cell1 = cells[index1];
                for (int ix2 = ix1; ix2 <= ix1+rx; ++ix2) {
                    if (ix2 >= nx) break;
                    for (int iy2 = iy1; iy2 <= iy1+ry; ++iy2) {
                        if (iy2 >= ny) break;
                        int index2 = grid_coord_to_index(ix2, iy2);
                        auto cell2 = cells[index2];
                        if (cell2.empty()) continue;
                        for (auto& atom1 : cell1) {
                            for (auto& atom2 : cell2) {
                                if (atom1 != atom2) {
                                    float dx = atom1->x - atom2->x;
                                    float dy = atom1->y - atom2->y;
                                    if (dx * dx + dy * dy < radius * radius) {
                                        pairs.push_back(std::make_pair(atom1,atom2));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return pairs;
    }
};
