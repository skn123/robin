from __future__ import generators
import robin, os.path
import __builtin__

if os.path.islink(__file__): __file__ = os.readlink(__file__)
here = os.path.dirname(__file__)
machine = os.getenv("MACHINE")
libtemplate = "%(sopre)s%%s%(platspec)s-%(ver)s%(pyspec)s%(soext)s"
lib = libtemplate % robin.__dict__ % "robin_stl"
robin.loadLibrary(__name__, robin.locate(__file__, lib))

ostringstream = std.ostringstream
istringstream = std.istringstream
ifstream = std.ifstream
ofstream = std.ofstream
string = std.string
string.__to__ = str

truetype = { double: float, long: int, ulong: int, uint: int, \
			 char: str, uchar: str, schar: str,
			 longlong: long, ulonglong: long}

def _sum_tuples(tuplelst):
	sumlst = [0 for i in tuplelst[0]]
	for curtuple in tuplelst:
		for i in range(len(curtuple)):
			sumlst[i] = sumlst[i] + curtuple[i]
	return tuple(sumlst)

def _make_vector_functor(vectype, volatile = False):
	def make_any_vector(lst, vectype = vectype):
		vec = vectype()
		for item in lst:
			vec.push_back(item)
		if volatile:
			vec.__owner__ = STLContainer.STLOwner(lst, vec)
		return vec

	return make_any_vector

def _make_list_functor(listtype, volatile = False):
	def make_any_list(lst, listtype = listtype):
		l = listtype()
		for item in lst:
			l.push_back(item)
		if volatile:
			l.__owner__ = STLContainer.STLOwner(lst, l)
		return l

	return make_any_list

def _make_set_functor(settype, volatile = False):
	def make_any_set(lst, settype = settype):
		set = settype()
		for item in lst:
			set.insert(item)
		if volatile:
			set.__owner__ = STLContainer.STLOwner(lst, set)
		return set

	return make_any_set

def _make_map_functor(maptype, volatile = False):
	def make_any_map(dic, maptype = maptype):
		map = maptype()
		for item in dic.keys():
			map.insert((item,dic[item]))
		if volatile:
			map.__owner__ = STLContainer.STLOwner(dic, map)
		return map

	return make_any_map

def _make_container_weigher(el):
	def weigh_any_container(insight, el = el):
		tinsight = type(insight)
		if tinsight is el:
			w = (0, 1, 0, 0)
		else:
			w = robin.weighConversion(
					insight,
					truetype.get(el, el))
			w = _sum_tuples( [w, (0, 1, 0, 0)] )
		return w

	return weigh_any_container

def _make_map_weigher(el):
	def weigh_any_map(insight, el = el):
		total_weight = (0, 0, 0, 0)
		for i in range(len(insight)):
			if insight[i] is el[i]:
				w = (0, 1, 0, 0)
			else:
				w = robin.weighConversion(
						insight,
						truetype.get(el[i], el[i]))
				w = _sum_tuples( [w, (0, 1, 0, 0)] )
			total_weight = _sum_tuples( [total_weight, w] )
		return total_weight

	return weigh_any_map

def _vector_from_list(vl, el = None):
	if el:
		vl.__from__[[]] = _make_vector_functor(vl), 2, _make_container_weigher(el)
		vl.__from_volatile__[[]] = \
				_make_vector_functor(vl, True), 2, _make_container_weigher(el)
	else:
		vl.__from__[[]] = _make_vector_functor(vl)
		vl.__from_volatile__[[]] = _make_vector_functor(vl, True)
	vl.__getslice__ = lambda self, fr, to: [self[i] for i in xrange(fr,to)]

def _list_from_list(ll, el = None):
	if el:
		ll.__from__[[]] = _make_list_functor(ll), 2, _make_container_weigher(el)
		ll.__from_volatile__[[]] = \
				_make_list_functor(ll, True), 2, _make_container_weigher(el)
	else:
		ll.__from__[[]] = _make_list_functor(ll)
		ll.__from_volatile__[[]] = _make_list_functor(ll, True)
	ll.__len__ = lambda self: len([i for i in gen(self)])
	ll.__getitem__ = lambda self, index: [i for i in gen(self)][index]

def _set_from_list(sl, el = None):
	if el:
		sl.__from__[[]] = _make_set_functor(sl), 2, _make_container_weigher(el)
		sl.__from_volatile__[[]] = \
				_make_set_functor(sl, True), 2, _make_container_weigher(el)
	else:
		sl.__from__[[]] = _make_set_functor(sl)
		sl.__from_volatile__[[]] = _make_set_functor(sl, True)
	sl.__len__ = lambda self: len([i for i in gen(self)])
	sl.__getitem__ = lambda self, index: [i for i in gen(self)][index]

