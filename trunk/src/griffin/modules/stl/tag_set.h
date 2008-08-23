#error this file should never be included

namespace std {

template < class T >
class set
{
public:
	class iterator
	{
	public:
		const T& operator*();
		iterator& operator++();
		bool operator==(const iterator &i) const;
		bool operator!=(const iterator &i) const;
	};
	
	set();
	set(const std::set<T>& other);

	iterator begin();
	iterator end();
	
	unsigned long size() const;
	bool empty() const;
	
	void insert(const T &element);
	void erase(iterator pos);
	void erase(const T &element);

	iterator find(const T &element);
	
	~set();
};

}
