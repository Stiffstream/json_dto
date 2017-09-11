/*
	A sample using json_dto with user-defined types in different namespaces.
*/

#include <iostream>
#include <string>
#include <ctime>
#include <regex>
#include <array>

#include <json_dto/pub.hpp>

// read_json_value and write_json_value for std::tm are defined as
// template specialization inside json_dto namespace.
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

namespace bounded_types
{

template< typename T, T min, T max, T default_value = T{} >
class bounded_value_t
{
public :
	using value_type = T;

	bounded_value_t()
		{
			ensure_valid( m_value );
		}
	bounded_value_t( value_type value ) : m_value{ ensure_valid( value ) }
		{}

	value_type get() const { return m_value; }
	void set( value_type v ) { m_value = ensure_valid( v ); }

private :
	value_type m_value{ default_value };

	static value_type
	ensure_valid( value_type v )
		{
			if( v < min || v > max )
				throw std::invalid_argument{ "value is out of range!" };
			return v;
		}

};

// read_json_value and write_json_value for bounded_value_t are
// defined in bounded_types namespace.
// They will be found by argument dependent lookup.
template< typename T, T min, T max, T default_value >
void
read_json_value(
	bounded_value_t< T, min, max, default_value > & value,
	const rapidjson::Value & from )
{
	using json_dto::read_json_value;

	T v{ default_value };
	read_json_value( v, from );

	value.set( v );
}

template< typename T, T min, T max, T default_value >
void
write_json_value(
	const bounded_value_t< T, min, max, default_value > & value,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	using json_dto::write_json_value;
	write_json_value( value.get(), object, allocator );
}

} /* namespace bounded_types */

namespace importance_levels
{

enum class level_t
	{
		low,
		normal,
		high
	};

constexpr level_t low = level_t::low;
constexpr level_t normal = level_t::normal;
constexpr level_t high = level_t::high;

std::ostream &
operator<<( std::ostream & to, level_t l )
{
	return (to << (low == l ? "low" : normal == l ? "normal" : "high"));
}

// read_json_value and write_json_value for level_t are
// defined in importance_levels namespace.
// They will be found by argument dependent lookup.
void
read_json_value(
	level_t & value,
	const rapidjson::Value & from )
{
	using json_dto::read_json_value;

	std::string representation;
	read_json_value( representation, from );

	if( "low" == representation )
		value = low;
	else if( "normal" == representation )
		value = normal;
	else if( "high" == representation )
		value = high;
	else
		throw std::runtime_error{ "invalid representation of importance level: "
			+ representation };
}

void
write_json_value(
	const level_t & value,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	using json_dto::write_json_value;

	std::string representation{ low == value ? "low" :
			normal == value ? "normal" : "high" };

	write_json_value( representation, object, allocator );
}

} /* namespace importance_levels */

// Message.
struct message_t
{
	using priority_t = bounded_types::bounded_value_t< int, 0, 9, 5 >;

	message_t() {}

	message_t(
		std::string from,
		std::tm when,
		std::string text,
		priority_t::value_type priority,
		importance_levels::level_t importance )
		:	m_from{ std::move( from ) }
		,	m_when{ when }
		,	m_text{ std::move( text ) }
		,	m_priority{ priority }
		,	m_importance{ importance }
	{}

	// Who sent a message.
	std::string m_from;

	// When the message was sent (unixtime).
	std::tm m_when;

	// Message text.
	std::string m_text;

	// Priority of the message.
	priority_t m_priority;

	// Importance of message.
	importance_levels::level_t m_importance;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory( "text", m_text )
			& json_dto::mandatory( "priority", m_priority )
			& json_dto::optional( "importance", m_importance,
					importance_levels::normal );
	}
};

const std::string json_data{
R"JSON({
  "from" : "json_dto",
  "when" : "2016.09.28 19:55:00",
  "text" : "Hello world!",
  "priority" : 8
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
				<< "\tpriority: " << msg.m_priority.get() << "\n"
				<< "\timportance: " << msg.m_importance << "\n"
				<< "\ttext: " << msg.m_text
				<< std::endl;
		}

		{
			auto t = std::time( nullptr );
			const message_t msg{
				"json_dto",
				*std::localtime( &t ),
				"Hello once again!",
				3,
				importance_levels::low };

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
