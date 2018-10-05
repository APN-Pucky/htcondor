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
#include "credd.h"
#include "condor_config.h"
#include "condor_daemon_core.h"
#include "condor_debug.h"
#include "store_cred.h"
#include "internet.h"
#include "get_daemon_name.h"
#include "subsystem_info.h"
#include "credmon_interface.h"
#include "ipv6_hostname.h"
#include "secure_file.h"

//-------------------------------------------------------------

CredDaemon *credd;

CredDaemon::CredDaemon() : m_name(NULL), m_update_collector_tid(-1)
{

	reconfig();

		// Command handler for the user condor_store_cred tool
	daemonCore->Register_Command( STORE_CRED, "STORE_CRED", 
								(CommandHandler)&store_cred_handler, 
								"store_cred_handler", NULL, WRITE, 
								D_FULLDEBUG, true /*force authentication*/ );

		// Command handler for daemons to get the password
	daemonCore->Register_Command( CREDD_GET_PASSWD, "CREDD_GET_PASSWD", 
								(CommandHandler)&get_cred_handler,
								"get_cred_handler", NULL, DAEMON,
								D_FULLDEBUG, true /*force authentication*/ );

		// NOP command for testing authentication
	daemonCore->Register_Command( CREDD_NOP, "CREDD_NOP",
								(CommandHandlercpp)&CredDaemon::nop_handler,
								"nop_handler", this, DAEMON,
								D_FULLDEBUG );

		// NOP command for testing authentication
	daemonCore->Register_Command( CREDD_REFRESH_ALL, "CREDD_REFRESH_ALL",
								(CommandHandlercpp)&CredDaemon::refresh_all_handler,
								"refresh_all_handler", this, DAEMON,
								D_FULLDEBUG );

		// See if creds are present for all modules requested
	daemonCore->Register_Command( ZKM_QUERY_CREDS, "ZKM_QUERY_CREDS",
								(CommandHandlercpp)&CredDaemon::zkm_query_creds,
								"zkm_query_creds", this, DAEMON,
								D_FULLDEBUG );

		// set timer to periodically advertise ourself to the collector
	daemonCore->Register_Timer(0, m_update_collector_interval,
		(TimerHandlercpp)&CredDaemon::update_collector, "update_collector", this);

	// only sweep if we have a cred dir
	char* p = param("SEC_CREDENTIAL_DIRECTORY");
	if(p) {
		// didn't need the value, just to see if it's defined.
		free(p);
		dprintf(D_FULLDEBUG, "CREDD: setting sweep_timer_handler\n");
		int sec_cred_sweep_interval = param_integer("SEC_CREDENTIAL_SWEEP_INTERVAL", 30);
		m_cred_sweep_tid = daemonCore->Register_Timer( sec_cred_sweep_interval, sec_cred_sweep_interval,
								(TimerHandlercpp)&CredDaemon::sweep_timer_handler,
								"sweep_timer_handler", this );
	}
}

CredDaemon::~CredDaemon()
{
	// tell our collector we're going away
	invalidate_ad();

	if (m_name != NULL) {
		free(m_name);
	}
}

void
CredDaemon::reconfig()
{
	// get our daemon name; if CREDD_HOST is defined, we use it
	// as our name. this is because clients that have CREDD_HOST
	// defined will query the Collector for a CredD ad that has
	// a name matching its setting for CREDD_HOST. but CREDD_HOST
	// will not necessarily match what default_daemon_name()
	// returns - for example, if CREDD_HOST is set to a DNS alias
	//
	if (m_name != NULL) {
		free(m_name);
	}
	m_name = param("CREDD_HOST");
	if (m_name == NULL) {
		char* tmp = default_daemon_name();
		ASSERT(tmp != NULL);
		m_name = strdup(tmp);
		ASSERT(m_name != NULL);
		delete[] tmp;
	}
	if(m_name == NULL) {
		EXCEPT("default_daemon_name() returned NULL");
	}

	// how often do we update the collector?
	m_update_collector_interval = param_integer ("CREDD_UPDATE_INTERVAL",
												 5 * MINUTE);

	// fill in our classad
	initialize_classad();

	// reset the timer (if it exists) to update the collector
	if (m_update_collector_tid != -1) {
		daemonCore->Reset_Timer(m_update_collector_tid, 0, m_update_collector_interval);
	}
}

