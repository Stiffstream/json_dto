#include <catch2/catch.hpp>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

struct first_test
{
	const std::uint32_t m_version{ 42 };
	std::uint32_t m_payload;

	first_test( std::uint32_t payload ) : m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "version", m_version )
			& json_dto::mandatory( "payload", m_payload );
	}
};

struct second_test
{
	std::uint32_t version() const noexcept { return 42; }

	std::uint32_t m_payload;

	second_test( std::uint32_t payload ) : m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "version", version() )
			& json_dto::mandatory( "payload", m_payload );
	}
};

struct complex_id
{
	std::uint32_t m_a;
	std::uint32_t m_b;

	template< typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "a", m_a )
			& json_dto::mandatory( "b", m_b );
	}
};

struct third_test
{
	complex_id id() const noexcept { return { 1, 2 }; }

	std::uint32_t m_payload{};

	third_test() = default;
	third_test( std::uint32_t payload ) : m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "id", id() )
			& json_dto::mandatory( "payload", m_payload );
	}
};

struct fourth_test
{
	const std::vector< std::uint32_t > m_ids{ 1u, 2u, 3u, 4u };
	std::uint32_t m_payload{};

	fourth_test( std::uint32_t payload ) : m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "ids", m_ids )
			& json_dto::mandatory( "payload", m_payload );
	}
};

struct fifth_test
{
	std::vector< std::uint32_t > ids() const { return { 1u, 2u, 3u, 4u }; }
	std::uint32_t m_payload{};

	fifth_test( std::uint32_t payload ) : m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "ids", ids() )
			& json_dto::mandatory( "payload", m_payload );
	}
};

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
		std::sprintf( buf, "0x%x", v );

		write_json_value( json_dto::make_string_ref(buf), to, allocator );
	}
};

struct sixth_test
{
	std::vector< int > ids() const { return { 16, 32, 48 }; }
	std::uint32_t m_payload{};

	sixth_test( std::uint32_t payload ) : m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory(
					json_dto::apply_to_content_t< hex_writer_t >{},
					"ids", ids() )
			& json_dto::mandatory( "payload", m_payload );
	}
};

TEST_CASE( "write const field" , "[const-field]" )
{
	const first_test obj{ 55 };
	const auto str_image = json_dto::to_json( obj );

	REQUIRE( R"json({"version":42,"payload":55})json" == str_image );
}

TEST_CASE( "write result of a method" , "[const-method]" )
{
	const second_test obj{ 55 };
	const auto str_image = json_dto::to_json( obj );

	REQUIRE( R"json({"version":42,"payload":55})json" == str_image );
}

TEST_CASE( "write result of a method with complex type" , "[const-method]" )
{
	const third_test obj{ 55 };
	const auto str_image = json_dto::to_json( obj );

	REQUIRE( R"json({"id":{"a":1,"b":2},"payload":55})json" == str_image );
}

TEST_CASE( "write const std::vector field" , "[const-field]" )
{
	const fourth_test obj{ 55 };
	const auto str_image = json_dto::to_json( obj );

	REQUIRE( R"json({"ids":[1,2,3,4],"payload":55})json" == str_image );
}

TEST_CASE( "write result of a method with std::vector" , "[const-method]" )
{
	const fifth_test obj{ 55 };
	const auto str_image = json_dto::to_json( obj );

	REQUIRE( R"json({"ids":[1,2,3,4],"payload":55})json" == str_image );
}

TEST_CASE( "write result of a method with std::vector and hex_writer_t",
		"[const-method]" )
{
	const sixth_test obj{ 55 };
	const auto str_image = json_dto::to_json( obj );

	REQUIRE( R"json({"ids":["0x10","0x20","0x30"],"payload":55})json" == str_image );
}

