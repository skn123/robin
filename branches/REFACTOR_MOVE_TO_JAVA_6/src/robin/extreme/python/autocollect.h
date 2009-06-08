#ifndef ROBIN_EXTREME_PYTHON_AUTOCOLLECT
#define ROBIN_EXTREME_PYTHON_AUTOCOLLECT

class CollectMe1 { };

template <typename T> class CollectMe2 { };

typedef CollectMe2<int> CollectMe2A;
typedef CollectMe2<long> CollectMe2B;

class CollectMe3 {

 public:
	class CollectMe4 { };

	typedef CollectMe2<char> CollectMe2C;
	
	CollectMe2<CollectMe4> autoinstantiate() { return CollectMe2<CollectMe4>(); }
};

#endif
