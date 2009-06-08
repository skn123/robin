#ifndef ROBIN_DEMO_CONTAINER_H
#define ROBIN_DEMO_CONTAINER_H


template < typename T >
class Container
{
public:
  virtual void clear() = 0;
  virtual void add(const T& i) = 0;
};


#endif
