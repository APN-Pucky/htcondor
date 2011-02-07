/***************************************************************
Fermitools Software Legal Information (Modified BSD License)

COPYRIGHT STATUS: Dec 1st 2001, Fermi National Accelerator Laboratory (FNAL)
documents and software are sponsored by the U.S. Department of Energy under
Contract No. DE-AC02-76CH03000. Therefore, the U.S. Government retains a
world-wide non-exclusive, royalty-free license to publish or reproduce these
documents and software for U.S. Government purposes. All documents and
software available from this server are protected under the U.S. and Foreign
Copyright Laws, and FNAL reserves all rights.

    * Distribution of the software available from this server is free of
      charge subject to the user following the terms of the Fermitools
      Software Legal Information.

    * Redistribution and/or modification of the software shall be accompanied
      by the Fermitools Software Legal Information (including the copyright
      notice).

    * The user is asked to feed back problems, benefits, and/or suggestions
      about the software to the Fermilab Software Providers.

    * Neither the name of Fermilab, the FRA, nor the names of the contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

DISCLAIMER OF LIABILITY (BSD): THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FERMILAB,
OR THE FRA, OR THE U.S. DEPARTMENT of ENERGY, OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Liabilities of the Government: This software is provided by FRA, independent
from its Prime Contract with the U.S. Department of Energy. FRA is acting
independently from the Government and in its own private capacity and is not
acting on behalf of the U.S. Government, nor as its contractor nor its agent.
Correspondingly, it is understood and agreed that the U.S. Government has no
connection to this software and in no manner whatsoever shall be liable for
nor assume any responsibility or obligation for any claim, cost, or damages
arising out of or resulting from the use of the software available from this
server.

Export Control: All documents and software available from this server are
subject to U.S. export control laws. Anyone downloading information from this
server is obligated to secure any necessary Government licenses before
exporting documents or software obtained from this server.
****************************************************************/

#include "condor_common.h"
#include "sandbox_manager.h"
#include "util_lib_proto.h"
#include <string>
#include <iostream>
using namespace std;

CSandboxManager::CSandboxManager()
{
	// TODO: Fill in more details. Don't know yet
	cout << "CSandboxManager::CSandboxManager called" << std::endl;
	init();
}


CSandboxManager::~CSandboxManager()
{
	cout << "CSandboxManager::~CSandboxManager called" << std::endl;
	map<string, CSandbox*>::iterator iter;

	// Empty the map
	for (iter = sandboxMap.begin(); iter != sandboxMap.end(); ++iter) {
		if (iter->second) {
			delete iter->second;
		}
	}
}


char*
//CSandboxManager::registerSandbox(JobInfoCommunicator* jic, const char* sDir)
CSandboxManager::registerSandbox(const char* sDir)
{
	dprintf(D_ALWAYS, "CSandboxManager::registerSandbox called \n");
	cout << "CSandboxManager::registerSandbox called" << std::endl;
	//CSandbox *sandbox = new CSandbox(jic, sDir);
	CSandbox *sandbox = new CSandbox(sDir);
	this->sandboxMap[sandbox->getId()] = sandbox;
	return (char*)sandbox->getId().c_str();
}


string
CSandboxManager::transferSandbox(const char* sid)
{
	string s_sid(sid);
	if (sandboxMap.find(s_sid) != sandboxMap.end())
		return sandboxMap[s_sid]->getSandboxDir();
	return "(NULL)";
	//cout << "CSandboxManager::transferSandbox called" << endl;
	//return 1;
}