void
CredDaemon::sweep_timer_handler( void )
{
	dprintf(D_FULLDEBUG, "CREDD: calling and resetting sweep_timer_handler()\n");
#ifndef WIN32
	credmon_sweep_creds();
#endif  // WIN32
	int sec_cred_sweep_interval = param_integer("SEC_CREDENTIAL_SWEEP_INTERVAL", 30);
	daemonCore->Reset_Timer (m_cred_sweep_tid, sec_cred_sweep_interval, sec_cred_sweep_interval);
}


void
CredDaemon::initialize_classad()
{
	m_classad.Clear();

	SetMyTypeName(m_classad, CREDD_ADTYPE);
	SetTargetTypeName(m_classad, "");

	MyString line;

	line.formatstr("%s = \"%s\"", ATTR_NAME, m_name );
	m_classad.Insert(line.Value());

	line.formatstr ("%s = \"%s\"", ATTR_CREDD_IP_ADDR,
			daemonCore->InfoCommandSinfulString() );
	m_classad.Insert(line.Value());

        // Publish all DaemonCore-specific attributes, which also handles
        // SUBSYS_ATTRS for us.
    daemonCore->publish(&m_classad);
}

void
CredDaemon::update_collector()
{
	daemonCore->sendUpdates(UPDATE_AD_GENERIC, &m_classad, NULL, true);
}

void
CredDaemon::invalidate_ad()
{
	ClassAd query_ad;
	SetMyTypeName(query_ad, QUERY_ADTYPE);
	SetTargetTypeName(query_ad, CREDD_ADTYPE);

	MyString line;
	line.formatstr("%s = TARGET.%s == \"%s\"", ATTR_REQUIREMENTS, ATTR_NAME, m_name);
    query_ad.Insert(line.Value());
	query_ad.Assign(ATTR_NAME,m_name);

	daemonCore->sendUpdates(INVALIDATE_ADS_GENERIC, &query_ad, NULL, true);
}

void
CredDaemon::nop_handler(int, Stream*)
{
	return;
}


