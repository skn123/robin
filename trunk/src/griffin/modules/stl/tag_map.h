
namespace std {

template <class KEY, class VALUE>
class map
{
public:
	typedef pair<const KEY,VALUE> value_type;
	
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
	int size() const;
	
	VALUE& operator[](const KEY &key);
	int erase(const KEY &key);
	//void insert(const pair<const KEY,VALUE> &val);
	iterator find(const KEY &key);

	~map();
};

}

