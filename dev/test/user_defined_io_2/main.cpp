#include <catch2/catch.hpp>

#include <iostream>
#include <chrono>
#include <ctime>
#include <regex>
#include <array>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

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

template< typename T, T min, T max, T default_value >
void
read_json_value(
	bounded_value_t< T, min, max, default_value > & value,
	const rapidjson::Value & from )
{
	using namespace json_dto;

	T v{ default_value };
	read_json_value( v, from );

	value.set( v );
}

template< typename T, T min, T max, T default_value >
void
write_json_value(
	const bounded_value_t< T, min, max, default_value > & value,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	using namespace json_dto;

	write_json_value( value.get(), object, allocator );
}

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
}

TEST_CASE( "user defined io write" , "write" )
{
	const data_t src{ year_day_t{ 255 } };

	const auto r = json_dto::to_json( src );
	REQUIRE( R"({"yday":255})" == r );
}

