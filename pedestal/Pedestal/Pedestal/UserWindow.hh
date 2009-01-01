/*	=============
 *	UserWindow.hh
 *	=============
 */

#ifndef PEDESTAL_USERWINDOW_HH
#define PEDESTAL_USERWINDOW_HH

// Standard C++
#include <memory>

// Pedestal
#include "Pedestal/Superview.hh"
#include "Pedestal/Window.hh"


namespace Pedestal
{
	
	class UserView : public Superview
	{
		private:
			boost::shared_ptr< View > itsView;
		
		public:
			~UserView();
			
			View& Subview()  { return *itsView; }
			
			void Set( boost::shared_ptr< View > const& view )  { itsView = view; }
			void Set( std::auto_ptr    < View >        view )  { itsView = view; }
			
			View      & Get()        { return *itsView; }
			View const& Get() const  { return *itsView; }
			
			template < class ViewType >
			ViewType& Get()  { return static_cast< ViewType& >( *itsView ); }
			
			template < class ViewType >
			const ViewType& Get() const  { return static_cast< const ViewType& >( *itsView ); }
			
			void Resize( short width, short height )  { itsView->Resize( width, height ); }
			
			void Draw( const Rect& bounds )  { itsView->Draw( bounds ); }
	};
	
	class UserWindow : public Window
	{
		private:
			boost::shared_ptr< View > itsView;
		
		public:
			UserWindow( const NewWindowContext&  context,
			            DefProcID                defProcID = DefProcID() )
			:
				Window( context, defProcID )
			{
			}
			
			boost::shared_ptr< View >& GetView()  { return itsView; }
	};
	
}

#endif

