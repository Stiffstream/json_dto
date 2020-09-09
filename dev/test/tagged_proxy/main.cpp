#include <catch2/catch.hpp>

#include <iostream>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

namespace test
{

struct my_enum_image {};

}

namespace json_dto
{

template< typename T >
struct tagged_proxy_io_t< test::my_enum_image, T >
{
	static_assert( std::is_enum<T>::value, "T should be an enum type" );

	static void
	read_json_value(
		T & value,
		const rapidjson::Value & from )
	{
		using json_dto::read_json_value;

		using ut = std::underlying_type_t< T >;

		ut representation;
		read_json_value( representation, from );

		value = static_cast<T>(representation);
	}

	static void
	write_json_value(
		const T & value,
		rapidjson::Value & object,
		rapidjson::MemoryPoolAllocator<> & allocator )
	{
		using json_dto::write_json_value;

		using ut = std::underlying_type_t< T >;

		const ut representation{ static_cast<ut>(value) };

		write_json_value( representation, object, allocator );
	}
};

} /* namespace json_dto */

namespace test
{

enum class level_t : unsigned short
	{
		low,
		normal,
		high
	};

enum class category_t : int
	{
		ordinary = 10,
		notice = 25,
		warning = 100
	};

struct data_t
{
	level_t m_level;
	category_t m_category;

	template<typename IO>
	void json_io( IO & io )
	{
		io & json_dto::mandatory(
				"level",
				json_dto::tagged_proxy<my_enum_image>(m_level) )
			& json_dto::mandatory(
				"cat",
				json_dto::tagged_proxy<my_enum_image>(m_category) );
	}
};

} /* namespace test */

TEST_CASE( "read_json_value" , "read" )
{
	const std::string json_data{
		R"JSON(
		{
			"level": 1,
			"cat" : 25
		})JSON" };
	auto obj = json_dto::from_json< test::data_t >( json_data );

	REQUIRE( test::level_t::normal == obj.m_level );
	REQUIRE( test::category_t::notice == obj.m_category );
}

TEST_CASE( "write_json_value" , "write" )
{
	const test::data_t src{ test::level_t::high, test::category_t::warning };

	const auto r = json_dto::to_json( src );
	REQUIRE( R"({"level":2,"cat":100})" == r );
}

