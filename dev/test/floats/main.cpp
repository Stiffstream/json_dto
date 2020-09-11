#include <catch2/catch.hpp>

#include <iostream>
#include <limits>
#include <type_traits>
#include <cmath>
#include <cctype>

#undef RAPIDJSON_WRITE_DEFAULT_FLAGS
#define RAPIDJSON_WRITE_DEFAULT_FLAGS rapidjson::kWriteNanAndInfFlag

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

using namespace json_dto;

struct floats_t
{
	float m_num_float;
	double m_num_double;

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional( "num_float", m_num_float, 0.0f )
			& optional( "num_double", m_num_double, 0.0 );
	}
};

TEST_CASE( "default init" , "[init]" )
{
	{
		const char * json_str = "{}";

		auto dto = json_dto::from_json< floats_t >( json_str );

		REQUIRE( Approx(0.0) == dto.m_num_float );
		REQUIRE( Approx(0.0) == dto.m_num_double );
	}
}

TEST_CASE( "point notation" , "[point_notation]" )
{
	{
		const char * json_str = "{"
			"\"num_float\": 3.14,"
			"\"num_double\": 2.718281828459"
		"}";

		auto dto = json_dto::from_json< floats_t >( json_str );

		REQUIRE( std::fabs( 3.14 - dto.m_num_float ) < 0.0001 );
		REQUIRE( std::fabs( 2.718281828459 - dto.m_num_double ) < 0.0000000000001 );
	}

	{
		const char * json_str = "{"
			"\"num_float\": 3,"
			"\"num_double\": 2"
		"}";

		auto dto = json_dto::from_json< floats_t >( json_str );

		REQUIRE( Approx(3.0) == dto.m_num_float );
		REQUIRE( Approx(2.0) == dto.m_num_double );
	}
}

TEST_CASE( "NaN" , "[nan]" )
{
	{
		const floats_t v{ 0.0f, std::numeric_limits<double>::quiet_NaN() };

		auto str = json_dto::to_json( v );

		REQUIRE( R"({"num_double":NaN})" == str );

		const auto dv =
				json_dto::from_json< floats_t, rapidjson::kParseNanAndInfFlag >( str );
		REQUIRE( Approx(0.0) == dv.m_num_float );
		REQUIRE( std::isnan(dv.m_num_double) );
	}

	{
		floats_t dv;

		REQUIRE_NOTHROW( dv = json_dto::from_json< floats_t,
				rapidjson::kParseNanAndInfFlag >( R"({"num_double":NaN})" ) );

		REQUIRE( Approx(0.0) == dv.m_num_float );
		REQUIRE( std::isnan(dv.m_num_double) );
	}
}

bool equal_caseless(
	json_dto::string_ref_t a,
	json_dto::string_ref_t b ) noexcept
{
	if( a.length == b.length )
		return std::equal( a.s, a.s + a.length, b.s,
				[]( unsigned char ch1, unsigned char ch2 ) noexcept {
					return std::tolower(ch1) == std::tolower(ch2);
				} );

	return false;
}

struct custom_floating_point_reader_writer
{
	template< typename T >
	void read( T & v, const rapidjson::Value & from ) const
	{
		if( from.IsNumber() )
		{
			json_dto::read_json_value( v, from );
			return;
		}
		else if( from.IsString() )
		{
			const json_dto::string_ref_t str_v{ from.GetString() };
			if( equal_caseless( str_v, "nan" ) )
			{
				v = std::numeric_limits<T>::quiet_NaN();
				return;
			}
			else if( equal_caseless( str_v, "inf" ) )
			{
				v = std::numeric_limits<T>::infinity();
				return;
			}
			else if( equal_caseless( str_v, "-inf" ) )
			{
				v = -std::numeric_limits<T>::infinity();
				return;
			}
		}

		throw json_dto::ex_t{ "unable to parse value" };
	}

	template< typename T >
	void
	write(
		T & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		using json_dto::write_json_value;
		using json_dto::string_ref_t;

		if( std::isnan(v) )
			write_json_value( string_ref_t{"nan"}, to, allocator );
		else if( v > std::numeric_limits<T>::max() )
			write_json_value( string_ref_t{"inf"}, to, allocator );
		else if( v < std::numeric_limits<T>::min() )
			write_json_value( string_ref_t{"-inf"}, to, allocator );
		else
			write_json_value( v, to, allocator );
	}
};

struct floats2_t
{
	float m_num_float;
	double m_num_double;

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional( custom_floating_point_reader_writer{},
					"num_float", m_num_float, 0.0f )
			& optional( custom_floating_point_reader_writer{},
					"num_double", m_num_double, 0.0 );
	}
};

TEST_CASE( "custom NaN" , "[nan]" )
{
	{
		const floats2_t v{ 0.0f, std::numeric_limits<double>::quiet_NaN() };

		auto str = json_dto::to_json( v );

		REQUIRE( R"({"num_double":"nan"})" == str );

		const auto dv =
				json_dto::from_json< floats2_t, rapidjson::kParseNanAndInfFlag >( str );
		REQUIRE( Approx(0.0) == dv.m_num_float );
		REQUIRE( std::isnan(dv.m_num_double) );
	}

	{
		const floats2_t v{
				std::numeric_limits<float>::infinity(),
				-std::numeric_limits<double>::infinity()
		};

		auto str = json_dto::to_json( v );

		REQUIRE( R"({"num_float":"inf","num_double":"-inf"})" == str );

		const auto dv =
				json_dto::from_json< floats2_t, rapidjson::kParseNanAndInfFlag >( str );
		REQUIRE( v.m_num_float == dv.m_num_float );
		REQUIRE( v.m_num_double == dv.m_num_double );
	}

	{
		floats2_t dv;

		REQUIRE_NOTHROW( dv = json_dto::from_json< floats2_t,
				rapidjson::kParseNanAndInfFlag >( R"({"num_double":"Nan"})" ) );

		REQUIRE( Approx(0.0) == dv.m_num_float );
		REQUIRE( std::isnan(dv.m_num_double) );
	}
}

