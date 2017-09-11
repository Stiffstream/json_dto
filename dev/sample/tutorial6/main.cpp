/*
	A sample using json_dto
*/

#include <iostream>
#include <string>
#include <ctime>

#include <json_dto/pub.hpp>

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

	// Message text.
	std::string m_text;

	// Log level.
	// By default is constructed with null value.
	json_dto::nullable_t< std::int32_t > m_log_level{};
};

namespace json_dto
{

template< typename Json_Io >
void json_io( Json_Io & io, message_t & msg )
{
	io & json_dto::mandatory( "from", msg.m_from )
		& json_dto::mandatory( "when", msg.m_when )
		& json_dto::mandatory( "text", msg.m_text )
		& json_dto::optional( "log_level", msg.m_log_level, nullptr );
}

} /* namespace json_dto */

const std::string json_data{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "text" : "Hello world!",
  "log_level" : 2
})JSON" };

int
main( int , char *[] )
{
	try
	{
		{
			auto msg = json_dto::from_json< message_t >( json_data );

			const auto t = static_cast< std::time_t >( msg.m_when );
			std::cout
				<< "Deserialized from JSON:\n"
				<< "\tfrom: " << msg.m_from << "\n"
				<< "\twhen: " << std::ctime( &t )
				<< "\ttext: " << msg.m_text;

			// If field is defined then its value can be printed.
			if( msg.m_log_level )
				std::cout << "\n\tlog_level: " << *msg.m_log_level;

			std::cout << std::endl;
		}

		{
			message_t msg{
				"json_dto",
				std::time( nullptr ),
				"Hello once again!" };

			// Set nullable field explicitly.
			msg.m_log_level = 1;

			std::cout
				<< "\nSerialized to JSON 1:\n"
				<< json_dto::to_json( msg ) << std::endl;

			msg.m_log_level = nullptr; // equivalent to msg.m_log_level.reset();

			std::cout
				<< "\nSerialized to JSON 2:\n"
				<< json_dto::to_json( msg ) << std::endl;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

