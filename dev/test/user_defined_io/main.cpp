#include <catch2/catch.hpp>

#include <iostream>
#include <chrono>
#include <ctime>
#include <regex>
#include <array>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

using namespace json_dto;

struct data_t
{
	data_t()
	{}

	data_t(
		std::chrono::system_clock::duration duration,
		std::chrono::system_clock::time_point time_point,
		std::tm dt )
		:	m_duration{ std::move( duration ) }
		,	m_time_point{ std::move( time_point ) }
		,	m_dt{ dt }
	{}

	std::chrono::system_clock::duration m_duration{};
	std::chrono::system_clock::time_point m_time_point{};
	std::tm m_dt{};
};

namespace json_dto
{

// -------------------------------------------------------------------

//
// rw json value for std::chrono::system_clock::duration
//

template <>
void
read_json_value(
	std::chrono::system_clock::duration & v,
	const rapidjson::Value & object )
{
	try
	{
		std::int64_t representation;
		read_json_value( representation, object );

		v = std::chrono::microseconds{ representation };
	}
	catch( const std::exception & ex )
	{
		throw std::runtime_error{
			std::string{ "unable to read std::chrono::system_clock::duration: " } +
			ex.what() };
	}
}

template <>
void
write_json_value(
	const std::chrono::system_clock::duration & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	std::int64_t representation =
		std::chrono::duration_cast< std::chrono::microseconds >( v ).count();

	write_json_value( representation, object, allocator );
}

// -------------------------------------------------------------------

//
// rw json value for std::chrono::system_clock::time_point
//

template <>
void
read_json_value(
	std::chrono::system_clock::time_point & v,
	const rapidjson::Value & object )
{
	try
	{
		std::int64_t representation;
		read_json_value( representation, object );

		v =
			std::chrono::system_clock::time_point{
				std::chrono::milliseconds{ representation } };
	}
	catch( const std::exception & ex )
	{
		throw std::runtime_error{
			std::string{ "unable to read std::chrono::system_clock::time_point: " } +
			ex.what() };
	}
}

template <>
void
write_json_value(
	const std::chrono::system_clock::time_point & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	std::int64_t representation =
		std::chrono::duration_cast< std::chrono::milliseconds >(
			v.time_since_epoch() ).count();

	write_json_value( representation, object, allocator );
}

// -------------------------------------------------------------------

//
// rw json value for std::tm
//

template <>
void
read_json_value(
	std::tm & v,
	const rapidjson::Value & object )
{
	try
	{
		std::string representation;
		read_json_value( representation, object );

		const std::regex dt_regex{
			R"regex(^(\d{4})\.(\d{2})\.(\d{2}) (\d{2}):(\d{2}):(\d{2})$)regex" };

		std::smatch match_results;

		if( !std::regex_match( representation, match_results, dt_regex ) )
		{
			throw std::runtime_error{
				"invalid timesptamp string: \"" +
				representation + "\""};
		}

		v.tm_year = std::stoi( match_results[ 1 ] ) - 1900;
		v.tm_mon = std::stoi( match_results[ 2 ] ) - 1;
		v.tm_mday = std::stoi( match_results[ 3 ] );
		v.tm_hour = std::stoi( match_results[ 4 ] );
		v.tm_min = std::stoi( match_results[ 5 ] );
		v.tm_sec = std::stoi( match_results[ 6 ] );
	}
	catch( const std::exception & ex )
	{
		throw std::runtime_error{
			std::string{ "unable to read std::tm: " } +
			ex.what() };
	}
}

template <>
void
write_json_value(
	const std::tm & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	std::array< char, 64 > buf;

	std::sprintf(
		buf.data(),
		"%04d.%02d.%02d %02d:%02d:%02d",
		v.tm_year + 1900,
		v.tm_mon + 1,
		v.tm_mday,
		v.tm_hour,
		v.tm_min,
		v.tm_sec );

	std::string representation{ buf.data() };

	write_json_value( representation, object, allocator );
}

// -------------------------------------------------------------------

template < typename Json_Io >
void
json_io( Json_Io & io, data_t & value )
{
	io
		& mandatory( "duration", value.m_duration )
		& mandatory( "time_point", value.m_time_point )
		& mandatory( "dt", value.m_dt );
}

} /* namespace json_dto */


TEST_CASE( "user defined io read" , "read" )
{
	const std::string json_data{
		R"JSON(
		{
			"duration": 3661420899,
			"time_point": 1471509776042,
			"dt": "2016.08.18 10:44:35"
		})JSON" };

	auto obj = json_dto::from_json< data_t > ( json_data );

	REQUIRE( obj.m_duration == std::chrono::microseconds{ 3661420899LL } );
	REQUIRE( obj.m_time_point.time_since_epoch() ==
		std::chrono::milliseconds{ 1471509776042 } );

	const auto dt = obj.m_dt;
	REQUIRE( dt.tm_year + 1900 == 2016 );
	REQUIRE( dt.tm_mon + 1 == 8 );
	REQUIRE( dt.tm_mday == 18 );
	REQUIRE( dt.tm_hour == 10 );
	REQUIRE( dt.tm_min == 44 );
	REQUIRE( dt.tm_sec == 35 );
}

TEST_CASE( "user defined io write" , "write" )
{
	const auto now = std::chrono::system_clock::now();

	const std::chrono::system_clock::time_point now_time_point{
		std::chrono::duration_cast< std::chrono::milliseconds >(
			now.time_since_epoch() ) };

	const auto now_dur{
		std::chrono::duration_cast< std::chrono::microseconds >(
			now.time_since_epoch() ) };

	std::tm dt;
	{
		auto t = std::chrono::system_clock::to_time_t( now );
		dt = *std::gmtime( &t );
	}

	const data_t source_obj{ now_dur, now_time_point, dt };
	const std::string json_data = json_dto::to_json( source_obj );

	auto obj = json_dto::from_json< data_t > ( json_data );

	REQUIRE( obj.m_duration == source_obj.m_duration );
	REQUIRE( obj.m_time_point.time_since_epoch() ==
		source_obj.m_time_point.time_since_epoch() );

	const auto dt_results = obj.m_dt;
	const auto source_dt = source_obj.m_dt;
	REQUIRE( dt_results.tm_year == source_dt.tm_year );
	REQUIRE( dt_results.tm_mon == source_dt.tm_mon );
	REQUIRE( dt_results.tm_mday == source_dt.tm_mday );
	REQUIRE( dt_results.tm_hour == source_dt.tm_hour );
	REQUIRE( dt_results.tm_min == source_dt.tm_min );
	REQUIRE( dt_results.tm_sec == source_dt.tm_sec );
}
