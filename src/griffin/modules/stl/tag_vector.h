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
	
	class const_iterator
	{
	public:
		const T& operator*();
		const_iterator& operator++();
		bool operator==(const const_iterator &i) const;
		bool operator!=(const const_iterator &i) const;
	};
	
	vector();
	vector(const std::vector<T>& other);

	iterator begin();
	iterator end();

	void reserve(unsigned long n);	
	unsigned long size() const;
	bool empty() const;
	T operator[](unsigned long index) const;

	void push_back(const T &element);
	void erase(iterator pos);
#if 0
	/*
	 * operator== should be wrapped but only when it is valid. 
	 * 	( when T::operator== is also defined )
	 * 
	 * Currently there is no mechanism to decide the inclusion of operator== 
	 * depending on T so we are forced not to wrap it.
	 * 
	 */
	bool operator==(const vector &other) const;
#endif
	~vector();
};

}
