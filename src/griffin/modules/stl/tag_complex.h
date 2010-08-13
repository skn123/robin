#error this file should never be included

namespace std {

  template < typename RealType >
  class complex {
  public:
    complex();
    complex(RealType re);
    complex(RealType re, RealType im);

    complex(const complex& copy);

    RealType real();
    RealType imag();
    bool operator==(const complex&) const;
  };

}
