/*
	A sample using json_dto with custom Reader_Writer.
*/

#include <iostream>
#include <string>
#include <ctime>

#include <json_dto/pub.hpp>

enum class level_t : std::uint16_t
{
	low, normal, high
};

enum class category_t : int
{
	info, notice, alarm
};

// Reader_Writer for textual representation of level_t/category_t.
struct textual_reader_writer_t
{
	void
	read( level_t & v, const rapidjson::Value & from ) const
	{
		using json_dto::read_json_value;

		std::string str_v;
		read_json_value( str_v, from );
		if( "low" == str_v ) v = level_t::low;
		else if( "normal" == str_v ) v = level_t::normal;
		else if( "high" == str_v ) v = level_t::high;
		else throw json_dto::ex_t{ "invalid value for level_t" };
	}

	void
	write(
		const level_t & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		using json_dto::write_json_value;
		using json_dto::string_ref_t;

		switch( v )
		{
			case level_t::low:
				write_json_value( string_ref_t{ "low" }, to, allocator );
			break;

			case level_t::normal:
				write_json_value( string_ref_t{ "normal" }, to, allocator );
			break;

			case level_t::high:
				write_json_value( string_ref_t{ "high" }, to, allocator );
			break;
		}
	}

	void
	read( category_t & v, const rapidjson::Value & from ) const
	{
		using json_dto::read_json_value;

		std::string str_v;
		read_json_value( str_v, from );

		if( "info" == str_v ) v = category_t::info;
		else if( "notice" == str_v ) v = category_t::notice;
		else if( "alarm" == str_v ) v = category_t::alarm;
		else throw json_dto::ex_t{ "invalid value for category_t" };
	}

	void
	write(
		const category_t & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		using json_dto::write_json_value;
		using json_dto::string_ref_t;

		switch( v )
		{
			case category_t::info:
				write_json_value( string_ref_t{ "info" }, to, allocator );
			break;

			case category_t::notice:
				write_json_value( string_ref_t{ "notice" }, to, allocator );
			break;

			case category_t::alarm:
				write_json_value( string_ref_t{ "alarm" }, to, allocator );
			break;
		}
	}
};

// Reader_Writer for numeric representation of level_t/category_t.
struct numeric_reader_writer_t
{
	template< typename Field_Type >
	void
	read( Field_Type & v, const rapidjson::Value & from ) const
	{
		static_assert( std::is_enum<Field_Type>::value,
				"Field_Type is expected to be an enum type" );

		using json_dto::read_json_value;

		using ut = std::underlying_type_t< Field_Type >;

		ut representation;
		read_json_value( representation, from );

		v = static_cast<Field_Type>(representation);
	}

	template< typename Field_Type >
	void
	write(
		const Field_Type & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		static_assert( std::is_enum<Field_Type>::value,
				"Field_Type is expected to be an enum type" );

		using json_dto::write_json_value;

		using ut = std::underlying_type_t< Field_Type >;

		const ut representation{ static_cast<ut>(v) };

		write_json_value( representation, to, allocator );
	}
};

// Description of message category and level.
struct message_properties_t
{
	level_t m_level{ level_t::low };
	category_t m_category{ category_t::info };

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::optional( textual_reader_writer_t{},
				"level", m_level, level_t::low )
			& json_dto::optional( textual_reader_writer_t{},
				"cat", m_category, category_t::info );
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

	// Message category as numeric code.
	category_t m_category;
	// Message importance level as numeric code.
	level_t m_level;

	// Optional description of message properties.
	json_dto::nullable_t< message_properties_t > m_properties;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "from", m_from )
			& json_dto::mandatory( "when", m_when )
			& json_dto::mandatory( "text", m_text )
			& json_dto::mandatory( numeric_reader_writer_t{},
					"category-code", m_category )
			& json_dto::mandatory( numeric_reader_writer_t{},
					"level-code", m_level )
			& json_dto::optional( "properties", m_properties, nullptr );
	}
};

const std::string json_data{
R"JSON({
  "from" : "json_dto",
  "when" : 1474884330,
  "text" : "Hello world!",
  "category-code" : 0,
  "level-code" : 1
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
				<< "\tcat: " << static_cast<int>(msg.m_category)
				<< "\tlvl: " << static_cast<unsigned>(msg.m_level)
				<< "\ttext: " << msg.m_text << std::endl;
		}

		{
			message_t msg{ "json_dto", std::time( nullptr ), "Hello once again!" };
			msg.m_category = category_t::alarm;
			msg.m_level = level_t::normal;
			msg.m_properties = message_properties_t{
					msg.m_level, msg.m_category };

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
