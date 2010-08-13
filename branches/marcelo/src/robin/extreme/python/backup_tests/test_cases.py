# -*- mode: python; tab-width: 4; py-indent-offset: 4; python-indent: 4 -*-

#############################################################################
# Robin & Python - test cases
#############################################################################

# Imports PyUnit.
from unittest import TestCase, main
from exceptions import Exception, ZeroDivisionError
import random

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

