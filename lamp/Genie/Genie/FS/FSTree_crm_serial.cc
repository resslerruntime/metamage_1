/*	====================
 *	FSTree_crm_serial.cc
 *	====================
 */

#if !TARGET_API_MAC_CARBON

#include "Genie/FS/FSTree_crm_serial.hh"

// iota
#include "iota/decimal.hh"

// poseven
#include "poseven/types/errno_t.hh"

// ClassicToolbox
#include "ClassicToolbox/CRMSerialDevices.h"

// Genie
#include "Genie/FS/basic_directory.hh"
#include "Genie/FS/FSTree_Generated.hh"
#include "Genie/FS/FSTree_Property.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	
	static inline N::CRMDeviceID GetKeyFromParent( const FSTreePtr& parent )
	{
		return N::CRMDeviceID( atoi( parent->Name().c_str() ) );
	}
	
	static N::CRMDeviceID GetKey( const FSTree* that )
	{
		return GetKeyFromParent( that->ParentRef() );
	}
	
	
	static N::CRMRecPtr GetCRMRecPtrFromID( N::CRMDeviceID id )
	{
		N::CRMResource_Container crmResources = N::CRMResources( N::crmSerialDevice );
		
		typedef N::CRMResource_Container::const_iterator Iter;
		
		for ( Iter it = crmResources.begin();  it != crmResources.end();  ++it )
		{
			N::CRMRecPtr rec = *it;
			
			if ( rec->crmDeviceID == id )
			{
				return rec;
			}
		}
		
		return NULL;
	}
	
	extern const FSTree_Premapped::Mapping sys_mac_crm_serial_N_Mappings[];
	
	static FSTreePtr serial_lookup( const FSTreePtr& parent, const std::string& name )
	{
		const N::CRMDeviceID key = N::CRMDeviceID( atoi( name.c_str() ) );
		
		// We can't just call N::CRMSearch< N::crmSerialDevice >( key ),
		// because that returns the *next* record.  So, linear search it is.
		
		if ( GetCRMRecPtrFromID( key ) == NULL )
		{
			poseven::throw_errno( ENOENT );
		}
		
		return Premapped_Factory< sys_mac_crm_serial_N_Mappings >( parent, name );
	}
	
	class crm_IteratorConverter
	{
		public:
			FSNode operator()( const N::CRMSerialDevice_Container::value_type& rec ) const
			{
				const int key = rec->crmDeviceID;
				
				const ino_t inode = key;
				
				std::string name = iota::inscribe_decimal( key );
				
				return FSNode( inode, name );
			}
	};
	
	static void serial_iterate( FSTreeCache& cache )
	{
		crm_IteratorConverter converter;
		
		N::CRMSerialDevice_Container sequence = N::CRMSerialDevices();
		
		std::transform( sequence.begin(),
		                sequence.end(),
		                std::back_inserter( cache ),
		                converter );
	}
	
	
	typedef StringHandle CRMSerialRecord::*StringSelector;
	
	static std::string GetSelectedString( const FSTree* that, StringSelector selector )
	{
		N::CRMDeviceID key = GetKey( that );
		
		N::CRMRecPtr crmRec = GetCRMRecPtrFromID( key );
		
		N::CRMSerialPtr serialPtr = NN::Convert< N::CRMSerialPtr >( crmRec );
		
		StringHandle h = serialPtr->*selector;
		
		return NN::Convert< std::string >( N::Str255( *h ) );
	}
	
	// These are necessary because CW Pro 6 doesn't support pointer-to-member template parameters.
	
	struct sys_mac_crm_serial_N_name
	{
		static std::string Read( const FSTree* that, bool binary )
		{
			return GetSelectedString( that, &CRMSerialRecord::name );
		}
	};
	
	struct sys_mac_crm_serial_N_input
	{
		static std::string Read( const FSTree* that, bool binary )
		{
			return GetSelectedString( that, &CRMSerialRecord::inputDriverName );
		}
	};
	
	struct sys_mac_crm_serial_N_output
	{
		static std::string Read( const FSTree* that, bool binary )
		{
			return GetSelectedString( that, &CRMSerialRecord::outputDriverName );
		}
	};
	
	
	class sys_mac_crm_serial_N_icon
	{
		private:
			typedef N::CRMDeviceID Key;
		
		public:
			static std::string Read( const FSTree* that )
			{
				Key key = GetKey( that );
				
				N::CRMRecPtr crmRec = GetCRMRecPtrFromID( key );
				
				N::CRMSerialPtr serialPtr = NN::Convert< N::CRMSerialPtr >( crmRec );
				
				CRMIconHandle iconHandle = serialPtr->deviceIcon;
				
				CRMIconRecord icon = **iconHandle;
				
				// 32-bit * 32-bit = 1024 bits = 128 bytes, x2 = 256 bytes
				const std::size_t maskedIconSize = sizeof icon.oldIcon + sizeof icon.oldMask;
				
				std::string iconData( (const char*) &icon.oldIcon, maskedIconSize );
				
				return iconData;
			}
	};
	
	
	static FSTreePtr Name_Factory( const FSTreePtr&    parent,
	                               const std::string&  name )
	{
		return New_FSTree_Property( parent,
		                            name,
		                            &sys_mac_crm_serial_N_name::Read );
	}
	
	static FSTreePtr Input_Factory( const FSTreePtr&    parent,
	                                const std::string&  name )
	{
		return New_FSTree_Property( parent,
		                            name,
		                            &sys_mac_crm_serial_N_input::Read );
	}
	
	static FSTreePtr Output_Factory( const FSTreePtr&    parent,
	                                 const std::string&  name )
	{
		return New_FSTree_Property( parent,
		                            name,
		                            &sys_mac_crm_serial_N_output::Read );

	}
	
	static FSTreePtr Icon_Factory( const FSTreePtr&    parent,
	                               const std::string&  name )
	{
		return New_FSTree_Generated( parent,
		                             name,
		                             &sys_mac_crm_serial_N_icon::Read );
	}
	
	const FSTree_Premapped::Mapping sys_mac_crm_serial_N_Mappings[] =
	{
		{ "name",   &Name_Factory   },
		{ "input",  &Input_Factory  },
		{ "output", &Output_Factory },
		{ "icon",   &Icon_Factory   },
		
		{ NULL, NULL }
	};
	
	FSTreePtr New_FSTree_sys_mac_crm_serial( const FSTreePtr& parent, const std::string& name )
	{
		return new_basic_directory( parent, name, serial_lookup, serial_iterate );
	}
	
}

#endif

