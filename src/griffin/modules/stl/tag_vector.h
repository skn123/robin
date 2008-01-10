#error this file should never be included

namespace std {

template < class T >
class vector
{
public:
	class iterator
	{
	public:
		T& operator*();
		iterator& operator++();
		bool operator==(const iterator &i) const;
		bool operator!=(const iterator &i) const;
	};
	
	vector();
	vector(const std::vector<T>& other);

	iterator begin();
	iterator end();
	
	unsigned long size() const;
	bool empty() const;
	T operator[](unsigned long index) const;

	void push_back(const T &element);
	void erase(iterator pos);
	
	~vector();
};

}