/*
bool
CSandboxManager::transferSandbox(int mode, Stream* s)
{
	ReliSock* rsock = (ReliSock*)s;
	TransferDaemon *td = NULL;
	MyString fquser;
	ClassAd reqad, respad;
	MyString jids, jids_allow, jids_deny;
	int protocol;
	int cluster, proc;
	ClassAd *tmp_ad = NULL;

	// TODO: Findout what the heck this does?
	mode = mode; // quiet the compiler

    cout << "Entering CSandboxManager::transferSandbox()" << std::endl;

	// Make sure this connection is authenticated, and we know who
	// the user is. Also, set a timeout, since we don't want to
	// block long trying to read from our client.   
    rsock->timeout( 20 );

	// Authenticate the socket
    if( ! rsock->triedAuthentication() ) {
		CondorError errstack;
		if( ! SecMan::authenticate_sock(rsock, WRITE, &errstack) ) {
			// We failed to authenticate, we should bail out now
			// since we don't know what user is trying to perform
			// this action.
			// TODO: it'd be nice to print out what failed, but we
			// need better error propagation for that...
			errstack.push(	"SANDBOXMANAGER", SECMAN_ERR_AUTHENTICATION_FAILED,
							"Failure transfering sandbox. Authentication failed" );
			dprintf( D_ALWAYS, "requestSandBoxLocation() aborting: %s\n",
                     errstack.getFullText() );

			respad.Assign(ATTR_TREQ_INVALID_REQUEST, TRUE);
			respad.Assign(ATTR_TREQ_INVALID_REASON, "Authentication failed.");
			respad.put(*rsock);
			rsock->end_of_message();

			// TODO: For now accept the authentication failures
			// Eventually we will need this anyways and then just uncomment
			// the line below

			//return false;
		}
	}

	// to whom does the client authenticate?
	fquser = rsock->getFullyQualifiedUser();

	rsock->decode();

	////////////////////////////////////////////////////////////////////////
	// read the request ad from the client about what it wants to transfer
	////////////////////////////////////////////////////////////////////////

	// This request ad from the client will contain
	//  ATTR_TREQ_DIRECTION
	//  ATTR_TREQ_PEER_VERSION
	//  ATTR_TREQ_HAS_CONSTRAINT
	//  ATTR_TREQ_JOBID_LIST
	//  ATTR_TREQ_XFP
	//
	//  OR
	//
	//  ATTR_TREQ_DIRECTION
	//  ATTR_TREQ_PEER_VERSION
	//  ATTR_TREQ_HAS_CONSTRAINT
	//  ATTR_TREQ_CONSTRAINT
	//  ATTR_TREQ_XFP
	// 
	// TODO: Need better understanding.
	// The above request ad seems to be valid for a schedd but from startd
	// point of view, we should expect a sandboxid. 
	// For now assume ATTR_TREQ_JOBID_LIST is your sandbox id list
	reqad.initFromStream(*rsock);
	rsock->end_of_message();

	// TODO: Understand if we need this?
	if (reqad.LookupBool(ATTR_TREQ_HAS_CONSTRAINT, has_constraint) == 0) {
		cout 	<< "CSandboxManager::transferSandbox(): Client reqad from "
				<< "must have %s as an attribute." << std::endl
				<< fquser.Value() <<  ATTR_TREQ_HAS_CONSTRAINT;

        respad.Assign(ATTR_TREQ_INVALID_REQUEST, TRUE);
        respad.Assign(ATTR_TREQ_INVALID_REASON, "Missing constraint bool.");
        respad.put(*rsock);
        rsock->end_of_message();

        return false;
    }

	////////////////////////////////////////////////////////////////////////
	// Let's validate the jobid set the user wishes to modify with a
	// file transfer. The reason we sometimes use a constraint and sometimes
	// not is an optimization for speed. If the client already has the
	// ads, then we don't iterate over the job queue log, which is 
	// extremely expensive.
	////////////////////////////////////////////////////////////////////////


	/////////////
	// The user specified the jobids directly it would like to work with.
	// We assume the client already has the ads it wishes to transfer.
	/////////////
	if (!has_constraint) {

		//dprintf(D_ALWAYS, "Submittor provides procids.\n");
		cout << "Startd provides sandbox ids" << std::endl;

		modify_allow_jobs = new ExtArray<PROC_ID>;
		ASSERT(modify_allow_jobs);

		modify_deny_jobs = new ExtArray<PROC_ID>;
        ASSERT(modify_deny_jobs);

		if (reqad.LookupString(ATTR_TREQ_JOBID_LIST, jids) == 0) {
            //dprintf(D_ALWAYS, "requestSandBoxLocation(): Submitter "
            //    "%s's reqad must have %s as an attribute.\n",
			cout 	<< "CSandboxManager::transferSandbox() " << fquser.Value()
					<< "'s reqad must have " << ATTR_TREQ_JOBID_LIST 
					<< " as an attribute." << edt::endl;

            respad.Assign(ATTR_TREQ_INVALID_REQUEST, TRUE);
            respad.Assign(ATTR_TREQ_INVALID_REASON, "Missing jobid list.");
            respad.put(*rsock);
            rsock->end_of_message();

            return false;
		}

		//////////////////////
		// convert the stringlist of jobids into an actual ExtArray of
		// PROC_IDs. we are responsible for this newly allocated memory.
		//////////////////////
		jobs = mystring_to_procids(jids);

		if (jobs == NULL) {
            // can't have no constraint and no jobids, bail.
            // dprintf(D_ALWAYS, "Submitter %s sent inconsistant ad with no "
            //    "constraint and also no jobids on which to perform sandbox "
            //    "manipulations.\n", fquser.Value());
			cout	<< "Submitter " << fquser.Value() 
					<< " sent inconsistant ad with no constraint and also "
					<< "no jobids on which to perform sandbox "
					<< "manipulations" << std::endl;

			respad.Assign(ATTR_TREQ_INVALID_REQUEST, TRUE);
			respad.Assign(ATTR_TREQ_INVALID_REASON,
                "No constraint and no jobid list.");
			respad.put(*rsock);
			rsock->end_of_message();

			return false;
		}

	}
	else {
	}



	return true;
}

*/

