#include <catch2/catch.hpp>

#include <iostream>
#include <limits>
#include <tuple>
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
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer(
						json_dto::inside_array::member( m_x.m_a ),
						json_dto::inside_array::member( m_x.m_b ),
						json_dto::inside_array::member( m_x.m_c ) ),
					"x", m_x );
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
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer(
						json_dto::inside_array::member(
							simple_int_reader_writter_t{}, m_x.m_a ),
						json_dto::inside_array::member( m_x.m_b ),
						json_dto::inside_array::member( m_x.m_c ) ),
					"x", m_x );
	}
};

struct tuple_holder_t
{
	std::tuple<int, int, std::string, int> m_x;

	tuple_holder_t()
		: m_x{ 0, 1, "zero", 2 }
	{}

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer(
						json_dto::inside_array::member( std::get<0>(m_x) ),
						json_dto::inside_array::member(
							simple_int_reader_writter_t{}, std::get<1>(m_x) ),
						json_dto::inside_array::member( std::get<2>(m_x) ),
						json_dto::inside_array::member( std::get<3>(m_x) ) ),
					"x", m_x );
	}
};

struct at_least_checker_one_t
{
	int m_x1{};
	int m_x2{};
	int m_x3{};
	int m_x4{};

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer<
							json_dto::inside_array::details::all_members_required_t >(
						json_dto::inside_array::member( m_x1 ),
						json_dto::inside_array::member( m_x2 ),
						json_dto::inside_array::member( m_x3 ),
						json_dto::inside_array::member( m_x4 ) ),
					"x", *this );
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

TEST_CASE( "inside-array-tuple-custom-reader-writer" , "[inside-array][tuple][reader-writer]" )
{
	const char * json_str =
		R"({
			"x":[ 54, -1, "five", 24678 ]
		})";

	auto r = json_dto::from_json<tuple_holder_t>( json_str );

	REQUIRE( std::make_tuple(54, -1, std::string{"five"}, 24678) == r.m_x );

	r.m_x = std::make_tuple(-2, 27, std::string{"nullptr"}, 0);

	const auto str = json_dto::to_json( r );
	REQUIRE( R"json({"x":[-2,27,"nullptr",0]})json" == str );
}

TEST_CASE( "inside-array-at-least-limit-one" , "[inside-array][at-least][reader-writer]" )
{
	const char * json_str =
		R"({
			"x":[ 1, 2, 3, 4 ]
		})";

	auto r = json_dto::from_json<at_least_checker_one_t>( json_str );

	REQUIRE( 1 == r.m_x1 );
	REQUIRE( 2 == r.m_x2 );
	REQUIRE( 3 == r.m_x3 );
	REQUIRE( 4 == r.m_x4 );
}

