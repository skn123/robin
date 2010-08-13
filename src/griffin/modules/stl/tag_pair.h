#error this file should never be included

namespace std {

template <class T1, class T2>
class pair
{
public:
	T1 first;
	T2 second;
	
	pair();
	pair(const T1 &a, const T2 &b);
	template <class U1, class U2>
	pair(const pair<U1,U2> &other);
	bool operator==(const pair &) const;
};
	
}

