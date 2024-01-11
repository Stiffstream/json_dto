#include <catch2/catch.hpp>

#include <json_dto/pub.hpp>

namespace test
{

struct data_t
{
	std::vector<json_dto::nullable_t<std::string>> m_values;

	template<typename I>
	void json_io(I & io)
	{
		io & json_dto::optional_no_default("values", m_values)
			;
	}
};

} /* namespace test */

using namespace test;

TEST_CASE( "vector_of_nullable from_json", "[basic]" )
{
	{
		const std::string json_data{
			R"JSON(
			{"values":[null]}
			)JSON" };
		auto obj = json_dto::from_json< data_t >( json_data );

		REQUIRE( 1u == obj.m_values.size() );
		REQUIRE( !obj.m_values[0].has_value() );
	}

	{
		const std::string json_data{
			R"JSON(
			{"values":["a",null,"b", null, null, "c"]}
			)JSON" };
		auto obj = json_dto::from_json< data_t >( json_data );

		REQUIRE( 6u == obj.m_values.size() );
		REQUIRE( obj.m_values[0].has_value() );
		REQUIRE( "a" == *(obj.m_values[0]) );
		REQUIRE_FALSE( obj.m_values[1].has_value() );
		REQUIRE( obj.m_values[2].has_value() );
		REQUIRE( "b" == *(obj.m_values[2]) );
		REQUIRE_FALSE( obj.m_values[3].has_value() );
		REQUIRE_FALSE( obj.m_values[4].has_value() );
		REQUIRE( obj.m_values[5].has_value() );
		REQUIRE( "c" == *(obj.m_values[5]) );
	}
}

TEST_CASE( "vector_of_nullable to_json" , "[basic]" )
{
	{
		data_t data;
		data.m_values.push_back( json_dto::nullable_t<std::string>{} );

		REQUIRE( json_dto::to_json(data) == R"JSON({"values":[null]})JSON" );
	}

	{
		data_t data;
		data.m_values.push_back( json_dto::nullable_t<std::string>{} );
		data.m_values.push_back( json_dto::nullable_t<std::string>{"a"} );
		data.m_values.push_back( json_dto::nullable_t<std::string>{} );
		data.m_values.push_back( json_dto::nullable_t<std::string>{} );
		data.m_values.push_back( json_dto::nullable_t<std::string>{"b"} );
		data.m_values.push_back( json_dto::nullable_t<std::string>{} );
		data.m_values.push_back( json_dto::nullable_t<std::string>{"c"} );
		data.m_values.push_back( json_dto::nullable_t<std::string>{"d"} );

		REQUIRE( json_dto::to_json(data)
				== R"JSON({"values":[null,"a",null,null,"b",null,"c","d"]})JSON" );
	}
}

