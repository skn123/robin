#ifndef ROBIN_CONFIG_H
#define ROBIN_CONFIG_H


namespace Robin {

//==================================================
// Endianity issues
//==================================================
enum endianity_t { BIG, LITTLE };

#ifdef __linux
const endianity_t MACH_ENDIAN = LITTLE;
#else
const endianity_t MACH_ENDIAN = BIG;
#endif

} // end of namespace Robin


#endif
