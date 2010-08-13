import robin
import document


class Documentation:
	gopher = { }
	loaded = { }
	history = []
	alias_index = { }


def _find_datafile(fileid, extpath = [], FILE_EXTS = [".pyc", ".robindoc"]):
	import re, sys
	def filenames():
		return [fileid.split(".")[-1] + ext for ext in FILE_EXTS]
	if "." not in fileid:
		return _search_on_path(sys.path + extpath, filenames())
	else:
		subpath = _find_submodule_name(re.sub(r"\.[^.]*$", "", fileid))
		return _search_on_path([subpath], filenames())
	
def _search_on_path(paths, filenames):
	import os
	for path in paths:
		for filename in filenames:
			candidate = os.path.join(path, filename)
			if os.path.exists(candidate): return open(candidate, "rb")
	raise IOError, "can't find any of the following files: %s" % filenames
    
def _find_submodule_name(modulename):
	return _find_submodule(modulename)[1]

def _find_submodule(modulename):
	import imp
	names = modulename.split(".")
	package = imp.find_module(names[0])
	for submodule in names[1:]:
		package = imp.find_module(submodule, [package[1]])

	return package

_dummyGopher = document.Gopher()
_dummyGopher.toc = document.TableOfContents()


def loadModuleDocumentation(module):
	"""loadModuleDocumentation(module) -> Gopher

	Attempts to load a Gopher documentation object for that module. The
	dictionary at Documentation.gopher is used to determine where to load
	the module from. A string entry there specifies a Python module path - in
	that case, loadModuleDocumentation will open the file and read its contents
	using cPickle."""
	def loadFromFile(module):
		docat = Documentation.gopher[module]
		if Documentation.loaded.has_key(docat):
			# already loaded
			Documentation.gopher[module] = Documentation.loaded[docat]
		else:
			import cPickle, os.path
			moduledir = robin.here(module.__file__)
			docdb = _find_datafile(Documentation.gopher[module], [moduledir])
			Documentation.gopher[module] = Documentation.loaded[docat] \
				  = cPickle.load(docdb)

	while module.__name__.endswith(".template_classes_robin") \
			and not Documentation.gopher.has_key(module):
		import sys
		module = sys.modules[module.__name__[:-len(".template_classes_robin")]]
		del sys

	# Load the documentation
	try:
		if not Documentation.gopher.has_key(module):
			# - cannot find documentation for module
			raise KeyError, "no association"
		
		if type(Documentation.gopher[module]) is str:
			loadFromFile(module)

		return Documentation.gopher[module]
	except:
		import warnings
		warnings.warn("Robin: no documentation loaded for module " + \
					  module.__name__)
		return _dummyGopher


import site

class Helper(site._Helper):

	def getModuleTOC(self, module):
		gopher = loadModuleDocumentation(module)
		toc = gopher.toc
		searchLevel = self._getSearchLevel(toc)

		if searchLevel > 0:
			subtoc = toc.findAllByName(module.__name__)
			if subtoc:
				toc = subtoc[0]
		return gopher, toc

	def getClassTOC(self, klass):
		import sys
		module = sys.modules[klass.__module__]
		gopher = loadModuleDocumentation(module)
		searchLevel = self._getSearchLevel(gopher.toc)
		page = gopher.toc[self._classname(klass),searchLevel]
		return gopher, page

	def getFunctionTOC(self, func):
		import sys
		module = sys.modules[func.__module__]
		gopher, toc = self.getModuleTOC(module)
		globaltoc = toc["Global Functions"]
		search = toc.findAllByName(self._classname(func))
		# Construct the result page
		if len(search) == 1:
			page = search[0]
		else:
			page = document.TableOfContents(func.__name__)
			page.topics = [(1,x) for x in search]
		return gopher, page

	def isKeyModule(self, obj):
		"""Checks whether 'obj' is a key of Documentation.gopher."""
		try:
			return Documentation.gopher.has_key(obj)
		except TypeError:
			# unhashable type
			return 0
		

	def __call__(self, obj):
		if type(obj) is int:
			# This is an index to the last documentation gopher used
			if not Documentation.history:
				raise IndexError, "no current help page"
			else:
				gopher = Documentation.history[-1].__gopher__
				page = gopher[obj]
		elif self.isKeyModule(obj):
			# This is a Robin module
			gopher, page = self.getModuleTOC(obj)
		elif isinstance(obj, robin.ClassType):
			# This is a class of the new model.
			gopher, page = self.getClassTOC(obj)
		elif isinstance(type(obj), robin.ClassType):
			# This is a class instance of the new model.
			gopher, page = self.getClassTOC(type(obj))
		elif type(obj) is robin.FunctionType and obj.__self__ is None:
			# This is a Robin wrapped global function
			gopher, page = self.getFunctionTOC(obj)
		elif type(obj) is robin.FunctionType and \
			 isinstance(type(obj.__self__), robin.ClassType):
			# This is a Robin wrapped method
			bclass = type(obj.__self__)
			gopher, page = self.getClassTOC(bclass)
			search = page.findAllByName(obj.__name__)
			if len(search) == 1:
				page = search[0]
			else:
				mintoc = document.TableOfContents(self._classname(bclass) + \
												  " / " + obj.__name__)
				mintoc.topics = map(lambda x: (1,x), search)
				page = mintoc
		else:
			# Fall back to Python help
			return site._Helper.__call__(self, obj)

		if isinstance(page, document.Document):
			multi = page.multiid()
			if multi: page = page.multidoc(multi)
		if page is not None:
			page.__gopher__ = gopher
			Documentation.history.append(page)
		return page


	def _getSearchLevel(self, toc):
		try:
		    return toc.searchLevel
		except AttributeError:
		    return 0

	def _classname(self, klass):
		# - strip away template arguments 
		bal = 0
		s = ""
		for c in klass.__name__:
			if c == "<": bal += 1
			elif c == ">": bal -= 1
			elif bal == 0: s += c
		# - extract short name
		return s.split("::")[-1]
	
import sys
sys.modules["__main__"].help = Helper()
del sys
