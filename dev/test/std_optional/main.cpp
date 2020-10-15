#include <catch2/catch.hpp>

#include <json_dto/pub.hpp>

#if defined( JSON_DTO_SUPPORTS_STD_OPTIONAL )

struct hex_reader_writer
{
	void read(
		std::uint32_t & key,
		const rapidjson::Value & from ) const
	{
		if( !from.IsString() )
			throw std::runtime_error( "string value expected" );

		const char * str_v = from.GetString();
		if( std::strlen(str_v) < 2u )
			throw std::runtime_error( "invalid value length" );

		if( '#' != *str_v )
			throw std::runtime_error( "invalid value format" );

		if( 1 != std::sscanf( str_v+1, "%x", &key ) )
			throw std::runtime_error( "unable to parse key value" );
	}

	void write(
		const std::uint32_t & key,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		char buf[16];
		std::sprintf( buf, "#%x", key );
		to.SetString( buf, allocator );
	}
};

struct data_t
{
	json_dto::cpp17::optional< int > m_i;
	json_dto::cpp17::optional< float > m_f;
	json_dto::cpp17::optional< std::string > m_s;
	json_dto::cpp17::optional< std::vector< std::string > > m_vs;
	json_dto::cpp17::optional< std::uint32_t > m_uint32_hex;

	template< typename IO >
	void json_io( IO & io )
	{
		using namespace json_dto;

		io & optional( "int", m_i, json_dto::cpp17::nullopt() )
			& optional( "float", m_f, json_dto::cpp17::nullopt() )
			& optional( "str", m_s, json_dto::cpp17::nullopt() )
			& optional( "str_vect", m_vs, json_dto::cpp17::nullopt() )
			& optional( apply_to_content_t< hex_reader_writer >{},
					"uint32_hex",
					m_uint32_hex,
					json_dto::cpp17::nullopt() );
	}
};

TEST_CASE( "read from json" , "read" )
{
	{
		const std::string json_data{
			R"JSON(
			{}
			)JSON" };
		auto obj = json_dto::from_json< data_t >( json_data );

		REQUIRE( !obj.m_i );
		REQUIRE( !obj.m_f );
		REQUIRE( !obj.m_s );
		REQUIRE( !obj.m_vs );
		REQUIRE( !obj.m_uint32_hex );
	}
	{
		const std::string json_data{
			R"JSON(
			{
				"int":123456
			}
			)JSON" };
		auto obj = json_dto::from_json< data_t >( json_data );

		REQUIRE( obj.m_i );
		REQUIRE( 123456 == *obj.m_i );
		REQUIRE( !obj.m_f );
		REQUIRE( !obj.m_s );
		REQUIRE( !obj.m_vs );
		REQUIRE( !obj.m_uint32_hex );
	}
	{
		const std::string json_data{
			R"JSON(
			{
				"float":12.3456,
				"str":"Hello!"
			}
			)JSON" };
		auto obj = json_dto::from_json< data_t >( json_data );

		REQUIRE( !obj.m_i );
		REQUIRE( obj.m_f );
		REQUIRE( Approx(12.3456) == *obj.m_f );
		REQUIRE( obj.m_s );
		REQUIRE( "Hello!" == *obj.m_s );
		REQUIRE( !obj.m_vs );
		REQUIRE( !obj.m_uint32_hex );
	}
	{
		const std::string json_data{
			R"JSON(
			{
				"str":"Hello!",
				"str_vect":["s1", "s2", "s3"],
				"uint32_hex": "#ff1d"
			}
			)JSON" };
		auto obj = json_dto::from_json< data_t >( json_data );

		REQUIRE( !obj.m_i );
		REQUIRE( !obj.m_f );
		REQUIRE( obj.m_s );
		REQUIRE( "Hello!" == *obj.m_s );

		REQUIRE( obj.m_vs );
		const std::vector< std::string > vs{ "s1", "s2", "s3" };
		REQUIRE( vs == *obj.m_vs );

		REQUIRE( obj.m_uint32_hex );
		REQUIRE( 0xff1d == *obj.m_uint32_hex );
	}
}

TEST_CASE( "write to json" , "write" )
{
	{
		data_t obj;
		const auto r = json_dto::to_json( obj );

		REQUIRE( R"({})" == r );
	}
	{
		data_t obj;
		obj.m_i = 25;
		const auto r = json_dto::to_json( obj );

		REQUIRE( R"({"int":25})" == r );
	}
	{
		data_t obj;
		obj.m_i = 25;
		obj.m_s = std::string{"Bye"};
		const auto r = json_dto::to_json( obj );

		REQUIRE( R"({"int":25,"str":"Bye"})" == r );
	}
	{
		data_t obj;
		obj.m_i = 25;
		obj.m_s = std::string{"Bye"};
		obj.m_vs = std::vector<std::string>{"s3", "s4", "s5"};
		obj.m_uint32_hex = 0x1df3u;
		const auto r = json_dto::to_json( obj );

		REQUIRE( R"({"int":25,"str":"Bye","str_vect":["s3","s4","s5"],"uint32_hex":"#1df3"})" == r );
	}
}

#endif

