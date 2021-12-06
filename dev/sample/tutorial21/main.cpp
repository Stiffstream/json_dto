/*
	A sample using json_dto
*/

#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include <json_dto/pub.hpp>

// Message.
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
	// Message headers.
	std::vector< std::string > m_headers;
	// Message text.
	std::string m_text;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory_with_null_as_default( "headers", m_headers )
			& json_dto::mandatory( "text", m_text );
	}
};

std::ostream & operator<<(
	std::ostream & to,
	const message_t & msg )
{
	const auto t = static_cast< std::time_t >( msg.m_when );
	to << "from: " << msg.m_from << "\n"
		<< "when: " << std::ctime( &t )
		<< "headers: [";
	for( auto it = msg.m_headers.begin(), it_end = msg.m_headers.end();
			it != it_end; ++it )
	{
		if( it != msg.m_headers.begin() )
			to << ", ";
		to << *it;
	}
	to << "]\n"
		<< "text: " << msg.m_text;

	return to;
}

const std::string json_data_1{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "headers" : [ "A", "B", "C" ],
  "text" : "Hello world!"
})JSON" };

const std::string json_data_2{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "headers" : null,
  "text" : "Hello world!"
})JSON" };

int
main( int , char *[] )
{
	try
	{
		{
			auto msg = json_dto::from_json< message_t >( json_data_1 );

			std::cout
				<< "1) Deserialized from JSON:\n"
				<< msg
				<< std::endl;

			std::cout << "---\n"
				<< "Serialized to JSON: "
				<< json_dto::to_json( msg ) << "\n"
				<< "==="
				<< std::endl;
		}

		{
			auto msg = json_dto::from_json< message_t >( json_data_2 );

			std::cout
				<< "2) Deserialized from JSON:\n"
				<< msg
				<< std::endl;

			std::cout << "---\n"
				<< "Serialized to JSON: "
				<< json_dto::to_json( msg ) << "\n"
				<< "==="
				<< std::endl;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

