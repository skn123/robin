from __future__ import generators

import unittest
import re

import sourceanalysis

# stubs for abstract classes
class TemplateParameterStub(sourceanalysis.TemplateParameter): pass
class EntityStub(sourceanalysis.Entity): pass
class HintStub(sourceanalysis.Hint): pass

# stubs for exposing protected methods
class ScopeStub(sourceanalysis.Scope):
    def mirrorRelationToMember(self, *args):
        return self.super__mirrorRelationToMember(*args)

class IncompleteTemplateInstanceStub(sourceanalysis.IncompleteTemplateInstance):
    def copyInnerAggregate(self, *args):
        return self.super__copyInnerAggregate(*args)
    def copyInnerAlias(self, *args):
        return self.super__copyInnerAlias(*args)

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

    def setUp(self):
        self.entity = EntityStub()

    def tearDown(self):
        del self.entity

    def test_default_name(self):
        assert self.entity.getName() == "anonymous"

    def test_properties(self):
        _verify_collection_accessors(
            items       = [sourceanalysis.Entity.Property() for i in xrange(5)],
            add_item    = self.entity.addProperty,
            get_iterator = self.entity.propertyIterator,
        )

    def test_hints(self):
        _verify_collection_accessors(
            items        = [HintStub() for i in xrange(5)],
            add_item     = self.entity.addHint,
            get_iterator = self.entity.hintIterator,
        )

    def test_template_parameters(self):
        _verify_collection_accessors(
            items        = [TemplateParameterStub() for i in xrange(5)],
            add_item     = self.entity.addTemplateParameter,
            get_iterator = self.entity.templateParameterIterator,
        )

    def test_affiliates(self):
        affiliates = [
            sourceanalysis.FriendConnection(EntityStub(), EntityStub())
            for i in xrange(5)
        ]
        _verify_collection_accessors(
            items        = affiliates,
            add_item     = self.entity.connectToAffiliate,
            get_iterator = self.entity.affiliatesIterator,
        )

    def test_isTemplated(self):
        assert not self.entity.isTemplated()
        self.entity.addTemplateParameter(TemplateParameterStub())
        assert self.entity.isTemplated()

    def test_toString(self):
        self.entity.setName("booga")
        assert re.search("EntityStub.*\(booga\)", self.entity.toString())

    def test_lookForHint(self):
        assert self.entity.lookForHint(HintStub) is None

        hint = HintStub()
        self.entity.addHint(hint)

        assert self.entity.lookForHint(HintStub) is hint

    def test_setTemplateParameters(self):
        parameters = [TemplateParameterStub() for i in xrange(5)]
        self.entity.setTemplateParameters(_create_vector(parameters))

        for template_parameter in self.entity.templateParameterIterator():
            assert template_parameter.getContainer() is self.entity

    def test_getFullName_toplevel(self):
        self.entity.setName("Shibby")
        assert self.entity.getFullName() == "Shibby"

    def test_getFullName_contained(self):
        container = EntityStub(name="scooby")
        self.entity.connectToContainer(container, self.entity)
        self.entity.setName("doo")

        assert self.entity.getFullName() == "scooby::doo"

    # TODO: nontrivial methods that are missing tests:
    #  * setDeclarationAt, 2 overloaded variants
    #  * setDefinitionAt, 2 overloaded variants

