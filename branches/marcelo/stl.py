from __future__ import generators
import robin_v1_1, robin, os.path
import __builtin__
import weakref


if os.path.islink(__file__): __file__ = os.readlink(__file__)
here = os.path.dirname(__file__)
machine = os.getenv("MACHINE")
moduletemplate = "%(sopre)s%%s%(platspec)s_%(ver)s%(pyspec)s"
libtemplate = "%(sopre)s%%s%(platspec)s_%(ver)s%(pyspec)s%(soext)s"


path_stl = libtemplate % robin.__dict__ % "robin_stl"
robin_v1_1.loadLibrary(__name__, path_stl)

path_stl_py = moduletemplate % robin.__dict__ % "stl_py"
stl_py = __import__(path_stl_py)  

ostringstream = std.ostringstream
istringstream = std.istringstream
ifstream = std.ifstream
ofstream = std.ofstream
string = std.string
string.__to__ = str


#These are optimization functions which are used to create and access a vector very fast
fillFromList = {c_long: stl_py._fillVectorOfLongFromList,
                c_int: stl_py._fillVectorOfIntFromList,
                uint: stl_py._fillVectorOfUnsignedIntFromList,
                ulonglong: stl_py._fillVectorOfUnsignedLongLongFromList,
                longlong: stl_py._fillVectorOfLongLongFromList }
fillToList = {c_long: stl_py._fillListFromVectorOfLong,
              c_int: stl_py._fillListFromVectorOfInt,
              uint: stl_py._fillListFromVectorOfUnsignedInt,
              ulonglong: stl_py._fillListFromVectorOfUnsinedLongLong,
              longlong: stl_py._fillListFromVectorOfLongLong}


def _basicRefillList(cobj,lst):
    '''
    a function which is able to make lst to hold the copies
    of the elements in cobj. 
    lst must be a python list.
    cobj can be used for basic containers as std::vector, std::list, std::set 
    '''
    del lst[:]
    lst.extend(__builtin__.list(cobj))

def _fill_list_from_vector_functor(vecttype,elemtype):
    return fillToList.get(elemtype, _basicRefillList)

def _make_list_from_vector_functor(vecttype,elemtype):
    fill_func = _fill_list_from_vector_functor(vecttype,elemtype)
    def mklist(vec):
        lst = []
        fill_func(vec,lst)
        return lst
    return mklist

def _make_vector_functor(vectype, volatile, elemtype):
    '''
    Returns a function which can convert a list to a vector.
    @param vectype The type of the destination vector, for example 'stl.vector[int]'
    @param volatile Whatever the vector will be handled to a function by reference.
                    In that case the list will be updated with the new contents of the vector
                    when the function exits.
    @param elemtype The type of the element in the vector, for example 'int'
    '''
    

    def basic_fillVector(lst,vec):
        '''
        An unoptimized function which knows how to copy all the elements of
        a list to a std::vector
        '''
        vec.reserve(len(lst))
        push_back_in_vec =  robin.preResolve(vec.push_back)
        for item in lst:
            push_back_in_vec(item)

    if fillFromList.has_key(elemtype):
       fillFromListFunc = fillFromList[elemtype]
    else:
       fillFromListFunc = basic_fillVector
    fillToListFunc = _fill_list_from_vector_functor(vectype,elemtype)   
    
    def make_any_vector(lst):
        vec = vectype()
        fillFromListFunc(lst,vec)
        if volatile: 
            # 'owner' will be the object which updates the contents of 'lst' from 'vec',
            # in its destructor.
            owner = STLContainer.STLOwner(lst, vec, fillToListFunc)
            # The next line makes 'owner' not to be garbage collected until
            # vec is deleted.
            vec.__owner__ = owner
        return vec
    return make_any_vector

def _make_list_functor(listtype, volatile = False):
    def make_any_list(lst, listtype = listtype):
        l = listtype()
        import robin
        push_back_in_l = robin.preResolve(l.push_back)
        for item in lst:
            push_back_in_l(item)
        if volatile:
            l.__owner__ = STLContainer.STLOwner(lst, l,_basicRefillList)
        return l

    return make_any_list

def _make_set_functor(settype, volatile = False):
    def make_any_set(lst, settype = settype):
        set = settype()
        for item in lst:
            set.insert(item)
        if volatile:
            set.__owner__ = STLContainer.STLOwner(lst, set,_basicRefillList)
        return set

    return make_any_set

def _basicRefillDict(cobj,dct):
    '''
    a function which is able to make dct to hold the copies
    of the elements in cobj. 
    cobj can be used for basic containers as std::map
    dct must be a python dict.
    '''
    dct.clear()
    for item in gen(cobj):
        if isinstance(item,tuple):
            dct[item[0]] = item[1]
        else:
            dct[item.first] = item.second
    

def _map_to_dict(cobj):
    d = { }
    _basicRefillDict(cobj,d)
    return d


def _make_map_functor(maptype, volatile = False):
    def make_any_map(dic, maptype = maptype):
        map = maptype()
        for item in dic.keys():
            map.insert((item,dic[item]))
        if volatile:
            map.__owner__ = STLContainer.STLOwner(dic, map,_basicRefillDict)
        return map

    return make_any_map



