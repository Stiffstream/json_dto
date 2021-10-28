#include <catch2/catch.hpp>

#include <iostream>
#include <limits>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

#include <test/helper.hpp>

namespace test_stuff
{

template< typename F >
struct serialize_only_proxy_t
{
	using field_type = const F;

	const F * m_field;
};

template< typename F >
serialize_only_proxy_t<F> serialize_only( const F & field ) noexcept
{
	return { &field };
}

} /* namespace test_stuff */

namespace json_dto
{

template<
		typename Reader_Writer,
		typename Field_Type,
		typename Manopt_Policy,
		typename Validator >
class binder_data_holder_t<
		Reader_Writer,
		const test_stuff::serialize_only_proxy_t<Field_Type>,
		Manopt_Policy,
		Validator >
	:	public binder_data_holder_t<
			Reader_Writer,
			typename test_stuff::serialize_only_proxy_t<Field_Type>::field_type,
			Manopt_Policy,
			Validator >
{
	using serialize_only_proxy =
			test_stuff::serialize_only_proxy_t<Field_Type>;

	using actual_field_type = typename serialize_only_proxy::field_type;

	using base_type = binder_data_holder_t<
			Reader_Writer,
			actual_field_type,
			Manopt_Policy,
			Validator >;

public:
	binder_data_holder_t(
		Reader_Writer && reader_writer,
		string_ref_t field_name,
		const serialize_only_proxy & proxy,
		Manopt_Policy && manopt_policy,
		Validator && validator )
		:	base_type{
				std::move(reader_writer),
				field_name,
				*(proxy.m_field),
				std::move(manopt_policy),
				std::move(validator)
			}
	{}
};

template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
struct binder_read_from_implementation_t<
			binder_data_holder_t<
				Reader_Writer,
				const test_stuff::serialize_only_proxy_t<Field_Type>,
				Manopt_Policy,
				Validator
			>
		>
{
	using data_holder_t = binder_data_holder_t<
			Reader_Writer,
			const test_stuff::serialize_only_proxy_t<Field_Type>,
			Manopt_Policy,
			Validator >;

	static void
	read_from(
		const data_holder_t & /*binder_data*/,
		const rapidjson::Value & /*object*/ )
	{
		// Nothing to do.
	}

};

} /* namespace json_dto */

using namespace json_dto;

template < typename T >
bool
eq( const T & left, const T & right )
{
	return left == right;
}

template < typename T >
bool
eq( const nullable_t< T > & left, const nullable_t< T > & right )
{
	if( !left && !right )
		return true;

	return left && right && eq( *left, *right );
}

//
// hex_writer_t
//
struct hex_writer_t
{
	void write(
		int v, 
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		using json_dto::write_json_value;

		char buf[32];
		std::sprintf( buf, "%x", v );

		write_json_value( json_dto::make_string_ref(buf), to, allocator );
	}
};

struct nullable_vector_of_ints_t
{
	json_dto::nullable_t< std::vector<int> > m_values;

	nullable_vector_of_ints_t() = default;
	nullable_vector_of_ints_t( std::vector<int> values )
		:	m_values{ std::move(values) }
	{}

	template< typename Io >
	void json_io( Io & io )
	{
		io & json_dto::mandatory(
				json_dto::apply_to_content_t<
					json_dto::apply_to_content_t<hex_writer_t> >{},
				"values",
				test_stuff::serialize_only( m_values ) );
	}
};

TEST_CASE("nullable vector with custom hex_writer",
		"[vector][nullable][hex_writer]")
{
	SECTION("null vector")
	{
		const std::string json_str =
				R"JSON({"values":null})JSON";

		nullable_vector_of_ints_t dto;

		REQUIRE( json_str == to_json( dto ) );
	}

	SECTION("empty vector")
	{
		const std::string json_str =
				R"JSON({"values":[]})JSON";

		nullable_vector_of_ints_t dto{
				std::vector<int>{}
		};

		REQUIRE( json_str == to_json( dto ) );
	}

	SECTION("not-empty vector")
	{
		const std::string json_str =
				R"JSON({"values":["0","1","a","f","10","20"]})JSON";

		nullable_vector_of_ints_t dto{
				std::vector<int>{0, 1, 10, 15, 16, 32}
		};

		REQUIRE( json_str == to_json( dto ) );

		auto deserialized_dto = from_json<nullable_vector_of_ints_t>( json_str );
		REQUIRE( !deserialized_dto.m_values );
	}
}

struct maybe_null_vector_of_ints_t
{
	std::vector<int> m_values;

	maybe_null_vector_of_ints_t() = default;
	maybe_null_vector_of_ints_t( std::vector<int> values )
		:	m_values{ std::move(values) }
	{}

	template< typename Io >
	void json_io( Io & io )
	{
		io & json_dto::mandatory_maybe_null(
				json_dto::apply_to_content_t<
					json_dto::apply_to_content_t<hex_writer_t> >{},
				"values",
				test_stuff::serialize_only( m_values ) );
	}
};

TEST_CASE("maybe_null vector with custom hex_writer",
		"[vector][maybe_null][hex_writer]")
{
	SECTION("empty vector")
	{
		const std::string json_str =
				R"JSON({"values":[]})JSON";

		maybe_null_vector_of_ints_t dto{
				std::vector<int>{}
		};

		REQUIRE( json_str == to_json( dto ) );
	}

	SECTION("not-empty vector")
	{
		const std::string json_str =
				R"JSON({"values":["0","1","a","f","10","20"]})JSON";

		maybe_null_vector_of_ints_t dto{
				std::vector<int>{0, 1, 10, 15, 16, 32}
		};

		REQUIRE( json_str == to_json( dto ) );

#if 0
		auto deserialized_dto = from_json<maybe_null_vector_of_ints_t>( json_str );
		REQUIRE( deserialized_dto.m_values.empty() );
#endif
	}
}

