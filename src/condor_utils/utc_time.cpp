/***************************************************************
 *
 * Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/


#include "condor_common.h"
#include "utc_time.h"

#if defined(HAVE__FTIME)
# include <sys/timeb.h>
#endif


void condor_gettimestamp( struct timeval &tv )
{
#if defined(WIN32)
	// Windows8 has GetSystemTimePreciseAsFileTime which returns sub-microsecond system times.
	static bool check_for_precise = false;
	static void (WINAPI*get_precise_time)(unsigned long long * ft) = NULL;
	static BOOLEAN (WINAPI* time_to_1970)(unsigned long long * ft, unsigned long * epoch_time);
	if ( ! check_for_precise) {
		HMODULE hmod = GetModuleHandle("Kernel32.dll");
		if (hmod) { *(FARPROC*)&get_precise_time = GetProcAddress(hmod, "GetSystemTimePreciseAsFileTime"); }
		hmod = GetModuleHandle("ntdll.dll");
		if (hmod) { *(FARPROC*)&time_to_1970 = GetProcAddress(hmod, "RtlTimeToSecondsSince1970"); }
		check_for_precise = true;
	}
	unsigned long long nanos = 0;
	if (get_precise_time) {
		get_precise_time(&nanos);
		unsigned long now = 0;
		time_to_1970(&nanos, &now);
		tv.tv_sec = now;
		tv.tv_usec = (int)((nanos / 10) % 1000000);
	} else {
		struct _timeb tb;
		_ftime(&tb);
		tv.tv_sec = tb.time;
		tv.tv_usec = tb.millitm * 1000;
	}
#elif defined(HAVE_GETTIMEOFDAY)
	gettimeofday( &tv, NULL );
#else
#error Neither _ftime() nor gettimeofday() are available!
#endif
}

UtcTime::UtcTime( bool get_time )
{
	m_tv.tv_sec = 0;
	m_tv.tv_usec = 0;
	if ( get_time ) {
		getTime( );
	}
}

void
UtcTime::getTime()
{
	condor_gettimestamp( m_tv );
}

double
UtcTime::difference( const UtcTime* other_time ) const
{
	if( ! other_time ) {
		return 0.0;
	}
	return difference( *other_time );
}

double
UtcTime::difference( const UtcTime &other_time ) const
{
	double other = other_time.combined();
	double me = combined();

	return me - other;
}

bool
operator==(const UtcTime &lhs, const UtcTime &rhs) 
{
	return ((lhs.seconds() == rhs.seconds()) &&
			(lhs.microseconds() == rhs.microseconds()) );
}
