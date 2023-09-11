#pragma once

#include "buildnumber.h" // defines VERSION_INT

// VERSION_INT is defined in buildnumber.h and is written to by the builder, inserting the current build number.
// VERSION_INT is used for the 4th component eg. 4.1.1.[36]

// The product and file version could be different.
// For example, VIZ 4.0 works with max files version 4.2

// 3ds Max internal version number is managed using the following scheme:
// Major = Main release number (ex. 3ds Max 2013 (SimCity) = 15).
// Minor = Product Update; PUs would typically be starting at 1
// Micro = Used for security fixes (even numbers) and for private hot fixes (odd numbers)
// Point = Build Number (this always increments for a major release)
//
// Example for Imoogi = 20:
//			First Release Update would be:	20.1.0.<build>
//			Second Release Update would be:	20.2.0.<build>
//			Etc.
//

// MAX File version:
#ifndef MAX_VERSION_MAJOR
#define MAX_VERSION_MAJOR 25
#endif

#ifndef MAX_VERSION_MINOR
#define MAX_VERSION_MINOR 0
#endif

#ifndef MAX_VERSION_POINT
#define MAX_VERSION_POINT VERSION_INT
#endif

// MAX Product version
#ifndef MAX_PRODUCT_VERSION_MAJOR
#define MAX_PRODUCT_VERSION_MAJOR 25
#endif

#ifndef MAX_PRODUCT_VERSION_MINOR
#define MAX_PRODUCT_VERSION_MINOR 0
#endif

#ifndef MAX_PRODUCT_VERSION_MICRO
#define MAX_PRODUCT_VERSION_MICRO 0
#endif

#ifndef MAX_PRODUCT_VERSION_POINT
#define MAX_PRODUCT_VERSION_POINT VERSION_INT
#endif

#ifndef MAX_PRODUCT_YEAR_NUMBER
#define MAX_PRODUCT_YEAR_NUMBER 2023
#endif

// MAX_RELEASE_EXTERNAL is an alternative for MAX_RELEASE (plugapi.h)
// Define it when you want the "branded" (UI) version number to be different 
// from the internal one.
//#define MAX_RELEASE_EXTERNAL	10900

// This should be left blank for the main release up until the first Update, Security fix or HF.
// This is displayed in the about box, concatenated after the MAX_PRODUCT_YEAR_NUMBER
// For example:
// RTM: 2018
// HF: 2018.0.1 HF
// Update: 2018.1 Update
// SF: 2018.1.2 SF
// HF: 2018.1.3 HF
// Update: 2018.2 Update
// Include the ".": ex: ".1.2 SF\0"
#ifndef MAX_PRODUCT_VERSION_DESC
#define MAX_PRODUCT_VERSION_DESC "\0"
#endif


#define _MAX_VERSION(a, b, c,d) a##b##c##d
#define MAX_VERSION _MAX_VERSION(MAX_VERSION_MAJOR, MAX_VERSION_MINOR, MAX_PRODUCT_VERSION_MICRO, VERSION_INT)
