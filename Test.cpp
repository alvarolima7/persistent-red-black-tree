#include <random>

#include "RBTree.h"

int randomInt()
{
	static std::random_device dev;
	static std::mt19937 rng(dev());
	static std::uniform_int_distribution<std::mt19937::result_type> distribuition(0, INT32_MAX);

	return distribuition(rng);
}

int randomInt(int min, int max)
{
	return min + randomInt() % (max - min);
}

void fillWithRandomInts(int* arr, int size)
{
	for (int i = 0; i < size; i++)
		arr[i] = i;
	for (int i = 0; i < size; i++)
	{
		int randomIndex = randomInt(0, size);
		int randomIndex2 = randomInt(0, size);
		std::swap(arr[randomIndex], arr[randomIndex2]);
	}
}

void doRandomInsertions()
{
	constexpr int arrSize = 1000;
	int* values = new int[arrSize];
	for (int i = 0; i < 1000; i++)
	{
		fillWithRandomInts(values, arrSize);
		RBTree tree;
		for (int i = 0; i < arrSize; i++)
			tree.Insert(values[i]);
	}

	delete[] values;
}

void doRandomDeletions()
{
	constexpr int arrSize = 10000;
	int* values = new int[arrSize];
	for (int i = 0; i < arrSize; i++)
	{
		fillWithRandomInts(values, arrSize);
		RBTree tree;
		for (int i = 0; i < arrSize; i++)
			tree.Insert(values[i]);

		for (int i = 0; i < arrSize; i++)
			tree.Delete(values[i]);
	}

	delete[] values;
}

void visualizeInsertions()
{
	constexpr int arrSize = 10;
	int values[arrSize];
	for (int n = 0; n < 1000; n++)
	{
		fillWithRandomInts(values, arrSize);
		RBTree tree;
		std::cout << "\n\n =-=-=-=-=-=-=-= New =-=-=-=-=-=-=-= \n\n";
		for (int i = 0; i < arrSize; i++)
		{
			std::cout << "\nInserting " << values[i] << "  __________________________\n\n";
			tree.Insert(values[i]);
			tree.Print();
			std::cin.get();
		}
	}
}

void visualizeDeletions()
{
	constexpr int arrSize = 10;
	int values[arrSize];
	for (int n = 0; n < 1000; n++)
	{
		fillWithRandomInts(values, arrSize);
		RBTree tree;
		for (int i = 0; i < arrSize; i++)
			tree.Insert(values[i]);

		std::cout << "\n\n =-=-=-=-=-=-=-= New =-=-=-=-=-=-=-= \n\n";
		tree.Print();

		for (int i = 0; i < arrSize; i++)
		{
			std::cout << "\n\n\ndeleting " << values[i] << "  __________________________\n";
			std::cin.get();
			tree.Delete(values[i]);
			tree.Print();
		}
	}
}

int main()
{
	//doRandomInsertions();
	//doRandomDeletions();
	//visualizeInsertions();
	visualizeDeletions();

	return 0;
}