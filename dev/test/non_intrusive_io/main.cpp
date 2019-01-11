#include <catch2/catch.hpp>

#include <iostream>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

#include <test/helper.hpp>

using namespace json_dto;

struct data_t
{
	data_t()
	{}

	data_t(
		std::int32_t num_int32,
		double num_double,
		std::string str )
		:	m_num_int32{ num_int32 }
		,	m_num_double{ num_double }
		,	m_string{ std::move( str ) }
	{}

	std::int32_t m_num_int32{ 0 };
	double m_num_double{ 0.0 };
	std::string m_string{};
};

struct complex_data_t
{
	data_t m_d1{};
	data_t m_d2{};
	json_dto::nullable_t< data_t > m_d3{};
	json_dto::nullable_t< data_t > m_d4{};
};

namespace json_dto
{

template < typename Json_Io >
void
json_io( Json_Io & io, data_t & value )
{
	io
		& optional( "num_int32", value.m_num_int32, 0 )
		& optional( "num_double", value.m_num_double, 0.0 )
		& mandatory( "string", value.m_string );
}

template < typename Json_Io >
void
json_io( Json_Io & io, complex_data_t & value )
{
	io
		& optional_no_default( "d1", value.m_d1 )
		& mandatory( "d2", value.m_d2 )
		& optional_no_default( "d3", value.m_d3 )
		& mandatory( "d4", value.m_d4 );
}

} /* namespace json_dto */

TEST_CASE( "all-defined" , "[all-defined]" )
{
	SECTION( "read" )
	{
		const std::string json_data{
			R"JSON(
			{
				"d1": {"num_int32": 42, "num_double": 3.14, "string": "xyz"},
				"d2": {"num_int32": 42, "num_double": 3.14, "string": "xyz"},
				"d3": {"num_int32": 42, "num_double": 3.14, "string": "xyz"},
				"d4": {"num_int32": 42, "num_double": 3.14, "string": "xyz"}
			})JSON" };

		auto obj = json_dto::from_json< complex_data_t >( json_data );

		REQUIRE( obj.m_d1.m_num_int32 == 42 );
		REQUIRE( equal( obj.m_d1.m_num_double , 3.14 ) );
		REQUIRE( obj.m_d1.m_string == "xyz" );

		REQUIRE( obj.m_d2.m_num_int32 == 42 );
		REQUIRE( equal( obj.m_d2.m_num_double, 3.14 ) );
		REQUIRE( obj.m_d2.m_string == "xyz" );

		REQUIRE( obj.m_d3 );
		REQUIRE( obj.m_d3->m_num_int32 == 42 );
		REQUIRE( equal( obj.m_d3->m_num_double, 3.14 ) );
		REQUIRE( obj.m_d3->m_string == "xyz" );

		REQUIRE( obj.m_d4 );
		REQUIRE( obj.m_d4->m_num_int32 == 42 );
		REQUIRE( equal( obj.m_d4->m_num_double, 3.14 ) );
		REQUIRE( obj.m_d4->m_string == "xyz" );
	}

	SECTION( "write" )
	{
		data_t data{ 42, 3.14, "XYZ" };
		complex_data_t source_obj;

		source_obj.m_d1 = data;
		source_obj.m_d2 = data;
		source_obj.m_d3 = data;
		source_obj.m_d4 = data;

		const std::string json_data = json_dto::to_json( source_obj );

		auto obj = json_dto::from_json< complex_data_t >( json_data );

		REQUIRE( obj.m_d1.m_num_int32 == source_obj.m_d1.m_num_int32 );
		REQUIRE( equal( obj.m_d1.m_num_double, source_obj.m_d1.m_num_double ) );
		REQUIRE( obj.m_d1.m_string == source_obj.m_d1.m_string );

		REQUIRE( obj.m_d2.m_num_int32 == source_obj.m_d2.m_num_int32 );
		REQUIRE( equal( obj.m_d2.m_num_double, source_obj.m_d2.m_num_double ) );
		REQUIRE( obj.m_d2.m_string == source_obj.m_d2.m_string );

		REQUIRE( obj.m_d3 );
		REQUIRE( obj.m_d3->m_num_int32 == source_obj.m_d3->m_num_int32 );
		REQUIRE( equal( obj.m_d3->m_num_double, source_obj.m_d3->m_num_double ) );
		REQUIRE( obj.m_d3->m_string == source_obj.m_d3->m_string );

		REQUIRE( obj.m_d4 );
		REQUIRE( obj.m_d4->m_num_int32 == source_obj.m_d4->m_num_int32 );
		REQUIRE( equal( obj.m_d4->m_num_double, source_obj.m_d4->m_num_double ) );
		REQUIRE( obj.m_d4->m_string == source_obj.m_d4->m_string );
	}
}

