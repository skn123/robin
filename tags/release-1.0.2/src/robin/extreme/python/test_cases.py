#############################################################################
# Robin & Python - test cases
#############################################################################

# Imports PyUnit.
from unittest import *
from exceptions import Exception, ZeroDivisionError
import random


class LanguageTest(TestCase):

	def setUp(self):
		global language
		import language

	def testPublicDataMembers(self):
		data = 92
		u = language.DataMembers(data)
		self.assertEquals(u.square, data * data);
		self.assertEquals(type(u.e), language.El);
		self.assertEquals(u.m, language.EM);

		self.assertEquals(u.zero, 0)

	def testFloat(self):
		a = language.AssignmentOperator(0.5)
		f = a.x_factor

	def testFloatTouchup(self):
		prim = language.Primitives()
		
		for i in range(10):
			f = prim.getAFloat()
			s = prim.getAStringFloat()
			fs = "%.3f" % f
			self.assertEquals(s, fs)

		floats = list(prim.getManyFloats())
		sfloats = list(prim.getManyStringFloats())
		for i in range(len(floats)):
			fs = "%.3f" % floats[i]
			self.assertEquals(sfloats[i], fs)

	def testDoubleTouchup(self):
		prim = language.Primitives()

		for i in range(10):
			d = prim.getADouble()
			dr = prim.getADoubleRef()
			s = prim.getAStringDouble()
			ds = "%.3f" % d
			drs = "%.3f" % dr
			self.assertEquals(s, ds)
			self.assertEquals(s, drs)

	def testPrivateDataMembers(self):
		try:
			data = 92
			u = language.DataMembers(data)
			k = u.hidden
			self.failUnless(0, "AttributeError not thrown")
		except AttributeError:
			pass # OK!

	def testErroneousConversion(self):
		"""testErroneousConversion - Check Python conversions with error
		awareness."""
		# This conversion generates a Python error when x==0
		language.Conversions.Edge.__from__[0] = \
							  lambda x: [100/x, language.Conversions.Edge()][1]
		cnv = language.Conversions()
		cnv.apply(3)
		try:
			cnv.apply(0)
		except RuntimeError, value:
			self.failUnless(str(value).find(ZeroDivisionError.__name__) >= 0)

	def testConversionPriorities(self):
		"""testConversionPriorities - Verify that the priorities of certain
		conversions are higher than others, specifically that std::string
		conversions are higher than char*."""
		# Construct the class with a string, and verify that the std::string
		# constructor was used, and not char*
		conv = language.StandardLibrary.UsingStringConversions("test")
		self.failUnless(conv.getConversionType() == 1,
						"char* conversion favoured over std::string")

	def testVectors(self):
		import stl
		ve =  language.StandardLibrary.UsingVectors([])
		vl =  language.StandardLibrary.UsingVectors([1,2,3])
		vd =  language.StandardLibrary.UsingVectors([1.0,2.0,3.0])
		vs =  language.StandardLibrary.UsingVectors(["one","two","three"])
		vll = language.StandardLibrary.UsingVectors([1l,2l,5l])
		self.assertEquals(vl.getVectorType(), 1,
						  "vector<double> favoured over vector<long>")
		self.assertEquals(vd.getVectorType(), 0,
						  "vector<long> favoured over vector<double>")
		self.assertEquals(vs.getVectorType(), 2,
						  "vector<string> not favoured over others")
		self.assertEquals(vll.getVectorType(), 3,
						  "vector<long long> not favoured over others")

	def testVectorOfChar(self):
		import stl
		ve =  language.StandardLibrary.UsingVectors([])
		ve.atof(['0','.','5','\0'])
		self.assertEquals([0.5], ve.get())
		
	def testExceptions(self):
		try:
			e = language.Exceptions()
			e.cry()
			self.failUnless(0, "exception not thrown by cry()")
		except RuntimeError:
			pass

	def testInners(self):
		a = language.Inners()
		b = language.Inners.StructIn()
		c = language.Inners.StructIn.StructInStruct()

	def testStaticMethods(self):
		language.Inners().staticMethodIn()
		language.Inners.staticMethodIn()
	
	def testVolatileVector(self):
		l = range(10)
		lorig = l[:]
		language.StandardLibrary.UsingVectors.modifyVectorInPlace(l)
		for i in xrange(len(l)):
			self.failUnless(l[i] == lorig[i] * 2)

	def testShorts(self):
		p = language.Primitives()
		p.shorten(5)
		p.ushorten(5)

	def testEnums(self):
		self.failUnless(type(language.EM) is language.Em);
		self.assertEquals(type(language.EM), language.Em);
		language.Em(1)

	def testDynamicCast(self):
		ii = language.NonAbstract.factorize()
		ii_a = language.dynamic_cast[language.Abstract](ii)
		ii_na = language.dynamic_cast[language.NonAbstract](ii_a)
		ii_na.abstraction()


class ThreadingTest(TestCase):

	def setUp(self):
		global threads
		import threads

	def testMultipleThreads(self):
		"""testMultipleThreads - Check multi-threaded operation of wrapped
		functions."""

		# Use the two functions in ThreadList to add 2 different characters
		# to the same stream, and checking if it is spread out equally

		import thread
		import threading
		import stl
		
		tl = threads.ThreadList()

		def writeChar(c, times, e):
			tl.writeChar(c, times)
			e.set()

		threadCount = 2
		charAmount  = 100

		e = [threading.Event() for i in range(threadCount)]
		for i in range(threadCount):
			thread.start_new_thread(writeChar, (str(i),charAmount, e[i]))
		for ei in e:
			ei.wait()

		# check if all the 'a's are in the begining, and if they are fail
		# the test
		s = tl.getString()
		for i in xrange(len(s)):
			if s[i] != "0":
				self.failUnless(i < (len(s) / 2))
				return
		


