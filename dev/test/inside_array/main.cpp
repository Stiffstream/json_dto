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
					json_dto::inside_array().with( m_x.m_a ).with( m_x.m_b ) );
	}
};

} /* namespace test */

using namespace test;

TEST_CASE( "inside-array-simple" , "[inside-array][no-reader-writer" )
{
	const char * json_str =
		R"({
			"x":[ 1, "two" ]
		})";

	const auto r = json_dto::from_json<simple_outer_t>( json_str );

	REQUIRE( 1 == r.m_x.m_a );
	REQUIRE( "two" == r.m_x.m_b );

	const auto str = json_dto::to_json( r );
	REQUIRE( R"json({"x":[1,"two"]})json" == str );
}

