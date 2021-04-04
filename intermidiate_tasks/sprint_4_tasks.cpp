//
// Created by azakharov on 4/1/2021.
//

#include "sprint_4_tasks.h"

#include <iostream>

namespace intermediate {

using namespace std::literals;

Tower::Tower(int disks_num) {
    FillTower(disks_num);
}

int Tower::GetDisksNum() const {
    return disks_.size();
}

void Tower::SetDisks(int disks_num) {
    FillTower(disks_num);
}

void Tower::MoveTopDiscToAnotherTower(Tower& other) {
    if (disks_.empty())
        throw std::invalid_argument("Could not move disc from the empty tower"s);

    if (int top_disc = disks_.back(); other.disks_.empty() || top_disc <= other.disks_.back()) {
        other.disks_.emplace_back(top_disc);
        disks_.pop_back();
    } else
        throw std::invalid_argument("Could not move BIG disc on the LITTLE one"s);
}

void Tower::FillTower(int disks_num) {
    for (int i = disks_num; i > 0; i--)
        disks_.push_back(i);
}

void MoveDisc(Tower& rod_from, Tower& rod_to) {
    try {
        rod_from.MoveTopDiscToAnotherTower(rod_to);
    } catch (std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
    }
}

void Hanoi(int discs_left_count, Tower& rod_from, Tower& rod_to, Tower& rod_aux) {
    if (discs_left_count == 1) {
        MoveDisc(rod_from, rod_to);
        return;
    }
    Hanoi(discs_left_count - 1, rod_from, rod_aux, rod_to);
    MoveDisc(rod_from, rod_to);
    Hanoi(discs_left_count - 1, rod_aux, rod_to, rod_from);
}

void SolveHanoi(std::vector<Tower>& towers) {
    int disks_num = towers[0].GetDisksNum();
    Hanoi(disks_num, towers[0], towers[2], towers[1]);
}
}  // namespace intermediate