void
CredDaemon::zkm_query_creds( int, Stream* s)
{
	ReliSock* r = (ReliSock*)s;
	r->decode();
	MyString user, modules;
	r->code(user);
	r->code(modules);
	r->end_of_message();

	dprintf(D_ALWAYS, "ZKM: got zkm_query_creds for user %s modules %s\n", user.Value(), modules.Value());

	MyString URL;

	StringList mod(modules.Value(),",");
	MyString missing;
	char* onemod;
	mod.rewind();
	while(onemod=mod.next()) {
		MyString tmpfname = onemod;
		tmpfname += ".top";

		dprintf(D_ALWAYS, "ZKM: looking for %s\n", tmpfname.Value());
		if (!credmon_poll_continue(user.Value(), 0, tmpfname.Value())) {
			if(!missing.empty()) {
				missing += ",";
			}
			dprintf(D_ALWAYS, "ZKM: missing %s\n", tmpfname.Value());
			missing += onemod;
		}
	}

	if(!missing.empty()) {
		// create unique request file with classad metadata
		char *key = Condor_Crypt_Base::randomHexKey(32);

		MyString contents;

		StringList mis(missing.Value(),",");
		char* onemis;
		mis.rewind();
		while(onemis = mis.next()) {
			// fill in everything we need to pass
			ClassAd ad;
			MyString tmpname;
			MyString tmpvalue;
			ad.Assign("LocalUser", user);
			ad.Assign("Provider", onemis);

			tmpname = onemis;
			tmpname += "_CLIENT_ID";
			param(tmpvalue, tmpname.c_str());
			ad.Assign("ClientId", tmpvalue);

			// this is a hack.  secret needs to be in a root-owned file, not in config.
			tmpname = onemis;
			tmpname += "_CLIENT_SECRET";
			param(tmpvalue, tmpname.c_str());
			ad.Assign("ClientSecret", tmpvalue);

			tmpname = onemis;
			tmpname += "_RETURN_URL_SUFFIX";
			param(tmpvalue, tmpname.c_str());

			MyString hn = get_local_fqdn();
			char* h = param("ZKMHOST");
			if(h) {
				hn = h;
				free(h);
			}
			URL = "https://";
			URL += hn;
			URL += tmpvalue;
			ad.Assign("ReturnUrl", URL);
			URL = "";

			tmpname = onemis;
			tmpname += "_AUTHORIZATION_URL";
			param(tmpvalue, tmpname.c_str());
			ad.Assign("AuthorizationUrl", tmpvalue);

			tmpname = onemis;
			tmpname += "_TOKEN_URL";
			param(tmpvalue, tmpname.c_str());
			ad.Assign("TokenUrl", tmpvalue);

			tmpname = onemis;
			tmpname += "_USER_URL";
			param(tmpvalue, tmpname.c_str());
			ad.Assign("UserUrl", tmpvalue);

			// serialize classad into string
			MyString ad_string;
			sPrintAd(ad_string, ad);

			dprintf(D_ALWAYS, "ZKM: appending:%s\n", ad_string.Value());
			if(!contents.empty()) {
				contents += "\n";
			}
			contents += ad_string;
		}

		// write the file into sec_credential_dir
	        auto_free_ptr cred_dir(param("SEC_CREDENTIAL_DIRECTORY"));
		if(!cred_dir) {
			EXCEPT("NO SEC_CREDENTIAL_DIRECTORY");
		}

		MyString path = (char*)(cred_dir.ptr());
		path += "/";
		path += key;

		dprintf(D_ALWAYS, "ZKM: writing to %s.  data len %i:\n%s\n", path.Value(), contents.Length(), contents.Value());
	        int rc = write_secure_file(path.Value(), contents.Value(), contents.Length(), true);

		if (rc != SUCCESS) {
			dprintf(D_ALWAYS, "ZKM: failed to write secure temp file %s\n", path.Value());
		} else {
			MyString hn = get_local_fqdn();
			char* h = param("ZKMHOST");
			if(h) {
				hn = h;
				free(h);
			}
			URL = "https://";
			URL += hn;
			URL += "/key/";
			URL += key;
		}

		dprintf(D_ALWAYS, "ZKM: sending URL '%s'\n", URL.Value());
	}

	r->encode();
	r->code(URL);
	r->end_of_message();
}

void
CredDaemon::refresh_all_handler( int, Stream* s)
{
	ReliSock* r = (ReliSock*)s;
	r->decode();
	ClassAd ad;
	getClassAd(r, ad);
	r->end_of_message();

	// don't actually care (at the moment) what's in the ad, it's for
	// forward/backward compatibility.

#ifndef WIN32
	// refresh ALL credentials
	if(credmon_poll( NULL, true, true )) {
		ad.Assign("result", "success");
	} else {
		ad.Assign("result", "failure");
	}
#else   // WIN32
	// this command handler shouldn't be getting called on windows, so fail.
	ad.Assign("result", "failure");
#endif  // WIN32

	r->encode();
	dprintf(D_SECURITY | D_FULLDEBUG, "CREDD: refresh_all sending response ad:\n");
	dPrintAd(D_SECURITY | D_FULLDEBUG, ad);
	putClassAd(r, ad);
	r->end_of_message();
}

//-------------------------------------------------------------

void
main_init(int /*argc*/, char * /*argv*/ [])
{
	dprintf(D_ALWAYS, "main_init() called\n");

	credd = new CredDaemon;
}

//-------------------------------------------------------------

void
main_config()
{
	dprintf(D_ALWAYS, "main_config() called\n");

	credd->reconfig();
}

//-------------------------------------------------------------

void main_shutdown_fast()
{
	dprintf(D_ALWAYS, "main_shutdown_fast() called\n");

	delete credd;

	DC_Exit(0);
}

//-------------------------------------------------------------

void main_shutdown_graceful()
{
	dprintf(D_ALWAYS, "main_shutdown_graceful() called\n");

	delete credd;

	DC_Exit(0);
}

//-------------------------------------------------------------

int
main( int argc, char **argv )
{
	set_mySubSystem( "CREDD", SUBSYSTEM_TYPE_DAEMON );

	dc_main_init = main_init;
	dc_main_config = main_config;
	dc_main_shutdown_fast = main_shutdown_fast;
	dc_main_shutdown_graceful = main_shutdown_graceful;
	return dc_main( argc, argv );
}
