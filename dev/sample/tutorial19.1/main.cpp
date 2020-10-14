/*
	A sample using map<uint32_t, int> with several custom reader_writers.
*/

#include <cstdio>
#include <iostream>
#include <map>

#include <json_dto/pub.hpp>

// Basic functionality for (de)serializing of values.
struct basic_reader_writer
{
	void read(
		int & value,
		const rapidjson::Value & from ) const
	{
		// Just use json_dto functionality.
		json_dto::read_json_value( value, from );
	}

	void write(
		const int & value,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		// Just use json_dto functionality.
		json_dto::write_json_value( value, to, allocator );
	}
};

// Reader_Writer for the case of simple representation of keys.
struct simple_reader_writer : basic_reader_writer
{
	using basic_reader_writer::read;
	using basic_reader_writer::write;

	void read(
		json_dto::mutable_map_key_t<std::uint32_t> & key,
		const rapidjson::Value & from ) const
	{
		if( !from.IsString() )
			throw std::runtime_error( "string value expected" );

		if( 1 != std::sscanf( from.GetString(), "%u", &key.v ) )
			throw std::runtime_error( "unable to parse key value" );
	}

	void write(
		const json_dto::const_map_key_t<std::uint32_t> & key,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		char buf[32];
		std::sprintf( buf, "%u", key.v );
		to.SetString( buf, allocator );
	}
};

// Reader_Writer for the case of representation of keys in hexadecimal form.
struct color_hex_reader_writer : basic_reader_writer
{
	using basic_reader_writer::read;
	using basic_reader_writer::write;

	void read(
		json_dto::mutable_map_key_t<std::uint32_t> & key,
		const rapidjson::Value & from ) const
	{
		if( !from.IsString() )
			throw std::runtime_error( "string value expected" );

		const char * str_v = from.GetString();
		if( std::strlen(str_v) < 2u )
			throw std::runtime_error( "invalid value length" );

		if( '#' != *str_v )
			throw std::runtime_error( "invalid value format" );

		if( 1 != std::sscanf( str_v+1, "%x", &key.v ) )
			throw std::runtime_error( "unable to parse key value" );
	}

	void write(
		const json_dto::const_map_key_t<std::uint32_t> & key,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		char buf[16];
		std::sprintf( buf, "#%06x", key.v );
		to.SetString( buf, allocator );
	}
};

// Data type to be (de)serialized.
struct data_t
{
	std::map< std::uint32_t, int > m_weights;
	std::map< std::uint32_t, int > m_colors;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory(
					// Use apply_to_content_t from json_dto to apply
					// simple_reader_writer to every member of m_weights.
					json_dto::apply_to_content_t< simple_reader_writer >{},
					"weights", m_weights )
			& json_dto::mandatory(
					json_dto::apply_to_content_t< color_hex_reader_writer >{},
					"colors", m_colors )
			;
	}
};

const std::string json_data{
R"JSON({
  "weights" : {"1": 1, "2": 2, "3": 3},
  "colors" : {"#000000":1, "#FF0000": 2, "#00FF00": 3, "#0000FF": 4}
})JSON" };

int
main( int , char *[] )
{
	try
	{
		{
			auto data = json_dto::from_json< data_t >( json_data );

			std::cout << "Deserialized from JSON:\n"
				<< "weights: ";
			for( const auto & kv : data.m_weights )
				std::cout << kv.first << "->" << kv.second << ", ";
			std::cout << "\n" "colors: ";
			for( const auto & kv : data.m_colors )
				std::cout << std::hex << kv.first << std::dec << "->" << kv.second << ", ";
			std::cout << std::endl;
		}

		{
			data_t data;
			data.m_weights[ 3 ] = 33;
			data.m_weights[ 2 ] = 22;
			data.m_weights[ 1 ] = 11;

			data.m_colors[ 0x00FF00 ] = 2;
			data.m_colors[ 0xFF00FF ] = 3;

			std::cout
				<< "\nSerialized to JSON:\n"
				<< json_dto::to_json( data ) << std::endl;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

