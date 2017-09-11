/*
	A sample using json_dto
*/

#include <thread>
#include <iostream>
#include <string>
#include <ctime>

#include <json_dto/pub.hpp>

// Message source.
struct message_source_t
{
	message_source_t() {}

	message_source_t(
		std::int32_t thread_id,
		std::string subsystem )
		:	m_thread_id{ thread_id }
		,	m_subsystem{ std::move( subsystem ) }
	{}

	std::int32_t m_thread_id{ 0 };
	std::string m_subsystem{};

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::optional( "thread_id", m_thread_id, 0 )
			& json_dto::mandatory( "subsystem", m_subsystem );
	}
};

// Single mesage.
struct message_t
{
	message_t() {}

	message_t(
		message_source_t from,
		std::int64_t when,
		std::string text )
		:	m_from{ std::move( from ) }
		,	m_when{ when }
		,	m_text{ std::move( text ) }
	{}

	message_t(
		std::int64_t when,
		std::string text )
		:	m_when{ when }
		,	m_text{ std::move( text ) }
	{}

	// Who sent a message.
	json_dto::nullable_t< message_source_t > m_from{};

	// When the message was sent (unixtime).
	std::int64_t m_when{};

	// Message text.
	std::string m_text{};

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::optional( "from", m_from, nullptr )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory( "text", m_text );
	}
};

// A bunch of messages, all from one source.
struct message_pack_t
{
	message_pack_t() {}

	message_pack_t(
		message_source_t from )
		:	m_from{ std::move( from ) }
	{}

	// Who sent a messages.
	message_source_t m_from;

	std::vector< message_t > m_messages;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "messages", m_messages );
	}
};

const std::string json_data{
R"JSON({
  "from" :
    {
      "subsystem": "json_dto"
    },
  "messages" :
    [
      {
        "when" : 1474884330,
        "text" : "Hello"
      },
      {
        "when" : 1474884331,
        "text" : " world!"
      }
    ]
})JSON" };

int
main( int , char *[] )
{
	try
	{
		{
			auto msg_pack = json_dto::from_json< message_pack_t >( json_data );

			std::cout
				<< "Deserialized from JSON:\n"
				<< "\tfrom: " << msg_pack.m_from.m_subsystem << "\n"
				<< "\tmessages:\n";

			for( const auto & msg : msg_pack.m_messages )
			{
				const auto t = static_cast< std::time_t >( msg.m_when );
				std::cout
					<< "\t\tmessage:\n"
					<< "\t\t\twhen: " << std::ctime( &t )
					<< "\t\t\ttext: " << msg.m_text << "\n";
			}
			std::cout << std::endl;
		}

		{
			message_pack_t msg_pack{ message_source_t{ 42, "json_dto sample" } };

			msg_pack.m_messages.emplace_back( std::time( nullptr ), "Hello" );
			msg_pack.m_messages.emplace_back( std::time( nullptr ), "once" );
			msg_pack.m_messages.emplace_back( std::time( nullptr ), "again!" );

			std::cout
				<< "\nSerialized to JSON:\n"
				<< json_dto::to_json( msg_pack ) << std::endl;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

