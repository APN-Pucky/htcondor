/*********************************************************************
 *
 * Condor ClassAd library
 * Copyright (C) 1990-2001, CONDOR Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI, and Rajesh Raman.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 *
 *********************************************************************/

#ifndef __COLLECTION_H__
#define __COLLECTION_H__

#include "view.h"
#include "source.h"
#include "sink.h"

BEGIN_NAMESPACE( classad );


class ClassAdCollectionInterface {
public:
	ClassAdCollectionInterface( );
	virtual ~ClassAdCollectionInterface( );

	enum {
	    ClassAdCollOp_NoOp                  = 10000,

		__ClassAdCollOp_ViewOps_Begin__,
		ClassAdCollOp_CreateSubView         = __ClassAdCollOp_ViewOps_Begin__,
		ClassAdCollOp_CreatePartition       ,
		ClassAdCollOp_DeleteView            ,
		ClassAdCollOp_SetViewInfo           ,
		ClassAdCollOp_AckViewOp             ,
		__ClassAdCollOp_ViewOps_End__       = ClassAdCollOp_AckViewOp,

		__ClassAdCollOp_ClassAdOps_Begin__,
		ClassAdCollOp_AddClassAd            =__ClassAdCollOp_ClassAdOps_Begin__,
		ClassAdCollOp_UpdateClassAd         ,
		ClassAdCollOp_ModifyClassAd         ,
		ClassAdCollOp_RemoveClassAd         ,
		ClassAdCollOp_AckClassAdOp          ,
		__ClassAdCollOp_ClassAdOps_End__    = ClassAdCollOp_AckClassAdOp,

		__ClassAdCollOp_XactionOps_Begin__,
		ClassAdCollOp_OpenTransaction       =__ClassAdCollOp_XactionOps_Begin__,
		ClassAdCollOp_AckOpenTransaction    ,
		ClassAdCollOp_CommitTransaction     ,
		ClassAdCollOp_AbortTransaction      ,
		ClassAdCollOp_AckCommitTransaction  ,
		ClassAdCollOp_ForgetTransaction     ,
		__ClassAdCollOp_XactionOps_End__    = ClassAdCollOp_ForgetTransaction,

		__ClassAdCollOp_ReadOps_Begin__     ,
		ClassAdCollOp_GetClassAd            = __ClassAdCollOp_ReadOps_Begin__,
		ClassAdCollOp_GetViewInfo           ,
		ClassAdCollOp_GetSubordinateViewNames,
		ClassAdCollOp_GetPartitionedViewNames,
		ClassAdCollOp_FindPartitionName     ,
		ClassAdCollOp_IsActiveTransaction   ,
		ClassAdCollOp_IsCommittedTransaction,
		ClassAdCollOp_GetAllActiveTransactions,
		ClassAdCollOp_GetAllCommittedTransactions,
		ClassAdCollOp_GetServerTransactionState,
		ClassAdCollOp_AckReadOp             ,
		__ClassAdCollOp_ReadOps_End__       = ClassAdCollOp_AckReadOp,

		__ClassAdCollOp_MiscOps_Begin__     ,
		ClassAdCollOp_Connect               = __ClassAdCollOp_MiscOps_Begin__,
		ClassAdCollOp_QueryView             ,
		ClassAdCollOp_Disconnect            ,
		__ClassAdCollOp_MiscOps_End__       = ClassAdCollOp_Disconnect
	};

	static const char * const CollOpStrings[];

	enum AckMode { _DEFAULT_ACK_MODE, WANT_ACKS, DONT_WANT_ACKS };

		// outcome from a commit
	enum { XACTION_ABORTED, XACTION_COMMITTED, XACTION_UNKNOWN };

        // Logfile control
	virtual bool InitializeFromLog( const string &filename ) = 0;
	bool TruncateLog( );


