/**********************************************************************
 *<
	FILE: units.h

	DESCRIPTION: Include file for real-world units support

	CREATED BY: Tom Hudson

	HISTORY:

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "strclass.h"
#include "maxtypes.h"

// The unit types we support
#define UNITS_INCHES		0
#define UNITS_FEET			1
#define UNITS_MILES			2
#define UNITS_MILLIMETERS	3
#define UNITS_CENTIMETERS	4
#define UNITS_METERS		5
#define UNITS_KILOMETERS	6

#define MAX_UNIT_TYPES 7

// The unit display types
#define UNITDISP_GENERIC	0
#define UNITDISP_METRIC		1
#define UNITDISP_US			2
#define UNITDISP_CUSTOM		3

#define MAX_UNITDISP_TYPES 4

// Metric display options
#define UNIT_METRIC_DISP_MM	0
#define UNIT_METRIC_DISP_CM	1
#define UNIT_METRIC_DISP_M	2
#define UNIT_METRIC_DISP_KM	3

#define MAX_METRIC_DISP_TYPES 4

// US display options
#define UNIT_US_DISP_FRAC_IN	0
#define UNIT_US_DISP_DEC_IN		1
#define UNIT_US_DISP_FRAC_FT	2
#define UNIT_US_DISP_DEC_FT		3
#define UNIT_US_DISP_FT_FRAC_IN	4
#define UNIT_US_DISP_FT_DEC_IN	5

#define MAX_US_DISP_TYPES 6

// US display options
#define UNIT_FRAC_1_1	0
#define UNIT_FRAC_1_2	1
#define UNIT_FRAC_1_4	2
#define UNIT_FRAC_1_8	3
#define UNIT_FRAC_1_10	4
#define UNIT_FRAC_1_16	5
#define UNIT_FRAC_1_32	6
#define UNIT_FRAC_1_64	7
#define UNIT_FRAC_1_100	8

#define MAX_FRAC_TYPES 9

// Units designator types
#define UNIT_DESIG_MM		0
#define UNIT_DESIG_CM		1
#define UNIT_DESIG_M		2
#define UNIT_DESIG_KM		3
#define UNIT_DESIG_IN		4
#define UNIT_DESIG_FT		5
#define UNIT_DESIG_CUSTOM	6

#define UNIT_DESIG_TYPES	 7

// Display information structure

/*! This is the display information structure. This structure holds the information
describing 3ds Max's current system of measurement. This includes the type of units
used, how they are displayed and any custom unit name and scale.
*/
struct DispInfo: public MaxHeapOperators {
	/*! Unit Display Type. One of the following values:
	- UNITDISP_GENERIC
	- UNITDISP_METRIC
	- UNITDISP_US
	- UNITDISP_CUSTOM
	*/
	int dispType;				// Display type	(UNITDISP_GENERIC, UNITDISP_METRIC, UNITDISP_US, UNITDISP_CUSTOM)
	/*! Metric display option. One of the following values:
	- UNIT_METRIC_DISP_MM
	- UNIT_METRIC_DISP_CM
	- UNIT_METRIC_DISP_M
	- UNIT_METRIC_DISP_KM
	*/
	int metricDisp;				// Metric display option
	/*! US display option. One of the following values:
	- UNIT_US_DISP_FRAC_IN
	- UNIT_US_DISP_DEC_IN
	- UNIT_US_DISP_FRAC_FT
	- UNIT_US_DISP_DEC_FT
	- UNIT_US_DISP_FT_FRAC_IN
	- UNIT_US_DISP_FT_DEC_IN
	*/
	int usDisp;					// US display option
	/*! US fraction option. One of the following values:
	- UNIT_FRAC_1_1
	- UNIT_FRAC_1_2
	- UNIT_FRAC_1_4
	- UNIT_FRAC_1_8
	- UNIT_FRAC_1_10
	- UNIT_FRAC_1_16
	- UNIT_FRAC_1_32
	- UNIT_FRAC_1_64
	- UNIT_FRAC_1_100
	*/
	int usFrac;					// US fraction option
	/*! Custom unit name. */
	MSTR customName;			// Custom unit name
	/*! Custom unit value. */
	float customValue;			// Custom unit value
	/*! Custom unit reference. \n Note: The three custom settings above
        are related as follows: If for example the customName is set to
        "FL" for furlongs (which is equal to 660 feet), customValue should equal
        660.0 and customUnit should equal UNITS_FEET.*/
	int customUnit;				// Custom unit reference
	};

/*! Retrieves the System Unit scale in terms of the specified unit type. For example, GetSystemUnitScale(UNITS_INCHES) returns the number of inches per unit. Returns -1.0 if an invalid unit type is supplied.
 */
CoreExport double GetSystemUnitScale(int type);

//! \brief Computes a scaling factor for going from given units to others.	
/*!
\param [in] fromType - type of units from which we start (a value going from UNITS_INCHES to UNITS_KILOMETERS).
\param [in] fromScale - scale of units from which we start.
\param [in] toType - type of units into which we want to rescale (a value going from UNITS_INCHES to UNITS_KILOMETERS).
\param [in] toScale - scale of units into which we want to rescale.
\return - The scale factor that is needed to scale from given units to others. 
*/
CoreExport double GetRelativeScale(int fromType, float fromScale, int toType, float toScale);

