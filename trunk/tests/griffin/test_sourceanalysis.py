import unittest
import sourceanalysis

class AggregateTests(unittest.TestCase):
    def test_addBase(self):
        aggregate = sourceanalysis.Aggregate()
        new_base = sourceanalysis.Aggregate()
        visibility = 2

        connection = aggregate.addBase(new_base, visibility)

        self.assertEqual(visibility, connection.getVisibility())
        assert connection.getBase() is new_base


def suite():
    return unittest.makeSuite(AggregateTests)

if __name__ == "__main__": unittest.main()
