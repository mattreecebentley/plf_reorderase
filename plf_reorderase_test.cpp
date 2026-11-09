#include "plf_reorderase.h"
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm> // std::sort
#include <string>
#include <list>
#include <forward_list>
#include <functional> // std::logical_not

struct small_struct
{
	double *empty_field_1;
	double unused_number;
	unsigned int empty_field2;
	double *empty_field_3;
	double number;
	unsigned int empty_field4;

	// This function is required for testing std::multiset:
	inline bool operator < (const small_struct &rh) const
	{
		return number < rh.number;
	}

	// This function is required for testing std::multiset:
	inline bool operator == (const small_struct &rh) const
	{
		return number == rh.number;
	}

	small_struct(const unsigned int num) noexcept: number(num) {};
};


// Element types which take each of the copy paths in deque range reorderase:
struct movable_struct // not trivially copyable, nothrow move-assignable
{
	int number;
	std::string text;

	movable_struct(const int num): number(num), text(40, 'a') {}
	bool operator < (const movable_struct &rh) const { return number < rh.number; }
	bool operator == (const movable_struct &rh) const { return number == rh.number; }
};


struct copy_only_struct // copy assignment is not noexcept and there is no move assignment, so a temporary buffer is used
{
	int number;

	copy_only_struct(const int num): number(num) {}
	copy_only_struct(const copy_only_struct &source): number(source.number) {}
	copy_only_struct & operator = (const copy_only_struct &source) { number = source.number; return *this; }
	bool operator < (const copy_only_struct &rh) const { return number < rh.number; }
	bool operator == (const copy_only_struct &rh) const { return number == rh.number; }
};



struct move_only_struct // no copy operations, nothrow move assignment
{
	int number;

	move_only_struct(const int num): number(num) {}
	move_only_struct(move_only_struct &&source) noexcept: number(source.number) {}
	move_only_struct & operator = (move_only_struct &&source) noexcept { number = source.number; return *this; }
	bool operator < (const move_only_struct &rh) const { return number < rh.number; }
	bool operator == (const move_only_struct &rh) const { return number == rh.number; }
};


struct throwing_move_only_struct // no copy operations, and move assignment is not noexcept, so moving is the only option
{
	int number;

	throwing_move_only_struct(const int num): number(num) {}
	throwing_move_only_struct(throwing_move_only_struct &&source): number(source.number) {}
	throwing_move_only_struct & operator = (throwing_move_only_struct &&source) { number = source.number; return *this; }
	bool operator < (const throwing_move_only_struct &rh) const { return number < rh.number; }
	bool operator == (const throwing_move_only_struct &rh) const { return number == rh.number; }
};


struct trivial_move_only_struct // trivially copyable, but with no copy operations
{
	int number;

	trivial_move_only_struct(const int num): number(num) {}
	trivial_move_only_struct(trivial_move_only_struct &&source) = default;
	trivial_move_only_struct & operator = (trivial_move_only_struct &&source) = default;
	bool operator < (const trivial_move_only_struct &rh) const { return number < rh.number; }
	bool operator == (const trivial_move_only_struct &rh) const { return number == rh.number; }
};


// Range-erase every sub-range of deques of up to 12 elements, including ranges which start closer to the front than their length, and check the remaining elements:
template <class element_type>
bool test_deque_range_reorderase(const char *type_name)
{
	for (int size = 1; size != 13; ++size)
	{
		for (int first = 0; first != size; ++first)
		{
			for (int last = first; last != size + 1; ++last)
			{
				std::deque<element_type> deque_test, expected;

				for (int number = 0; number != size; ++number)
				{
					deque_test.push_back(element_type(number));

					if (number < first || number >= last)
					{
						expected.push_back(element_type(number));
					}
				}

				plf::reorderase(deque_test, deque_test.begin() + first, deque_test.begin() + last);
				std::sort(deque_test.begin(), deque_test.end());

				if (deque_test != expected)
				{
					std::cout << "Deque range reorderase test failed for " << type_name << ": size " << size << ", erasing [" << first << ", " << last << ")\n";
					return false;
				}
			}
		}
	}

	std::cout << "Deque range reorderase test passed for " << type_name << "\n";
	return true;
}



