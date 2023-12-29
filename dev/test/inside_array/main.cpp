#include <catch2/catch.hpp>

#include <iostream>
#include <limits>
#include <type_traits>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

namespace test
{

struct simple_nested_t
{
	int m_a{};
	std::string m_b{};
	unsigned int m_c{};
};

struct simple_outer_t
{
	simple_nested_t m_x;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "x",
					json_dto::inside_array()
						.with( m_x.m_a )
						.with( m_x.m_b )
						.with( m_x.m_c ) );
	}
};

struct simple_int_reader_writter_t
{
	void read( int & v, const rapidjson::Value & object ) const
	{
		json_dto::read_json_value( v, object );
	}

	void write(
		const int & v,
		rapidjson::Value & object,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		json_dto::write_json_value( v, object, allocator );
	}
};

struct outer_with_custom_reader_writer_t
{
	simple_nested_t m_x;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "x",
					json_dto::inside_array()
						.with( simple_int_reader_writter_t{}, m_x.m_a )
						.with( m_x.m_b )
						.with( m_x.m_c ) );
	}
};

} /* namespace test */

using namespace test;

TEST_CASE( "inside-array-simple" , "[inside-array][no-reader-writer]" )
{
	const char * json_str =
		R"({
			"x":[ 1, "two", 55555]
		})";

	const auto r = json_dto::from_json<simple_outer_t>( json_str );

	REQUIRE( 1 == r.m_x.m_a );
	REQUIRE( "two" == r.m_x.m_b );
	REQUIRE( 55555 == r.m_x.m_c );

	const auto str = json_dto::to_json( r );
	REQUIRE( R"json({"x":[1,"two",55555]})json" == str );
}

TEST_CASE( "inside-array-with-custom-reader-writer" , "[inside-array][reader-writer]" )
{
	const char * json_str =
		R"({
			"x":[ 54, "five", 24678 ]
		})";

	auto r = json_dto::from_json<outer_with_custom_reader_writer_t>( json_str );

	REQUIRE( 54 == r.m_x.m_a );
	REQUIRE( "five" == r.m_x.m_b );
	REQUIRE( 24678 == r.m_x.m_c );

	r.m_x.m_a = -333;
	r.m_x.m_b = "nullptr";
	r.m_x.m_c = 0;

	const auto str = json_dto::to_json( r );
	REQUIRE( R"json({"x":[-333,"nullptr",0]})json" == str );
}

