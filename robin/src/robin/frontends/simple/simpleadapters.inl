// -*- mode: C++; tab-width: 4; c-basic-offset: 4 -*-

/**
 * @file
 *
 * @par SOURCE
 * frontends/simple/simpleadapters.inl
 *
 * @par PACKAGE
 * Robin
 */


namespace Robin {


/**
 * Takes the 'value' field of the concrete element
 * type and pushes it into the arguments buffer.
 */
template < class CType, class SimpleType >
void SimpleAdapter<CType,SimpleType>::put(ArgumentsBuffer& argsbuf,
										  scripting_element value)
{
	Simple::Element *element = (Simple::Element *)value;
	SimpleType *concrete = dynamic_cast<SimpleType *>(element);
	argsbuf.push(concrete->value);
}

/**
 * Builds a new Simple::Element from the value,
 * which is interpreted as a 'CType'.
 */
template < class CType, class SimpleType >
scripting_element SimpleAdapter<CType,SimpleType>::get(basic_block data)
{
	Simple::Element *element = Simple::build(*reinterpret_cast<CType*>(&data));
	return (scripting_element)element;
}



/**
 * Takes the 'value' field of the concrete element
 * type and pushes it into the arguments buffer.
 */
template < class CType, class SimpleType >
void SimpleAdapterWithCast<CType,SimpleType>::put(ArgumentsBuffer& argsbuf,
												  scripting_element value)
{
	Simple::Element *element = (Simple::Element *)value;
	SimpleType *concrete = dynamic_cast<SimpleType *>(element);
	argsbuf.push(CType(concrete->value));
}

/**
 * Builds a new Simple::Element from the value,
 * which is interpreted as a 'CType'.
 */
template < class CType, class SimpleType >
scripting_element SimpleAdapterWithCast<CType,SimpleType>::get(basic_block
															   data)
{
	Simple::Element *element = Simple::buildElement<SimpleType>((CType)data);
	return (scripting_element)element;
}



/**
 * Takes the 'value' field of the concrete element
 * type and pushes its address into the arguments buffer.
 */
template < class CType, class SimpleType >
void SimpleLargeAdapter<CType,SimpleType>::put(ArgumentsBuffer& argsbuf,
											   scripting_element value)
{
	Simple::Element *element = (Simple::Element *)value;
	SimpleType *concrete = dynamic_cast<SimpleType *>(element);
	argsbuf.push(&(concrete->value));
}

/**
 * Builds a new Simple::Element from the value,
 * which is interpreted as a pointer to 'CType'.
 * The original value is deleted after the conversion.
 */
template < class CType, class SimpleType >
scripting_element SimpleLargeAdapter<CType,SimpleType>::get(basic_block data)
{
	CType *pvalue = (CType*)data;
	Simple::Element *element = Simple::build(*pvalue);
	delete pvalue;
	return (scripting_element)element;
}



} // end of namespace Robin
