#include "plf_reorderase.h"
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm> // std::sort
#include <string>

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


	// Test sub-range reorderase_all against every pattern of small containers:
	if (!test_ranged_reorderase_all<std::vector<int> >("vector of int") || !test_ranged_reorderase_all<std::deque<int> >("deque of int") || !test_ranged_reorderase_all<std::vector<movable_struct> >("vector of movable_struct") || !test_ranged_reorderase_all<std::deque<movable_struct> >("deque of movable_struct") || !test_ranged_reorderase_all<std::vector<copy_only_struct> >("vector of copy_only_struct") || !test_ranged_reorderase_all<std::deque<copy_only_struct> >("deque of copy_only_struct"))
	{
		std::cin.get();
		return 1;
	}

	std::cin.get();
}