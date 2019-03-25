#include <catch2/catch.hpp>

#include <json_dto/pub.hpp>

#include <deque>
#include <list>
#include <forward_list>

#include <map>
#include <unordered_map>

template<typename T>
struct data_with_t
{
	T m_data;

	template<typename Json_Io>
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "data", m_data );
	}
};

TEST_CASE( "deque<int>: read from json" , "read-deque-int" )
{
	{
		const std::string json_data{
			R"JSON(
			{"data":[2, 5, 1, 9, 0]}
			)JSON" };
		auto obj = json_dto::from_json< data_with_t< std::deque<int> > >(
				json_data );

		const std::deque<int> expected{ 2, 5, 1, 9, 0 };
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "deque<int>: write to json" , "write-deque-int" )
{
	{
		data_with_t< std::deque<int> > obj{ {1, 2, 3, 4, 5} };
		const auto r = json_dto::to_json( obj );

		REQUIRE( R"({"data":[1,2,3,4,5]})" == r );
	}
}

TEST_CASE( "list<int>: read from json" , "read-list-int" )
{
	{
		const std::string json_data{
			R"JSON(
			{"data":[2, 5, 1, 9, 0]}
			)JSON" };
		auto obj = json_dto::from_json< data_with_t< std::list<int> > >(
				json_data );

		const std::list<int> expected{ 2, 5, 1, 9, 0 };
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "list<int>: write to json" , "write-list-int" )
{
	{
		data_with_t< std::list<int> > obj{ {1, 2, 3, 4, 5} };
		const auto r = json_dto::to_json( obj );

		REQUIRE( R"({"data":[1,2,3,4,5]})" == r );
	}
}

TEST_CASE( "forward_list<int>: read from json" , "read-forward_list-int" )
{
	{
		const std::string json_data{
			R"JSON(
			{"data":[2, 5, 1, 9, 0]}
			)JSON" };
		auto obj = json_dto::from_json<
				data_with_t< std::forward_list<int> > >( json_data );

		const std::forward_list<int> expected{ 2, 5, 1, 9, 0 };
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "forward_list<int>: write to json" , "write-forward_list-int" )
{
	{
		data_with_t< std::forward_list<int> > obj{ {1, 2, 3, 4, 5} };
		const auto r = json_dto::to_json( obj );

		REQUIRE( R"({"data":[1,2,3,4,5]})" == r );
	}
}

TEST_CASE( "map<string, int>: read from json" , "read-map-string-int" )
{
	{
		const std::string json_data{
			R"JSON(
			{"data":{"one":1, "two":2, "three":3}}
			)JSON" };
		auto obj = json_dto::from_json<
				data_with_t< std::map<std::string, int> > >( json_data );

		const std::map<std::string, int> expected{
			{"one", 1}, {"three", 3}, {"two", 2}
		};
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "map<string, int>: write to json" , "write-map-string-int" )
{
	data_with_t< std::map<std::string, int> > obj{
		{ {"one", 1}, {"three", 3}, {"two", 2} }
	};
	const auto r = json_dto::to_json( obj );

	REQUIRE( R"({"data":{"one":1,"three":3,"two":2}})" == r );
}

TEST_CASE( "multimap<string, int>: write to json" , "write-multimap-string-int" )
{
	data_with_t< std::multimap<std::string, int> > obj{
		{ {"one", 1}, {"three", 3}, {"two", 2},
			{"one", 11},
			{"three", 33}, {"three", 333}
		}
	};
	const auto r = json_dto::to_json( obj );

	REQUIRE( R"({"data":{"one":1,"three":3,"two":2}})" == r );
}

#if 0

TEST_CASE( "hash_map<string, int>: read from json" , "read-hash_map-string-int" )
{
	{
		const std::string json_data{
			R"JSON(
			{"data":{"one":1, "two":2, "three":3}}
			)JSON" };
		auto obj = json_dto::from_json<
				data_with_t< std::unordered_map<std::string, int> > >( json_data );

		const std::unordered_map<std::string, int> expected{
			{"one", 1}, {"three", 3}, {"two", 2}
		};
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "hash_map<string, int>: write to json" , "write-hash_map-string-int" )
{
	using namespace Catch::Matchers;

	data_with_t< std::unordered_map<std::string, int> > obj{
		{ {"one", 1}, {"three", 3}, {"two", 2} }
	};
	const auto r = json_dto::to_json( obj );

	REQUIRE_THAT( r,
			Contains(R"("one":1)") &&
			Contains(R"("two":2)") &&
			Contains(R"("three":3)") );
}

#endif
