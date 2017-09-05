#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <iostream>
#include <chrono>
#include <ctime>
#include <regex>
#include <array>

#include <rapidjson/document.h>

template< typename T, T min, T max, T default_value = T{} >
class bounded_value_t
{
	T m_value{ default_value };

	static T
	ensure_valid( T v )
		{
			if( v < min || v > max )
				throw std::invalid_argument{ "value is out of range!" };
			return v;
		}

public :
	bounded_value_t()
		{
			ensure_valid( m_value );
		}
	bounded_value_t( T value ) : m_value{ ensure_valid( value ) }
		{}

	T get() const { return m_value; }
	void set( T v ) { m_value = ensure_valid( v ); }
};

namespace json_dto
{

template< typename T, T min, T max, T default_value >
void
read_json_value(
	const rapidjson::Value & from,
	bounded_value_t< T, min, max, default_value > & value );

} /* namespace json_dto */

#include <json_dto/pub.hpp>

namespace json_dto
{

template< typename T, T min, T max, T default_value >
void
read_json_value(
	const rapidjson::Value & from,
	bounded_value_t< T, min, max, default_value > & value )
{
	T v{ default_value };
	read_json_value( from, v );

	value.set( v );
}

} /* namespace json_dto */

using year_day_t = bounded_value_t< int, 1, 366, 1 >;

struct data_t
{
	year_day_t m_day;

	template<typename IO>
	void json_io( IO & io )
	{
		io & json_dto::mandatory( "yday", m_day );
	}
};



TEST_CASE( "user defined io read" , "read" )
{
	const std::string json_data{
		R"JSON(
		{
			"yday": 365
		})JSON" };
	auto obj = json_dto::from_json< data_t >( json_data );

	REQUIRE( 365 == obj.m_day.get() );
#if 0
	REQUIRE( obj.m_time_point.time_since_epoch() ==
		std::chrono::milliseconds{ 1471509776042 } );

	const auto dt = obj.m_dt;
	REQUIRE( dt.tm_year + 1900 == 2016 );
	REQUIRE( dt.tm_mon + 1 == 8 );
	REQUIRE( dt.tm_mday == 18 );
	REQUIRE( dt.tm_hour == 10 );
	REQUIRE( dt.tm_min == 44 );
	REQUIRE( dt.tm_sec == 35 );
#endif
}

TEST_CASE( "user defined io write" , "write" )
{
}
