/*
	A sample using map<int, int> with overloads for read/write_json_value.
*/

#include <cstdio>
#include <iostream>
#include <map>

#include <json_dto/pub.hpp>

// New overloads of read/write_json_value for integer keys.
namespace json_dto
{

inline void read_json_value(
	mutable_map_key_t<int> key, const rapidjson::Value & from )
{
	if( !from.IsString() )
		throw std::runtime_error( "string value expected" );

	if( 1 != std::sscanf( from.GetString(), "%d", &key.v ) )
		throw std::runtime_error( "unable to parse key value" );
}

inline void write_json_value(
	const_map_key_t<int> key,
	rapidjson::Value & to,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	char buf[32];
	std::sprintf( buf, "%d", key.v );
	to.SetString( buf, allocator );
}

} /* namespace json_dto */

// Data type to be (de)serialized.
struct data_t
{
	std::map< int, int > m_values;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "values", m_values );
	}
};

const std::string json_data{
R"JSON({
  "values" : {"1": 1, "2": 2, "3": 3}
})JSON" };

int
main( int , char *[] )
{
	try
	{
		{
			auto data = json_dto::from_json< data_t >( json_data );

			std::cout << "Deserialized from JSON:\n";
			for( const auto & kv : data.m_values )
				std::cout << kv.first << "->" << kv.second << ", ";
			std::cout << std::endl;
		}

		{
			data_t data;
			data.m_values[ 3 ] = 33;
			data.m_values[ 2 ] = 22;
			data.m_values[ 1 ] = 11;

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

