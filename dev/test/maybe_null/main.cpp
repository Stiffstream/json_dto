#include <catch2/catch.hpp>

#include <iostream>
#include <limits>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

#include <test/helper.hpp>

using namespace json_dto;

TEST_CASE("maybe_null_field_wrapper_t must be trivially copyable", "[basics]" )
{
	REQUIRE( std::is_trivially_copyable< maybe_null_field_wrapper_t< int > >::value );
	REQUIRE( std::is_trivially_copyable<
			maybe_null_field_wrapper_t< std::string >
		>::value );

	REQUIRE( std::is_trivially_copyable<
			maybe_null_field_wrapper_t< std::vector< std::string > >
		>::value );

	REQUIRE( std::is_trivially_copyable<
			maybe_null_field_wrapper_t< std::vector< std::string > >
		>::value );
}

//
// mandatory_binds_dto_t
//

struct mandatory_binds_dto_t
{
	std::int32_t m_number{ -1 };
	std::string  m_string{ "SPOILED" };
	std::vector< std::string > m_strings{ "SPOIL", "DEFAULT", "CONSTRUCTOR" };

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& mandatory( "number", maybe_null( m_number ) )
			& mandatory( "string", maybe_null( m_string ) )
			& mandatory( "strings", maybe_null( m_strings ) )
		;
	}
};

TEST_CASE("mandatory", "[maybe_null]" )
{
	SECTION( "read nulls" )
	{
		auto dto = json_dto::from_json< mandatory_binds_dto_t >( R"({
			"number" : null,
			"string" : null,
			"strings": null
		})" );

		REQUIRE( dto.m_number == 0 );
		REQUIRE( dto.m_string.empty() );
		REQUIRE( dto.m_strings.empty() );
	}

	SECTION( "read non-nulls" )
	{
		auto dto = json_dto::from_json< mandatory_binds_dto_t >( R"({
			"number" : 100,
			"string" : "qwe123",
			"strings": ["1", "2"]
		})" );

		REQUIRE( dto.m_number == 100 );
		REQUIRE( dto.m_string == "qwe123" );
		REQUIRE( dto.m_strings.size() == 2 );
		REQUIRE( dto.m_strings[0] == "1" );
		REQUIRE( dto.m_strings[1] == "2" );
	}
}

struct optional_binds_dto_t
{
	std::int32_t m_number{ -1 };
	std::string  m_string{ "SPOILED" };
	std::vector< std::string > m_strings{ "SPOIL", "DEFAULT", "CONSTRUCTOR" };

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional( "number", maybe_null( m_number ), 42 )
			& optional( "string", maybe_null( m_string ), "default" )
			& optional(
				"strings",
				maybe_null( m_strings ),
				decltype(m_strings){ "xyz", "qwe", "123" } )
		;
	}
};

TEST_CASE("optional", "[maybe_null]" )
{
	SECTION( "read nulls" )
	{
		auto dto = json_dto::from_json< optional_binds_dto_t >( R"({
			"number" : null,
			"string" : null,
			"strings": null
		})" );

		REQUIRE( dto.m_number == 42 );
		REQUIRE( dto.m_string == "default" );
		REQUIRE( dto.m_strings.size() == 3 );
		REQUIRE( dto.m_strings[ 0 ] == "xyz" );
		REQUIRE( dto.m_strings[ 1 ] == "qwe" );
		REQUIRE( dto.m_strings[ 2 ] == "123" );
	}

	SECTION( "read empty" )
	{
		auto dto = json_dto::from_json< optional_binds_dto_t >( R"({})" );

		REQUIRE( dto.m_number == 42 );
		REQUIRE( dto.m_string == "default" );
		REQUIRE( dto.m_strings.size() == 3 );
		REQUIRE( dto.m_strings[ 0 ] == "xyz" );
		REQUIRE( dto.m_strings[ 1 ] == "qwe" );
		REQUIRE( dto.m_strings[ 2 ] == "123" );
	}

	SECTION( "read explicitly specified" )
	{
		auto dto = json_dto::from_json< optional_binds_dto_t >( R"({
			"number" : 100,
			"string" : "qwe123",
			"strings": ["1", "2"]
		})" );

		REQUIRE( dto.m_number == 100 );
		REQUIRE( dto.m_string == "qwe123" );
		REQUIRE( dto.m_strings.size() == 2 );
		REQUIRE( dto.m_strings[0] == "1" );
		REQUIRE( dto.m_strings[1] == "2" );
	}
}

struct optional_no_default_binds_dto_t
{
	std::int32_t m_number{ -1 };
	std::string  m_string{ "SPOILED" };
	std::vector< std::string > m_strings{ "SPOIL", "DEFAULT", "CONSTRUCTOR" };

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional_no_default( "number", maybe_null( m_number ) )
			& optional_no_default( "string", maybe_null( m_string ) )
			& optional_no_default( "strings", maybe_null( m_strings ) )
		;
	}
};

TEST_CASE("optional_no_default", "[maybe_null]" )
{
	SECTION( "read nulls" )
	{
		auto dto = json_dto::from_json< optional_no_default_binds_dto_t >( R"({
			"number" : null,
			"string" : null,
			"strings": null
		})" );

		REQUIRE( dto.m_number == 0 );
		REQUIRE( dto.m_string.empty() );
		REQUIRE( dto.m_strings.empty() );
	}

	SECTION( "read empty" )
	{
		auto dto = json_dto::from_json< optional_no_default_binds_dto_t >( R"({})" );

		REQUIRE( dto.m_number == -1 );
		REQUIRE( dto.m_string == "SPOILED" );
		REQUIRE( dto.m_strings.size() == 3 );
		REQUIRE( dto.m_strings[ 0 ] == "SPOIL" );
		REQUIRE( dto.m_strings[ 1 ] == "DEFAULT" );
		REQUIRE( dto.m_strings[ 2 ] == "CONSTRUCTOR" );

	}

	SECTION( "read explicitly specified" )
	{
		auto dto = json_dto::from_json< optional_no_default_binds_dto_t >( R"({
			"number" : 100,
			"string" : "qwe123",
			"strings": ["1", "2"]
		})" );

		REQUIRE( dto.m_number == 100 );
		REQUIRE( dto.m_string == "qwe123" );
		REQUIRE( dto.m_strings.size() == 2 );
		REQUIRE( dto.m_strings[0] == "1" );
		REQUIRE( dto.m_strings[1] == "2" );
	}
}
