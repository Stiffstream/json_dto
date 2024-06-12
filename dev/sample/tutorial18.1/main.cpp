/*
	A sample using json_dto with custom Reader_Writer.
*/

#include <iostream>
#include <string>
#include <ctime>

#include <json_dto/pub.hpp>

namespace tutorial_18_1
{

struct extension_t
{
	std::string m_id;
	std::string m_payload;

	extension_t() = default;
	extension_t( std::string id, std::string payload )
		: m_id{ std::move(id) }
		, m_payload{ std::move(payload) }
	{}

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory("Id", m_id)
			& json_dto::mandatory("Payload", m_payload)
			;
	}
};

std::string to_string( const std::vector< extension_t > & what )
{
	std::string result;
	for( const auto & e : what )
	{
		if( !result.empty() )
			result += ", ";
		result += "('" + e.m_id + "':'" + e.m_payload + "')";
	}

	return result;
}

// Reader_Writer for vector of extension_t objects.
struct extension_reader_writer_t
{
	void
	read( std::vector< extension_t > & to, const rapidjson::Value & from ) const
	{
		using json_dto::read_json_value;

		to.clear();

		if( from.IsObject() )
		{
			extension_t single_value;
			read_json_value( single_value, from );
			to.push_back( std::move(single_value) );
		}
		else if( from.IsArray() )
		{
			read_json_value( to, from );
		}
		else
		{
			throw std::runtime_error{ "Unexpected format of extension_t value" };
		}
	}

	void
	write(
		const std::vector< extension_t > & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		using json_dto::write_json_value;
		if( 1u == v.size() )
			write_json_value( v.front(), to, allocator );
		else
			write_json_value( v, to, allocator );
	}
};

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
	// Message text.
	std::string m_text;

	// Extension(s) for a message.
	std::vector< extension_t > m_extension;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory( "text", m_text )
			& json_dto::mandatory( extension_reader_writer_t{},
					"extension", m_extension )
			;
	}
};

} /* namespace tutorial_18_1 */

using namespace tutorial_18_1;

const std::string json_data_1{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "text" : "Hello world!",
  "extension" : [
     {"Id": "000", "Payload": "---000---"},
     {"Id": "001", "Payload": "---001---"}
  ]
})JSON" };

const std::string json_data_2{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "text" : "Hello world!",
  "extension" : {"Id": "000", "Payload": "---000---"}
})JSON" };


int
main( int , char *[] )
{
	try
	{
		{
			auto msg = json_dto::from_json< message_t >( json_data_1 );

			const auto t = static_cast< std::time_t >( msg.m_when );
			std::cout
				<< "Deserialized from JSON:\n"
				<< "\t     from: " << msg.m_from << "\n"
				<< "\t     when: " << std::ctime( &t )
				<< "\t     text: " << msg.m_text << "\n"
				<< "\textension: " << to_string(msg.m_extension) << std::endl;
		}

		{
			auto msg = json_dto::from_json< message_t >( json_data_2 );

			const auto t = static_cast< std::time_t >( msg.m_when );
			std::cout
				<< "Deserialized from JSON:\n"
				<< "\t     from: " << msg.m_from << "\n"
				<< "\t     when: " << std::ctime( &t )
				<< "\t     text: " << msg.m_text << "\n"
				<< "\textension: " << to_string(msg.m_extension) << std::endl;
		}

		{
			message_t msg{ "json_dto", std::time( nullptr ), "Hello once again!" };
			msg.m_extension.emplace_back( "0001", "---0001---" );
			msg.m_extension.emplace_back( "0002", "---0002---" );

			std::cout
				<< "\nSerialized to JSON:\n"
				<< json_dto::to_json( msg ) << std::endl;
		}

		{
			message_t msg{ "json_dto", std::time( nullptr ), "Hello once again!" };
			msg.m_extension.emplace_back( "0001", "---0001---" );

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
