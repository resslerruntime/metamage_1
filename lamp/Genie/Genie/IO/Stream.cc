/*	=========
 *	Stream.cc
 *	=========
 */

#include "Genie/IO/Stream.hh"

// Standard C++
#include <algorithm>

// poseven
#include "poseven/types/errno_t.hh"

// vfs
#include "vfs/filehandle/functions/nonblocking.hh"

// Genie
#include "Genie/api/yield.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	
	
	StreamHandle::StreamHandle( int                                flags,
	                            const vfs::filehandle_method_set*  methods )
	:
		IOHandle( flags, methods )
	{
	}
	
	StreamHandle::StreamHandle( const vfs::node*                   file,
	                            int                                flags,
	                            const vfs::filehandle_method_set*  methods )
	:
		vfs::filehandle( file, flags, methods )
	{
	}
	
	StreamHandle::~StreamHandle()
	{
	}
	
	ssize_t StreamHandle::SysRead( char* data, std::size_t byteCount )
	{
		p7::throw_errno( EPERM );
		
		return 0;
	}
	
	ssize_t StreamHandle::SysWrite( const char* data, std::size_t byteCount )
	{
		p7::throw_errno( EPERM );
		
		return 0;
	}
	
	void StreamHandle::TryAgainLater() const
	{
		try_again( is_nonblocking( *this ) );
	}
	
}

