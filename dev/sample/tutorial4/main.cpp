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
		std::string text,
		std::string text_format,
		bool is_private )
		:	m_from{ std::move( from ) }
		,	m_when{ when }
		,	m_text{ std::move( text ) }
		,	m_text_format{ std::move( text_format ) }
		,	m_is_private{ std::move( is_private ) }
	{}

	// Who sent a message.
	std::string m_from;

	// When the message was sent (unixtime).
	std::int64_t m_when;

	// Message text.
	std::string m_text;

	// Text format.
	std::string m_text_format;

	// Privacy flag.
	bool m_is_private{ false };
};

namespace json_dto
{

template< typename Json_Io >
void json_io( Json_Io & io, message_t & msg )
{
	io & json_dto::mandatory( "from", msg.m_from )
		& json_dto::mandatory( "when", msg.m_when )
		& json_dto::mandatory( "text", msg.m_text )
		& json_dto::optional( "text_format", msg.m_text_format, "text/plain" )
		& json_dto::optional_no_default( "is_private", msg.m_is_private );
}

} /* namespace json_dto */

const std::string json_data{
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
		{
			auto msg = json_dto::from_json< message_t >( json_data );

			const auto t = static_cast< std::time_t >( msg.m_when );
			std::cout
				<< "Deserialized from JSON:\n"
				<< "\tfrom: " << msg.m_from << "\n"
				<< "\twhen: " << std::ctime( &t )
				<< "\ttext: " << msg.m_text << "\n"
				<< "\ttext_format: " << msg.m_text_format << "\n"
				<< "\tis_private: " << msg.m_is_private << std::endl;
		}

		{
			const message_t msg{
				"json_dto",
				std::time( nullptr ),
				"Hello once again!",
				"text/plain",
				true };

			std::cout
				<< "\nSerialized to JSON:\n"
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

