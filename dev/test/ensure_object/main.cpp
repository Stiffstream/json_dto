#include <catch2/catch.hpp>

#include <iostream>
#include <limits>
#include <type_traits>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>



struct interval_t
{
	std::int32_t m_start{};
	std::int64_t m_finish{};

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "start", m_start )
			& json_dto::mandatory( "finish", m_finish );
	}
};

struct wrapper_t
{
	interval_t m_interval;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "interval", m_interval );
	}
};

struct interval_opt_t
{
	std::int32_t m_start;
	std::int64_t m_finish;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::optional( "start", m_start, 0 )
			& json_dto::optional( "finish", m_finish, 0 );
	}
};

struct wrapper_opt_t
{
	interval_opt_t m_interval;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::optional_no_default( "interval", m_interval );
	}
};

TEST_CASE( "ensure-object" , "[object]" )
{
	const char * json_str =
		R"({
			"interval":1
		})";

	REQUIRE_THROWS( json_dto::from_json<wrapper_t>( json_str ) );
	REQUIRE_THROWS( json_dto::from_json<wrapper_opt_t>( json_str ) );
}

TEST_CASE( "ensure-not-object" , "[object]" )
{
	const char * json_str =
		R"({
			"interval":{
				"start":{},
				"from":{}
			}
		})";

	REQUIRE_THROWS( json_dto::from_json<wrapper_t>( json_str ) );
	REQUIRE_THROWS( json_dto::from_json<wrapper_opt_t>( json_str ) );
}

