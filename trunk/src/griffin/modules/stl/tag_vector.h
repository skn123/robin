
namespace std {

template < class T >
class vector
{
public:
	vector();
	vector(const std::vector<T>& other);
	unsigned long size() const;
	T operator[](unsigned long index) const;
	void push_back(T element);
	~vector();
};

}
