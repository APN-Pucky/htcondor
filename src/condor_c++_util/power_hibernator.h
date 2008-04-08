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

#ifndef _HIBERNATER_H_
#define _HIBERNATER_H_

/***************************************************************
 * Base Hibernator class
 ***************************************************************/

class Hibernator {

public:

	/* The following are the standard sleep states: doHibernate()
	   makes the machine to enter the given sleep state, while 
	   getStates() returns a mask describing the sleep states
	   supported on the current machine, if any (read: NONE) 
	*/

	enum SLEEP_STATE {
		NONE = 0,	 /* No sleep states supported */
		S1   = 0x01, /* Sleep */
		S2   = 0x02, /* (not used) */
		S3   = 0x04, /* Suspend to RAM */
		S4   = 0x08, /* Hibernate */
		S5   = 0x10  /* Shutdown (i.e. soft-off) */
	};

	/** @name Instantiation. 
		*/
	//@{
	
	/// Constructor
	Hibernator () throw ();

	/// Destructor
	virtual ~Hibernator () throw (); 

	//@}

	/** @name Power management.
		Basic checks to determined if we want to use the power
		management capabilities of the OS.
		*/
	//@{
	
	/** Signal the OS to enter hibernation.
		@return true if the machine will enter hibernation; otherwise, false.
		@param State of hibernation to enter.
		@param Should the computer be forced into the hibernation state?
		*/
	bool doHibernate ( SLEEP_STATE state, bool force = true ) const;

	//@}

	/** @name State management.
		Basic management call for supported OS hibernation states.
		*/
	//@{
	
	/** Retrieve the hibernation states supported by the OS.
		@return a mask of the states supported by the OS.
		@see SLEEP_STATE
		*/
	unsigned short getStates () const;

	//@}

	/** We use this to create sleep objects so we don't need to 
		deal with the differences between OSs at the invocation 
		level.
		@return if the OS is supported a valid Hibernator*; otherwise NULL.
	*/
	static Hibernator* createHibernator ();

	
	/** @name Conversion functions.
		These can be used to convert between sleep states 
		(i.e. SLEEP_STATE) and their string representation 
		and vice versa 
	*/
	//@{
	
	static SLEEP_STATE intToSleepState ( int x );
	static int sleepStateToInt ( SLEEP_STATE state );
	static char const* sleepStateToString ( SLEEP_STATE state );

	//@}

protected:

	/* Override this to enter the given sleep state on a 
	   particular OS */
	virtual bool enterState ( SLEEP_STATE state, bool force ) const = 0;

	/* Set the supported sleep states */
	void setStates ( unsigned short states );

private:
	
	/* OS agnostic sleep state representation */
	unsigned short _states;

};

/***************************************************************
 * Specialized classes 
 ***************************************************************/

/***************************************************************
 * MsWindowsHibernator class
 ***************************************************************/

#if defined ( WIN32 )

class MsWindowsHibernator : public Hibernator {

public:

	MsWindowsHibernator () throw ();
	virtual ~MsWindowsHibernator () throw ();

protected:

	/* Enter the given sleep state.  Can be any of S[1-5],
	   but only S[3-5] truly exist on Windows */
	bool enterState ( SLEEP_STATE state, bool force ) const;

private:

	/* Auxiliary function to shutdown the machine */
	bool tryShutdown ( bool force ) const;

	/* Discover supported sleep states */
	virtual void initStates ();

};

#endif // WIN32

#endif // _HIBERNATER_H_