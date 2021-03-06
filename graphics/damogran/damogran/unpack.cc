/*
	unpack.cc
	---------
*/

#include "damogran/unpack.hh"

// Standard C
#include <string.h>


namespace damogran
{

typedef signed char int8_t;

const uint8_t* validate( const uint8_t* src, const uint8_t* end, long size )
{
	bool unaligned = false;
	
	do
	{
		if ( src + 2 > end )  return NULL;
		
		const int8_t c0 = *src++;
		const int8_t c1 = *src++;
		
		unaligned = ! unaligned;
		
		if ( c0 == 0 )
		{
			if ( c1 == 0 )
			{
				continue;
			}
			
			if ( c1 < 0 )
			{
				const size_t n = 2 * (uint8_t) -c1;
				
				if ( n & 0x2 )
				{
					unaligned = ! unaligned;
				}
				
				if ( (size -= n) < 0 )  return NULL;
				
				src += n;
				
				continue;
			}
			
			// c1 > 0
			
			unaligned = ! unaligned;
			
			if ( src + 2 > end )  return NULL;
			
			const uint8_t c2 = *src++;
			const uint8_t c3 = *src++;
			
			const size_t n = 2 * (c1 * 256 + c2 + 1);
			
			if ( (size -= n) < 0 )  return NULL;
		}
		else if ( c0 > 0 )
		{
			const size_t n = 2 * (c0 + 1);
			
			if ( (size -= n) < 0 )  return NULL;
		}
		else
		{
			return NULL;  // (c0 < 0) is undefined
		}
	}
	while ( unaligned  ||  size > 0 );
	
	return src;
}

const uint8_t* unpack( const uint8_t* src, uint8_t* dst, uint8_t* end )
{
	bool unaligned = false;
	
	do
	{
		const int8_t c0 = *src++;
		const int8_t c1 = *src++;
		
		unaligned = ! unaligned;
		
		if ( c0 == 0 )
		{
			if ( c1 == 0 )
			{
				continue;
			}
			
			if ( c1 < 0 )
			{
				const size_t n = 2 * (uint8_t) -c1;
				
				if ( n & 0x2 )
				{
					unaligned = ! unaligned;
				}
				
				memcpy( dst, src, n );
				
				src += n;
				dst += n;
				
				continue;
			}
			
			// c1 > 0
			
			unaligned = ! unaligned;
			
			const uint8_t c2 = *src++;
			const uint8_t c3 = *src++;
			
			const size_t n = 2 * (c1 * 256 + c2 + 1);
			
			memset( dst, c3, n );
			
			dst += n;
		}
		else if ( c0 > 0 )
		{
			const size_t n = 2 * (c0 + 1);
			
			memset( dst, c1, n );
			
			dst += n;
		}
	}
	while ( unaligned  ||  dst < end );
	
	return src;
}

}  // namespace damogran
