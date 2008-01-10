# -*- mode: python; tab-width: 4; py-indent-offset: 4; python-indent: 4 -*-

#############################################################################
# Robin & Python - test cases
#############################################################################

# Imports PyUnit.
from unittest import TestCase, main
from exceptions import Exception, ZeroDivisionError
import random


class LanguageTest(TestCase):

	def setUp(self):
		global language
		import language

	def testPublicDataMembers1(self):
		data = 92
		u = language.DataMembers(data)
		self.assertEquals(u.square, data * data);
		self.assertEquals(type(u.e), language.El);
		self.assertEquals(u.m, language.EM);

		self.assertEquals(u.zero, 0)

	def testPublicDataMembers2(self):
		data = 26
		u = language.DataMembers(data)
		u.v.v = data
		self.assertEquals(u.v.v, data)

	def testGlobalVariable(self):
		self.assertEquals(language.global_one.square, 1.0)

	def testFloat(self):
		a = language.AssignmentOperator(0.5)
		f = a.x_factor

	def testFloatTouchup(self):
		prim = language.Primitives()
		
		for i in range(20):
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

	def testLongAndLongLong(self):
		prim = language.Primitives()
		self.assertEquals(prim.setLong(6), 2)
		self.assertEquals(prim.setLong(1l << 40), 1)
		self.assertEquals(prim.setLong(6l), 1)
		self.assertEquals(prim.setLong(6l, True), 3)
		self.assertEquals(prim.setLong(0xffffffff, True), 3)

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

	def testComplex(self):
		uc = language.StandardLibrary.UsingComplex()
		self.assertEquals(uc.pivot(), 1+1j)
		uc.append(5,1j)
		uc.append(5+1j, 1+5j)

	def testVectorOfComplex(self):
		uc = language.StandardLibrary.UsingComplex()
		qubits = [(6j+1,4+3j), (5, 2j)]
		tensor = [(5+30j), (-12+2j), (20+15j), (-6+8j)]
		for q in qubits: uc.append(*q)
		self.assertEquals(uc.tensor(), tensor)

	def testPointerPrimitives(self):
		import robin
		a = robin.pointer(language.Pointers.InnerStruct)
		language.Pointers.pointerToPointer(a)
		sst = robin.dereference(a)
		self.assertEquals(sst.member, 97)

	def testClassTypeConsistency(self):
		import pprint, StringIO
		p = language.Pointers()
		s = StringIO.StringIO()
		pprint.pprint(p, stream=s)
		self.assertEquals(s.getvalue(), "%r\n" % p)

	def testTypedefs(self):
		c = language.Typedefs();
		c.setUint(10);
		c.setMyDouble(4.5);
		c.setMyQuintuple(4.5);

		self.assertEquals(c.getUint(), 10);
		self.assertEquals("%.3f" % c.getMyDouble(), "%.3f" % 4.5)
		self.assertEquals("%.3f" % c.getMyQuintuple(), "%.3f" % 4.5)
	
	def testPublicDoubleAssignment(self):
		c = language.PublicDouble();
		c.foo = 10.5
		c.floatfoo = 10.5
		self.assertEquals("%.3f" % c.foo, "%.3f" % 10.5)
		self.assertEquals("%.3f" % c.floatfoo, "%.3f" % 10.5)


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

    def testNestedTemplateArgs(self):
        t = templates.NestedTemplate();
        self.assertEquals(t.method(), 1);



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


