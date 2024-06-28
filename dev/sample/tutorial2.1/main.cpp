/*
	A sample using json_dto
*/

#include <iostream>
#include <string>
#include <ctime>

#include <json_dto/pub.hpp>

namespace A
{

struct two_values_t
{
	int m_a{};
	int m_b{};

	two_values_t() = default;

	two_values_t( int a, int b ) : m_a{ a }, m_b{ b } {}
};

} /* namespace A */

namespace json_dto
{

template< typename Json_Io >
void json_io( Json_Io & io, A::two_values_t & v )
{
	io & json_dto::mandatory( "A", v.m_a )
		& json_dto::mandatory( "B", v.m_b )
		;
}

} /* namespace json_dto */

namespace B
{

template< typename V >
struct payload_t
{
	std::string m_description;
	V m_payload;

	payload_t() = default;

	payload_t( std::string description, V payload )
		: m_description{ std::move(description) }
		, m_payload{ std::move(payload) }
	{}
};

} /* namespace B */

namespace json_dto
{

template< typename Json_Io, typename V >
void json_io( Json_Io & io, B::payload_t<V> & v )
{
	io & json_dto::mandatory( "Desc", v.m_description )
		& json_dto::mandatory( "Value", v.m_payload )
		;
}

} /* namespace json_dto */

namespace C
{

struct message_t
{
	message_t() {}

	message_t(
		std::string from,
		std::int64_t when,
		std::string text,
		A::two_values_t ids )
		:	m_from{ std::move( from ) }
		,	m_when{ "unixtime", when }
		,	m_text{ "message-body", std::move( text ) }
		,	m_ids{ "markers", ids }
	{}

	// Who sent a message.
	std::string m_from;
	// When the message was sent (unixtime).
	B::payload_t< std::int64_t > m_when;
	// Message text.
	B::payload_t< std::string > m_text;
	// Message markers.
	B::payload_t< A::two_values_t > m_ids;
};

} /* namespace C */

namespace json_dto
{

template< typename Json_Io >
void json_io( Json_Io & io, C::message_t & msg )
{
	io & json_dto::mandatory( "from", msg.m_from )
		& json_dto::mandatory( "when", msg.m_when )
		& json_dto::mandatory( "text", msg.m_text )
		& json_dto::mandatory( "ids", msg.m_ids )
		;
}

} /* namespace json_dto */

int
main( int , char *[] )
{
	try
	{
		std::string serialized_value;

		{
			const C::message_t msg{
					"json_dto",
					std::time( nullptr ),
					"Hello once again!",
					{ 42, 100500 }
				};

			serialized_value = json_dto::to_json( msg );
			std::cout
				<< "\nSerialized to JSON:\n"
				<< serialized_value << std::endl;
		}
		{
			auto msg = json_dto::from_json< C::message_t >( serialized_value );

			const auto t = static_cast< std::time_t >( msg.m_when.m_payload );
			std::cout
				<< "Deserialized from JSON:\n"
				<< "\tfrom: " << msg.m_from << "\n"
				<< "\twhen: " << std::ctime( &t )
				<< "\ttext: " << msg.m_text.m_payload << "\n"
				<< "\t ids: " << msg.m_ids.m_payload.m_a << ", "
						<< msg.m_ids.m_payload.m_b
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

