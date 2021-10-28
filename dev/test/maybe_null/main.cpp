#include <catch2/catch.hpp>

#include <iostream>
#include <vector>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

using namespace json_dto;

struct simple_data_t
{
	std::vector< std::string > m_first;

	template< typename Io >
	void json_io( Io & io )
	{
		io & mandatory_maybe_null( "first", m_first );
	}
};

TEST_CASE( "simple_data" , "read-write" )
{
	SECTION( "field present" )
	{
		const std::string json_data{
			R"JSON(
			{
				"first": ["A", "B", "C"]
			})JSON" };

		auto obj = json_dto::from_json< simple_data_t > ( json_data );

		REQUIRE( obj.m_first == std::vector< std::string >{ "A", "B", "C" } );

		const auto serialized = json_dto::to_json( obj );

		REQUIRE( serialized == R"JSON({"first":["A","B","C"]})JSON" );
	}

	SECTION( "field is null" )
	{
		const std::string json_data{
			R"JSON(
			{
				"first": null
			})JSON" };

		auto obj = json_dto::from_json< simple_data_t > ( json_data );

		REQUIRE( obj.m_first == std::vector< std::string >{} );
	}
}