// Single-element reorderase is used like erase() in an erase loop, so the returned iterator has to be usable when the last element is erased:
template <class container_type>
bool test_single_reorderase_erase_loop(const char *container_name)
{
	for (int size = 1; size != 9; ++size)
	{
		for (int bits = 0; bits != (1 << size); ++bits)
		{
			container_type container_test;
			int expected_size = 0;

			for (int index = 0; index != size; ++index)
			{
				const int number = (bits >> index) & 1;
				container_test.push_back(number);
				expected_size += 1 - number;
			}

			for (typename container_type::iterator current = container_test.begin(); current != container_test.end();)
			{
				if (*current == 1)
				{
					current = plf::reorderase(container_test, current);
				}
				else
				{
					++current;
				}
			}

			if (static_cast<int>(container_test.size()) != expected_size || std::count(container_test.begin(), container_test.end(), 1) != 0)
			{
				std::cout << "Single reorderase erase loop test failed for " << container_name << ": size " << size << ", bits " << bits << "\n";
				return false;
			}
		}

		container_type single; // Erasing the only element
		single.push_back(1);
		const typename container_type::iterator returned = plf::reorderase(single, single.begin());

		if (returned != single.end() || !single.empty())
		{
			std::cout << "Single reorderase of the last element test failed for " << container_name << "\n";
			return false;
		}
	}

	std::cout << "Single reorderase erase loop test passed for " << container_name << "\n";
	return true;
}



// Range-erase every sub-range of containers of up to 12 elements, and check that the returned iterator points to the element following the erased range (as with erase(first, last)), which is at first's former index, that the elements before the range are unchanged, and the remaining elements:
template <class container_type>
bool test_range_reorderase_return(const char *container_name)
{
	typedef typename container_type::value_type element_type;

	for (int size = 1; size != 13; ++size)
	{
		for (int first = 0; first != size; ++first)
		{
			for (int last = first; last != size + 1; ++last)
			{
				container_type container, expected;

				for (int number = 0; number != size; ++number)
				{
					container.push_back(element_type(number));

					if (number < first || number >= last)
					{
						expected.push_back(element_type(number));
					}
				}

				const typename container_type::iterator result = plf::reorderase(container, container.begin() + first, container.begin() + last);
				bool prefix_unchanged = true;

				for (int index = 0; index != first; ++index)
				{
					if (!(container[index] == element_type(index)))
					{
						prefix_unchanged = false;
					}
				}

				const int result_index = static_cast<int>(result - container.begin());
				std::sort(container.begin(), container.end());

				if (result_index != first || !prefix_unchanged || container != expected)
				{
					std::cout << "Range reorderase return test failed for " << container_name << ": size " << size << ", erasing [" << first << ", " << last << "), returned index " << result_index << "\n";
					return false;
				}
			}
		}
	}

	std::cout << "Range reorderase return test passed for " << container_name << "\n";
	return true;
}



// Erase the 1s from every sub-range of every container of up to 8 zeroes and ones, and check the count and the remaining elements. The 1s outside of the sub-range must stay:
template <class container_type>
bool test_ranged_reorderase_all(const char *container_name)
{
	for (int size = 1; size != 9; ++size)
	{
		for (int bits = 0; bits != (1 << size); ++bits)
		{
			for (int first = 0; first != size + 1; ++first)
			{
				for (int last = first; last != size + 1; ++last)
				{
					container_type container_test, expected;
					std::size_t expected_count = 0;

					for (int index = 0; index != size; ++index)
					{
						const int number = (bits >> index) & 1;
						container_test.push_back(typename container_type::value_type(number));

						if (number == 1 && index >= first && index < last)
						{
							++expected_count;
						}
						else
						{
							expected.push_back(typename container_type::value_type(number));
						}
					}

					const std::size_t count = plf::reorderase_all(container_test, container_test.begin() + first, container_test.begin() + last, 1);
					std::sort(container_test.begin(), container_test.end());
					std::sort(expected.begin(), expected.end());

					if (count != expected_count || container_test != expected)
					{
						std::cout << "Ranged reorderase_all test failed for " << container_name << ": size " << size << ", bits " << bits << ", erasing 1s from [" << first << ", " << last << ")\n";
						return false;
					}
				}
			}
		}
	}

	std::cout << "Ranged reorderase_all test passed for " << container_name << "\n";
	return true;
}



inline int element_number(const int number)
{
	return number;
}


template <class element_type>
inline int element_number(const element_type &element)
{
	return element.number;
}



struct is_odd
{
	template <class element_type>
	bool operator() (const element_type &element) const
	{
		return (element_number(element) & 1) != 0;
	}
};



