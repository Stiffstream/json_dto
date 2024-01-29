#include <catch2/catch.hpp>

#include <iostream>
#include <limits>
#include <tuple>
#include <type_traits>

#include <json_dto/pub.hpp>
#include <json_dto/validators.hpp>

namespace test
{

struct simple_nested_t
{
	int m_a{};
	std::string m_b{};
	unsigned int m_c{};
};

struct simple_outer_t
{
	simple_nested_t m_x;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer(
						json_dto::inside_array::member( m_x.m_a ),
						json_dto::inside_array::member( m_x.m_b ),
						json_dto::inside_array::member( m_x.m_c ) ),
					"x", m_x );
	}
};

struct simple_int_reader_writter_t
{
	void read( int & v, const rapidjson::Value & object ) const
	{
		json_dto::read_json_value( v, object );
	}

	void write(
		const int & v,
		rapidjson::Value & object,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		json_dto::write_json_value( v, object, allocator );
	}
};

struct outer_with_custom_reader_writer_t
{
	simple_nested_t m_x;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer(
						json_dto::inside_array::member(
							simple_int_reader_writter_t{}, m_x.m_a ),
						json_dto::inside_array::member( m_x.m_b ),
						json_dto::inside_array::member( m_x.m_c ) ),
					"x", m_x );
	}
};

struct tuple_holder_t
{
	std::tuple<int, int, std::string, int> m_x;

	tuple_holder_t()
		: m_x{ 0, 1, "zero", 2 }
	{}

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer(
						json_dto::inside_array::member( std::get<0>(m_x) ),
						json_dto::inside_array::member(
							simple_int_reader_writter_t{}, std::get<1>(m_x) ),
						json_dto::inside_array::member( std::get<2>(m_x) ),
						json_dto::inside_array::member( std::get<3>(m_x) )
					), "x", m_x );
	}
};

struct at_least_checker_zero_t
{
	int m_x1{};
	int m_x2{};
	int m_x3{};
	int m_x4{};

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer<
							json_dto::inside_array::at_least<0> >(
						json_dto::inside_array::member( m_x1 ),
						json_dto::inside_array::member( m_x2 ),
						json_dto::inside_array::member( m_x3 ),
						json_dto::inside_array::member( m_x4 ) ),
					"x", *this );
	}
};

struct at_least_checker_two_t
{
	int m_x1{};
	int m_x2{};
	int m_x3{};
	int m_x4{};

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer<
							json_dto::inside_array::at_least<2> >(
						json_dto::inside_array::member( m_x1 ),
						json_dto::inside_array::member( m_x2 ),
						json_dto::inside_array::member( m_x3 ),
						json_dto::inside_array::member( m_x4 ) ),
					"x", *this );
	}
};

struct copyable_but_not_movable_t
{
	int m_v{};

	copyable_but_not_movable_t() = default;
	copyable_but_not_movable_t(int v) : m_v{v} {}

	copyable_but_not_movable_t(
		const copyable_but_not_movable_t &) = default;
	copyable_but_not_movable_t &
	operator=(
		const copyable_but_not_movable_t &) = default;

	copyable_but_not_movable_t(
		copyable_but_not_movable_t &&) = delete;
	copyable_but_not_movable_t &
	operator=(
		copyable_but_not_movable_t &&) = delete;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "v", m_v );
	}
};

struct moveable_with_move_flag_t
{
	int m_v{};
	bool m_move_operator_used{ false };

	moveable_with_move_flag_t() = default;
	moveable_with_move_flag_t(int v) : m_v{v} {}

	moveable_with_move_flag_t(
		const moveable_with_move_flag_t & other)
		:	m_v{ other.m_v }
		,	m_move_operator_used{ false }
	{}

	moveable_with_move_flag_t &
	operator=(
		const moveable_with_move_flag_t & other)
	{
		m_v = other.m_v;
		m_move_operator_used = false;

		return *this;
	}

	moveable_with_move_flag_t(
		moveable_with_move_flag_t && other) noexcept
		:	m_v{ other.m_v }
		,	m_move_operator_used{ false }
	{}

	moveable_with_move_flag_t &
	operator=(
		moveable_with_move_flag_t && other) noexcept
	{
		m_v = other.m_v;
		m_move_operator_used = true;

		return *this;
	}

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "v", m_v );
	}
};

