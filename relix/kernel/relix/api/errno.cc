/*
	errno.cc
	--------
*/

#include "relix/api/errno.hh"

// Standard C
#include <errno.h>

// poseven
#include "poseven/types/errno_t.hh"

// relix-kernel
#include "relix/api/os_exception.hh"
#include "relix/signal/caught_signal.hh"


namespace relix
{
	
	namespace p7 = poseven;
	
	
	static int errno_from_exception()
	{
		try
		{
			throw;
		}
		catch ( const p7::errno_t& errnum )
		{
			return errnum;
		}
		catch ( const caught_signal& signal )
		{
			the_caught_signal = signal;
			
			return EINTR;
		}
		catch ( ... )
		{
			try
			{
				int errnum = errno_from_os_exception();
				
				if ( errnum > 0 )
				{
					return errnum;
				}
			}
			catch ( ... )
			{
			}
		}
		
		return EINVAL;
	}
	
	int set_errno_from_exception()
	{
		return set_errno( errno_from_exception() );
	}
	
}