/*! Retrieves the System Unit settings in effect. These are the System Unit type (Inches, Feet, Meters, and so on) and the System Unit scale settings. These are the values that users enter in the "1 Unit = XXX field" of the System Unit Setup dialog box.
 */
CoreExport void GetSystemUnitInfo(int *type,float *scale);

/*! Sets the System Unit settings. These are the System Unit type (Inches, Feet, Meters, and so on) and the System Unit scale settings. */
CoreExport int SetSystemUnitInfo(int type,float scale);

/*! Retrieves the current unit display information. */
CoreExport void GetUnitDisplayInfo(DispInfo *info);
/*! Sets the unit display information used by 3ds Max. */
CoreExport int SetUnitDisplayInfo(DispInfo *info);
/*! Returns the current unit display type to one of the following values:
- UNITDISP_GENERIC
- UNITDISP_METRIC
- UNITDISP_US
- UNITDISP_CUSTOM 
 */
CoreExport int GetUnitDisplayType();
/*! Sets the current unit display type to one of the following values: 
- UNITDISP_GENERIC
- UNITDISP_METRIC
- UNITDISP_US
- UNITDISP_CUSTOM 
 */
CoreExport int SetUnitDisplayType(int type);
CoreExport BOOL IsValidUnitCharacter(int ch);	// Returns TRUE if character valid for unit type

// The US unit defaults
#define US_UNIT_DEFAULT_FEET 0
#define US_UNIT_DEFAULT_INCHES 1

// Get/set the default units for US entry fields -- Inches or feet
CoreExport void SetUSDefaultUnit(int type);
CoreExport int GetUSDefaultUnit();

CoreExport const MCHAR *GetFirstUnitName();
CoreExport const MCHAR *GetNextUnitName();
CoreExport const MCHAR *GetFirstMetricDisp();
CoreExport const MCHAR *GetNextMetricDisp();
CoreExport const MCHAR *GetFirstUSDisp();
CoreExport const MCHAR *GetNextUSDisp();
CoreExport const MCHAR *GetFirstFraction();
CoreExport const MCHAR *GetNextFraction();
/*! Converts the specified value to an ASCII representation according to the current unit scale. Note that this can cause a string overflow, especially when the units are set to miles or kilometers. If an overflow occurs the function returns a null string (_T("")). So, developers can check for this condition using something like if (buf[0] == '\0') to see if an overflow occurred.
 */
CoreExport const MCHAR *FormatUniverseValue(float value);
/*! Parses the specified string using the current unit settings and converts it to a floating point value. If an error occurs in the parsing then valid is set to FALSE.
 */
CoreExport float DecodeUniverseValue(const MCHAR *string, BOOL *valid = NULL);


//
// Time units
//

// Ways to display time
enum TimeDisp {
	DISPTIME_FRAMES,
	DISPTIME_SMPTE,
	DISPTIME_FRAMETICKS,
	DISPTIME_TIMETICKS
	};

// Formats a time value into a string based on the current frame rate, etc.
CoreExport void TimeToString(TimeValue t,MSTR &string);
CoreExport BOOL StringToTime(MSTR string,TimeValue &t);

CoreExport int GetFrameRate();
CoreExport void SetFrameRate(int rate);
CoreExport int GetTicksPerFrame();
CoreExport void SetTicksPerFrame(int ticks);
CoreExport TimeDisp GetTimeDisplayMode();
CoreExport void SetTimeDisplayMode(TimeDisp m);
CoreExport int LegalFrameRate(int r);

// Register a default value for a distance value.  The section and key are used
// to place the default in the .INI file.  The default has separate values for
// both Imperial and Mtric systems.  The Imperial default is given in inches
// and the Metric default is given in Meters.
/*! Registers a default value for a distance value. The default has separate values for both Imperial and Metric systems. The Imperial default is given in inches and the Metric default is given in Meters.
 */
CoreExport void RegisterDistanceDefault(const MCHAR* section, const MCHAR* key, float inchesDefault, float metersDefault);

// Restore a default value for a distance value.  The section and key are used
// to place the default in the .INI file.  The default has separate values for
// both Imperial and Metric systems.  The Imperial default is given in inches
// and the Metric default is given in Meters.
/*! Restores the original default value for a distance value. */
CoreExport void RestoreDistanceDefault(const MCHAR* section, const MCHAR* key, float inchesDefault, float metersDefault);

// Get the default for the given section and key in the defaults file.
// The system returns the default relative to the current system units.
// If the units are Imperial (inches, feet, etc.) it returns the Imperial
// version of the default, otherwise it returns the Metric version.
// In either case the default value is returned in the current system units.
/*! Gets the default for the given section and key in the defaults file. The system returns the default relative to the current system units. If the units are Imperial (inches, feet, and so on) it returns the Imperial version of the default, otherwise it returns the Metric version. In either case the default value is returned in the current system units.
 */
CoreExport float GetDistanceDefault(const MCHAR* section, const MCHAR* key);

// Sets the defaults for the given section and key in the defaults file.
// The system sets the default relative to the current system units.
// If the units are Imperial (inches, feet, etc.) it sets the Imperial
// version of the default, otherwise it sets the Metric version.
/*! Sets the defaults for the given section and key in the defaults file. The system sets the default relative to the current system units. If the units are Imperial (inches, feet, and so on) it sets the Imperial version of the default, otherwise it sets the Metric version. 
 */
CoreExport void SetDistanceDefault(const MCHAR* section, const MCHAR* key, float value);

