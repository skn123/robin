
namespace std {

template <class KEY, class VALUE>
class map
{
public:
	typedef pair<KEY,VALUE> value_type;
	
	class iterator
	{
	public:
		pair<const KEY,VALUE>& operator*();
		iterator& operator++();
		bool operator==(const iterator &i) const;
		bool operator!=(const iterator &i) const;
	};
		
	map();

	iterator begin();
	iterator end();

	bool empty() const;
	unsigned long size() const;	
	VALUE& operator[](const KEY &key);
	iterator find(const KEY &key);
	
	int erase(const KEY &key);
	void insert(const pair<KEY,VALUE> &val);

	~map();
};

}

