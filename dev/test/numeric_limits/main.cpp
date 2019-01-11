#include <catch2/catch.hpp>

#include <iostream>
#include <limits>
#include <type_traits>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

using namespace json_dto;

void
parse_object_from_string(
	const std::string & json,
	rapidjson::Document & doc )
{
	doc.Parse( json.c_str() );
}

//
// simple_optional
//

struct nums_t
{
	std::int16_t m_num_int16;
	std::int32_t m_num_int32;
	std::int64_t m_num_int64;

	std::uint16_t m_num_uint16;
	std::uint32_t m_num_uint32;
	std::uint64_t m_num_uint64;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional( "num_int16", m_num_int16, std::int16_t{ 0 } )
			& optional( "num_int32", m_num_int32, std::int32_t{ 0 } )
			& optional( "num_int64", m_num_int64, std::int64_t{ 0 } )

			& optional( "num_uint16", m_num_uint16, std::uint16_t{ 0 } )
			& optional( "num_uint32", m_num_uint32, std::uint32_t{ 0 } )
			& optional( "num_uint64", m_num_uint64, std::uint64_t{ 0 } );
	}
};

template < typename NUMBER_TYPE >
void
check_min_max(
	const std::string & key_name,
	NUMBER_TYPE nums_t::* value_ptr )
{
	SECTION( "check min" )
	{
		const auto min_value = std::numeric_limits< NUMBER_TYPE >::min();

		auto dto =
			json_dto::from_json< nums_t >(
				"{"
					"\"" + key_name + "\":" + std::to_string( min_value ) +
				"}" );

		REQUIRE( ( min_value == dto.*value_ptr ) );
	}

	SECTION( "check max" )
	{
		const auto max_value = std::numeric_limits< NUMBER_TYPE >::max();
		auto dto =
			json_dto::from_json< nums_t >(
				"{"
					"\"" + key_name + "\":" + std::to_string( max_value ) +
				"}" );

		REQUIRE( ( max_value == dto.*value_ptr ) );
	}
}

template < typename BIGGER_NUMBER_TYPE, typename NUMBER_TYPE >
void
check_out_of_range(
	const std::string & key_name )
{
	if( std::is_signed< BIGGER_NUMBER_TYPE >::value )
		SECTION( "check less" )
		{
			BIGGER_NUMBER_TYPE min_value = std::numeric_limits< NUMBER_TYPE >::min();
			--min_value;

			bool error = false;
			try
			{
				json_dto::from_json< nums_t >(
					"{"
						"\"" + key_name + "\":" + std::to_string( min_value ) +
					"}" );
			}
			catch( ... )
			{
				error = true;
			}

			REQUIRE( error );
		}

	SECTION( "check greater" )
	{
		BIGGER_NUMBER_TYPE max_value = std::numeric_limits< NUMBER_TYPE >::max();
		++max_value;

		bool error = false;
		try
		{
			json_dto::from_json< nums_t >(
				"{"
					"\"" + key_name + "\":" + std::to_string( max_value ) +
				"}" );
		}
		catch( ... )
		{
			error = true;
		}

		REQUIRE( error );
	}
}

TEST_CASE( "limits-int16" , "[int16]" )
{
	SECTION( "min/max" )
	{
		check_min_max( "num_int16", &nums_t::m_num_int16 );
	}

	SECTION( "out of range" )
	{

		check_out_of_range< std::int32_t, std::int16_t >( "num_int16" );
	}
}

TEST_CASE( "limits-uint16" , "[uint16]" )
{
	SECTION( "min/max" )
	{
		check_min_max( "num_uint16", &nums_t::m_num_uint16 );
	}

	SECTION( "out of range" )
	{
		check_out_of_range< std::uint32_t, std::uint16_t >( "num_uint16" );
	}
}

TEST_CASE( "limits-int32" , "[int32]" )
{
	SECTION( "min/max" )
	{
		check_min_max( "num_int32", &nums_t::m_num_int32 );
	}

	SECTION( "out of range" )
	{
		check_out_of_range< std::int64_t, std::int32_t >( "num_int32" );
	}
}

TEST_CASE( "limits-uint32" , "[uint32]" )
{
	SECTION( "min/max" )
	{
		check_min_max( "num_uint32", &nums_t::m_num_uint32 );
	}

	SECTION( "out of range" )
	{
		check_out_of_range< std::uint64_t, std::uint32_t >( "num_uint32" );
	}
}

TEST_CASE( "limits-int64" , "[int64]" )
{
	SECTION( "min/max" )
	{
		check_min_max( "num_int64", &nums_t::m_num_int64 );
	}
}

TEST_CASE( "limits-uint64" , "[uint64]" )
{
	SECTION( "min/max" )
	{
		check_min_max( "num_uint64", &nums_t::m_num_uint64 );
	}
}
