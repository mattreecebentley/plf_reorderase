#include "plf_reorderase.h"
#include <iostream>
#include <vector>
#include <deque>

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


int main()
{
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
	
	std::cin.get();
}