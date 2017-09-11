/*
	A sample using json_dto
*/

#include <iostream>
#include <string>
#include <ctime>
#include <algorithm>

#include <json_dto/pub.hpp>

void
check_all_7bit(
	const std::string & text )
{
	auto it =
		std::find_if(
			std::begin( text ),
			std::end( text ),
			[]( char c ){ return c & 0x80; } );

	if( std::end( text ) != it )
	{
		throw std::runtime_error{
			"non 7bit char at pos " +
			std::to_string( std::distance( std::begin( text ), it ) ) };
	}
}

struct message_t
{
	message_t() {}

	message_t(
		std::string from,
		std::int64_t when,
		std::string text )
		:	m_from{ std::move( from ) }
		,	m_when{ when }
		,	m_text{ std::move( text ) }
	{}

	// Who sent a message.
	std::string m_from;

	// When the message was sent (unixtime).
	std::int64_t m_when;

	// Message text. Must be 7bit ascii.
	std::string m_text;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory( "text", m_text, check_all_7bit );
	}
};

std::string json_data{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "text" : "Hello world!"
})JSON" };

int
main( int , char *[] )
{
	try
	{
		try
		{
			auto msg = json_dto::from_json< message_t >( json_data );

			const auto t = static_cast< std::time_t >( msg.m_when );
			std::cout
				<< "Deserialized from JSON:\n"
				<< "\tfrom: " << msg.m_from << "\n"
				<< "\twhen: " << std::ctime( &t )
				<< "\ttext: " << msg.m_text << std::endl;

			// Make data invalid:
			json_data[ json_data.find( msg.m_text ) + 1 ] = -1;

			// This must throw:
			json_dto::from_json< message_t >( json_data );
		}
		catch( const std::exception & ex )
		{
			std::cerr << "Error: " << ex.what() << std::endl;
		}

		try
		{
			const message_t msg{
				"json_dto",
				std::time( nullptr ),
				"Hello once again!" };

			std::cout
				<< "\nSerialized to JSON:\n"
				<< json_dto::to_json( msg ) << std::endl;

			// This must throw:
			json_dto::to_json(
				message_t{
					"json_dto",
					std::time( nullptr ),
					"Hello once \xFF again!" } );
		}
		catch( const std::exception & ex )
		{
			std::cerr << "Error: " << ex.what() << std::endl;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

