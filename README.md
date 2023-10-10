# plf_reorderase
A faster method for singular erasures, ranged erasures, and erase_if-style erasures for vector/deque/static_vector when element order is not important. Benchmark averages follow, averaged across four types (int, double, 40-byte struct, 490-byte struct) and numbers of elements ranging from 10 to 920000. See website for full breakdown of details:

* Single erasures: 1010566% faster than .erase(position) on average (ie. 10157 x original speed), with a minimum of 10% faster for a vector of 10 ints, and a maximum of 25974010% faster for 870000 490-byte structs.
* Ranged erasures: 12570% faster than .erase(first, last) on average, with a minimum of 20% faster for a vector of 10 ints, and a with a maximum of 77679% for 920000 490-byte structs.
* std::erase_if/std::erase-style erasures: This typically only outperforms the std:: equivalents on types larger than 25 bytes. 38% faster than std::erase_if(container, predicate)/std::erase(container, value) on average, with a minimum of 10% slower for 50 doubles, and a maximum of 180% for 920000 490-byte structs.

Function descriptions follow, for all other information including full benchmarks, see https://plflib.org/reorderase.htm



* template <class container_type, class iterator_type = typename container_type::iterator>
constexpr void reorderase (container_type &container, iterator_type position)

  Erase single element. Use wherever you would use a container's .erase(position) member function normally.

* template <class container_type, class iterator_type = typename container_type::iterator>
constexpr void reorderase (container_type &container, iterator_type first, iterator_type last)

  Erase a range of elements. Use wherever you would use a container's .erase(first, last) member function normally. Example:

* template <class container_type, class predicate_function>
constexpr void reorderase_all_if (container_type &container, predicate_function predicate)

  Erase all elements in a container which match the predicate. Use wherever you would use std::erase_if normally. 

* template <class container_type, class value_type>
constexpr void reorderase_all (container_type &container, value_type value)

  Erase all elements in a container which are equal to value. Use wherever you would use std::erase normally.

* template <class container_type, class iterator_type, class predicate_function>
constexpr void reorderase_all_if (container_type &container, iterator_type first, iterator_type last, predicate_function predicate)

  Erase all elements in a sub-range of a container which match the predicate. Equivalent to std::erase_if, if there were an equivalent of std::erase_if for sub-ranges of containers. Example using a lambda:

* template <class container_type, class iterator_type, class value_type>
constexpr void reorderase_all (container_type &container, iterator_type first, iterator_type last, value_type value)

  Erase all elements in a sub-range of a container which are equal to value. Will use appropriate elements from the back of the container to fill the holes. Equivalent to std::erase, if there were an equivalent of std::erase for sub-ranges of containers. value must be able to be static_cast'd to container_type::value_type.

* template <class container_type, class iterator_type, class predicate_function>
constexpr void reorderase_all_if (container_type &container, iterator_type first, iterator_type last, predicate_function predicate)

  Erase all elements in a sub-range of a container which match the predicate. Equivalent to std::erase_if, if there were an equivalent of std::erase_if for sub-ranges of containers.

* template <class container_type, class iterator_type, class value_type>
constexpr void reorderase_all (container_type &container, iterator_type first, iterator_type last, value_type value)

  Erase all elements in a sub-range of a container which are equal to value. Will use appropriate elements from the back of the container to fill the holes. Equivalent to std::erase, if there were an equivalent of std::erase for sub-ranges of containers. value must be able to be static_cast'd to container_type::value_type. 
  
* template <class iterator_type, class predicate_function>
constexpr iterator_type partition (iterator_type first, iterator_type last, predicate_function predicate)

  My personal algorithm for partitioning. Use wherever you would use std::partition.

* template <class iterator_type, class predicate_function>
constexpr iterator_type destructive_partition (iterator_type first, iterator_type last, predicate_function predicate)

  Destructively partition all elements which match the predicate in a range ie. move elements from the back of the range (which don't match the predicate) on top of elements matching the predicate, leaving a trailing range of moved-from elements at the back of the range. Use wherever you would use std::partition but only if you don't want to preserve the elements which would be, in std::partition, moved to the back of the range. Returns an iterator pointing to the first moved-from element in the trailing range.