		// View creation/deletion/interrogation
	virtual bool CreateSubView( const ViewName &viewName,
				const ViewName &parentViewName,
				const string &constraint, const string &rank,
				const string &partitionExprs ) = 0;
	virtual bool CreatePartition( const ViewName &viewName,
				const ViewName &parentViewName,
				const string &constraint, const string &rank,
				const string &partitionExprs, ClassAd *rep ) = 0;
	virtual bool DeleteView( const ViewName &viewName ) = 0;
	virtual bool SetViewInfo( const ViewName &viewName, 
				const string &constraint, const string &rank, 
				const string &partitionAttrs ) = 0;
	virtual bool GetViewInfo( const ViewName &viewName, ClassAd *&viewInfo )=0;
		// Child view interrogation
	virtual bool GetSubordinateViewNames( const ViewName &viewName,
				vector<string>& views ) = 0;
	virtual bool GetPartitionedViewNames( const ViewName &viewName,
				vector<string>& views ) = 0;
	virtual bool FindPartitionName( const ViewName &viewName, ClassAd *rep, 
				ViewName &partition ) = 0;


		// ClassAd manipulation 
	virtual bool AddClassAd( const string &key, ClassAd *newAd ) = 0;
	virtual bool UpdateClassAd( const string &key, ClassAd *updateAd ) = 0;
	virtual bool ModifyClassAd( const string &key, ClassAd *modifyAd ) = 0;
	virtual bool RemoveClassAd( const string &key ) = 0;
	virtual ClassAd *GetClassAd(const string &key ) = 0;


		// Mode management
	bool SetAcknowledgementMode( AckMode );
	AckMode GetAcknowledgementMode( ) const { return( amode ); }


		// Transaction management
	virtual bool OpenTransaction( const string &xactionName) = 0;
	bool SetCurrentTransaction( const string &xactionName );
	void GetCurrentTransaction( string &xactionName ) const;
	virtual bool CloseTransaction( const string &xactionName, bool commit,
				int &outcome )=0;

	virtual bool IsMyActiveTransaction( const string &xactionName ) = 0;
	virtual void GetMyActiveTransactions( vector<string>& ) = 0;
	virtual bool IsActiveTransaction( const string &xactionName ) = 0;
	virtual bool GetAllActiveTransactions( vector<string>& ) = 0;
	virtual bool IsCommittedTransaction( const string &xactionName ) = 0;
	virtual bool GetAllCommittedTransactions( vector<string>& ) = 0;


		// misc
	static inline const char *GetOpString( int op ) {
		return( op>=ClassAdCollOp_NoOp && op<=__ClassAdCollOp_MiscOps_End__ ? 
				CollOpStrings[op-ClassAdCollOp_NoOp] : "(unknown)" );
	}

protected:
		// Utility functions to make collection log records
	ClassAd *_CreateSubView( const ViewName &viewName,
				const ViewName &parentViewName,
				const string &constraint, const string &rank,
				const string &partitionExprs );
	ClassAd *_CreatePartition( const ViewName &viewName,
				const ViewName &parentViewName,
				const string &constraint, const string &rank,
				const string &partitionExprs, ClassAd *rep );
	ClassAd *_DeleteView( const ViewName &viewName );
	ClassAd *_SetViewInfo( const ViewName &viewName, 
				const string &constraint, const string &rank, 
				const string &partitionAttrs );
	ClassAd *_AddClassAd( const string &xactionName, 
				const string &key, ClassAd *newAd );
	ClassAd *_UpdateClassAd( const string &xactionName, 
				const string &key, ClassAd *updateAd );
	ClassAd *_ModifyClassAd( const string &xactionName, 
				const string &key, ClassAd *modifyAd );
	ClassAd *_RemoveClassAd( const string &xactionName,
				const string &key );

		// mode management data
	AckMode			amode;
	string			currentXactionName;

		// function which executes log records in recovery mode
	virtual bool OperateInRecoveryMode( ClassAd* ) = 0;

		// utility functions to operate on logs and log metadata
	ClassAd 		*ReadLogEntry( FILE * );
	bool			WriteLogEntry( FILE *, ClassAd *, bool sync=true );
	bool 			ReadLogFile( );
	string 			logFileName;
	ClassAdParser	parser;
	ClassAdUnParser	unparser;
	FILE			*log_fp;

		// override for client and server
	virtual bool LogState( FILE * ) = 0;
};


END_NAMESPACE

#endif
