#include <catch2/catch.hpp>

#include <json_dto/pub.hpp>

#include <deque>
#include <list>
#include <forward_list>

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