TEST_CASE( "mand-defined" , "[mand-defined]" )
{
	const std::string json_data{
		R"JSON(
			{
				"d2": {"num_int32": 42, "num_double": 3.14, "string": "xyz"},
				"d4": {"num_int32": 42, "num_double": 3.14, "string": "xyz"}
			})JSON" };

	auto obj = json_dto::from_json< complex_data_t >( json_data );

	REQUIRE( obj.m_d1.m_num_int32 == 0 );
	REQUIRE( equal( obj.m_d1.m_num_double, 0.0 ) );
	REQUIRE( obj.m_d1.m_string.empty() );

	REQUIRE( obj.m_d2.m_num_int32 == 42 );
	REQUIRE( equal( obj.m_d2.m_num_double, 3.14 ) );
	REQUIRE( obj.m_d2.m_string == "xyz" );

	REQUIRE_FALSE( obj.m_d3 );

	REQUIRE( obj.m_d4 );
	REQUIRE( obj.m_d4->m_num_int32 == 42 );
	REQUIRE( equal( obj.m_d4->m_num_double, 3.14 ) );
	REQUIRE( obj.m_d4->m_string == "xyz" );
}

TEST_CASE( "non-nullable-defined" , "[non-nullble-defined]" )
{
	SECTION( "read" )
	{
		const std::string json_data{
			R"JSON(
			{
				"d1": {"num_int32": 42, "num_double": 3.14, "string": "xyz"},
				"d2": {"num_int32": 42, "num_double": 3.14, "string": "xyz"},
				"d3": null,
				"d4": null
			})JSON" };

		auto obj = json_dto::from_json< complex_data_t >( json_data );

		REQUIRE( obj.m_d1.m_num_int32 == 42 );
		REQUIRE( equal( obj.m_d1.m_num_double, 3.14 ) );
		REQUIRE( obj.m_d1.m_string == "xyz" );

		REQUIRE( obj.m_d2.m_num_int32 == 42 );
		REQUIRE( equal( obj.m_d2.m_num_double, 3.14 ) );
		REQUIRE( obj.m_d2.m_string == "xyz" );

		REQUIRE_FALSE( obj.m_d3 );
		REQUIRE_FALSE( obj.m_d4 );
	}

	SECTION( "write" )
	{
		data_t data{ 42, 3.14, "XYZ" };
		complex_data_t source_obj;

		source_obj.m_d1 = data;
		source_obj.m_d2 = data;

		const std::string json_data = json_dto::to_json( source_obj );

		auto obj = json_dto::from_json< complex_data_t >( json_data );

		REQUIRE( obj.m_d1.m_num_int32 == source_obj.m_d1.m_num_int32 );
		REQUIRE( equal( obj.m_d1.m_num_double, source_obj.m_d1.m_num_double ) );
		REQUIRE( obj.m_d1.m_string == source_obj.m_d1.m_string );

		REQUIRE( obj.m_d2.m_num_int32 == source_obj.m_d2.m_num_int32 );
		REQUIRE( equal( obj.m_d2.m_num_double, source_obj.m_d2.m_num_double ) );
		REQUIRE( obj.m_d2.m_string == source_obj.m_d2.m_string );

		REQUIRE_FALSE( obj.m_d3 );
		REQUIRE_FALSE( obj.m_d4 );
	}
}
