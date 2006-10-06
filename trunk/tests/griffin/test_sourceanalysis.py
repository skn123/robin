import unittest
import sourceanalysis

class AggregateTests(unittest.TestCase):
    def setUp(self):
        self.aggregate = sourceanalysis.Aggregate()

    def tearDown(self):
        del self.aggregate

    def test_addBase_retval(self):
        new_base = sourceanalysis.Aggregate()
        visibility = 2

        connection = self.aggregate.addBase(new_base, visibility)

        assert visibility == connection.getVisibility()
        assert connection.getBase() is new_base

    def test_basesIterator_empty(self):
        assert not self.aggregate.baseIterator().hasNext()

    def test_basesIterator_several(self):
        bases = [sourceanalysis.Aggregate() for i in xrange(5)]
        for base in bases:
            self.aggregate.addBase(base, 1)

        for expected_base, connection in zip(bases,self.aggregate.baseIterator()):
            assert connection.getBase() is expected_base


def suite():
    return unittest.makeSuite(AggregateTests)

if __name__ == "__main__": unittest.main()