struct at_least_checker_two_with_defaults_1_t
{
	int m_x1{};
	int m_x2{};
	copyable_but_not_movable_t m_x3;
	copyable_but_not_movable_t m_x4;

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		const copyable_but_not_movable_t x3_default( 45 );
		// Should be treated as const reference to the default value.
		copyable_but_not_movable_t x4_default( 56 );

		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer<
							json_dto::inside_array::at_least<2> >(
						json_dto::inside_array::member( m_x1 ),
						json_dto::inside_array::member( m_x2 ),
						json_dto::inside_array::member_with_default_value( m_x3, x3_default ),
						json_dto::inside_array::member_with_default_value( m_x4, x4_default ) ),
					"x", *this );
	}
};

struct at_least_checker_two_with_defaults_2_t
{
	struct custom_reader_writer_t
		:	public json_dto::default_reader_writer_t
	{
	};

	int m_x1{};
	int m_x2{};
	copyable_but_not_movable_t m_x3;
	copyable_but_not_movable_t m_x4;

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		const copyable_but_not_movable_t x3_default( 45 );
		// Should be treated as const reference to the default value.
		copyable_but_not_movable_t x4_default( 56 );

		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer<
							json_dto::inside_array::at_least<2> >(
						json_dto::inside_array::member( m_x1 ),
						json_dto::inside_array::member( m_x2 ),
						json_dto::inside_array::member_with_default_value(
								custom_reader_writer_t{}, m_x3, x3_default ),
						json_dto::inside_array::member_with_default_value(
								custom_reader_writer_t{}, m_x4, x4_default ) ),
					"x", *this );
	}
};

struct at_least_checker_two_with_defaults_3_t
{
	int m_x1{};
	int m_x2{};
	moveable_with_move_flag_t m_x3;
	moveable_with_move_flag_t m_x4;

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer<
							json_dto::inside_array::at_least<2> >(
						json_dto::inside_array::member( m_x1 ),
						json_dto::inside_array::member( m_x2 ),
						json_dto::inside_array::member_with_default_value( m_x3,
								moveable_with_move_flag_t{ 45 } ),
						json_dto::inside_array::member_with_default_value( m_x4,
								moveable_with_move_flag_t{ 56 } ) ),
					"x", *this );
	}
};

struct at_least_checker_two_with_defaults_4_t
{
	struct custom_reader_writer_t
		:	public json_dto::default_reader_writer_t
	{
	};

	int m_x1{};
	int m_x2{};
	moveable_with_move_flag_t m_x3;
	moveable_with_move_flag_t m_x4;

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer<
							json_dto::inside_array::at_least<2> >(
						json_dto::inside_array::member( m_x1 ),
						json_dto::inside_array::member( m_x2 ),
						json_dto::inside_array::member_with_default_value(
								custom_reader_writer_t{},
								m_x3,
								moveable_with_move_flag_t{ 45 } ),
						json_dto::inside_array::member_with_default_value(
								custom_reader_writer_t{},
								m_x4,
								moveable_with_move_flag_t{ 56 } ) ),
					"x", *this );
	}
};

struct constrained_values_t
{
	int m_a{};
	int m_b{2};

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::inside_array::reader_writer(
						json_dto::inside_array::member( m_a,
							json_dto::min_max_constraint( -10, 10 ) ),
						json_dto::inside_array::member( m_b,
							json_dto::one_of_constraint( {0, 2, 4, 6, 8, 10} ) ) ),
					"x", *this );
	}
};

} /* namespace test */

using namespace test;

TEST_CASE( "inside-array-simple" , "[inside-array][no-reader-writer]" )
{
	{
		const char * json_str =
			R"({
				"x":[ 1, "two", 55555]
			})";

		const auto r = json_dto::from_json<simple_outer_t>( json_str );

		REQUIRE( 1 == r.m_x.m_a );
		REQUIRE( "two" == r.m_x.m_b );
		REQUIRE( 55555 == r.m_x.m_c );

		const auto str = json_dto::to_json( r );
		REQUIRE( R"json({"x":[1,"two",55555]})json" == str );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, "two", 55555, null]
			})";

		simple_outer_t r;
		REQUIRE_THROWS( json_dto::from_json( json_str, r ) );
	}
}