class InheritanceTest(TestCase):

	def setUp(self):
		global stl
		import stl

	def testExtendingViaDerivation(self):
		class MyString(stl.string):
			def thrice(self):
				return self.delim.join([str(self)] * 3)

		token = "TOKEN"
		ms = MyString(token)
		ms.delim = " "
		self.assertEquals(str(ms), token)
		self.assertEquals(ms.size(), len(token))
		self.assertEquals(ms.thrice(), " ".join([token] * 3))

	def testOverrideCtor(self):
		class MyString(stl.string):
			def __init__(self, count):
				stl.string.__init__(self, "A" * count)
		self.assertEquals(str(MyString(9)), "AAAAAAAAA")
		# - check the special case where there is an error in __init__
		try:
			t = MyString("H")
			self.fail('previous line should have caused an exception')
		except TypeError:
			pass

	def testDtor(self):
		class MyString(stl.string):
			def __del__(self):
				vars["counter"] -= 1
		t = MyString("H")
		vars = {"counter": 1}
		del t
		self.assertEquals(vars["counter"], 0)

	def testObjectCleanup(self):
		import inheritance
		vars = {"counter": 0}
		class MyFunctor(inheritance.IFunctor):
			def __init__(self):
				vars["counter"] += 1
			def __del__(self):
				vars["counter"] -= 1

		objs = [MyFunctor() for i in xrange(200)]
		self.assertEquals(vars["counter"], 200)
		del objs
		self.assertEquals(vars["counter"], 0)

	def testInterceptors(self):
		import inheritance, robin
		class MyFunctor(inheritance.IFunctor):
			def operate(self, string, index):
				q = str(string)*index
				return robin.disown(stl.string(q))

		f = MyFunctor()
		elements = ["Aaron", "Mike", "Joseph"]
		melements = inheritance.mapper(elements, f)
		self.assertEquals(len(elements), len(melements))
		for i in xrange(len(elements)):
			self.assertEquals(elements[i] * i, melements[i])

        self.assertEquals(reduce(inheritance.mul, [f,f], 9), 9.0)

    def testInterceptorsNonPure1(self):
        import inheritance, robin
        class Functor(inheritance.IFunctor):
            pass
        class MyFunctor(inheritance.IFunctor):
            def __init__(self, fac):
                self.fac = fac
            def factor(self):
                return self.fac

        fs = [MyFunctor(0.25), Functor(), MyFunctor(0.5)]
        self.assertEquals(reduce(inheritance.mul, fs, 2), 0.25)

    def testInterceptorsNonPure2(self):
        import inheritance, robin
        class FunctorImpl(inheritance.IFunctorImpl):
            pass
        class MyFunctorImpl(inheritance.IFunctorImpl):
            def operate(self, s, n):
                return robin.disown(stl.string(str(s) + str(n)))

        fi = FunctorImpl()
        mfi = MyFunctorImpl()
        elements = ["Elaine", "John", "Theodore"]
        melements1 = inheritance.mapper(elements, fi)
        melements2 = inheritance.mapper(elements, mfi)
        self.assertEquals(melements1, elements)
        self.assertEquals(melements2, 
            map(lambda x,y:x+str(y), elements, range(len(elements))))

    def testInterceptorsForNonPure(self):
        import inheritance
        class MoreTainted(inheritance.ITaintedVirtual):
            def __init__(self, taintParam):
                inheritance.ITaintedVirtual.__init__(self, taintParam)
                self.pythonTaint = 5

            def returnTaint(self):
                return inheritance.TaintedVirtual.returnTaint(self) * 2

            def pythonIsPure(self):
                return 10;

        class Untainter(MoreTainted):
            def __init__(self):
                inheritance.ITaintedVirtual.__init__(self, 0)

            def returnTaint(self):
                return -1

            def returnRealTaint(self):
                return MoreTainted.returnTaint(self)

            def returnFilth(self):
                return 0

        class EvenMoreTainted(MoreTainted):
            def __init__(self):
                MoreTainted.__init__(self,500)


        
        mt = MoreTainted(5)
        self.assertEquals(mt.returnTaint(), 10)
        self.assertEquals(mt.returnFilth(), -1)
        self.assertEquals(mt.pythonIsPure(), 10)
        self.assertEquals(mt.pythonTaint,5)
        
        umt = Untainter()
        self.assertEquals(umt.returnTaint(),-1)
        self.assertEquals(umt.returnRealTaint(),0)
        self.assertEquals(umt.returnFilth(),0)
        self.assertEquals(umt.pythonIsPure(),10)
        try:
            i = umt.pythonTaint
            self.fail()
        except:
            pass


        emt = EvenMoreTainted()
        self.assertEquals(emt.pythonTaint,5)
        self.assertEquals(emt.returnTaint(), 1000)
        self.assertEquals(emt.pythonIsPure(), 10)
        self.assertEquals(emt.returnFilth(), -1)