def _map_from_dict(md, el = None):
	if el:
		md.__from__[{}] = _make_map_functor(md), 2, _make_map_weigher(el)
		md.__from_volatile__[{}] = \
				_make_map_functor(md, True), 2, _make_map_weigher(el)
	else:
		md.__from__[{}] = _make_map_functor(md)
		md.__from_volatile__[{}] = _make_map_functor(md, True)
	def getitem(self, key):
		try:
			self.find(key)
		except RuntimeError:
			raise TypeError("map key and value must be of the type " + str(self))
		if self.find(key) == self.end():
			raise KeyError(key)
		return getattr(self,"operator[]")(key)
	md.__getitem__ = md.__getsubscript__ = getitem
	def setitem(self, key, value):
		self.insert((key,value))
	md.__setitem__ = md.__setsubscript__ = setitem
	def delitem(self, key):
		self.erase(key)
	md.__delitem__ = md.__delsubscript__ = delitem
	def size(self):
		return self.size()
	md.__len__ = md.__mapsize__ = size

def _map_to_dict(md):
	d = { }
	for item in gen(md):
		if isinstance(item,tuple):
			d[item[0]] = item[1]
		else:
			d[item.first] = item.second
	return d

def _pair_with_tuple(pr):
	pr.__getitem__ = lambda self,i: (self.first, self.second)[i]
	pr.__len__ = lambda self: 2

	pr.__from__[()] = lambda (first,second): pr(first, second)

def build_vector(vectype, datalist):
	"""
	stl.build_vector(vectype, datalist) --> a vectype object
	Builds an instance of the given vector 'vectype' with the elements of
	'datalist'.
	Placement is done using std::vector<T>::push_back; element types must
	be convertible to T or an error occurs.
	"""
	vec = vectype()
	for item in datalist:
		vec.push_back(item)
	return vec

def guess_container_type(contname, conttype):
	try:
		inside = conttype.__name__[(len(contname) + len("< ")):-len(" >")]
		return robin.classByName(inside)
	except KeyError:
		return None

def couple(stltype, prefix = None, element = None):
	if prefix is None:
		prefix = stltype.__name__.split("<")[0]
	
	if prefix == "std::vector":
		_vector_from_list(stltype, element or _guess_container_type(prefix, stltype))
		stltype.__to__ = __builtin__.list
	elif prefix == "std::list":
		_list_from_list(stltype, element or _guess_container_type(prefix, stltype))
		stltype.__to__ = __builtin__.list
	elif prefix == "std::set":
		_set_from_list(stltype, element or _guess_container_type(prefix, stltype))
		stltype.__to__ = __builtin__.list
	elif prefix == "std::pair":
		_pair_with_tuple(stltype)
		stltype.__to__ = tuple
	elif prefix == "std::complex":
		stltype.__from__[0j] = lambda z, c=stltype: c(z.real, z.imag)
		stltype.__to__ = lambda z: __builtin__.complex(z.real(), z.imag())
	elif prefix == "std::map":
		_map_from_dict(stltype, element)
		stltype.__to__ = _map_to_dict
	else:
		raise TypeError, "invalid STL prefix: " + prefix


# STL container template dictionary
class STLContainer(dict):
	class STLOwner:
		def __init__(self, l, c):
			import weakref
			self.l = l
			self.c = weakref.ref(c)
			
		def __del__(self):
			del self.l[:]
			self.l.extend(__builtin__.list(self.c()))
	
	def __setitem__(self, key, value):
		try:
			self.couple(value, truetype.get(key, key))
			dict.__setitem__(self, key, value)
		except:
			import traceback
			traceback.print_exc()

	def couple(self, conttype, elemtype):
		couple(conttype, element = elemtype)

# std::vector
vector = STLContainer()
robin.declareTemplate("std::vector", vector)

# std::list
list = STLContainer()
robin.declareTemplate("std::list", list)

# std::set
set = STLContainer()
robin.declareTemplate("std::set", set)

# std::pair
pair = STLContainer()
robin.declareTemplate("std::pair", pair)

# std::map
map = STLContainer()
robin.declareTemplate("std::map", map)

# std::complex
complex = STLContainer()
robin.declareTemplate("std::complex", complex)
robin.familiarize(__builtin__.complex)

# generators
def gen(container):
	iter = container.begin()
	while not iter == container.end():
		yield getattr(iter, "operator *")()
		getattr(iter, "operator++")()

# backward compatibility
Vector = STLContainer
Vector.VectorOwner = Vector.STLOwner
make_vector_weigher = _make_container_weigher
guess_vector_type = guess_container_type


# std::streambuf
class streambuf(Igluebuf):

    def __init__(self, fileobj):
        self._file = fileobj

    def write(self, data):
        self._file.write(str(data))

    def read(self, n):
        return string(self._file.read(n))


# std::iostream
class _ostream(std.ostream):

    def __init__(self, file):
        self.gb = streambuf(file.pyfile)
        std.ostream.__init__(self, self.gb)


class _istream(std.istream):

    def __init__(self, file):
        self.gb = streambuf(file.pyfile)
        std.istream.__init__(self, self.gb)


class file(object): 

    def __init__(self, pyfile):
        self.pyfile = pyfile


class ifile(file): pass
class ofile(file): pass



robin.familiarize(ofile)
robin.familiarize(ifile)
robin.familiarize(file)

std.ostream.__from__[ofile(None)] = _ostream
std.ostream.__from__[file(None)]  = _ostream
std.istream.__from__[ifile(None)] = _istream
std.istream.__from__[file(None)]  = _istream


del os