// Test partition and destructive_partition against every pattern of odd and even numbers in small containers, including empty containers and containers where every element matches:
template <class container_type>
bool test_partition(const char *container_name)
{
	typedef typename container_type::iterator iterator_type;

	for (unsigned int size = 0; size != 7; ++size)
	{
		for (unsigned int bits = 0; bits != (1u << size); ++bits)
		{
			std::vector<int> source;
			std::size_t expected_count = 0;

			for (unsigned int index = 0; index != size; ++index)
			{
				const int number = static_cast<int>(index * 2 + ((bits >> index) & 1u));
				source.push_back(number);
				expected_count += (bits >> index) & 1u;
			}

			container_type partitioned(source.begin(), source.end()), destroyed(source.begin(), source.end());
			const iterator_type partition_point = plf::partition(partitioned.begin(), partitioned.end(), is_odd());
			const iterator_type destroyed_point = plf::destructive_partition(destroyed.begin(), destroyed.end(), is_odd());
			bool passed = (static_cast<std::size_t>(std::distance(partitioned.begin(), partition_point)) == expected_count && static_cast<std::size_t>(std::distance(destroyed.begin(), destroyed_point)) == expected_count);

			for (iterator_type current = partitioned.begin(); current != partitioned.end(); ++current)
			{
				passed = passed && (is_odd()(*current) == (std::distance(partitioned.begin(), current) < static_cast<std::ptrdiff_t>(expected_count)));
			}

			std::vector<int> sorted;

			for (iterator_type current = partitioned.begin(); current != partitioned.end(); ++current)
			{
				sorted.push_back(element_number(*current));
			}

		 // partition must not lose or duplicate elements
			std::sort(sorted.begin(), sorted.end());
			passed = passed && (sorted == source);

			std::vector<int> odd_numbers, kept_numbers; // destructive_partition must keep exactly the odd elements at the front

			for (std::vector<int>::iterator current = source.begin(); current != source.end(); ++current)
			{
				if (is_odd()(*current))
				{
					odd_numbers.push_back(*current);
				}
			}

			for (iterator_type current = destroyed.begin(); current != destroyed_point; ++current)
			{
				kept_numbers.push_back(element_number(*current));
			}

			std::sort(kept_numbers.begin(), kept_numbers.end());
			passed = passed && (kept_numbers == odd_numbers);

			if (!passed)
			{
				std::cout << "Partition test failed for " << container_name << ": size " << size << ", bits " << bits << "\n";
				return false;
			}
		}
	}

	std::cout << "Partition test passed for " << container_name << "\n";
	return true;
}



// Partitioning with raw pointers and with an iterator which dereferences to a temporary:
bool test_partition_raw_pointer()
{
	int numbers[6] = {1, 2, 3, 4, 5, 7}, destroyed[6] = {1, 2, 3, 4, 5, 7};
	std::vector<bool> flags(5, true);
	flags[1] = false;
	flags[3] = false;

	if (plf::partition(numbers, numbers + 6, is_odd()) != numbers + 4 || plf::destructive_partition(destroyed, destroyed + 6, is_odd()) != destroyed + 4 || plf::partition(flags.begin(), flags.end(), std::logical_not<bool>()) != flags.begin() + 2 || flags[0] || flags[1] || !flags[2])
	{
		std::cout << "Partition test failed for raw pointers or vector<bool>\n";
		return false;
	}

	std::cout << "Partition test passed for raw pointers and vector<bool>\n";
	return true;
}