std::vector<string>
CSandboxManager::getExpiredSandboxIds(void)
{
	cout << "CSandboxManager::getExpiredSandboxIds called" << std::endl;
	time_t now;
	time(&now);
	vector<string> ids;
	map<string, CSandbox*>::iterator iter;

	for (iter = sandboxMap.begin(); iter != sandboxMap.end(); ++iter) {
		cout	<< "Checking sandbox " << iter->second->getCreateTime()
				<< std::endl;
		if((unsigned int)now > (unsigned int)iter->second->getCreateTime()) {
			ids.push_back(iter->first);
		}
	}
	return ids;
}


void
CSandboxManager::listRegisteredSandboxes(void)
{
	cout << "CSandboxManager::listRegisteredSandboxes called" << std::endl;
	map<string, CSandbox*>::iterator iter;
	for (iter = sandboxMap.begin(); iter != sandboxMap.end(); ++iter) {
		std::cout	<< "DETAILS FOR: " << iter->first << std::endl
					<< (iter->second)->getDetails() << std::endl;
	}
}


void
CSandboxManager::unregisterSandbox(const char* id)
{
	cout	<< "CSandboxManager::listRegisteredSandboxes(char*) called" 
			<< std::endl;
	string ids;
	ids.assign(id);
	this->unregisterSandbox(ids);
}


void
CSandboxManager::unregisterSandbox(const string id)
{
	// TODO: Need more details. Right now just delete the sandbox object
	map<string, CSandbox*>::iterator iter;
	cout 	<< "CSandboxManager::listRegisteredSandboxes called for " 
			<< id << std::endl;
	// Find and delete the sandbox object
	iter = sandboxMap.find(id);
	delete iter->second;
	sandboxMap.erase(id);
}


void
CSandboxManager::unregisterAllSandboxes(void)
{
	cout << "CSandboxManager::unregisterAllSandboxes called" << std::endl;
	map<string, CSandbox*>::iterator iter;
	for (iter = sandboxMap.begin(); iter != sandboxMap.end(); ++iter) {
		this->unregisterSandbox(iter->first);
	}
}

////////////////

void 
CSandboxManager::initIterator(void)
{
	this->m_iter = this->sandboxMap.begin();
}
	
std::string 
CSandboxManager::getNextSandboxId(void) 
{
	this->m_iter++;
	if (this->m_iter == this->sandboxMap.end())
		return "";
	return this->m_iter->first;
	

}


/*****************************************************************************
* PRIVATE MEMBERS FUNCTIONS
*****************************************************************************/

void
CSandboxManager::init(void)
{
	cout << "CSandboxManager::init called" << std::endl;
	// TODO: Fill in more details. Don't know yet
	return;
}
