import string
import weakref
import htmllib
import html.textformat
import pickle_weakref

class Indexed:

	index = 1

	def __init__(self):
		Indexed.index = Indexed.index + 1
		self.index = Indexed.index

ESC_SHADE = "\033[7m"
ESC_BOLD = "\033[1m"
ESC_NORM = "\033[0m"
TAB_WIDTH = 4

# Styles
LOW =  { 'shade': "", 'bold': "", 'norm': "" }
HIGH = { 'shade': "\033[7m", 'bold': "\033[1m", 'norm': "\033[0m" }

def setStyle(style):
	global ESC_SHADE
	global ESC_BOLD
	global ESC_NORM
	ESC_SHADE = style['shade']
	ESC_BOLD  = style['bold']
	ESC_NORM  = style['norm']

def linefill(s, width, fill):
	return s + (fill * (width - len(s)))

def string_ljust(s, width, fill = ' '):
	"""Improved string.ljust: supports multi-line strings and
	expands tabs."""
	lines = string.split(string.expandtabs(s, TAB_WIDTH), "\n")
	lines = map(lambda x, w=width, f=fill: string.ljust(x, w), lines[:-1]) +\
			[ linefill(lines[-1], width, fill) ]
	return string.join(lines, "\n")

class Document(Indexed):
	"""A single documentation block,
	generally describing a specific particle
	(class, function, method, etc.)."""

	def __init__(self, title = None):
		Indexed.__init__(self)
		self.title = title
		self.alias = None
		self.toc = [ ]
		self.sections = { }

	def newSection(self, heading, contents = None):
		if contents is None: contents = [ ]
		self.toc.append(heading)
		self.sections[heading] = contents
		self.current_section = heading

	def addText(self, text):
		"""	Concatenates text to the end of the current section."""
		try:
			sect = self.sections[self.current_section]
		except AttributeError:
			# No current section
			return

		sect.append(text)

	def multiid(self):
		if self.sections.has_key("multidoc"):
			return "".join(self.sections["multidoc"]).strip()
		else:
			return None

	def multidoc(self, id):
		up = self.up()
		hostid = id.replace("guest", "host")
		guestid = id.replace("host", "guest")
		member = lambda id: id is not None and \
				 (id.find(hostid) >= 0 or id.find(guestid) >= 0)
		all = up.findAllByCriteria(lambda x: member(x.multiid()))
		return MultiDocument(all)


	def __str__(self):
		result = "\n" + ESC_SHADE + string_ljust(self.title,75) + \
				 ESC_NORM + "\n\n"
		for heading in self.toc:
			contents = self.sections[heading]
			result = result + ESC_BOLD + heading + ESC_NORM + "\n"
			text = ""
			for piece in contents:
				text += html.textformat.html2text(piece)
			for line in string.split(text, "\n"):
				result = result + "    " + line + "\n"

		return result

	def __repr__(self):
		return str(self)


class MultiDocument(Document):

	def __init__(self, fragments):
		Document.__init__(self, title = "\n".join([fragment.title
												   for fragment in fragments]))
		for fragment in fragments:
			if fragment.multiid().find("host") >= 0:
				for heading, contents in fragment.sections.items():
					if heading != "multidoc":
						self.newSection(heading, contents)

	

