#ifndef CFTP_FRAMES_H
#define CFTP_FRAMES_H

/*
  This is a definition of all the frame data structures used by the
  Cluster File Transfer Protocol for its network operations. The description
  of these structures was taken almost verbatim from the CFTP design document,
  with some minor changes to the error code values.
*/


/*
  These are the types of messages that the CFTP understands. Each
  identifier cooresponds to a single frame struct type below.
*/
enum 
MESSAGE_TYPES {
		// Discovery Frames
	DSF = 0x00,
	DRF = 0x01,
		// Session Frames
	SIF = 0x10,
	SAF = 0x11,
	SRF = 0x12,
	SCF = 0x13,
		// Transfer Frames
	DTF = 0x20,
	DAF = 0x21,
		// Teardown Frames
	FFF = 0x30,
	FAF = 0x31,
};

enum 
PARAMETER_FORMAT {
	CLASSAD = 0x00, // The format of parameters is via Condor's ClassAds
	SIMPLE = 0x01,
		// TODO: Other formats to come later
};

enum 
ERROR_CODES {
	NOERROR = 0x00,
	CLIENT_TIMEOUT = 0x01,
	SERVER_TIMEOUT = 0x02,
	UNKNOWN_FORMAT = 0x03,
	OVERLOADED = 0x04,
	TIMEOUT = 0x05,
	DUPLICATE_SESSION_TOKEN = 0x06,
	MISSING_PARAMETER = 0x07,
	NO_SESSION = 0x08,
	UNACCEPTABLE_PARAMETERS = 0x09,
	NO_DISK_SPACE = 0x0A,
};


/*
  Discovery Search Frame
*/

typedef struct _cftp_dsf_frame {
	unsigned char      MessageType;
	unsigned short int Reserved;
	unsigned char      AddressType;
	unsigned int       IPv4Address;
	unsigned int       IPv6Address[4];
	unsigned short int IPv4Port;
	unsigned short int IPv6Port;
} cftp_dsf_frame;

/*
  Discovery Response Frame
*/

typedef struct _cftp_drf_frame {
    unsigned char      MessageType;
    unsigned short int Reserved;
    unsigned char      AddressType;
    unsigned int       IPv4Address;
    unsigned int       IPv6Address[4];
    unsigned short int IPv4Port;
    unsigned short int IPv6Port;
} cftp_drf_frame;

/*
  Session Initiation Frame
*/

typedef struct _cftp_sif_frame {
	unsigned char      MessageType;
	unsigned short int ErrorCode;
	unsigned char      SessionToken;
	unsigned short int ParameterFormat;
	unsigned short int ParameterLength;
} cftp_sif_frame;

/*
   Session Acknowledgement Frame
*/

typedef struct _cftp_saf_frame {
	unsigned char      MessageType;
	unsigned short int ErrorCode;
    unsigned char      SessionToken;
    unsigned short int ParameterFormat;
    unsigned short int ParameterLength;
} cftp_saf_frame;

/*
  Session Ready Frame
*/

typedef struct _cftp_srf_frame {
	unsigned char      MessageType;
	unsigned short int ErrorCode;
    unsigned char      SessionToken;
} cftp_srf_frame;

/*
  Session Close Frame
*/

typedef struct _cftp_scf_frame {
	unsigned char      MessageType;
	unsigned short int ErrorCode;
    unsigned char      SessionToken;
} cftp_scf_frame;

/*
  Data Transfer Frame
*/

typedef struct _cftp_dtf_frame {
    unsigned char      MessageType;
    unsigned short int ErrorCode;
    unsigned char      SessionToken;
	unsigned short int DataSize;	
	unsigned short int BlockNum;
} cftp_dtf_frame;

/*
  Data Received Frame
*/

typedef struct _cftp_daf_frame {
    unsigned char      MessageType;
    unsigned short int ErrorCode;
    unsigned char      SessionToken;
} cftp_daf_frame;

/*
  File Finished Frame
*/ 

typedef struct _cftp_fff_frame {
    unsigned char      MessageType;
    unsigned short int ErrorCode;
    unsigned char      SessionToken;
} cftp_fff_frame;

/*
  File Acknowledgement Frame
*/

typedef struct _cftp_faf_frame {
    unsigned char      MessageType;
    unsigned short int ErrorCode;
    unsigned char      SessionToken;
} cftp_faf_frame;



#endif
