#include <catch2/catch.hpp>

#include <json_dto/pub.hpp>

#include <tuple>

#include <deque>
#include <list>
#include <forward_list>

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

	friend bool
	operator!=(const data_t & a, const data_t & b) noexcept
	{
		return std::tie(a.m_a, a.m_b) != std::tie(b.m_a, b.m_b);
	}

	friend bool
	operator<(const data_t & a, const data_t & b) noexcept
	{
		return a.m_a < b.m_a;
	}
};

template< typename C >
void
check_empty()
{
	C e1;

	const std::string json_data = json_dto::to_json(e1);

	auto e2 = json_dto::from_json< C >(json_data);

	REQUIRE( e1 == e2 );
}

template< typename C >
void
check_values( const C & cnt )
{
	const std::string json_data = json_dto::to_json(cnt);

	auto e2 = json_dto::from_json< C >(json_data);

	REQUIRE( cnt == e2 );
}

TEST_CASE( "deque-empty" )
{
	check_empty< std::deque<data_t> >();
}

TEST_CASE( "deque-values" )
{
	std::deque< data_t > e1{ {"a", 0}, {"b", 1}, {"c", 3} };

	check_values( e1 );
}

TEST_CASE( "list-empty" )
{
	check_empty< std::list<data_t> >();
}

TEST_CASE( "list-values" )
{
	std::list< data_t > e1{ {"a", 0}, {"b", 1}, {"c", 3} };

	check_values( e1 );
}

TEST_CASE( "forward_list-empty" )
{
	check_empty< std::forward_list<data_t> >();
}

TEST_CASE( "forward_list-values" )
{
	std::forward_list< data_t > e1{ {"a", 0}, {"b", 1}, {"c", 3} };

	check_values( e1 );
}

