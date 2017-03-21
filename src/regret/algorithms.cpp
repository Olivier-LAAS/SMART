/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RoutingManager.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 5 Apr 2015
 * @brief
 *
 */
#include <algorithm>

template <typename Iterator>
/**
 * @brief Utility function used by function getAll_K_Permutations()
 * @param first
 * @param k
 * @param last
 * @return
 */
inline bool next_combination(const Iterator first, Iterator k, const Iterator last)
{
    if ((first == last) || (first == k) || (last == k))
        return false;
    Iterator itr1 = first;
    Iterator itr2 = last;
    ++itr1;
    if (last == itr1)
        return false;
    itr1 = last;
    --itr1;
    itr1 = k;
    --itr2;
    while (first != itr1)
    {
        if (*--itr1 < *itr2)
        {
            Iterator j = k;
            while (!(*itr1 < *j)) ++j;
            std::iter_swap(itr1,j);
            ++itr1;
            ++j;
            itr2 = k;
            std::rotate(itr1,j,last);
            while (last != j)
            {
                ++j;
                ++itr2;
            }
            std::rotate(k,itr2,last);
            return true;
        }
    }
    std::rotate(first,k,last);
    return false;
}


/**
 * @brief Find all the permutations of size k, given a set of value
 * @param possibilities : the set of possible values
 * @param k : size of permutation
 * @return vector containing all the permutations
 */
template <typename Type>
std::vector<std::vector < Type> > getAll_K_Permutations(const std::vector<Type> & possibilities, size_t k) {
    std::vector<Type> tmp = possibilities;
    std::vector<std::vector <Type> > res;

    k = std::min(k, possibilities.size());

    std::sort(tmp.begin(), tmp.end()); // tri avant generation des permutations (indispensable)
    // generation de toutes les permutations
    do {
        std::vector<Type> b;
        for(size_t i = 0; i < k; i++)
            b.push_back(tmp[i]);
        do {
            res.push_back(b);
        } while(std::next_permutation(b.begin(), b.begin() + k));
    } while(next_combination(tmp.begin(), tmp.begin() + k, tmp.end()));

    return res;
}
