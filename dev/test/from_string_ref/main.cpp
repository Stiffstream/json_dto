#include <catch2/catch.hpp>

#include <iostream>
#include <sstream>

#include <rapidjson/document.h>
#include <json_dto/pub.hpp>

#include <test/helper.hpp>

using namespace json_dto;

struct supported_types_t
{
	supported_types_t() {}

	supported_types_t(
		bool val_bool,
		std::int16_t val_int16,
		std::uint16_t val_uint16,
		std::int32_t val_int32,
		std::uint32_t val_uint32,
		std::int64_t val_int64,
		std::uint64_t val_uint64,
		double val_double,
		std::string val_string )
		:	m_bool{ val_bool }
		,	m_int16{ val_int16 }
		,	m_uint16{ val_uint16 }
		,	m_int32{ val_int32 }
		,	m_uint32{ val_uint32 }
		,	m_int64{ val_int64 }
		,	m_uint64{ val_uint64 }
		,	m_double{ val_double }
		,	m_string{ std::move( val_string ) }
	{}

	bool m_bool{ false };

	std::int16_t m_int16{};
	std::uint16_t m_uint16{};

	std::int32_t m_int32{};
	std::uint32_t m_uint32{};

	std::int64_t m_int64{};
	std::uint64_t m_uint64{};
	double m_double{};

	std::string m_string{};
};

namespace json_dto
{

template < typename Json_Io >
void
json_io( Json_Io & io, supported_types_t & obj )
{
	io
		& mandatory( "bool", obj.m_bool )
		& mandatory( "int16", obj.m_int16 )
		& mandatory( "uint16", obj.m_uint16 )
		& mandatory( "int32", obj.m_int32 )
		& mandatory( "uint32", obj.m_uint32 )
		& mandatory( "int64", obj.m_int64 )
		& mandatory( "uint64", obj.m_uint64 )
		& mandatory( "double", obj.m_double )
		& mandatory( "string", obj.m_string );
}

} /* namespace json_dto */

TEST_CASE( "from_json from string_ref" , "[read]" )
{
	SECTION( "read valid (from_json(json))" )
	{
		const std::string json_data{
			R"JSON({
				"bool" : true,
				"int16" : -1,
				"uint16" : 2,
				"int32" : -4,
				"uint32" : 8,
				"int64" : -16,
				"uint64" : 32,
				"double" : 2.718281828,
				"string" : "TEST STRING"
			})JSON"
		};

		supported_types_t obj;

		obj = from_json< supported_types_t >(
				json_dto::make_string_ref( json_data ) );

		REQUIRE( obj.m_bool );
		REQUIRE( obj.m_int16 == -1 );
		REQUIRE( obj.m_uint16 == 2 );
		REQUIRE( obj.m_int32 == -4 );
		REQUIRE( obj.m_uint32 == 8 );
		REQUIRE( obj.m_int64 == -16LL );
		REQUIRE( obj.m_uint64 == 32ULL );
		REQUIRE( equal( obj.m_double , 2.718281828 ) );
		REQUIRE( obj.m_string == "TEST STRING" );
	}

	SECTION( "read part of string (from_json(json))" )
	{
		const std::string json_data{
			R"JSON({
				"bool" : true,
				"int16" : -1,
				"uint16" : 2,
				"int32" : -4,
				"uint32" : 8,
				"int64" : -16,
				"uint64" : 32,
				"double" : 2.718281828,
				"string" : "TEST STRING"
			} --123456--)JSON"
		};

		supported_types_t obj;

		obj = from_json< supported_types_t >(
				json_dto::make_string_ref(
						json_data.data(), json_data.size() - 10u ) );

		REQUIRE( obj.m_bool );
		REQUIRE( obj.m_int16 == -1 );
		REQUIRE( obj.m_uint16 == 2 );
		REQUIRE( obj.m_int32 == -4 );
		REQUIRE( obj.m_uint32 == 8 );
		REQUIRE( obj.m_int64 == -16LL );
		REQUIRE( obj.m_uint64 == 32ULL );
		REQUIRE( equal( obj.m_double , 2.718281828 ) );
		REQUIRE( obj.m_string == "TEST STRING" );
	}

	SECTION( "read valid (from_json(json, dest))" )
	{
		const std::string json_data{
			R"JSON({
				"bool" : true,
				"int16" : -1,
				"uint16" : 2,
				"int32" : -4,
				"uint32" : 8,
				"int64" : -16,
				"uint64" : 32,
				"double" : 2.718281828,
				"string" : "TEST STRING"
			})JSON"
		};

		supported_types_t obj;

		from_json(
				json_dto::make_string_ref( json_data ),
				obj );

		REQUIRE( obj.m_bool );
		REQUIRE( obj.m_int16 == -1 );
		REQUIRE( obj.m_uint16 == 2 );
		REQUIRE( obj.m_int32 == -4 );
		REQUIRE( obj.m_uint32 == 8 );
		REQUIRE( obj.m_int64 == -16LL );
		REQUIRE( obj.m_uint64 == 32ULL );
		REQUIRE( equal( obj.m_double , 2.718281828 ) );
		REQUIRE( obj.m_string == "TEST STRING" );
	}

	SECTION( "read part of string (from_json(json, dest))" )
	{
		const std::string json_data{
			R"JSON({
				"bool" : true,
				"int16" : -1,
				"uint16" : 2,
				"int32" : -4,
				"uint32" : 8,
				"int64" : -16,
				"uint64" : 32,
				"double" : 2.718281828,
				"string" : "TEST STRING"
			} --123456--)JSON"
		};

		supported_types_t obj;

		from_json(
				json_dto::make_string_ref(
						json_data.data(), json_data.size() - 10u ),
				obj );

		REQUIRE( obj.m_bool );
		REQUIRE( obj.m_int16 == -1 );
		REQUIRE( obj.m_uint16 == 2 );
		REQUIRE( obj.m_int32 == -4 );
		REQUIRE( obj.m_uint32 == 8 );
		REQUIRE( obj.m_int64 == -16LL );
		REQUIRE( obj.m_uint64 == 32ULL );
		REQUIRE( equal( obj.m_double , 2.718281828 ) );
		REQUIRE( obj.m_string == "TEST STRING" );
	}
}

