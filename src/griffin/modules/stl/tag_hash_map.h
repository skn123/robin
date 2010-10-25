#error this file should never be included

namespace __gnu_cxx {

template <class KEY, class VALUE>
class hash_map
{
public:
	class iterator
	{
	public:
		std::pair<KEY,VALUE> operator*();
		iterator& operator++();
		bool operator==(const iterator &i) const;
		bool operator!=(const iterator &i) const;
	};

	class const_iterator
	{
	public:
		const std::pair<KEY,VALUE> operator*();
		const_iterator& operator++();
		bool operator==(const const_iterator &i) const;
		bool operator!=(const const_iterator &i) const;
	};
		
	hash_map();
	hash_map(const hash_map<KEY,VALUE> &other);

	iterator begin();
	iterator end();

	bool empty() const;
	unsigned long size() const;	
	VALUE operator[](const KEY &key);
	iterator find(const KEY &key);
	
	int erase(const KEY &key);
	void insert(const std::pair<KEY,VALUE> &val);

	~hash_map();
};

}

