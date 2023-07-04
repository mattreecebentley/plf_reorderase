#include "plf_reorderase.h"
#include <iostream>
#include <vector>
#include <deque>

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

	std::deque<int> temp2({1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9});

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

	std::cin.get();
}