class HintsTest(TestCase):

	def testClue(self):
		import hints
		c = hints.Clue()
		self.assertEquals(c.get(), 0)
		self.assertEquals(c.gets().value, 1)
	

class AutocollectTest(TestCase):

	def testThatAllTheClassesExist(self):
		import autocollect
		autocollect.CollectMe1()
		autocollect.CollectMe3()
		autocollect.CollectMe3.CollectMe4()
	
	def testThatAllTheTemplatesExist(self):
		import autocollect, robin
		autocollect.CollectMe2[int]()
		autocollect.CollectMe2[long]()
		autocollect.CollectMe2[robin.char]()
		autocollect.CollectMe2[autocollect.CollectMe3.CollectMe4]()

	def testThatAllTheTypedefsExist(self):
		import autocollect
		autocollect.CollectMe2A()
		autocollect.CollectMe2B()
		autocollect.CollectMe3.CollectMe2C()


class MemoryManagementTest(TestCase):

	def setUp(self):
		global memprof
		import memprof

	def testBorrowed(self):
		self.assertEquals(memprof.getCounter(), 0)
		c = memprof.ConsumptionUnit()
		d = c.meAgain()
		del d
		del c
		self.assertEquals(memprof.getCounter(), 0)

	def testKeepAlive(self):
		self.assertEquals(memprof.getCounter(), 0)
		c = memprof.ConsumptionUnit()
		d = c.me()
		self.assertEquals(memprof.getCounter(), 1)
		del c
		self.assertEquals(memprof.getCounter(), 1)
		del d
		self.assertEquals(memprof.getCounter(), 0)
		c = memprof.ConsumptionUnit()
		d = c.meAgain()
		del c
		self.assertEquals(memprof.getCounter(), 1)
		del d

	def testNotKeepAlive(self):
		self.assertEquals(memprof.getCounter(), 0)
		c = memprof.ConsumptionUnit()
		d = c.notMe()
		self.assertEquals(memprof.getCounter(), 2)
		del c
		self.assertEquals(memprof.getCounter(), 1)
		del d
		self.assertEquals(memprof.getCounter(), 0)


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


class KwargsTest(TestCase):
    
    def setUp(self):
        import language
        self.k = language.KwClass()
    def testSimpleKwargs(self):
        self.k.setMembers(a=1,b=2)
        self.failUnless(self.k.m_a == 1)
        self.failUnless(self.k.m_b == 2)

    def testTransposedKwargs(self):
        self.k.setMembers(b=2,a=1)
        self.failUnless(self.k.m_a == 1)
        self.failUnless(self.k.m_b == 2)

    def testKwargsWithTransposedRealArgs(self):
        self.k.setMembers2(a=1,b=2)
        self.failUnless(self.k.m_a == 1)
        self.failUnless(self.k.m_b == 2)

    def testKwargsWithArgs(self):
        self.k.setMembersWithExtraArgs(10,b=2,a=1)
        self.failUnless(self.k.m_a == 1)
        self.failUnless(self.k.m_b == 2)
        self.failUnless(self.k.m_dummy == 10)

    def testKwargsWithArgs2(self):
        self.k.setMembersWithExtraArgs(10,1,b=2)
        self.failUnless(self.k.m_a == 1)
        self.failUnless(self.k.m_b == 2)
        self.failUnless(self.k.m_dummy == 10)

    def testBadKwargsNonLast(self):
        try:
            self.k.setMembers(1,a=1)
            self.fail()
        except:
            pass

        	

# If the tests are run in debug mode, don't include xParam tests which are for release
import os
if os.environ.has_key("ROBIN_DEBUG"):
    del xParamTest
    del ZCompanionTest
del os

# If the file was executed as it is, run the tests.
if __name__ == '__main__':
	main()
