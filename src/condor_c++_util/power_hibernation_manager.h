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

#ifndef _POWER_MANAGER_H_
#define _POWER_MANAGER_H_

/***************************************************************
 * Headers
 ***************************************************************/

#include "power_hibernator.h"

/***************************************************************
 * HibernationManager class
 ***************************************************************/

class HibernationManager {

public:

	/** @name Instantiation. 
		*/
	//@{
	
	/// Constructor
	HibernationManager () throw ();
	
	/// Destructor
	virtual ~HibernationManager () throw (); 
	
	//@}

	/** @name Power management.
		Basic checks to determined if we want to use the power
		management capabilities of the OS.
		*/
	//@{
	
	/** Signals the OS to enter hibernation.
		@param the hibernation state to place machine into
		@return true if the machine will enter hibernation; otherwise, false.
		@see canHibernate
		@see wantsHibernate
		*/
	bool doHibernate ( int state ) const;

	/** Determines if the power manager is capable of hibernating the machine.
		@return true if the machine will can be hibernated; otherwise, false.
		@see doHibernate
		@see wantsHibernate
		*/
	bool canHibernate () const;

	/** Determines if the user wants the machine to hibernate 
	    (based on the configuration file).
		@return true if the user wants the machine to enter 
		        hibernation; otherwise, false.
		@see doHibernate
		@see canHibernate
		*/
	bool wantsHibernate () const;

	//@}

	/** Get the time interval for checking the HIBERNATE expression.
		@return interval in seconds, or zero if no power management is to be used.
		@see doHibernate
		@see canHibernate
		@see wantsHibernate
		*/
	int  getHibernateCheckInterval () const;

	/** Reset all the internal values based on what the values in the 
	    configuration file.
		*/
	void update ();

private:
	
	Hibernator				*_hibernator;
	int						_interval;	

};

#endif // _POWER_MANAGER_H_