class ScopeTests(unittest.TestCase):
    def setUp(self):
        self.scope = ScopeStub(EntityStub())

    def tearDown(self):
        del self.scope

    def test_mirrorRelationToMember_ContainedConnection(self):
        DONT_CARE = sourceanalysis.Specifiers.DONT_CARE
        container = EntityStub()
        contained = EntityStub()
        connection = sourceanalysis.ContainedConnection(
                container, DONT_CARE, DONT_CARE, DONT_CARE, contained
            )
        self.scope.mirrorRelationToMember(contained, connection)

        assert contained.getContainerConnection() is connection

    def test_mirrorRelationToMember_FriendConnection(self):
        declaring = EntityStub()
        declared = EntityStub()
        connection = sourceanalysis.FriendConnection(declaring, declared)
        self.scope.mirrorRelationToMember(declared, connection)
        assert connection in declared.affiliatesIterator()

    def test_routines_as_collection(self):
        routines = [sourceanalysis.Routine() for i in xrange(5)]
        _verify_collection_accessors(
                items         = routines,
                add_item      = lambda routine: self.scope.addMember(routine, 1, 1, 1),
                get_iterator  = self.scope.routineIterator,
                extract_value = lambda connection: connection.getContained(),
        )

    def test_fields_as_collection(self):
        fields = [sourceanalysis.Field() for i in xrange(5)]
        _verify_collection_accessors(
            items         = fields,
            add_item      = lambda field: self.scope.addMember(field, 1, 1),
            get_iterator  = self.scope.fieldIterator,
            extract_value = lambda connection: connection.getContained(),
        )

    def test_aggregates_as_collection(self):
        aggregates = [sourceanalysis.Aggregate() for i in xrange(5)]
        _verify_collection_accessors(
            items         = aggregates,
            add_item      = lambda aggregate: self.scope.addMember(aggregate,1),
            get_iterator  = self.scope.aggregateIterator,
            extract_value = lambda connection: connection.getContained(),
        )

    def test_namespaces_as_collection(self):
        namespaces = [sourceanalysis.Namespace() for i in xrange(5)]
        _verify_collection_accessors(
            items         = namespaces,
            add_item      = lambda namespace: self.scope.addMember(namespace),
            get_iterator  = self.scope.namespaceIterator,
            extract_value = lambda connection: connection.getContained(),
        )

    def test_enums_as_collection(self):
        enums = [sourceanalysis.Enum() for i in xrange(5)]
        _verify_collection_accessors(
            items         = enums,
            add_item      = lambda enum: self.scope.addMember(enum,1),
            get_iterator  = self.scope.enumIterator,
            extract_value = lambda connection: connection.getContained(),
        )

    def test_aliases_as_collection(self):
        aliases = [sourceanalysis.Alias() for i in xrange(5)]
        _verify_collection_accessors(
            items         = aliases,
            add_item      = lambda alias: self.scope.addMember(alias,1),
            get_iterator  = self.scope.aliasIterator,
            extract_value = lambda connection: connection.getContained(),
        )

    def test_groups_as_collection(self):
        groups = [sourceanalysis.Group() for i in xrange(5)]
        _verify_collection_accessors(
            items         = groups,
            add_item      = self.scope.addGroup,
            get_iterator  = self.scope.groupIterator,
            extract_value = lambda connection: connection.getContained(),
        )

    def test_friends_as_collection(self):
        friends = [EntityStub() for i in xrange(5)]
        _verify_collection_accessors(
                items         = friends,
                add_item      = self.scope.addFriend,
                get_iterator  = self.scope.friendIterator,
                extract_value = lambda connection: connection.getDeclared(),
        )

    def test_groupByName(self):
        groups = [sourceanalysis.Group(name="group-%s"%i) for i in xrange(5)]
        for group in groups:
            self.scope.addGroup(group)
        assert self.scope.groupByName("group-3") is groups[3]

    def test_groupByName_failure(self):
        groups = [sourceanalysis.Group(name="group-%s"%i) for i in xrange(5)]
        for group in groups:
            self.scope.addGroup(group)
        self.assertRaises(
                sourceanalysis.ElementNotFoundException,
                self.scope.groupByName, "no-such-group"
            )

class DataTemplateParameterTests(unittest.TestCase):
    def setUp(self):
        self.parameter = sourceanalysis.DataTemplateParameter()

    def tearDown(self):
        del self.parameter

    def test_getType(self):
        _verify_type(self, self.parameter)

    def test_getDefaultValue_no_default(self):
        assert self.parameter.getDefaultValue() is None

    def test_getDefaultValue_no_default(self):
        self.parameter.setDefault("kazoo")
        assert self.parameter.getDefaultValue().getValueString() == "kazoo"

    # TODO: nontrivial methods that are missing tests:
    # * getDefaultValue(Iterator, Iterator)

class EnumTests(unittest.TestCase):
    def test_constants_as_collection(self):
        enum = sourceanalysis.Enum()
        constants = [sourceanalysis.Enum.Constant(str(i), i) for i in xrange(5)]
        _verify_collection_accessors(
            items        = constants,
            add_item     = enum.introduceConstant,
            get_iterator = enum.constantIterator,
        )

class FieldTests(unittest.TestCase):
    def test_getType(self):
        _verify_type(self, sourceanalysis.Field())

class IncompleteTemplateInstanceTests(unittest.TestCase):
    def setUp(self):
        self.instance = IncompleteTemplateInstanceStub()

    def tearDown(self):
        del self.instance

    def _check_number(self, item, add_item):
        assert len(list(self.instance.getScope().aggregateIterator())) == 0
        add_item(item, 1)
        assert len(list(self.instance.getScope().aggregateIterator())) == 1

    def _check_scope(self, item, add_item):
        add_item(item, 1)

        connection = self.instance.getScope().aggregateIterator().next()
        assert connection.getVisibility() == 1
        assert connection.getContained().getName() == item.getName()

    def test_copyInnerAggregate_number(self):
        self._check_number(
                item = sourceanalysis.Aggregate(),
                add_item = self.instance.copyInnerAggregate,
            )

    def test_copyInnerAggregate_scope(self):
        self._check_scope(
                item = sourceanalysis.Aggregate(),
                add_item = self.instance.copyInnerAggregate,
            )

    def test_copyInnerAlias_number(self):
        self._check_number(
                item = sourceanalysis.Alias(),
                add_item = self.instance.copyInnerAlias,
            )

    def test_copyInnerAlias_scope(self):
        self._check_scope(
                item = sourceanalysis.Alias(),
                add_item = self.instance.copyInnerAlias,
            )

def _create_vector(seq):
    import java.util
    result = java.util.Vector()
    for item in seq:
        result.add(item)
    return result

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

def _verify_type(self, instance):
    """
    There is duplication in the code regarding setting and getting types.
    This function helps prevent this duplication here too.
    """
    # Check error case
    self.assertRaises(sourceanalysis.MissingInformationException, instance.getType)

    # Check success case
    type = sourceanalysis.Type(None)
    instance.setType( type )
    assert instance.getType() is type

def _all_test_cases():
    from types import ClassType
    for object in globals().values():
        if type(object) == ClassType and issubclass(object, unittest.TestCase):
            yield object

def suite():
    result = unittest.TestSuite()
    for test_case in _all_test_cases():
        result.addTest(unittest.makeSuite(test_case))
    return result

if __name__ == "__main__": unittest.main()
