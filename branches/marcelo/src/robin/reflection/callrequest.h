/*
 * callrequest.h
 *
 *  Created on: Oct 4, 2009
 *  Author: Marcelo Taube
 */

#ifndef CALLREQUEST_H_
#define CALLREQUEST_H_
#include "callable.h"
#include "robintype.h"

namespace Robin {


class OverloadedSet;
typedef std::map<std::string, size_t> ArgumentIndices;

/**
 * CallRequest represents a try to call a function or a method.  It is similar to a
 * signature of a function in the sense that stores the types of the parameters, but
 * in this case it stores the type of the actual parameters passed to the function
 * and not the formal types specified in the signature.
 * Notice that a function call might fall so the types not always correspond to
 * a valid signature. Even if the call does not fail type conversions might be applied
 * which enables the use to call a function with different types that those it was
 * designed for.
 * The call can have parameters passed by position and parameters passed by name, like
 * in the following example:
 * 	>>> foo(2 , [], def=5, abc="sss" )
 * The types of the parameters in this example are int, list, int and string. They are
 * stored in the vector m_types (types used in this example are not necessary the
 * real types).
 * The types  are stored in a certain order. For parameters passed by position,
 * the order has to be the natural one (position 0 for the first parameter, position 1 for
 * the second, etc)
 * Parameters passed by name are called keyword parameters
 * All the keyword parameters should come after the regular parameters in both arrays.
 * The order of the keyword parameters has to be the same in both arrays and there is a
 * third parameter which specifies the names and the positions of the indexes.
 * In any case the order of the keyword parameters is lexicographic by param name.
 *
 * For the last shown example the fields in this class should be:
 * 	m_types       : [ int, list, int and string ]
 *  m_kwargorder  : {  abc : 2 , def : 3 }
 *
 */
class CallRequest
{

public:
	  CallRequest(
			const ActualArgumentList& args,
			const KeywordArgumentMap &kwargs,
			const OverloadedSet *calledFunc);


	  /**
	   * Return a number which is sensitive to the contents of the CallRequest.
	   * Warranted to be the same if the contents are the same.
	   */
	  size_t hashify() const;


	  /**
	   * It defines an order between all the request, to be used in trees
	   * like std::map
	   * Notice that operator< will be dependant on the order of the parameters,
	   * so it is important to always use the same order when creating calls.
	   */
	  bool operator<(const CallRequest &other) const;

	  /**
	   * Whatever two request are the same.
	   */
	  bool operator==(const CallRequest &other) const;

	  /**
	   * The number of parameters used in this call
	   */
	  const size_t m_nargs;

	  /**
	   * The number of parameters passed by Name
	   */
	  const size_t m_nargsPassedByName;

	  /**
	   * The number of parameters passed by position
	   */
	  const size_t m_nargsPassedByPosition;

	  /**
	   * The types of all the parameters, the first ones must be
	   * the parameters passed by position, the rest of the parameters
	   * are the ones passed by name.
	   * The order of those parameters passed by name are lexicographical
	   * by the name of the argument.
	   */
	  RobinTypes m_types;


 	 /**
  	  * These stores the names of the arguments passed by name and their corresponding
  	  * positions in m_types.
  	  * The order is lexicographical by argument name.
	  */
	  Handle<ArgumentIndices> m_kwargOrder;

	  /**
	   * Vector with the names of the arguments passed by name, are all the keys of
	   * m_kwargOrder. They are ordered in the proper order indicated in m_kwargOrder.
	   */
	  std::vector<std::string> m_argsPassedByName;

	  /**
	   * It is a (weak) reference to the function.
	   */
	  const OverloadedSet *m_calledFunc;

	  friend std::ostream &operator<<(std::ostream &o, const CallRequest &req );
};


} //namespace robin
#endif /* CALLREQUEST_H_ */
