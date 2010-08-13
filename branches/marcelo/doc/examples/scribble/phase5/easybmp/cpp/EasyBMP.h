/*************************************************
*                                                *
*  EasyBMP Cross-Platform Windows Bitmap Library * 
*                                                *
*  Author: Paul Macklin                          *
*   email: macklin01@users.sourceforge.net       *
* support: http://easybmp.sourceforge.net        *
*                                                *
*          file: EasyBMP.h                       * 
*    date added: 01-31-2005                      *
* date modified: 02-06-2005                      *
*       version: 1.00                            *
*                                                *
*   License: BSD (revised/modified)              *
* Copyright: 2005-6 by the EasyBMP Project       * 
*                                                *
* description: Main include file                 *
*                                                *
*************************************************/

#include <iostream>
#include <cmath>

#ifndef EasyBMP
#define EasyBMP

#ifdef __BCPLUSPLUS__ 
// The Borland compiler must use this because something
// is wrong with their cstdio file. 
#include <stdio.h>
#else 
#include <cstdio>
#endif

#ifdef _MSC_VER 
// If MSVC++ specific code is ever required, this is 
// where it goes.
#endif

#ifdef __GNUC__
// If g++ specific code is ever required, this is 
// where it goes. 
#endif

#ifdef __INTEL_COMPILER
// If Intel specific code is ever required, this is 
// where it goes. 
#endif

#ifndef _DefaultXPelsPerMeter_
#define _DefaultXPelsPerMeter_
int DefaultXPelsPerMeter = 3780;
// set to a default of 96 dpi 
#endif

#ifndef _DefaultYPelsPerMeter_
#define _DefaultYPelsPerMeter_
int DefaultYPelsPerMeter = 3780;
// set to a default of 96 dpi
#endif

#include "EasyBMP_DataStructures.h"
#include "EasyBMP_BMP.h"
#include "EasyBMP_VariousBMPutilities.h"

#ifndef _EasyBMP_Version_
#define _EasyBMP_Version_ "1.00"
#endif

#endif