def _add_conversions_from_list(elementRobinType,functorByValue, functorByReference ,stlType):
    '''
    It generates conversions from a list type to a requested stlType.
    @param elementRobinType It is the RobinType of the elements which belong to the list
    @param functorByValue It is a callable which receives the list and creates
                    a copy which belongs to stlType
    @param functorByReference It is similar to 'functorByValue' but the callable
                    is called only when creating a non const reference to get results back.
    '''
    listRobinType = robin.searchOrCreateListType(elementRobinType)
    constListRobinType = robin.searchOrCreateConstType(listRobinType)
    stlType.__from__[constListRobinType] = functorByValue, 1
    stlType.__from_volatile__[listRobinType] = functorByReference, 1


def _vector_from_list(vl, el):
    el = robin.getRealElementTypeFromDictionarykey(el)
    elementRobinType = robin.robinTypeFromTempArgument(el)
    
    _add_conversions_from_list(elementRobinType, _make_vector_functor(vl,False,el),
                               _make_vector_functor(vl, True,el), vl)
    vl.__getslice__ = lambda self, fr, to: [self[i] for i in xrange(fr,to)]
    

def _list_from_list(ll, el):
    el = robin.getRealElementTypeFromDictionarykey(el)
    elementRobinType = robin.robinTypeFromTempArgument(el)
    
    _add_conversions_from_list(elementRobinType, _make_list_functor(ll,False),
                               _make_list_functor(ll, True), ll)

    ll.__len__ = lambda self: len([i for i in gen(self)])
    ll.__getitem__ = lambda self, index: [i for i in gen(self)][index]

def _set_from_list(sl, el):
    el = robin.getRealElementTypeFromDictionarykey(el)
    elementRobinType = robin.robinTypeFromTempArgument(el)
    
    _add_conversions_from_list(elementRobinType, _make_set_functor(sl,False),
                               _make_set_functor(sl, True), sl)
    
    sl.__len__ = lambda self: len([i for i in gen(self)])
    sl.__getitem__ = lambda self, index: [i for i in gen(self)][index]

def _map_from_dict(md, el):
    key = robin.getRealElementTypeFromDictionarykey(el[0])
    value = robin.getRealElementTypeFromDictionarykey(el[1])
    keyRobinType = robin.robinTypeFromTempArgument(key)
    valueRobinType = robin.robinTypeFromTempArgument(value) 
    mapRobinType = robin.RobinType(md)
    dictRobinType = robin.searchOrCreateDictType(keyRobinType,valueRobinType)
    constDictRobinType = robin.searchOrCreateConstType(dictRobinType)
    md.__from__[constDictRobinType] = _make_map_functor(md,False), 1 
    md.__from_volatile__[dictRobinType] = _make_map_functor(md, True), 1   
    
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

def _guess_container_type(contname, conttype):
    try:
        inside = conttype.__name__[(len(contname) + len("< ")):-len(" >")]
        return robin.classByName(inside)
    except KeyError:
        return None

def couple(stltype, prefix = None, element = None):
    """
    Generate automatic conversions from python types and back.
    """
    if prefix is None:
        prefix = stltype.__name__.split("<")[0]
    
    if prefix == "std::vector":
        element = element or _guess_container_type(prefix, stltype)
        _vector_from_list(stltype, element)
        stltype.__to__ = _make_list_from_vector_functor(stltype, element)
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
    elif prefix == "__gnu_cxx::hash_map":
    	_map_from_dict(stltype, element)
    	stltype.__to__ = _map_to_dict
    else:
        raise TypeError, "invalid STL prefix: " + prefix


# STL container template dictionary
class STLContainer(dict):
    """
    The STLContainer class has two objectives.
    1) It is a dictionary class which is used for instantiations of stl templated classes.
        Example: stl.vector is a STLContainer and stl.vector[int] represents std::vector< int >
    
    2) When new types are registered in robin and added to the dictionary it takes care of 
        registering all the conversions needed by that type. This means that python lists
        can be converted to C++ vectors, python dictionaries can be converted to C++ maps,
        etc.
    """
    class STLOwner:
        """
        The STlOwner class is used when a temporary C++ container is generated.
        The temporary is generated by an automatic conversion when a robin-wrapped
        function is called. The temporary is stored in the variable 'c', while the 
        original Python container is stored in variable 'l'.
        
        When the wrapped function finishes running it is time to delete 'c', but
        it is warranted that the STLOwner object will be deleted before.
        That is the best moment to update 'l' from 'c', just in case that c has been
        changed by the function.
        """
        def __init__(self, l, c, funcCopyingBack):
            """
            'l' is the python object which 'c' represents in C++.
            'funcCopyingBack' should be a function which recieves 'c' and 'l' (in that order)
            and copies the values of c back to l. I
            """
            self.l = l
            self.c = weakref.ref(c)
            self.funcCopyingBack = funcCopyingBack
            
        def __del__(self):
            self.funcCopyingBack(self.c(),self.l)
    
    def __setitem__(self, key, value):
        try:
            self.couple(value, key)
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

# __gnu_cxx::hash_map
hash_map = STLContainer()
robin.declareTemplate("__gnu_cxx::hash_map", hash_map)


# std::complex
complex = STLContainer()
robin.declareTemplate("std::complex", complex)
robin.familiarize(__builtin__.complex)

# generators
def gen(container):
    iter = container.begin()
    end = container.end()
    while not iter == end:
        yield getattr(iter, "operator*")()
        getattr(iter, "operator++")()


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

std.ostream.__from_volatile__[ofile(None)] = _ostream
std.ostream.__from_volatile__[file(None)]  = _ostream
std.istream.__from_volatile__[ifile(None)] = _istream
std.istream.__from_volatile__[file(None)]  = _istream


del os
