from __future__ import generators

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

    def test_basesIterator_several(self):
        _verify_collection_accessors(
            items        = [sourceanalysis.Aggregate() for i in xrange(5)],
            add_item     = lambda base: self.aggregate.addBase(base, 1),
            get_iterator = self.aggregate.baseIterator,

            # the bases collection actually contains connections to the bases,
            # not the bases themselves
            extract_value = lambda connection: connection.getBase(),
        )

class EntityPropertyTests(unittest.TestCase):
    def test_isConcealed_true(self):
        assert sourceanalysis.Entity.Property(".foo", "").isConcealed()

    def test_isConcealed_false(self):
        assert not sourceanalysis.Entity.Property("foo", "").isConcealed()

    def test_default_name(self):
        assert sourceanalysis.Entity.Property().getName() == "anonymous"

    def test_default_value(self):
        assert sourceanalysis.Entity.Property().getValue() == ""

class EntityTests(unittest.TestCase):
    class EntityStub(sourceanalysis.Entity): pass

    def setUp(self):
        self.entity = self.EntityStub()

    def tearDown(self):
        del self.entity

    def test_default_name(self):
        assert self.entity.getName() == "anonymous"

    def test_add_and_get_properties(self):
        _verify_collection_accessors(
            items       = [sourceanalysis.Entity.Property() for i in xrange(5)],
            add_item    = self.entity.addProperty,
            get_iterator = self.entity.propertyIterator,
        )

def _verify_collection_accessors(
            items, add_item, get_iterator, extract_value = lambda x:x
        ):
    """
    Verifies a collection interface a class exposes, for example 'addFoo', and
    'fooIterator'.

    @param items: the items to enter in the collection
    @param add_item: a callable that adds an item (usually a bound method, e.g.
                     x.addFoo)
    @param get_iterator: a callable that returns a java.lang.Iterator or a
                         Python iterator, usually a bound method (e.g.
                         x.fooIterator)
    @param extract_value: an optional parameter, used when the collection
                          doesn't store the actual items but some wrapper
                          around it. The callable should receive whatever the
                          collection _does_ store as a parameter, and return
                          the original item.
    """
    # verify empty at start
    assert not get_iterator().hasNext()

    # TODO - assumes same order, which is not necessarily so...
    for item in items:
        add_item(item)

    actual_items = list(get_iterator())

    assert len(actual_items) == len(items)
    for expected_item, actual_item in zip(items, actual_items):
        value = extract_value(actual_item)
        assert expected_item is value

def _all_test_cases():
    from types import ClassType
    for name, object in globals().items():
        if type(object) == ClassType and issubclass(object, unittest.TestCase):
            yield object

def suite():
    result = unittest.TestSuite()
    for test_case in _all_test_cases():
        result.addTest(unittest.makeSuite(test_case))
    return result

if __name__ == "__main__": unittest.main()
