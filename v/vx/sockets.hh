/*
	sockets.hh
	----------
*/

#ifndef SOCKETS_HH
#define SOCKETS_HH

// vlib
#include "vlib/proc_info.hh"


namespace vlib
{
	
	extern const proc_info proc_accept;
	extern const proc_info proc_tcpconnect;
	extern const proc_info proc_tcplisten;
	
}

#endif