class TemplateTest(TestCase):

	def setUp(self):
		global templates
		import templates

	def testInnerTypedefs(self):
		holder = templates.Integer()
		templates.Integer.__not__ = "operator int"
		o = ~holder
		oo = holder.increment()
		self.assertEquals(o+1, oo)

	def testPublicDataMembers(self):
		fac1 = 792; fac2 = 1947
		m = templates.IntegerMult(fac1, fac2)
		self.assertEquals(m.result, fac1 * fac2)

	def testSupervised(self):
		carrier = templates.Carrier()
		self.assertEquals( carrier.kints, [666] )

	def testInheritance(self):
		pass

	def testDerivedFromVector(self):
		length = 12
		b5 = templates.Barrier5Vec()
		b5 = b5.barriers(length)
		self.assertEquals( len(b5), length )

	def testAliasing(self):
		sb = templates.Less.SoundBarrier()
		m = sb.squarer()
		self.assertEquals(m.result, templates.More.SOUND ** 2)

	def testTraits(self):
		n_iterations = 6
		first = 2
		
		it = templates.IntegerIterator()
		values = [it.nextValue() for i in xrange(n_iterations)]
		intvalues = [getattr(x,"operator int")() for x in values]
		self.assertEquals(intvalues, range(first, first + n_iterations))

	def testTemplateObject(self):
		"""Tests the functionality of robin.declareTemplate."""
		class TemplateObject(dict):
			def __setitem__(self, key, value):
				dict.__setitem__(self, key.__name__, 1)
				
		import robin
		t = TemplateObject()
		robin.declareTemplate("Less::Yets", t)
		self.assertEquals(t, {'Carrier':1})


class ProtocolsTest(TestCase):

	def setUp(self):
		global protocols
		import protocols

	def testComparison(self):
		"""testComparison - checks comparison operators"""
		Times = protocols.Times
		for repeat_count in xrange(700):
			# Choose two random values
			v1 = random.choice(xrange(25))
			v2 = random.choice(xrange(25))
			# Create Times instances
			t1 = Times(v1)
			t2 = Times(v2)
			# Commit comparison
			self.assertEquals(v1 == v2, t1 == t2)
			self.assertEquals(v1 != v2, t1 != t2)
			self.assertEquals(v1 < v2, t1 < t2)
			self.assertEquals(v1 > v2, t1 > t2)

	def testConversionToInt(self):
		"""testConversionToInt - checks the __int__ protocol"""
		Times = protocols.Times
		arg = 77
		t1 = Times(arg)
		self.assertEquals(int(t1), arg)

	def testArithmetics(self):
		"""testArithmetics - checks arithmetic operators"""
		Times = protocols.Times
		arg0, arg1 = 19, 6

		Times.__mul__ = "combine"
		Times.__rmul__ = "mul"
		times0, times1 = Times(arg0), Times(arg1)
		prod_times_int = times0 * arg1
		prod_int_times = arg0 * times1
		self.failUnless(isinstance(prod_int_times, int))
		self.failUnless(isinstance(prod_times_int, Times))
		self.assertEquals(int(prod_times_int), arg0 * arg1)
		self.assertEquals(int(prod_int_times), arg0 * arg1)


class MemoryManagementTest(TestCase):

	def setUp(self):
		global memprof
		import memprof

	def testBorrowed(self):
		c = memprof.ConsumptionUnit()
		d = c.meAgain()
		del d
		del c


class ZCompanionTest(TestCase):

	def setUp(self):
		import site
		global companion
		import companion

	def testNumericalPython(self):
		"""testNumericalPython - checks integration with numpy"""
		import Numeric
		import numpy

		MAX_VALUE = 30
		MAX_LENGTH = 120

		sz = random.choice(xrange(MAX_LENGTH))
		l = [random.choice(xrange(MAX_VALUE)) for x in xrange(sz)]
		narray = Numeric.array(l)
		mt = companion.ManyTimes()

		sum = mt.sum_1D(narray)
		self.assertEquals(sum, reduce(lambda x,y: x+y, l))
		zeros = mt.new_1D(sz)
	

class xParamTest(TestCase):

	def testxParse(self):
		import language
		import with_xparam
		import xparam

		arg = 71
		dm = xparam.xParse("DataMembers(%i)" % arg)
		self.assertEquals(type(dm), language.DataMembers)
		self.assertEquals(dm.square, arg * arg)

	def testSaver(self):
		import language
		import with_xparam
		import xparam
		import StringIO

		arg = 71
		dm = language.DataMembers(arg)
		out = StringIO.StringIO()
		xparam.Saver(out).save(dm)
		self.assertEquals(out.getvalue().strip(),
						  "DataMembers(%i)" % (arg * arg))
		

class DocumentationTest(TestCase):

	def testFindDatafile(self):
		import robinhelp
		# Search for a plain standard module in sys.path (this way we do not
		# have to create too many files)
		ospyfile = robinhelp._find_datafile("os", [], FILE_EXTS = [".py"])
		self.failUnless(ospyfile.name.endswith("/os.py"))
		# Search for John Smith
		johnsmithfile = robinhelp._find_datafile("smith.john")
		self.failUnless(johnsmithfile.read().startswith("MAGIC-PREFACE\n"))
	

# If the tests are run in debug mode, don't include xParam tests which are for release
import os
if os.environ.has_key("ROBIN_DEBUG"):
    del xParamTest
    del ZCompanionTest
del os

# If the file was executed as it is, run the tests.
if __name__ == '__main__':
	main()
