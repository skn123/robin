#ifndef ROBIN_TEST_PYTHON_KWARGS_H
#define ROBIN_TEST_PYTHON_KWARGS_H

#include "stls.h"


class KwClass {

    public:
        int m_a;
        int m_b;
        int m_dummy;

        KwClass() 
            : m_a(0), m_b(0), m_dummy(0)
        {
        }

        void setMembers(int a, int b) {
                m_a = a;
                m_b = b;
        }

        void setMembers2(int b, int a) {
                m_a = a;
                m_b = b;
        }

        void setMembersWithExtraArgs(int dummy, int a, int b) {
                m_dummy = dummy;
                m_a = a;
                m_b = b;
        }
};

/* TODO
class ans {
 public:
  ans(const StandardLibrary::DerivedFromVectorOfClass& a) { }
};
*/

#endif
