
namespace std {

template < class T >
class list
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
	
	list();
	list(const std::list<T>& other);

	iterator begin();
	iterator end();
	
	bool empty() const;
	unsigned long size() const;
	
	void push_back(const T &element);
	void insert(iterator pos, const T &element);
	void erase(iterator pos);
	
	~vector();
};

}

