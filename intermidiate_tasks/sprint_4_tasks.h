//
// Created by azakharov on 4/1/2021.
//

#pragma once

#include <vector>
#include <algorithm>

namespace intermediate {

class Tower {
   public:  // Constructors
    explicit Tower(int disks_num);

   public: // Methods
    [[nodiscard]] int GetDisksNum() const;

    void SetDisks(int disks_num);

    void MoveTopDiscToAnotherTower(Tower& other);

   private: // Methods
    void FillTower(int disks_num);

   private: // Fields
    std::vector<int> disks_;
};

void MoveDisc(Tower& rod_from, Tower& rod_to);

void Hanoi(int discs_left_count, Tower& rod_from, Tower& rod_to, Tower& rod_aux);

void SolveHanoi(std::vector<Tower>& towers);

template <typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end){
    if (std::distance(range_begin, range_end) < 2u)
        return;

    RandomIt middle = range_begin.begin();
    std::advance(middle, std::distance(range_begin, range_end) / 2);

    MergeSort(range_begin, middle);
    MergeSort(middle, range_begin);

    std::inplace_merge(range_begin, middle, range_end);
}


};  // namespace intermediate
