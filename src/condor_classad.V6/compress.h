/*********************************************************************
 *
 * Condor ClassAd library
 * Copyright (C) 1990-2003, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI and Rajesh Raman.
 *
 * This source code is covered by the Condor Public License, which can
 * be found in the accompanying LICENSE file, or online at
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
 *********************************************************************/


#if !defined( COMPRESS_H )
#define COMPRESS_H

#include "classad/classad_stl.h"
#include "classad/classad.h"

BEGIN_NAMESPACE(classad)

class ClassAdBin
{
	public:
    ClassAdBin( );
    ~ClassAdBin( );

    int         count;
    ClassAd     *ad;
};

typedef classad_hash_map< std::string, ClassAdBin*, StringHash > CompressedAds;

bool MakeSignature( ClassAd *ad, References &refs, std::string &sig );
bool Compress( ClassAdCollectionServer *server, LocalCollectionQuery *query,
			   const References &refs, CompressedAds& comp, 
			   std::list<ClassAd*> &rest);

END_NAMESPACE
#endif
