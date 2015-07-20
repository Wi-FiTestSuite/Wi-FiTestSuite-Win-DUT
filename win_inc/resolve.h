/*****************************************************************************
Copyright (c) Microsoft
All rights reserved.
Licensed under the Microsoft Limited Public License (the “License”); you may not
use this file except in compliance with the License.
You may obtain a copy of the License at http://msdn.microsoft.com/en-us/cc300389.

THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS
OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

See the Microsoft Limited Public License (Ms-LPL) for specific language governing
permissions and limitations under the License.
*****************************************************************************/

#ifndef _RESOLVE_H_
#define _RESOLVE_H_

#ifdef _cplusplus
extern "C" {
#endif

int              PrintAddress(SOCKADDR *sa, int salen);
int              FormatAddress(SOCKADDR *sa, int salen, char *addrbuf, int addrbuflen);
int              ReverseLookup(SOCKADDR *sa, int salen, char *namebuf, int namebuflen);
struct addrinfo *ResolveAddress(char *addr, char *port, int af, int type, int proto);

#ifdef _cplusplus
}
#endif

#endif
