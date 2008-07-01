
#ifndef ROBIN_EXTREME_PYTHON_HINTING_H
#define ROBIN_EXTREME_PYTHON_HINTING_H

#ifndef INSIDE_HINTED_H
#error This file must be included from within hinted.h
#endif


class Clue
{
public:
  hinted_size_t get() { return 0; }
  Templates<hinted_size_t> gets() { return Templates<hinted_size_t>(1); }
};


#endif