int main()
{
	// Test erase loops with single-element reorderase:
	if (!test_single_reorderase_erase_loop<std::vector<int> >("vector") || !test_single_reorderase_erase_loop<std::deque<int> >("deque"))
	{
		std::cin.get();
		return 1;
	}

	std::vector<int> temp({1, 5, 6, 8, 10, 20, 8, 2, 2, 2, 8});

	plf::reorderase_all(temp, 8); // std::erase equivalent

	for (std::vector<int>::iterator current = temp.begin(); current != temp.end(); ++current)
	{
		std::cout << *current << ", ";
	}

	std::cout << "\n";

	plf::reorderase(temp, temp.begin() + 2, temp.begin() + 4); // range-erase equivalent

	for (std::vector<int>::iterator current = temp.begin(); current != temp.end(); ++current)
	{
		std::cout << *current << ", ";
	}

	std::cout << "\n";

	plf::reorderase(temp, temp.end() - 4, temp.end() - 2);

	for (std::vector<int>::iterator current = temp.begin(); current != temp.end(); ++current)
	{
		std::cout << *current << ", ";
	}


	std::cout << "\n";

	plf::reorderase(temp, temp.begin() + 3);  // Singular-erase equivalent

	for (std::vector<int>::iterator current = temp.begin(); current != temp.end(); ++current)
	{
		std::cout << *current << ", ";
	}


	std::cout << "\n";

	std::deque<int> temp2({1, 2, 3, 4, 4, 5, 6, 7, 7, 4, 8, 9});

	plf::reorderase(temp2, temp2.begin() + 5);

	for (std::deque<int>::iterator current = temp2.begin(); current != temp2.end(); ++current)
	{
		std::cout << *current << ", ";
	}

	std::cout << "\n";

	plf::reorderase(temp2, temp2.begin() + 1, temp2.begin() + 3);

	for (std::deque<int>::iterator current = temp2.begin(); current != temp2.end(); ++current)
	{
		std::cout << *current << ", ";
	}

	std::cout << "\n";


	// Test sub-range based erase_if equivalent
	plf::reorderase_all(temp2, temp2.begin() + 1, temp2.begin() + 4, 4);

	for (std::deque<int>::iterator current = temp2.begin(); current != temp2.end(); ++current)
	{
		std::cout << *current << ", ";
	}
	
	std::cout << "\n";


	// Test range erase with deque:
	if (!test_deque_range_reorderase<int>("int") || !test_deque_range_reorderase<movable_struct>("movable_struct") || !test_deque_range_reorderase<copy_only_struct>("copy_only_struct"))
	{
		std::cin.get();
		return 1;
	}


	// Test the iterator returned by range reorderase:
	if (!test_range_reorderase_return<std::vector<int> >("vector of int") || !test_range_reorderase_return<std::deque<int> >("deque of int") || !test_range_reorderase_return<std::vector<movable_struct> >("vector of movable_struct") || !test_range_reorderase_return<std::deque<movable_struct> >("deque of movable_struct") || !test_range_reorderase_return<std::vector<copy_only_struct> >("vector of copy_only_struct") || !test_range_reorderase_return<std::deque<copy_only_struct> >("deque of copy_only_struct"))
	{
		std::cin.get();
		return 1;
	}


	// Test partitioning with forward, bidirectional and random-access iterators:
	if (!test_partition<std::forward_list<int> >("forward_list of int") || !test_partition<std::list<int> >("list of int") || !test_partition<std::vector<int> >("vector of int") || !test_partition<std::deque<int> >("deque of int") || !test_partition<std::forward_list<movable_struct> >("forward_list of movable_struct") || !test_partition<std::list<movable_struct> >("list of movable_struct") || !test_partition<std::vector<movable_struct> >("vector of movable_struct") || !test_partition<std::forward_list<copy_only_struct> >("forward_list of copy_only_struct") || !test_partition<std::list<copy_only_struct> >("list of copy_only_struct") || !test_partition<std::vector<copy_only_struct> >("vector of copy_only_struct") || !test_partition_raw_pointer())
	{
		std::cin.get();
		return 1;
	}


	// Test sub-range reorderase_all against every pattern of small containers:
	if (!test_ranged_reorderase_all<std::vector<int> >("vector of int") || !test_ranged_reorderase_all<std::deque<int> >("deque of int") || !test_ranged_reorderase_all<std::vector<movable_struct> >("vector of movable_struct") || !test_ranged_reorderase_all<std::deque<movable_struct> >("deque of movable_struct") || !test_ranged_reorderase_all<std::vector<copy_only_struct> >("vector of copy_only_struct") || !test_ranged_reorderase_all<std::deque<copy_only_struct> >("deque of copy_only_struct"))
	{
		std::cin.get();
		return 1;
	}



	// Test every function with move-only element types:
	if (!test_single_reorderase_erase_loop<std::vector<move_only_struct> >("vector of move_only_struct") || !test_single_reorderase_erase_loop<std::deque<move_only_struct> >("deque of move_only_struct") || !test_single_reorderase_erase_loop<std::vector<throwing_move_only_struct> >("vector of throwing_move_only_struct") || !test_single_reorderase_erase_loop<std::deque<throwing_move_only_struct> >("deque of throwing_move_only_struct")
		|| !test_deque_range_reorderase<move_only_struct>("move_only_struct") || !test_deque_range_reorderase<throwing_move_only_struct>("throwing_move_only_struct") || !test_deque_range_reorderase<trivial_move_only_struct>("trivial_move_only_struct")
		|| !test_range_reorderase_return<std::vector<move_only_struct> >("vector of move_only_struct") || !test_range_reorderase_return<std::vector<throwing_move_only_struct> >("vector of throwing_move_only_struct") || !test_range_reorderase_return<std::vector<trivial_move_only_struct> >("vector of trivial_move_only_struct")
		|| !test_ranged_reorderase_all<std::vector<move_only_struct> >("vector of move_only_struct") || !test_ranged_reorderase_all<std::deque<move_only_struct> >("deque of move_only_struct") || !test_ranged_reorderase_all<std::vector<throwing_move_only_struct> >("vector of throwing_move_only_struct") || !test_ranged_reorderase_all<std::deque<throwing_move_only_struct> >("deque of throwing_move_only_struct")
		|| !test_partition<std::forward_list<move_only_struct> >("forward_list of move_only_struct") || !test_partition<std::list<move_only_struct> >("list of move_only_struct") || !test_partition<std::vector<move_only_struct> >("vector of move_only_struct") || !test_partition<std::vector<throwing_move_only_struct> >("vector of throwing_move_only_struct"))
	{
		std::cin.get();
		return 1;
	}
	std::cin.get();
}