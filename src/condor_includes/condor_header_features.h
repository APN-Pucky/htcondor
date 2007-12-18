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

#ifndef CONDOR_SYS_FEATURES_H
#define CONDOR_SYS_FEATURES_H

#ifdef  __cplusplus
#define BEGIN_C_DECLS   extern "C" {
#define END_C_DECLS     }
#else
#define BEGIN_C_DECLS
#define END_C_DECLS
#endif

#if (defined(WIN32) && defined(_DLL)) 
#define DLL_IMPORT_MAGIC __declspec(dllimport)
#else
#define DLL_IMPORT_MAGIC  /* a no-op on Unix */
#endif

#endif /* CONDOR_SYS_FEATURES_H */
