/*	===================
 *	Vertice/Document.cc
 *	===================
 */

#include "Vertice/Document.hh"

// Standard C
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

// Standard C/C++
#include <cstdlib>

// Standard C++
#include <algorithm>
#include <list>
#include <map>
#include <string>
#include <vector>

// Io
#include "Io/TextInput.hh"

// Vectoria
#include "Vectoria/Point3D.hh"
#include "Vectoria/TriColor.hh"
#include "Vectoria/Units.hh"

// Vertice
#include "Vertice/Model.hh"
#include "Vertice/Port.hh"


namespace Vertice
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	namespace Ped = Pedestal;
	
	using V::X;
	using V::Y;
	using V::Z;
	
	
	class ParseError {};
	
	
	template < class Point >
	inline V::Translation MakeTranslation( const Point& pt )
	{
		return V::Translation( pt[ X ],
		                       pt[ Y ],
		                       pt[ Z ] );
	}
	
	static Rect MakeWindowRect()
	{
		Rect screenBounds = N::GetQDGlobalsScreenBits().bounds;
		short mbarHeight = ::GetMBarHeight();
		Rect rect = screenBounds;
		rect.top += mbarHeight + 22;
		rect.bottom -= 7;
		rect.left = 6;
		rect.right -= 7;
		short length = std::min( rect.bottom - rect.top, 400 );
		rect.bottom = rect.top + length;
		//short vMargin = ( screenBounds.bottom - rect.bottom ) - mbarHeight;
		short hMargin = ( screenBounds.right - length );
		rect.left = hMargin / 2;
		rect.right = hMargin / 2 + length;
		
		return rect;
		return N::InsetRect( rect, 4, 4 );
	}
	
	Window::Window( const boost::shared_ptr< Ped::WindowCloseHandler >&  handler, ConstStr255Param title )
	: 
		Ped::Window< PortView >( Ped::NewWindowContext( MakeWindowRect(),
		                                                title ),
		                         handler )
	{
	}
	
	class Parser
	{
		public:
			typedef std::map< std::string, V::Point3D::Type > PointMap;
			typedef std::map< std::string, ColorMatrix      > ColorMap;
			typedef std::map< std::string, IntensityMap     > IntensityMapMap;
		
		private:
			Scene*               itsScene;
			ColorMatrix          itsColor;
			V::Point3D::Type     itsOrigin;
			V::Degrees           itsTheta;
			V::Degrees           itsPhi;
			std::size_t          itsContextID;
			PointMap             itsPoints;
			ColorMap             itsColors;
			IntensityMapMap      itsIntensityMaps;
			const IntensityMap*  itsIntensityMap;
		
		public:
			Parser()  {}
			
			Parser( Scene& scene );
			
			~Parser()  {}
			
			void ParseLine( const std::string& line );
			
			ColorMatrix ReadColor( const char* begin, const char* end ) const;
			
			void Define        ( const char* begin, const char* end );
			void SetContext    ( const char* begin, const char* end );
			void MakeCamera    ( const char* begin, const char* end );
			void SetColor      ( const char* begin, const char* end );
			void SetMap        ( const char* begin, const char* end );
			void SetOrigin     ( const char* begin, const char* end );
			void Translate     ( const char* begin, const char* end );
			void SetTheta      ( const char* begin, const char* end );
			void SetPhi        ( const char* begin, const char* end );
			void AddMeshPoint  ( const char* begin, const char* end );
			void AddMeshPolygon( const char* begin, const char* end );
	};
	
	
	ColorMatrix Parser::ReadColor( const char* begin, const char* end ) const
	{
		double red, green, blue;
		
		int scanned = std::sscanf( begin, "%lf %lf %lf", &red, &green, &blue );
		
		if ( scanned == 3 )
		{
			return V::MakeRGB( red, green, blue );
		}
		else
		{
			ColorMap::const_iterator it = itsColors.find( std::string( begin, end ) );
			
			if ( it != itsColors.end() )
			{
				return it->second;
			}
		}
		
		throw ParseError();
	}
	
	static IntensityMap ReadMap( const char* begin, const char* end )
	{
		std::vector< double > values;
		
		errno = 0;
		
		while ( begin < end )
		{
			char* p = NULL;
			
			double value = std::strtod( begin, &p );
			
			if ( errno != 0 )
			{
				break;
			}
			
			begin = p;
			
			values.push_back( value );
		}
		
		std::size_t size = values.size();
		
		bool size_is_even_power_of_2 = (size & (size - 1)) == 0  &&  (size & 0x55555555) != 0;
		
		if ( !size_is_even_power_of_2 )
		{
			throw ParseError();
		}
		
		// Look, ma!  No branches!
		unsigned width = size >> 0 &   1
		               | size >> 1 &   2
		               | size >> 2 &   4
		               | size >> 3 &   8
		               | size >> 4 &  16
		               | size >> 5 &  32
		               | size >> 6 &  64
		               | size >> 7 & 128
		               | size >> 8 & 256;
		
		return IntensityMap( width, values );
	}
	
	void Parser::Define( const char* begin, const char* end )
	{
		const char* space = std::find( begin, end, ' ' );
		
		std::string type( begin, space );
		
		begin = space + (space != end);
		
		space = std::find( begin, end, ' ' );
		
		std::string name( begin, space );
		
		begin = space + (space != end);
		
		if ( type == "color" )
		{
			itsColors[ name ] = ReadColor( begin, end );
		}
		else if ( type == "map" )
		{
			itsIntensityMaps[ name ] = ReadMap( begin, end );
		}
	}
	
	void Parser::SetContext( const char* begin, const char* end )
	{
		std::string contextName( begin, end );
		
		itsContextID = itsScene->AddSubcontext( itsContextID,
		                                        contextName,
		                                        MakeTranslation(  itsOrigin ).Make(),
		                                        MakeTranslation( -itsOrigin ).Make() );
		
		itsOrigin = V::Point3D::Make( 0, 0, 0 );
	}
	
	void Parser::MakeCamera( const char* begin, const char* end )
	{
		itsScene->Cameras().push_back( Camera( itsContextID ) );
	}
	
	void Parser::SetColor( const char* begin, const char* end )
	{
		itsColor = ReadColor( begin, end );
		
		itsIntensityMap = NULL;
	}
	
	void Parser::SetMap( const char* begin, const char* end )
	{
		const char* space = std::find( begin, end, ' ' );
		
		std::string name( begin, space );
		
		IntensityMapMap::const_iterator it = itsIntensityMaps.find( name );
		
		if ( it == itsIntensityMaps.end() )
		{
			throw ParseError();
		}
		
		itsIntensityMap = &it->second;
	}
	
	void Parser::SetOrigin( const char* begin, const char* end )
	{
		double x, y, z;
		
		int scanned = std::sscanf( begin, "%lf %lf %lf", &x, &y, &z );
		
		if ( scanned == 3 )
		{
			itsOrigin = V::Point3D::Make( x, y, z );
		}
	}
	
	void Parser::Translate( const char* begin, const char* end )
	{
		double x, y, z;
		
		int scanned = std::sscanf( begin, "%lf %lf %lf", &x, &y, &z );
		
		if ( scanned == 3 )
		{
			itsOrigin += V::Vector3D::Make( x, y, z );
		}
	}
	
	void Parser::SetTheta( const char* begin, const char* end )
	{
		double theta;
		
		int scanned = std::sscanf( begin, "%lf", &theta );
		
		if ( scanned == 1 )
		{
			itsTheta = V::Degrees( theta );
		}
	}
	
	void Parser::SetPhi( const char* begin, const char* end )
	{
		double phi;
		
		int scanned = std::sscanf( begin, "%lf", &phi );
		
		if ( scanned == 1 )
		{
			itsPhi = V::Degrees( phi );
		}
	}
	
	void Parser::AddMeshPoint( const char* begin, const char* end )
	{
		const char* space = std::find( begin, end, ' ' );
		
		std::string name( begin, space );
		
		begin = space;
		
		while ( *begin == ' ' )
		{
			++begin;
		}
		
		double x, y, z;
		
		int scanned = std::sscanf( begin, "%lf %lf %lf", &x, &y, &z );
		
		if ( scanned == 3 )
		{
			itsPoints[ name ] = itsOrigin + V::Vector3D::Make( x, y, z );
		}
	}
	
	void Parser::AddMeshPolygon( const char* begin, const char* end )
	{
		std::vector< unsigned > offsets;
		
		Context& context = itsScene->GetContext( itsContextID );
		
		while ( begin < end )
		{
			const char* space = std::find( begin, end, ' ' );
			
			std::string ptName( begin, space );
			
			V::Point3D::Type pt = itsPoints[ ptName ];
			
			offsets.push_back( context.AddPointToMesh( pt ) );
			
			begin = space;
			
			while ( *begin == ' ' )
			{
				++begin;
			}
		}
		
		if ( !offsets.empty() )
		{
			if ( itsIntensityMap != NULL )
			{
				context.AddMeshPolygon( offsets, *itsIntensityMap );
			}
			else
			{
				context.AddMeshPolygon( offsets, itsColor );
			}
		}
	}
	
	typedef void ( Parser::*Handler )( const char*, const char* );
	
	static std::map< std::string, Handler > MakeHandlers()
	{
		std::map< std::string, Handler > handlers;
		
		handlers[ "camera"    ] = &Parser::MakeCamera;
		handlers[ "context"   ] = &Parser::SetContext;
		handlers[ "color"     ] = &Parser::SetColor;
		handlers[ "define"    ] = &Parser::Define;
		handlers[ "map"       ] = &Parser::SetMap;
		handlers[ "origin"    ] = &Parser::SetOrigin;
		handlers[ "translate" ] = &Parser::Translate;
		handlers[ "theta"     ] = &Parser::SetTheta;
		handlers[ "phi"       ] = &Parser::SetPhi;
		handlers[ "pt"        ] = &Parser::AddMeshPoint;
		handlers[ "poly"      ] = &Parser::AddMeshPolygon;
		handlers[ "polygon"   ] = &Parser::AddMeshPolygon;
		
		return handlers;
	}
	
	static Handler GetHandler( const std::string& command )
	{
		typedef std::map< std::string, Handler > Handlers;
		
		static Handlers handlers = MakeHandlers();
		
		Handlers::const_iterator it = handlers.find( command );
		
		if ( it == handlers.end() )  return NULL;
		
		return it->second;
	}
	
	Parser::Parser( Scene& scene ) : itsScene( &scene ),
	                                 itsOrigin( V::Point3D::Make( 0, 0, 0 ) ),
	                                 itsTheta    ( 0 ),
	                                 itsPhi      ( 0 ),
	                                 itsContextID( 0 ),
	                                 itsIntensityMap()
	{
	}
	
	void Parser::ParseLine( const std::string& line )
	{
		std::size_t iCmdStart = line.find_first_not_of( " \t" );
		
		if ( iCmdStart == std::string::npos )
		{
			return;
		}
		
		const char* end = &*line.end();
		
		const char* start = line.c_str() + iCmdStart;
		
		const char* stop = std::find( start, end, ' ' );
		
		std::string cmdname( start, stop );
		
		if ( Handler handler = GetHandler( cmdname ) )
		{
			for ( start = stop;  *start == ' ';  ++start )
			{
				
			}
			
			start = stop;
			
			while ( *start == ' ' )
			{
				++start;
			}
			
			(this->*handler)( start, end );
		}
	}
	
	void Window::Load( const FSSpec& file )
	{
		N::SetWTitle( Get(), file.name );
		
		Io::TextInputAdapter< NN::Owned< N::FSFileRefNum > > input( io::open_for_reading( file ) );
		
		Parser parser( ItsScene() );
		
		std::list< Parser > savedParsers;
		
		while ( input.Ready() )
		{
			std::string line = input.Read();
			
			if ( std::strchr( line.c_str(), '{' ) )
			{
				savedParsers.push_back( parser );
			}
			else if ( std::strchr( line.c_str(), '}' ) )
			{
				parser = savedParsers.back();
				savedParsers.pop_back();
			}
			else
			{
				parser.ParseLine( line );
			}
		}
	}
	
	void Window::Store()
	{

	}
	
}

