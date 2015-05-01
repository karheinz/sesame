// Copyright (c) 2015, Karsten Heinze <karsten.heinze@sidenotes.de>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <cstdlib>
#ifdef __gnu_linux__
#include <grp.h>
#endif
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "sesame/utils/resources.hpp"


namespace sesame { namespace utils {

bool isRoot()
{
   return ( getuid() == 0 );
}

bool hasRootPrivileges()
{
   return ( geteuid() == 0 );
}

/*
 * Based on function spc_drop_privileges(int permanent) provided
 * in "Secure Programming Handbook for C and C++" (section 1.3).
 */
bool dropPrivileges()
{
   // Prepare.
   uid_t newuid( getuid() );
   uid_t olduid( geteuid() );
   gid_t newgid( getgid() );
   gid_t oldgid( getegid() );

   /*
    * If root privileges are to be dropped, be sure to pare down the ancillary
    * groups for the process before doing anything else because the setgroups()
    * system call requires root privileges.  Drop ancillary groups regardless of
    * whether privileges are being dropped temporarily or permanently.
    */
   setgroups( 1, &newgid );

   if ( newgid != oldgid )
   {
#ifdef __gnu_linux__
     if ( setregid( newgid, newgid ) == -1 )
     {
        return false;
     }
#else
     if ( setegid( newgid ) == -1 )
     {
        return false;
     }
     if ( setgid( newgid ) == -1 )
     {
        return false;
     }
#endif
   }

   if ( newuid != olduid )
   {
#ifdef __gnu_linux__
     if (setreuid( newuid, newuid ) == -1 )
     {
        return false;
     }
#else
     seteuid( newuid );
     if ( setuid( newuid ) == -1 )
     {
        return false;
     }
#endif
   }

   /* verify that the changes were successful */
   if ( newgid != oldgid && ( setegid( oldgid ) != -1 || getegid() != newgid ) )
   {
      return false;
   }
   else if ( newuid != olduid && ( seteuid( olduid ) != -1 || geteuid() != newuid) )
   {
      return false;
   }
   else
   {
      return true;
   }
}

bool disableCoreFiles()
{
   const struct rlimit maxCoreFileSize( { 0, 0 } );
   return ( setrlimit( RLIMIT_CORE, &maxCoreFileSize ) == 0 );
}

bool lockMemory()
{
   const struct rlimit maxLockableMemory( { RLIM_INFINITY, RLIM_INFINITY } );
   if ( setrlimit( RLIMIT_MEMLOCK, &maxLockableMemory ) == -1 )
   {
      return false;
   }

   return ( mlockall( MCL_CURRENT | MCL_FUTURE ) == 0 );
}

} }