class TableOfContents(Indexed):

	def __init__(self, title = ""):
		Indexed.__init__(self)
		self.title = title
		self.alias = None
		self.topics = [ ]

	def append(self, level, topic):
		self.topics.append((level, topic))
		if not hasattr(topic, 'up'):
			topic.up = weakref.ref(self)

	def find(self, topic):
		"""Locates a (sub-)topic using the _exact_ title.
		Returns the index of the topic in the topics list."""
		for i in xrange(len(self.topics)):
			level, ctopic = self.topics[i]
			if ctopic.title == topic:
				return i
			
		raise KeyError, "topic not found"

	def appendUnder(self, parent, level, topic):
		"""Adds a sub-topic to a child topic of this TOC (under 'parent').
		The new topic is added as the last entry under the specified parent
		in the list."""
		try:
			for i in xrange(self.find(parent) + 1, len(self.topics)):
				if self.topics[i][0] < level:
					break
			self.topics.insert(i, (level, topic))
		except KeyError:
			# Parent topic does not exist
			self.append(level - 1, Document(title = parent))
			self.append(level, topic)

	def __repr__(self):
		repr = "\n" + ESC_SHADE + string_ljust(self.title,75) + \
		              ESC_NORM + "\n\n"
		for level, topic in self.topics:
			indent = level * 3
			repr = repr + \
				string_ljust((" " * indent) + topic.title + " ", 70, ".") + \
				" " + str(topic.index) + "\n"

		return repr

	def __getitem__(self, name):
		"""Finds an item in the table of contents by title."""
		import types
		if type(name) == types.TupleType:
			name, depth = name[0], name[1]
		else:
			depth = 0

		if depth == 0:
			for level, topic in self.topics:
				if topic.title == name or topic.alias == name:
					return topic
		else:
			for level, topic in self.topics:
				if isinstance(topic, TableOfContents):
					found = topic[name, depth - 1]
					if found:
						return found
		return None


	def findAllByName(self, name):
		"""Finds all the subtopics (and sub-subtopics) which has the given
		'name'."""
		criteria = lambda topic: hasattr(topic,'name') and topic.name == name
		return self.findAllByCriteria(criteria)

	def findAllByTitle(self, keyword):
		"""Finds all the subtopics (and sub-subtopics) in which the
		given keyword appears in the title."""
		criteria = lambda topic: string.find(topic.title, keyword) != -1
		return self.findAllByCriteria(criteria)
		
	def findAllByCriteria(self, criteria):
		"""Finds all the subtopics (and sub-subtopics) which fulfill a
		given criteria."""
		search_results = []
		
		for level, topic in self.topics:
			# Find more topics in the subtopics
			if criteria(topic):
				more = [topic]
			elif isinstance(topic, TableOfContents):
				more = topic.findAllByCriteria(criteria)
			else:
				more = []
			# Add 'more' to the list
			search_results.extend(more)

		return search_results

	

class KeyReference(Indexed):
	"""A semi-document object containing merely a reference to some
	other document contained in the same Gopher or some other container.
	Very useful as TOC entries."""

	def __init__(self, owner = None, title = "", ref = None):
		Indexed.__init__(self)
		self.title = title
		self.owner = owner
		self.reference = ref

	def __repr__(self):
		if self.owner is None or self.reference is None:
			return "-- Missing Link --"
		else:
			return repr(self.owner.fetchDocument(self.reference))
		

class Gopher:
	"""A storage room for documents. Arranges documents by their index
	and allows fetch."""

	def __init__(self):
		self.docs = { }
		self.indexref = { }

	def identify(self, item):
		entry = [ item ]
		storage = item.parentClass()
		while storage:
			entry[:0] = [ storage ]
			storage = storage.parentClass()

		storage = entry[0].parentNSpace()
		while storage:
			entry[:0] = [ storage ]
			storage = storage.parentNSpace()

		return string.join(map(lambda x: x.name(), entry), "::")
		

	def append(self, doc, item = None):

		if item is not None:
			if type(item) == type(""):
				storage_key = item
			else:
				storage_key = self.identify(item)
			
			try:
				self.indexref[storage_key].append(doc.index)
			except KeyError:
				self.indexref[storage_key] = [ doc.index ]

		self.docs[doc.index] = doc

		return doc.index
	

	def fetchDocument(self, item):
		if type(item) == type(0):
			return self.docs[item]
		elif type(item) == type(""):
			entry = item
			keys = self.indexref[entry]
		else:
			return None
		# Got keys - generate TOC
		if len(keys) == 1: return self[keys[0]]
		toc = TableOfContents(title = entry)
		for key in keys:
			toc.append(1, self.docs[key])
		return toc

	def __getitem__(self, item):
		return self.fetchDocument(item)
	
gopher = Gopher()
