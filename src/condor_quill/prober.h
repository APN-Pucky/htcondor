/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
  *
  * Condor Software Copyright Notice
  * Copyright (C) 1990-2006, Condor Team, Computer Sciences Department,
  * University of Wisconsin-Madison, WI.
  *
  * This source code is covered by the Condor Public License, which can
  * be found in the accompanying LICENSE.TXT file, or online at
  * www.condorproject.org.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  * AND THE UNIVERSITY OF WISCONSIN-MADISON "AS IS" AND ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  * WARRANTIES OF MERCHANTABILITY, OF SATISFACTORY QUALITY, AND FITNESS
  * FOR A PARTICULAR PURPOSE OR USE ARE DISCLAIMED. THE COPYRIGHT
  * HOLDERS AND CONTRIBUTORS AND THE UNIVERSITY OF WISCONSIN-MADISON
  * MAKE NO MAKE NO REPRESENTATION THAT THE SOFTWARE, MODIFICATIONS,
  * ENHANCEMENTS OR DERIVATIVE WORKS THEREOF, WILL NOT INFRINGE ANY
  * PATENT, COPYRIGHT, TRADEMARK, TRADE SECRET OR OTHER PROPRIETARY
  * RIGHT.
  *
  ****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/
#ifndef _PROBER_H_
#define _PROBER_H_

#include "condor_common.h"
#include "quill_enums.h"

//! Prober
/*! this polls and probes Job Qeueue Log (job_queue.log) file.
 *  So, it returns the result of polling: INIT_DB, ADDITION, 
 *  COMPRESSION, and so on.
 */
class Prober
{
public:
	//! constructor	
	Prober();
	//! destructor	
	~Prober();
	
	//! initialization
	void			Init();

	//! probe job_queue.log file
	ProbeResultType probe(ClassAdLogEntry *curCALogEntry,
						  int job_queue_fd);

	//! update state information about size of log file last probed etc.
	//! Call this after successfully responding to probe() result.
	void incrementProbeInfo();

		//
		// accessors
		//
	void			setJobQueueName(const char* jqn);
	char*			getJobQueueName();
	void			setLastModifiedTime(time_t t);
	time_t			getLastModifiedTime();
	void			setLastSize(size_t s);
	size_t			getLastSize();

	long int		getLastSequenceNumber();
	void			setLastSequenceNumber(long int seq_num);
	time_t			getLastCreationTime();
	void			setLastCreationTime(time_t ctime);

	long int		getCurProbedSequenceNumber();
	long int 		getCurProbedCreationTime();

private:
		// information about a job_queue.log file and polling state
	char			job_queue_name[_POSIX_PATH_MAX]; //!< job queue file path
		// stored metadata in DB
	long int		last_mod_time;	//!< last modification time
	long int		last_size;	//!< last size
	long int		last_seq_num;	//!< historical sequence number
	long int		last_creation_time; //!< creation time of cur file 
		// currently probed...
	long int		cur_probed_mod_time; //!< last modification time
	long int		cur_probed_size;	 //!< last size
	long int		cur_probed_seq_num;	//!< historical sequence number
	long int		cur_probed_creation_time;  //!< creation time of cur file 

	ClassAdLogEntry	lastCALogEntry;		//!< last command (ClassAd Log Entry)

};

#endif /* _PROBER_H_ */
