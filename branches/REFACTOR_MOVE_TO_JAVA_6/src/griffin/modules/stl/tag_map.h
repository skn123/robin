#error this file should never be included

namespace std {

template <class KEY, class VALUE>
class map
{
public:
	class iterator
	{
	public:
		pair<KEY,VALUE> operator*();
		iterator& operator++();
		bool operator==(const iterator &i) const;
		bool operator!=(const iterator &i) const;
	};

	class const_iterator
	{
	public:
		const pair<KEY,VALUE> operator*();
		const_iterator& operator++();
		bool operator==(const const_iterator &i) const;
		bool operator!=(const const_iterator &i) const;
	};
		
	map();
	map(const map<KEY,VALUE> &other);

	iterator begin();
	iterator end();

	bool empty() const;
	unsigned long size() const;	
	VALUE operator[](const KEY &key);
	iterator find(const KEY &key);
	
	int erase(const KEY &key);
	void insert(const pair<KEY,VALUE> &val);

	~map();
};

}

