#include "plf_reorderase.h"
#include <iostream>
#include <vector>

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


	std::cin.get();
}