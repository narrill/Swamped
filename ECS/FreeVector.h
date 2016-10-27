#pragma once
#include <vector>

using namespace std;

//A vector wrapper for easy index reuse
template <typename T>
class FreeVector {
public:
	FreeVector() {

	}
	~FreeVector() {
		//m_vector.clear();
		//m_freeIndices.clear();
	}

	//Adds the given item at the lowest available index
	unsigned int add(T item) {
		//if there are unused indices
		if (m_freeIndices.size() > 0)
		{
			//add at the first unused index
			unsigned int index = m_freeIndices[0];
			m_vector[index] = item;
			//then remove the index
			m_freeIndices.erase(m_freeIndices.begin());
			m_count++;
			return index;
		}
		//if there are no unused indices
		else
		{
			//add at the end of the vector
			m_vector.push_back(item);
			m_count++;
			return m_vector.size() - 1;
		}
	}

	//Marks an index as available
	void free(unsigned int index) {
		if (m_vector.size() > index)
		{
			m_freeIndices.push_back(index);
			m_count--;
		}
	}

	//Gets the size of the underlying vector
	size_t size() {
		return m_vector.size();
	}

	size_t count() {
		return m_count;
	}

	T& operator[] (const int index) {
		return m_vector[index];
	}
private:
	//Data store
	vector<T> m_vector;
	//List of available indices before the end of the vector
	vector<int> m_freeIndices;

	size_t m_count = 0;
};