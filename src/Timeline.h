/**
 * Simulation_p.h
 * Simulation private interface with instances
 */

#ifndef TIMELINE_H_INCLUDED
#define TIMELINE_H_INCLUDED

#include <memory>
#include <utility>
#include <iterator>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <queue>

namespace sim {

namespace heap_util {

template <typename Distance>
inline Distance heap_parent_index( Distance idx ) {
  return ( idx - 1 ) / 2;
}

template <typename RAIter, typename Distance, typename Tp>
void heap_remove_impl( RAIter first, Distance hole_idx, Tp value ) {
  Distance parent_idx = heap_parent_index( hole_idx );
  while ( hole_idx > 0 ) {
    *(first + hole_idx) = std::move( *(first + parent_idx) );
    hole_idx = parent_idx;
    parent_idx = heap_parent_index( hole_idx );
  }
  *(first + hole_idx) = std::move( value );
}

/**
 * @brief Remove an arbitrary element from a standard library heap
 * Artificially bubble up an element to the top of a heap and then pop it.
 * Remove the element in last - 1 after the operation.
 * @param first Start of heap.
 * @param last  End of heap.
 * @param pos   Element to remove.
 * @param comp  Comparison functor.
 * @ingroup heap_algorithms
 */
template <typename RAIter, typename Compare>
inline void heap_remove( RAIter first, RAIter last, RAIter pos, Compare comp ) {
  using Tp = typename std::iterator_traits<RAIter>::value_type;
  using Distance = typename std::iterator_traits<RAIter>::difference_type;

  heap_remove_impl( first, Distance( pos - first ), std::move( *pos ) );
  std::pop_heap( first, last, std::move( comp ) );
}

}  // namespace heap_util

/**
 * @brief Timeline keeps track of all scheduled events in time-order.
 * This class is implemented as a priority queue. The std::priority_queue would
 * be used but it does not allow for iteration or search.
 */
template <
    typename Tp,
    typename Sequence = std::vector<Tp>,
    typename Compare = std::greater<typename Sequence::value_type>>
class Timeline : public std::priority_queue<Tp, Sequence, Compare> {
public:
  using BaseType = std::priority_queue<Tp, Sequence, Compare>;
  using value_type = typename BaseType::value_type;
  using reference = typename BaseType::reference;
  using const_reference = typename BaseType::const_reference;
  using size_type = typename BaseType::size_type;
  using container_type = Sequence;
  using value_compare = Compare;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  // pull in priority_queue constructors
  using BaseType::BaseType;

  // Exposed iterators for finding and access that does not change ordering.
  // These member functions are delegated to the container. See container's
  // documentation but note iterators are always invalidated on insertion and
  // deletion due to this adapter's algorithms.
  inline const_iterator begin() const noexcept {
    return this->c.cbegin();
  }
  inline const_iterator end() const noexcept {
    return this->c.cend();
  }
  inline const_iterator cbegin() const noexcept {
    return this->c.cbegin();
  }
  inline const_iterator cend() const noexcept {
    return this->c.cend();
  }
  inline value_type take() {
    assert( !this->empty() );
    std::pop_heap( this->c.begin(), this->c.end(), this->comp );
    value_type top = std::move( this->c.back() );
    this->c.pop_back();
    return top; // counting on copy elision here
  }
  inline void erase( const_iterator pos ) {
    heap_util::heap_remove(
        this->c.begin(),
        this->c.end(),
        this->c.begin() + std::distance(pos, this->c.cbegin()),
        this->comp );
    this->c.pop_back();
  }
};

}  // namespace sim

#endif  // TIMELINE_H_INCLUDED
