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

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory( "text", m_text )
			& json_dto::optional( "log_level", m_log_level, nullptr );
	}
};

std::ostream &
operator<<( std::ostream & to, const message_t & v )
{
	return (to << "[from=" << v.m_from << ", when=" << v.m_when
		<< ", text='" << v.m_text << "', log_level="
		<< (v.m_log_level ? std::to_string(*v.m_log_level) : "null")
		<< "]");
}

struct messages_container_t
{
	std::vector< message_t > m_messages;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "messages", m_messages );
	}
};

std::ostream &
operator<<( std::ostream & to, const messages_container_t & v )
{
	for( const auto & msg : v.m_messages )
		to << msg << std::endl;
	return to;
}

int
main( int , char *[] )
{
	try
	{
		std::string json;
		{
			const auto now = std::time(nullptr);

			messages_container_t container;
			container.m_messages.emplace_back( "Alice", now, "Hello!" );
			container.m_messages.emplace_back( "Bob", now + 1, "Hi! How are you?" );
			container.m_messages.emplace_back( "Alice", now + 2, "I'm fine!" );
			container.m_messages.back().m_log_level = 3;
			
			json = json_dto::to_json( container );
		}

		std::cout << "JSON with messages:\n" << json << std::endl;

		{
			const auto msgs = json_dto::from_json< messages_container_t >( json );

			std::cout << "\nExtracted messages:\n" << msgs << std::endl;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

