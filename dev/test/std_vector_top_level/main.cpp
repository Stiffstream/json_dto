#include <catch2/catch.hpp>

#include <json_dto/pub.hpp>

#include <tuple>

struct data_t
{
	std::string m_a;
	int m_b;

	template<typename I>
	void json_io(I & io)
	{
		io & json_dto::mandatory("a", m_a)
			& json_dto::mandatory("b", m_b);
	}

	friend bool
	operator==(const data_t & a, const data_t & b) noexcept
	{
		return std::tie(a.m_a, a.m_b) == std::tie(b.m_a, b.m_b);
	}
};

TEST_CASE( "empty vectors" )
{
	std::vector< data_t > e1;

	const std::string json_data = json_dto::to_json(e1);
	REQUIRE( "[]" == json_data );

	auto e2 = json_dto::from_json< std::vector<data_t> >(json_data);
	REQUIRE( e1 == e2 );
}

TEST_CASE( "non-empty vectors" )
{
	std::vector< data_t > e1{ {"a", 0}, {"b", 1}, {"c", 3} };

	const std::string json_data = json_dto::to_json(e1);
	REQUIRE( R"JSON([{"a":"a","b":0},{"a":"b","b":1},{"a":"c","b":3}])JSON"
			== json_data );

	auto e2 = json_dto::from_json< std::vector<data_t> >(json_data);
	REQUIRE( e1 == e2 );
}

