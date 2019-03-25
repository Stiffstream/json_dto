#include <catch2/catch.hpp>

#include <json_dto/pub.hpp>

#include <deque>
#include <list>
#include <forward_list>

#include <map>
#include <unordered_map>

template<typename T>
struct data_with_deque_t
{
	std::deque<T> m_data;

	template<typename Json_Io>
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "data", m_data );
	}
};

template<typename T>
struct data_with_list_t
{
	std::list<T> m_data;

	template<typename Json_Io>
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "data", m_data );
	}
};

template<typename T>
struct data_with_forward_list_t
{
	std::forward_list<T> m_data;

	template<typename Json_Io>
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "data", m_data );
	}
};

template<typename K, typename V>
struct data_with_map_t
{
	std::map<K, V> m_data;

	template<typename Json_Io>
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "data", m_data );
	}
};

template<typename K, typename V>
struct data_with_unordered_map_t
{
	std::unordered_map<K, V> m_data;

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
		auto obj = json_dto::from_json< data_with_deque_t<int> >( json_data );

		const std::deque<int> expected{ 2, 5, 1, 9, 0 };
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "deque<int>: write to json" , "write-deque-int" )
{
	{
		data_with_deque_t<int> obj{ {1, 2, 3, 4, 5} };
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
		auto obj = json_dto::from_json< data_with_list_t<int> >( json_data );

		const std::list<int> expected{ 2, 5, 1, 9, 0 };
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "list<int>: write to json" , "write-list-int" )
{
	{
		data_with_list_t<int> obj{ {1, 2, 3, 4, 5} };
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
		auto obj = json_dto::from_json< data_with_forward_list_t<int> >( json_data );

		const std::forward_list<int> expected{ 2, 5, 1, 9, 0 };
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "forward_list<int>: write to json" , "write-forward_list-int" )
{
	{
		data_with_forward_list_t<int> obj{ {1, 2, 3, 4, 5} };
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
		auto obj = json_dto::from_json< data_with_map_t<std::string, int> >( json_data );

		const std::map<std::string, int> expected{
			{"one", 1}, {"three", 3}, {"two", 2}
		};
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "map<string, int>: write to json" , "write-map-string-int" )
{
	data_with_map_t<std::string, int> obj{
		{ {"one", 1}, {"three", 3}, {"two", 2} }
	};
	const auto r = json_dto::to_json( obj );

	REQUIRE( R"({"data":{"one":1,"three":3,"two":2}})" == r );
}

TEST_CASE( "hash_map<string, int>: read from json" , "read-hash_map-string-int" )
{
	{
		const std::string json_data{
			R"JSON(
			{"data":{"one":1, "two":2, "three":3}}
			)JSON" };
		auto obj = json_dto::from_json<
				data_with_unordered_map_t<std::string, int> >( json_data );

		const std::unordered_map<std::string, int> expected{
			{"one", 1}, {"three", 3}, {"two", 2}
		};
		REQUIRE( obj.m_data == expected );
	}
}

TEST_CASE( "hash_map<string, int>: write to json" , "write-hash_map-string-int" )
{
	using namespace Catch::Matchers;

	data_with_unordered_map_t<std::string, int> obj{
		{ {"one", 1}, {"three", 3}, {"two", 2} }
	};
	const auto r = json_dto::to_json( obj );

	REQUIRE_THAT( r,
			Contains(R"("one":1)") &&
			Contains(R"("two":2)") &&
			Contains(R"("three":3)") );
}