TEST_CASE( "inside-array-with-custom-reader-writer" , "[inside-array][reader-writer]" )
{
	const char * json_str =
		R"({
			"x":[ 54, "five", 24678 ]
		})";

	auto r = json_dto::from_json<outer_with_custom_reader_writer_t>( json_str );

	REQUIRE( 54 == r.m_x.m_a );
	REQUIRE( "five" == r.m_x.m_b );
	REQUIRE( 24678 == r.m_x.m_c );

	r.m_x.m_a = -333;
	r.m_x.m_b = "nullptr";
	r.m_x.m_c = 0;

	const auto str = json_dto::to_json( r );
	REQUIRE( R"json({"x":[-333,"nullptr",0]})json" == str );
}

TEST_CASE( "inside-array-tuple-custom-reader-writer" , "[inside-array][tuple][reader-writer]" )
{
	const char * json_str =
		R"({
			"x":[ 54, -1, "five", 24678 ]
		})";

	auto r = json_dto::from_json<tuple_holder_t>( json_str );

	REQUIRE( std::make_tuple(54, -1, std::string{"five"}, 24678) == r.m_x );

	r.m_x = std::make_tuple(-2, 27, std::string{"nullptr"}, 0);

	const auto str = json_dto::to_json( r );
	REQUIRE( R"json({"x":[-2,27,"nullptr",0]})json" == str );
}

TEST_CASE( "inside-array-at-least-limit-two" , "[inside-array][at-least][reader-writer]" )
{
	{
		const char * json_str =
			R"({
				"x":[ 1, 2, 3, 4 ]
			})";

		auto r = json_dto::from_json<at_least_checker_two_t>( json_str );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 3 == r.m_x3 );
		REQUIRE( 4 == r.m_x4 );
	}

	{
		const char * json_str =
			R"({
				"x":[ 11, 12, 13, 14, 15, 16, 17, 18 ]
			})";

		at_least_checker_two_t r;
		REQUIRE_THROWS( json_dto::from_json( json_str, r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2 ]
			})";

		at_least_checker_two_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3 = 35;
		r.m_x4 = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 0 == r.m_x3 );
		REQUIRE( 0 == r.m_x4 );

		REQUIRE( R"json({"x":[1,2,0,0]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[1]
			})";

		REQUIRE_THROWS( json_dto::from_json<at_least_checker_two_t>( json_str ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2, {"v":3}, {"v":4} ]
			})";

		at_least_checker_two_with_defaults_1_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3.m_v = 35;
		r.m_x4.m_v = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 3 == r.m_x3.m_v );
		REQUIRE( 4 == r.m_x4.m_v );

		REQUIRE( R"json({"x":[1,2,{"v":3},{"v":4}]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2 ]
			})";

		at_least_checker_two_with_defaults_1_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3.m_v = 35;
		r.m_x4.m_v = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 45 == r.m_x3.m_v );
		REQUIRE( 56 == r.m_x4.m_v );

		REQUIRE( R"json({"x":[1,2,{"v":45},{"v":56}]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2, {"v":3}, {"v":4} ]
			})";

		at_least_checker_two_with_defaults_2_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3.m_v = 35;
		r.m_x4.m_v = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 3 == r.m_x3.m_v );
		REQUIRE( 4 == r.m_x4.m_v );

		REQUIRE( R"json({"x":[1,2,{"v":3},{"v":4}]})json" == json_dto::to_json( r ) );
	}
	{
		const char * json_str =
			R"({
				"x":[ 1, 2 ]
			})";

		at_least_checker_two_with_defaults_2_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3.m_v = 35;
		r.m_x4.m_v = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 45 == r.m_x3.m_v );
		REQUIRE( 56 == r.m_x4.m_v );

		REQUIRE( R"json({"x":[1,2,{"v":45},{"v":56}]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2, {"v":3}, {"v":4} ]
			})";

		at_least_checker_two_with_defaults_3_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3.m_v = 35;
		r.m_x4.m_v = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 3 == r.m_x3.m_v );
		REQUIRE_FALSE( r.m_x3.m_move_operator_used );
		REQUIRE( 4 == r.m_x4.m_v );
		REQUIRE_FALSE( r.m_x4.m_move_operator_used );

		REQUIRE( R"json({"x":[1,2,{"v":3},{"v":4}]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2 ]
			})";

		at_least_checker_two_with_defaults_3_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3.m_v = 35;
		r.m_x4.m_v = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 45 == r.m_x3.m_v );
		REQUIRE( r.m_x3.m_move_operator_used );
		REQUIRE( 56 == r.m_x4.m_v );
		REQUIRE( r.m_x4.m_move_operator_used );

		REQUIRE( R"json({"x":[1,2,{"v":45},{"v":56}]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2, {"v":3}, {"v":4} ]
			})";

		at_least_checker_two_with_defaults_4_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3.m_v = 35;
		r.m_x4.m_v = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 3 == r.m_x3.m_v );
		REQUIRE_FALSE( r.m_x3.m_move_operator_used );
		REQUIRE( 4 == r.m_x4.m_v );
		REQUIRE_FALSE( r.m_x4.m_move_operator_used );

		REQUIRE( R"json({"x":[1,2,{"v":3},{"v":4}]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2 ]
			})";

		at_least_checker_two_with_defaults_4_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3.m_v = 35;
		r.m_x4.m_v = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 45 == r.m_x3.m_v );
		REQUIRE( r.m_x3.m_move_operator_used );
		REQUIRE( 56 == r.m_x4.m_v );
		REQUIRE( r.m_x4.m_move_operator_used );

		REQUIRE( R"json({"x":[1,2,{"v":45},{"v":56}]})json" == json_dto::to_json( r ) );
	}
}

