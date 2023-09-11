/**********************************************************************
 *<
	FILE: interval.h

	DESCRIPTION:  Defines TimeValue and Interval Classes

	CREATED BY: Rolf Berteig

	HISTORY: created 13 September 1994
             950818 - Added methods for setting start/end individually (gus)

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#pragma once
#include "maxheap.h"
#include "coreexp.h"
#include "maxtypes.h"

/*! \sa  Advanced Topics sections on
~{ Intervals }~
and ~{ Time Functions }~.\n\n
\par Description:
An Interval is a class that represents a length of time. It has two private
data members, start and end that are each TimeValues. A TimeValue is a single
instant in time. For more explanation see the Advanced Topics section on
~{ Intervals }~.
All the methods of this class are implemented by the system.\n\n
Definitions:
\code
#define FOREVER Interval(TIME_NegInfinity, TIME_PosInfinity)
#define NEVER Interval(TIME_NegInfinity, TIME_NegInfinity) 
\endcode
*/
class Interval: public MaxHeapOperators {
	// Warning - instances of this class are saved as a binary blob to scene file
	// Adding/removing members will break file i/o
	private:
		TimeValue	start;
		TimeValue	end;

	public:		
		/* 
		 Constructors:
		 */		
		/*! \remarks Constructor that assigns both the start and end times of
		the interval. If TimeValue <b>e</b> is less than\n\n
		TimeValue <b>s</b> the values are swapped before they are assigned.
		\par Parameters:
		<b>TimeValue s</b>\n\n
		Specifies the start time.\n\n
		<b>TimeValue e</b>\n\n
		Specifies the end time. */
		CoreExport Interval( TimeValue s, TimeValue e );
		/*! \remarks Constructor that returns an <b>EMPTY</b> interval, i.e.
		having a start and end time equal to <b>TIME_NegInfinity</b> */
		Interval() { SetEmpty(); } 

		/*! \remarks Checks for equality between two Intervals.
		\return  Nonzero if the intervals are equal; otherwise 0. */
		int operator==( const Interval& i ) const { return( i.start==start && i.end==end ); }
		/*! \remarks Checks for inequality between two Intervals.
		\return  false if and only if the intervals are not equal. */
        bool operator!=( const Interval& i ) const { return ((operator==(i)) == 0); }

        /*! \remarks Return Nonzero if the TimeValue passed is greater than or
		equal to the start value and less than or equal to the end value and
		not equal to <b>TIME_NegInfinity</b>. Returns 0 otherwise.
		\par Parameters:
		<b>const TimeValue t</b>
		\return  Nonzero if the TimeValue passed is greater than or equal to
		the start value and less than or equal to the end value and not equal
		to <b>TIME_NegInfinity</b>; otherwise 0. */
		CoreExport int InInterval(const TimeValue t) const;
		/*! \remarks Returns nonzero if the interval passed is contained
		within the interval; otherwise 0.\n\n
		return InInterval( interval.Start() ) \&\& InInterval( interval.End()
		);
		\par Parameters:
		<b>const Interval interval</b>\n\n
		The interval to check.
		\return  Returns nonzero if the interval passed is contained within the
		interval; otherwise 0. */
		int InInterval(const Interval interval) const { return InInterval( interval.Start() ) && InInterval( interval.End() ); }
        /*! Returns whether an interval is a subset of this interval.
            This method is different from InInterval() in the way that it deals with infinite and empty sets. Infinite sets are supersets
            of everything. Empty sets are subsets or everything. */
        bool IsSubset(const Interval& subset) const { return (IsInfinite() || subset.Empty() || InInterval(subset)); }

		/*! Returns whether an interval intersects with another interval. */
		bool Intersects(const Interval& other) const
		{ 
			if (Empty() || other.Empty())
				return false;
			return InInterval(other.Start()) || InInterval(other.End()) || other.InInterval(Start()) || other.InInterval(End());
		}

		/*! \remarks Returns 1 if the interval is <b>EMPTY</b>, i.e. has a
		start and end time equal to <b>TIME_NegInfinity</b>. Returns 0
		otherwise. */
		int Empty() const { return (start == TIME_NegInfinity) && (end == TIME_NegInfinity); }
		/*! \remarks Returns true if and only if the interval covers all possible time values, i.e. equals the result of calling SetInfinite(). */
		bool IsInfinite() const { return (start == TIME_NegInfinity) && (end == TIME_PosInfinity); }

		/*! \remarks Sets the start and end times for the interval.
		\par Parameters:
		<b>TimeValue s</b>\n\n
		Start time for the interval.\n\n
		<b>TimeValue e</b>\n\n
		End time for the interval. */
		void Set       ( TimeValue s, TimeValue e ) { start = s; end = e; }
		/*! \remarks Sets the start value only.
		\par Parameters:
		<b>TimeValue s</b>\n\n
		Start time for the interval. */
		void SetStart  ( TimeValue s )              { start = s;          }
		/*! \remarks Sets the end value only.
		\par Parameters:
		<b>TimeValue e</b>\n\n
		End time for the interval. */
		void SetEnd    ( TimeValue e )              { end = e;            }

		/*! \remarks Sets the interval to be <b>EMPTY</b>, i.e. having a start
		and end time equal to <b>TIME_NegInfinity</b>. */
		void SetEmpty() { start = TIME_NegInfinity; end = TIME_NegInfinity; }
		/*! \remarks Sets the interval to be <b>FOREVER</b>, i.e. have a start
		time equal <b>TIME_NegInfinity</b> and end time equal to
		<b>TIME_PosInfinity</b>. */
		void SetInfinite() { start = TIME_NegInfinity; end = TIME_PosInfinity; }
		/*! \remarks Sets both the start and end times to the time passed. */
		void SetInstant(const TimeValue t) { start = end = t; }
		/*! \remarks Returns the start time of the interval. */
		TimeValue Start() const { return start; }
		/*! \remarks Returns the end time of the interval. */
		TimeValue End() const { return end; }
		/*! \remarks Implemented by the System.\n\n
		Returns the duration of the interval (end points included).
		\par Operators:
		*/
		TimeValue Duration() const { return end-start+TimeValue(1); } // end points included

		// intersection of intervals
		/*! \remarks Intersects <b>Interval</b> and i. The interval will have
		a start time of the greater of the two interval start times, and an end
		value which is the lesser of the two end values. If the end time is
		less than the start time, both the start and end times are set to
		<b>TIME_NegInfinity</b>.
		\return  An Interval that is the intersection of the intervals. */
		CoreExport Interval operator&(const Interval i) const;
		/*! \remarks This updates the invoking interval so it will have a
		start time of the greater of the two interval start times, and an end
		value which is the lesser of the two end values.\n\n
		<b>return (*this = (*this\&i));</b> */
		Interval& operator&=(const Interval i) { return (*this = (*this&i)); }
		/*! \remarks Expands the <b>Interval</b> to include the TimeValue.\n\n
		<b>if (t\<start) start=t; if (t\>end) end=t; return *this;</b> */
		Interval& operator+=(const TimeValue t) { if (t<start) start=t; if (t>end) end=t; return *this; }

		/*! \remarks Offsets the entire interval; shifts the beginning and end by the provided time value.
        Only effective if the interval is neither empty nor infinite. The offset may be positive or negative.
        Upon returning, the duration of the interval will be identical to what it was before the call, unless
        this is made impossible by clamping at either end of the valid numerical range. */
        CoreExport void ApplyOffset(const TimeValue offset);

};

#define FOREVER Interval(TIME_NegInfinity, TIME_PosInfinity)
#define NEVER Interval(TIME_NegInfinity, TIME_NegInfinity)



