/*
	A sample using json_dto
*/

#include <iostream>
#include <string>
#include <ctime>
#include <regex>
#include <array>

#include <json_dto/pub.hpp>

namespace json_dto
{

template <>
void
read_json_value(
	std::tm & v,
	const rapidjson::Value & object )
{
	try
	{
		std::string representation;
		read_json_value( representation, object );

		const std::regex dt_regex{
			R"regex(^(\d{4})\.(\d{2})\.(\d{2}) (\d{2}):(\d{2}):(\d{2})$)regex" };

		std::smatch match_results;

		if( !std::regex_match( representation, match_results, dt_regex ) )
		{
			throw std::runtime_error{
				"invalid timesptamp string: \"" +
				representation + "\"" };
		}

		v.tm_year = std::stoi( match_results[ 1 ] ) - 1900;
		v.tm_mon = std::stoi( match_results[ 2 ] ) - 1;
		v.tm_mday = std::stoi( match_results[ 3 ] );
		v.tm_hour = std::stoi( match_results[ 4 ] );
		v.tm_min = std::stoi( match_results[ 5 ] );
		v.tm_sec = std::stoi( match_results[ 6 ] );
	}
	catch( const std::exception & ex )
	{
		throw std::runtime_error{
			std::string{ "unable to read std::tm: " } +
			ex.what() };
	}
}

template <>
void
write_json_value(
	const std::tm & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	std::array< char, 64 > buf;

	std::sprintf(
		buf.data(),
		"%04d.%02d.%02d %02d:%02d:%02d",
		v.tm_year + 1900,
		v.tm_mon + 1,
		v.tm_mday,
		v.tm_hour,
		v.tm_min,
		v.tm_sec );

	std::string representation{ buf.data() };

	write_json_value( representation, object, allocator );
}

} /* namespace json_dto */

// Message.
struct message_t
{
	message_t() {}

	message_t(
		std::string from,
		std::tm when,
		std::string text )
		:	m_from{ std::move( from ) }
		,	m_when{ when }
		,	m_text{ std::move( text ) }
	{}

	// Who sent a message.
	std::string m_from;

	// When the message was sent (unixtime).
	std::tm m_when;

	// Message text.
	std::string m_text;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory( "text", m_text );
	}
};

const std::string json_data{
R"JSON({
  "from" : "json_dto",
  "when" : "2016.09.28 19:55:00",
  "text" : "Hello world!"
})JSON" };

int
main( int , char *[] )
{
	try
	{
		{
			auto msg = json_dto::from_json< message_t >( json_data );

			auto tm = msg.m_when;
			const auto t = std::mktime( &tm );

			std::cout
				<< "Deserialized from JSON:\n"
				<< "\tfrom: " << msg.m_from << "\n"
				<< "\twhen: " << std::ctime( &t )
				<< "\ttext: " << msg.m_text << std::endl;
		}

		{
			auto t = std::time( nullptr );
			const message_t msg{ "json_dto", *std::localtime( &t ), "Hello once again!" };

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