TEST_CASE( "inside-array-at-least-limit-zero" , "[inside-array][at-least][reader-writer]" )
{
	{
		const char * json_str =
			R"({
				"x":[ 1, 2, 3, 4 ]
			})";

		auto r = json_dto::from_json<at_least_checker_zero_t>( json_str );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 3 == r.m_x3 );
		REQUIRE( 4 == r.m_x4 );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 2 ]
			})";

		at_least_checker_zero_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3 = 35;
		r.m_x4 = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 2 == r.m_x2 );
		REQUIRE( 0 == r.m_x3 );
		REQUIRE( 0 == r.m_x4 );

		REQUIRE( R"json({"x":[1,2,0,0]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1 ]
			})";

		at_least_checker_zero_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3 = 35;
		r.m_x4 = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 1 == r.m_x1 );
		REQUIRE( 0 == r.m_x2 );
		REQUIRE( 0 == r.m_x3 );
		REQUIRE( 0 == r.m_x4 );

		REQUIRE( R"json({"x":[1,0,0,0]})json" == json_dto::to_json( r ) );
	}

	{
		const char * json_str =
			R"({
				"x":[]
			})";

		at_least_checker_zero_t r;
		r.m_x1 = 33;
		r.m_x2 = 34;
		r.m_x3 = 35;
		r.m_x4 = 36;

		json_dto::from_json( json_str, r );

		REQUIRE( 0 == r.m_x1 );
		REQUIRE( 0 == r.m_x2 );
		REQUIRE( 0 == r.m_x3 );
		REQUIRE( 0 == r.m_x4 );

		REQUIRE( R"json({"x":[0,0,0,0]})json" == json_dto::to_json( r ) );
	}
}

TEST_CASE( "inside-array-validators" , "[inside-array][validators][reader-writer]" )
{
	{
		const char * json_str =
			R"({
				"x":[ 1, 8 ]
			})";

		auto r = json_dto::from_json<constrained_values_t>( json_str );

		REQUIRE( 1 == r.m_a );
		REQUIRE( 8 == r.m_b );
	}

	{
		const char * json_str =
			R"({
				"x":[ -11, 2 ]
			})";

		REQUIRE_THROWS( json_dto::from_json<constrained_values_t>( json_str ) );
	}

	{
		const char * json_str =
			R"({
				"x":[ 1, 7 ]
			})";

		REQUIRE_THROWS( json_dto::from_json<constrained_values_t>( json_str ) );
	}

	{
		constrained_values_t v;
		v.m_a = 11;

		REQUIRE_THROWS( json_dto::to_json( v ) );
	}

	{
		constrained_values_t v;
		v.m_b = 11;

		REQUIRE_THROWS( json_dto::to_json( v ) );
	}

	{
		constrained_values_t v;
		v.m_a = -3;
		v.m_b = 6;

		REQUIRE( R"json({"x":[-3,6]})json" == json_dto::to_json( v ) );
	}
}

