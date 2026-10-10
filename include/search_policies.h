#pragma  once
#include "types.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>

enum class SearchResult : uint8_t {
  found,    // price exists, iterator points to it
  notFound, // not found, insert before the returned iterator
};

struct LinearSearch {
  template <typename It,  typename Compare>
  static std::pair<SearchResult, It> search(It first, It last, const Price price, Compare comp) {
    It insertAt = last;
    for (It it = last; it != first;) {
      --it;
      if (it->first == price) {
        return {SearchResult::found, it};
      }
      if (!comp(it->first, price)) {
        insertAt = it; // price belongs before this element
      } else
        break;
    }
    return {SearchResult::notFound, insertAt};
  }
};

struct BinarySearch {
  template <typename It,  class Compare>
  static std::pair<SearchResult, It> search(It first, It last, const Price price, Compare comp) {
    auto it = std::lower_bound(first, last, price, [&comp](const auto& level, Price value){return comp(level.first,value);});
    if (it != last && it ->first == price) {
      return {SearchResult::found, it};
    }
    else return {SearchResult::notFound, it};
  }
};

struct BranchlessBinarySearch {
  template <typename It,  class Compare>
  static std::pair<SearchResult, It> search(It first, It last, const Price price, Compare comp) {
    auto length = last - first;
    while (length > 0) {
      auto half = length / 2;
      // multiplication by boolean result from comp() to encourage GCC to generate a CMOV
      first += comp(first[half].first, price) * (length - half);
      length = half;
    }

    if (first != last && first->first == price) return {SearchResult::found, first};
    if (first == last) return {SearchResult::notFound, first};
    return {SearchResult::notFound, first};
  }
};
