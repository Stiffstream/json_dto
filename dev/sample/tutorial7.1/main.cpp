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
	std::int32_t m_log_level{};

	std::vector< std::string > m_tags{};

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		using json_dto::maybe_null;
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory( "text", m_text )
			& json_dto::optional( "log_level", maybe_null( m_log_level ), 0 )
			& json_dto::optional_no_default( "tags", maybe_null( m_tags ) );
	}
};

const std::string json_data_with_values{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "text" : "Hello world!",
  "log_level" : 2,
  "tags" : [ "sample", "tutorial", "nullable fields", "arrays" ]
})JSON" };

const std::string json_data_with_nulls{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "text" : "Hello world!",
  "log_level" : null,
  "tags" : null
})JSON" };

int
main( int , char *[] )
{
	try
	{
		auto deserialize = []( const auto & json_data ) {
			auto msg = json_dto::from_json< message_t >( json_data );

			const auto t = static_cast< std::time_t >( msg.m_when );
			std::cout << "\tfrom: " << msg.m_from << "\n"
				<< "\twhen: " << std::ctime( &t )
				<< "\ttext: " << msg.m_text;

			// If field is defined then its value can be printed.
			std::cout << "\n\tlog_level: " << msg.m_log_level;

			std::cout << "\n\ttags: ";
			for( const auto & tag : msg.m_tags )
				std::cout << " [" << tag << "]";

			std::cout << std::endl;
		};

		std::cout << "Deserialized from JSON with values:\n";
		deserialize( json_data_with_values );
		std::cout << "\nDeserialized from JSON with nulls:\n";
		deserialize( json_data_with_nulls );

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

			msg.m_log_level = 0;

			// Add tags:
			msg.m_tags.emplace_back( "sample" );
			msg.m_tags.emplace_back( "tutorial" );

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

