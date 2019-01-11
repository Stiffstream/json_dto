#include <catch2/catch.hpp>

#include <iostream>
#include <limits>
#include <stdexcept>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>
#include <json_dto/validators.hpp>

#include <test/helper.hpp>

using namespace json_dto;


namespace mandatory_fields
{

// Numeric fields dto.
struct nums_t
{
	std::int16_t m_num_int16;
	std::int32_t m_num_int32;
	std::int64_t m_num_int64;

	nums_t()
	{}

	nums_t(
		std::int16_t num_int16,
		std::int32_t num_int32,
		std::int64_t num_int64 )
		:	m_num_int16{ num_int16 }
		,	m_num_int32{ num_int32 }
		,	m_num_int64{ num_int64 }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& mandatory(
				"num_int16",
				m_num_int16,
				min_max_constraint< decltype( m_num_int16 ) >( -10, 10 ) )
			& mandatory(
				"num_int32",
				m_num_int32,
				one_of_constraint< decltype( m_num_int32 ) >( { -10, -5, 0, 5, 10 } ) )
			& mandatory(
				"num_int64",
				m_num_int64,
				[]( const auto & f ){
					if( -10 > f || 10 < f )
						throw std::runtime_error{
							"value " + std::to_string( f ) +
							" is invalid" };
				} )
			;
	}
};

TEST_CASE( "num-mand-valid" , "[valid]" )
{
	SECTION( "read" )
	{
		const std::string json_str =
			R"JSON({
				"num_int16":5,
				"num_int32":-5,
				"num_int64":-10
			})JSON";

		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 5 );
		REQUIRE( dto.m_num_int32 == -5 );
		REQUIRE( dto.m_num_int64 == -10 );
	}

	SECTION( "write" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_int16":5,
					"num_int32":-5,
					"num_int64":-10
				})JSON" );

		REQUIRE( json_str == to_json( nums_t{ 5, -5, -10 } ) );
	}
}

TEST_CASE( "num-mand-invalid" , "[invalid]" )
{
	SECTION( "num_int16 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
				"num_int16":-11,
				"num_int32":-5,
				"num_int64":-10
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int16 write" )
	{
		try
		{
			const std::string json_str =
				R"JSON({"num_int16":55,"num_int32":-5,"num_int64":-10})JSON";
			const bool r = json_str != to_json( nums_t{ 55, -5, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
				"num_int16":10,
				"num_int32":-3,
				"num_int64":-10
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 write" )
	{
		try
		{
			const std::string json_str =
				R"JSON({"num_int16":5,"num_int32":1,"num_int64":-10})JSON";
			const bool r = json_str != to_json( nums_t{ 5, 1, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
				"num_int16":9,
				"num_int32":-10,
				"num_int64":2016
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 write" )
	{
		try
		{
			const std::string json_str =
				R"JSON({"num_int16":5,"num_int32":0,"num_int64":999})JSON";
			const bool r = json_str != to_json( nums_t{ 5, 0, 999 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

// String fields dto.
struct strings_t
{
	std::string m_s1;
	std::string m_s2;

	strings_t()
	{}

	strings_t(
		std::string s1,
		std::string s2 )
		:	m_s1{ std::move( s1 ) }
		,	m_s2{ std::move( s2 ) }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& mandatory(
				"s1",
				m_s1,
				one_of_constraint< decltype( m_s1 ) >(
					{
						"aaa",
						"bbb",
						"ccc",
						"ddd"
					} ) )
			& mandatory(
				"s2",
				m_s2,
				[]( const auto & f ){
					if( 5 < f.size() )
						throw std::runtime_error{
							"s2 string size must be <=5" };
				} )
			;
	}
};

TEST_CASE( "str-mand-valid" , "[valid]" )
{
	SECTION( "read" )
	{
		const std::string json_str =
			R"JSON({
				"s1":"aaa",
				"s2":"12345"
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 == "aaa" );
		REQUIRE( dto.m_s2 == "12345" );
	}

	SECTION( "write" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":"ccc",
					"s2":"abc"
				})JSON" );

		REQUIRE( json_str == to_json( strings_t{ "ccc", "abc" } ) );
	}
}

TEST_CASE( "str-mand-invalid" , "[invalid]" )
{
	SECTION( "s1 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":"abc",
					"s2":""
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s1 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"abc",
						"s2":"abc"
					})JSON" );

			const bool r = json_str != to_json( strings_t{ "abc", "abc" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":"ccc",
					"s2":"012345"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"bbb",
						"s2":"321321321321"
					})JSON" );
			const bool r = json_str != to_json( strings_t{ "bbb", "321321321321" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

} /* namespace mandatory_fields */

namespace optional_no_default_fields
{

// Numeric optional fields dto.
struct nums_t
{
	std::int16_t m_num_int16{};
	std::int32_t m_num_int32{};
	std::int64_t m_num_int64{};

	nums_t()
	{}

	nums_t(
		std::int16_t num_int16,
		std::int32_t num_int32,
		std::int64_t num_int64 )
		:	m_num_int16{ num_int16 }
		,	m_num_int32{ num_int32 }
		,	m_num_int64{ num_int64 }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional_no_default(
				"num_int16",
				m_num_int16,
				min_max_constraint< decltype( m_num_int16 ) >( -10, 10 ) )
			& optional_no_default(
				"num_int32",
				m_num_int32,
				one_of_constraint< decltype( m_num_int32 ) >( { -10, -5, 0, 5, 10 } ) )
			& optional_no_default(
				"num_int64",
				m_num_int64,
				[]( const auto & f ){
					if( -10 > f || 10 < f )
						throw std::runtime_error{
							"value " + std::to_string( f ) +
							" is invalid" };
				} )
			;
	}
};

TEST_CASE( "num-opt-no-default-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str = "{}";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 0 );
		REQUIRE( dto.m_num_int32 == 0 );
		REQUIRE( dto.m_num_int64 == 0 );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"num_int16":5
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 5 );
		REQUIRE( dto.m_num_int32 == 0 );
		REQUIRE( dto.m_num_int64 == 0 );
	}

	SECTION( "read 3" )
	{
		const std::string json_str =
			R"JSON({
				"num_int32":-5
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 0 );
		REQUIRE( dto.m_num_int32 == -5 );
		REQUIRE( dto.m_num_int64 == 0 );
	}

	SECTION( "read 4" )
	{
		const std::string json_str =
			R"JSON({
				"num_int64":-10
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 0 );
		REQUIRE( dto.m_num_int32 == 0 );
		REQUIRE( dto.m_num_int64 == -10 );
	}

	SECTION( "write" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_int16":0,
					"num_int32":0,
					"num_int64":0
				})JSON" );
		REQUIRE( json_str == to_json( nums_t{} ) );
	}
}

TEST_CASE( "num-opt-no-default-invalid" , "[invalid]" )
{
	SECTION( "num_int16 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":-11,
					"num_int64":-10
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int16 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":55,
						"num_int32":-5,
						"num_int64":-10
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 55, -5, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":10,
					"num_int32":-3
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":1,
						"num_int64":-10
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 5, 1, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int32":-10,
					"num_int64":2016
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":0,
						"num_int64":999
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 5, 0, 999 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}


// String fields dto.
struct strings_t
{
	std::string m_s1;
	std::string m_s2;

	strings_t()
	{}

	strings_t(
		std::string s1,
		std::string s2 )
		:	m_s1{ std::move( s1 ) }
		,	m_s2{ std::move( s2 ) }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional_no_default(
				"s1",
				m_s1,
				one_of_constraint< decltype( m_s1 ) >(
					{
						"",
						"aaa",
						"bbb",
						"ccc",
						"ddd"
					} ) )
			& optional_no_default(
				"s2",
				m_s2,
				[]( const auto & f ){
					if( 5 < f.size() )
						throw std::runtime_error{
							"s2 string size must be <=5" };
				} )
			;
	}
};

TEST_CASE( "str-opt-no-default-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str = "{}";

		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 == "" );
		REQUIRE( dto.m_s2 == "" );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"s1":"aaa"
				})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 == "aaa" );
		REQUIRE( dto.m_s2 == "" );
	}

	SECTION( "read 3" )
	{
		const std::string json_str =
			R"JSON({
				"s2":"12345"
				})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 == "" );
		REQUIRE( dto.m_s2 == "12345" );
	}

	SECTION( "write" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":"ccc",
					"s2":"abc"
				})JSON" );

		REQUIRE( json_str == to_json( strings_t{ "ccc", "abc" } ) );
	}
}

TEST_CASE( "str-opt-no-default-invalid" , "[invalid]" )
{
	SECTION( "s1 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":"abc"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s1 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"abc",
						"s2":"abc"
					})JSON" );
			const bool r = json_str != to_json( strings_t{ "abc", "abc" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s2":"012345"
					})JSON" );
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":"bbb",
					"s2":"321321321321"
				})JSON";
			const bool r = json_str != to_json( strings_t{ "bbb", "321321321321" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

} /* namespace optional_no_default_fields */

namespace optional_fields
{

// Numeric optional fields dto.
struct nums_t
{
	std::int16_t m_num_int16{ -10 };
	std::int32_t m_num_int32{ -10 };
	std::int64_t m_num_int64{ -10 };

	nums_t()
	{}

	nums_t(
		std::int16_t num_int16,
		std::int32_t num_int32,
		std::int64_t num_int64 )
		:	m_num_int16{ num_int16 }
		,	m_num_int32{ num_int32 }
		,	m_num_int64{ num_int64 }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional(
				"num_int16",
				m_num_int16,
				std::int16_t{ 0 },
				min_max_constraint< decltype( m_num_int16 ) >( -10, 10 ) )
			& optional(
				"num_int32",
				m_num_int32,
				0,
				one_of_constraint< decltype( m_num_int32 ) >( { -10, -5, 0, 5, 10 } ) )
			& optional(
				"num_int64",
				m_num_int64,
				0LL,
				[]( const auto & f ){
					if( -10 > f || 10 < f )
						throw std::runtime_error{
							"value " + std::to_string( f ) +
							" is invalid" };
				} )
			;
	}
};

TEST_CASE( "num-opt-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str = "{}";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 0 );
		REQUIRE( dto.m_num_int32 == 0 );
		REQUIRE( dto.m_num_int64 == 0 );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"num_int16":5
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 5 );
		REQUIRE( dto.m_num_int32 == 0 );
		REQUIRE( dto.m_num_int64 == 0 );
	}

	SECTION( "read 3" )
	{
		const std::string json_str =
			R"JSON({
				"num_int32":-5
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 0 );
		REQUIRE( dto.m_num_int32 == -5 );
		REQUIRE( dto.m_num_int64 == 0 );
	}

	SECTION( "read 4" )
	{
		const std::string json_str =
			R"JSON({
				"num_int64":-10
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 == 0 );
		REQUIRE( dto.m_num_int32 == 0 );
		REQUIRE( dto.m_num_int64 == -10 );
	}

	SECTION( "write 1" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_int16":-10,
					"num_int32":-10,
					"num_int64":-10
				})JSON" );
		REQUIRE( json_str == to_json( nums_t{} ) );
	}

	SECTION( "write 2" )
	{
		const std::string json_str ="{}";

		REQUIRE( json_str == to_json( nums_t{0, 0, 0 } ) );
	}
}

TEST_CASE( "num-opt-invalid" , "[invalid]" )
{
	SECTION( "num_int16 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":-11,
					"num_int64":-10
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int16 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":55,
						"num_int32":-5,
						"num_int64":-10
					})JSON" );

			const bool r = json_str != to_json( nums_t{ 55, -5, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":10,
					"num_int32":-3
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":1,
						"num_int64":-10
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 5, 1, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int32":-10,
					"num_int64":2016
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":0,
						"num_int64":999
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 5, 0, 999 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}


// String fields dto.
struct strings_t
{
	std::string m_s1;
	std::string m_s2;

	strings_t()
	{}

	strings_t(
		std::string s1,
		std::string s2 )
		:	m_s1{ std::move( s1 ) }
		,	m_s2{ std::move( s2 ) }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional(
				"s1",
				m_s1,
				"ddd",
				one_of_constraint< decltype( m_s1 ) >(
					{
						"",
						"aaa",
						"bbb",
						"ccc",
						"ddd"
					} ) )
			& optional(
				"s2",
				m_s2,
				"123",
				[]( const auto & f ){
					if( 5 < f.size() )
						throw std::runtime_error{
							"s2 string size must be <=5" };
				} )
			;
	}
};

TEST_CASE( "str-opt-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str = "{}";

		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 == "ddd" );
		REQUIRE( dto.m_s2 == "123" );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"s1":"aaa"
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 == "aaa" );
		REQUIRE( dto.m_s2 == "123" );
	}

	SECTION( "read 3" )
	{
		const std::string json_str =
			R"JSON({
				"s2":"12345"
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 == "ddd" );
		REQUIRE( dto.m_s2 == "12345" );
	}

	SECTION( "write" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":"ccc",
					"s2":"abc"
				})JSON" );

		REQUIRE( json_str == to_json( strings_t{ "ccc", "abc" } ) );
	}
}

TEST_CASE( "str-opt-invalid" , "[invalid]" )
{
	SECTION( "s1 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":"abc"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s1 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"abc",
						"s2":"abc"
					})JSON" );
			const bool r = json_str != to_json( strings_t{ "abc", "abc" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s2":"012345"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"bbb",
						"s2":"321321321321"
					})JSON" );
			const bool r = json_str != to_json( strings_t{ "bbb", "321321321321" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

} /* namespace optional_fields */

namespace mandatory_nullable_fields
{

// Numeric fields dto.
struct nums_t
{
	nullable_t< std::int16_t > m_num_int16;
	nullable_t< std::int32_t > m_num_int32;
	nullable_t< std::int64_t > m_num_int64;

	nums_t()
	{}

	nums_t(
		std::int16_t num_int16,
		std::int32_t num_int32,
		std::int64_t num_int64 )
		:	m_num_int16{ num_int16 }
		,	m_num_int32{ num_int32 }
		,	m_num_int64{ num_int64 }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& mandatory(
				"num_int16",
				m_num_int16,
				min_max_constraint< std::int16_t >( -10, 10 ) )
			& mandatory(
				"num_int32",
				m_num_int32,
				one_of_constraint< std::int32_t >( { -10, -5, 0, 5, 10 } ) )
			& mandatory(
				"num_int64",
				m_num_int64,
				[]( const auto & f ){
					if( f &&
						(-10 > *f || 10 < *f ) )
						throw std::runtime_error{
							"value " + std::to_string( f ) +
							" is invalid" };
				} )
			;
	}
};

TEST_CASE( "num-nullable-mand-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str =
			R"JSON({
				"num_int16":5,
				"num_int32":-5,
				"num_int64":-10
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 );
		REQUIRE( dto.m_num_int32 );
		REQUIRE( dto.m_num_int64 );

		REQUIRE( *dto.m_num_int16 == 5 );
		REQUIRE( *dto.m_num_int32 == -5 );
		REQUIRE( *dto.m_num_int64 == -10 );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"num_int16":null,
				"num_int32":null,
				"num_int64":null
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE_FALSE( dto.m_num_int16 );
		REQUIRE_FALSE( dto.m_num_int32 );
		REQUIRE_FALSE( dto.m_num_int64 );
	}

	SECTION( "write 1" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_int16":5,
					"num_int32":-5,
					"num_int64":-10
				})JSON" );

		REQUIRE( json_str == to_json( nums_t{ 5, -5, -10 } ) );
	}

	SECTION( "write 2" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_int16":null,
					"num_int32":null,
					"num_int64":null
				})JSON" );
		REQUIRE( json_str == to_json( nums_t{} ) );
	}
}

TEST_CASE( "num-nullable-mand-invalid" , "[invalid]" )
{
	SECTION( "num_int16 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":-11,
					"num_int32":-5,
					"num_int64":-10
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int16 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":55,
						"num_int32":-5,
						"num_int64":-10
					})JSON" );

			const bool r = json_str != to_json( nums_t{ 55, -5, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":null,
					"num_int32":-3,
					"num_int64":-10
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":1,
						"num_int64":-10
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 5, 1, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":null,
					"num_int32":-10,
					"num_int64":2016
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":0,
						"num_int64":999
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 5, 0, 999 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

// String fields dto.
struct strings_t
{
	nullable_t< std::string > m_s1;
	nullable_t< std::string > m_s2;

	strings_t()
	{}

	strings_t(
		std::string s1,
		std::string s2 )
		:	m_s1{ std::move( s1 ) }
		,	m_s2{ std::move( s2 ) }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& mandatory(
				"s1",
				m_s1,
				one_of_constraint< std::string >(
					{
						"aaa",
						"bbb",
						"ccc",
						"ddd"
					} ) )
			& mandatory(
				"s2",
				m_s2,
				[]( const auto & f ){
					if( f && 5 < f->size() )
						throw std::runtime_error{
							"s2 string size must be <=5" };
				} )
			;
	}
};

TEST_CASE( "str-nullable-man-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str =
			R"JSON({
				"s1":"aaa",
				"s2":"12345"
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 );
		REQUIRE( dto.m_s2 );

		REQUIRE( *dto.m_s1 == "aaa" );
		REQUIRE( *dto.m_s2 == "12345" );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"s1":null,
				"s2":null
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE_FALSE( dto.m_s1 );
		REQUIRE_FALSE( dto.m_s2 );
	}

	SECTION( "write 1" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":"ccc",
					"s2":"abc"
				})JSON" );

		REQUIRE( json_str == to_json( strings_t{ "ccc", "abc" } ) );
	}

	SECTION( "write 2" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":null,
					"s2":null
				})JSON" );

		REQUIRE( json_str == to_json( strings_t{} ) );
	}
}

TEST_CASE( "str-nullable-man-invalid" , "[invalid]" )
{
	SECTION( "s1 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":"abc",
					"s2":""
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s1 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"abc",
						"s2":"abc"
					})JSON" );

			const bool r = json_str != to_json( strings_t{ "abc", "abc" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":null,
					"s2":"012345"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"bbb",
						"s2":"321321321321"
					})JSON" );

			const bool r = json_str != to_json( strings_t{ "bbb", "321321321321" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

} /* namespace mandatory_nullable_fields */

namespace optional_nullable_no_default_fields
{

// Numeric optional fields dto.
struct nums_t
{
	nullable_t< std::int16_t > m_num_int16;
	nullable_t< std::int32_t > m_num_int32;
	nullable_t< std::int64_t > m_num_int64;

	nums_t()
	{}

	nums_t(
		std::int16_t num_int16,
		std::int32_t num_int32,
		std::int64_t num_int64 )
		:	m_num_int16{ num_int16 }
		,	m_num_int32{ num_int32 }
		,	m_num_int64{ num_int64 }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional_no_default(
				"num_int16",
				m_num_int16,
				min_max_constraint< std::int16_t >( -10, 10 ) )
			& optional_no_default(
				"num_int32",
				m_num_int32,
				one_of_constraint< std::int32_t >( { -10, -5, 0, 5, 10 } ) )
			& optional_no_default(
				"num_int64",
				m_num_int64,
				[]( const auto & f ){
					if( f &&
						(-10 > *f || 10 < *f ) )
						throw std::runtime_error{
							"value " + std::to_string( f ) +
							" is invalid" };
				} )
			;
	}
};

TEST_CASE( "num-nullable-opt-no-default-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str = "{}";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE_FALSE( dto.m_num_int16 );
		REQUIRE_FALSE( dto.m_num_int32 );
		REQUIRE_FALSE( dto.m_num_int64 );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"num_int16":5
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 );
		REQUIRE( *dto.m_num_int16 == 5 );
		REQUIRE_FALSE( dto.m_num_int32 );
		REQUIRE_FALSE( dto.m_num_int64 );
	}

	SECTION( "read 3" )
	{
		const std::string json_str =
			R"JSON({
				"num_int32":-5
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE_FALSE( dto.m_num_int16);
		REQUIRE( dto.m_num_int32 );
		REQUIRE( *dto.m_num_int32 == -5 );
		REQUIRE_FALSE( dto.m_num_int64 );
	}

	SECTION( "read 4" )
	{
		const std::string json_str =
			R"JSON({
				"num_int64":-10
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE_FALSE( dto.m_num_int16 );
		REQUIRE_FALSE( dto.m_num_int32 );
		REQUIRE( dto.m_num_int64 );
		REQUIRE( *dto.m_num_int64 == -10 );
	}

	SECTION( "write" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_int16":null,
					"num_int32":null,
					"num_int64":null
				})JSON" );
		REQUIRE( json_str == to_json( nums_t{} ) );
	}
}

TEST_CASE( "num-nullable-opt-no-default-invalid" , "[invalid]" )
{
	SECTION( "num_int16 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":-11,
					"num_int64":null
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int16 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":55,
						"num_int32":-5,
						"num_int64":-10
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 55, -5, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":null,
					"num_int32":-3
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":1,
						"num_int64":-10
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 5, 1, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int32":null,
					"num_int64":2016
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":0,
						"num_int64":999
					})JSON" );

			const bool r = json_str != to_json( nums_t{ 5, 0, 999 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

// String fields dto.
struct strings_t
{
	nullable_t< std::string > m_s1;
	nullable_t< std::string > m_s2;

	strings_t()
	{}

	strings_t(
		std::string s1,
		std::string s2 )
		:	m_s1{ std::move( s1 ) }
		,	m_s2{ std::move( s2 ) }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional_no_default(
				"s1",
				m_s1,
				one_of_constraint< std::string >(
					{
						"",
						"aaa",
						"bbb",
						"ccc",
						"ddd"
					} ) )
			& optional_no_default(
				"s2",
				m_s2,
				[]( const auto & f ){
					if( f && 5 < f->size() )
						throw std::runtime_error{
							"s2 string size must be <=5" };
				} )
			;
	}
};

TEST_CASE( "str-nullable-opt-no-default-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str = "{}";

		const auto dto = from_json< strings_t >( json_str );

		REQUIRE_FALSE( dto.m_s1 );
		REQUIRE_FALSE( dto.m_s2 );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"s1":null,
				"s2":null
			})JSON";

		const auto dto = from_json< strings_t >( json_str );

		REQUIRE_FALSE( dto.m_s1 );
		REQUIRE_FALSE( dto.m_s2 );
	}

	SECTION( "read 3" )
	{
		const std::string json_str =
			R"JSON({
				"s1":"aaa"
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1);
		REQUIRE( *dto.m_s1 == "aaa" );
		REQUIRE_FALSE( dto.m_s2 );
	}

	SECTION( "read 4" )
	{
		const std::string json_str =
			R"JSON({
				"s2":"12345"
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE_FALSE( dto.m_s1 );
		REQUIRE( dto.m_s2 );
		REQUIRE( *dto.m_s2 == "12345" );
	}

	SECTION( "write 1" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":"ccc",
					"s2":"abc"
				})JSON" );

		REQUIRE( json_str == to_json( strings_t{ "ccc", "abc" } ) );
	}

	SECTION( "write 2" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":null,
					"s2":null
				})JSON" );

		REQUIRE( json_str == to_json( strings_t{} ) );
	}
}

TEST_CASE( "str-nullable-opt-no-default-invalid" , "[invalid]" )
{
	SECTION( "s1 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":"abc"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s1 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"abc",
						"s2":"abc"
					})JSON" );

			const bool r = json_str != to_json( strings_t{ "abc", "abc" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s2":"012345"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"bbb",
						"s2":"321321321321"
					})JSON" );

			const bool r = json_str != to_json( strings_t{ "bbb", "321321321321" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

} /* namespace optional_nullable_no_default_fields */

namespace optional_nullable_fields
{

// Numeric optional fields dto.
struct nums_t
{
	nullable_t< std::int16_t > m_num_int16;
	nullable_t< std::int32_t > m_num_int32;
	nullable_t< std::int64_t > m_num_int64;

	nums_t()
	{}

	nums_t(
		std::int16_t num_int16,
		std::int32_t num_int32,
		std::int64_t num_int64 )
		:	m_num_int16{ num_int16 }
		,	m_num_int32{ num_int32 }
		,	m_num_int64{ num_int64 }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional(
				"num_int16",
				m_num_int16,
				nullptr,
				min_max_constraint< std::int16_t >( -10, 10 ) )
			& optional(
				"num_int32",
				m_num_int32,
				nullptr,
				one_of_constraint< std::int32_t >( { -10, -5, 0, 5, 10 } ) )
			& optional(
				"num_int64",
				m_num_int64,
				nullptr,
				[]( const auto & f ){
					if( f &&
						( -10 > *f || 10 < *f ) )
						throw std::runtime_error{
							"value " + std::to_string( f ) +
							" is invalid" };
				} )
			;
	}
};

TEST_CASE( "num-nullable-opt-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str = "{}";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE_FALSE( dto.m_num_int16 );
		REQUIRE_FALSE( dto.m_num_int32 );
		REQUIRE_FALSE( dto.m_num_int64 );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"num_int16":null,
				"num_int32":null,
				"num_int64":null
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE_FALSE( dto.m_num_int16 );
		REQUIRE_FALSE( dto.m_num_int32 );
		REQUIRE_FALSE( dto.m_num_int64 );
	}

	SECTION( "read 3" )
	{
		const std::string json_str =
			R"JSON({
				"num_int16":5
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE( dto.m_num_int16 );
		REQUIRE( *dto.m_num_int16 == 5 );
		REQUIRE_FALSE( dto.m_num_int32 );
		REQUIRE_FALSE( dto.m_num_int64 );
	}

	SECTION( "read 4" )
	{
		const std::string json_str =
			R"JSON({
				"num_int32":-5
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE_FALSE( dto.m_num_int16 );
		REQUIRE( dto.m_num_int32 );
		REQUIRE( *dto.m_num_int32 == -5 );
		REQUIRE_FALSE( dto.m_num_int64 );
	}

	SECTION( "read 5" )
	{
		const std::string json_str =
			R"JSON({
				"num_int64":-10
			})JSON";
		const auto dto = from_json< nums_t >( json_str );

		REQUIRE_FALSE( dto.m_num_int16 );
		REQUIRE_FALSE( dto.m_num_int32 );
		REQUIRE( dto.m_num_int64 );
		REQUIRE( *dto.m_num_int64 == -10 );
	}

	SECTION( "write 1" )
	{
		const std::string json_str = "{}";
		REQUIRE( json_str == to_json( nums_t{} ) );
	}

	SECTION( "write 2" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_int16":0,
					"num_int32":0,
					"num_int64":0
				})JSON" );

		REQUIRE( json_str == to_json( nums_t{0, 0, 0 } ) );
	}
}

TEST_CASE( "num-nullable-opt-invalid" , "[invalid]" )
{
	SECTION( "num_int16 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":-11,
					"num_int64":-10
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int16 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":55,
						"num_int32":-5,
						"num_int64":-10
					})JSON" );
			const bool r = json_str != to_json( nums_t{ 55, -5, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int16":10,
					"num_int32":-3
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int32 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":1,
						"num_int64":-10
					})JSON" );

			const bool r = json_str != to_json( nums_t{ 5, 1, -10 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"num_int32":-10,
					"num_int64":2016
				})JSON";
			from_json< nums_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "num_int64 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"num_int16":5,
						"num_int32":0,
						"num_int64":999
					})JSON" );

			const bool r = json_str != to_json( nums_t{ 5, 0, 999 } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

// String fields dto.
struct strings_t
{
	nullable_t< std::string > m_s1;
	nullable_t< std::string > m_s2;

	strings_t()
	{}

	strings_t(
		std::string s1,
		std::string s2 )
		:	m_s1{ std::move( s1 ) }
		,	m_s2{ std::move( s2 ) }
	{}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional(
				"s1",
				m_s1,
				"ddd",
				one_of_constraint< std::string >(
					{
						"",
						"aaa",
						"bbb",
						"ccc",
						"ddd"
					} ) )
			& optional(
				"s2",
				m_s2,
				"123",
				[]( const auto & f ){
					if( f &&
						5 < f->size() )
						throw std::runtime_error{
							"s2 string size must be <=5" };
				} )
			;
	}
};

TEST_CASE( "str-nullable-opt-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str =
			R"JSON({
				"s1":null,
				"s2":null
			})JSON";

		const auto dto = from_json< strings_t >( json_str );

		REQUIRE_FALSE( dto.m_s1 );
		REQUIRE_FALSE( dto.m_s2 );
	}

	SECTION( "read 2" )
	{
		const std::string json_str = "{}";

		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 );
		REQUIRE( dto.m_s2 );
		REQUIRE( *dto.m_s1 == "ddd" );
		REQUIRE( *dto.m_s2 == "123" );
	}

	SECTION( "read 3" )
	{
		const std::string json_str =
			R"JSON({
				"s1":"aaa"
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 );
		REQUIRE( *dto.m_s1 == "aaa" );
		REQUIRE( dto.m_s2 );
		REQUIRE( *dto.m_s2 == "123" );
	}

	SECTION( "read 4" )
	{
		const std::string json_str =
			R"JSON({
				"s1":"aaa",
				"s2":null
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 );
		REQUIRE( *dto.m_s1 == "aaa" );
		REQUIRE_FALSE( dto.m_s2 );
	}

	SECTION( "read 5" )
	{
		const std::string json_str =
			R"JSON({
				"s2":"12345"
			})JSON";
		const auto dto = from_json< strings_t >( json_str );

		REQUIRE( dto.m_s1 );
		REQUIRE( *dto.m_s1 == "ddd" );
		REQUIRE( dto.m_s2 );
		REQUIRE( *dto.m_s2 == "12345" );
	}

	SECTION( "write 1" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":"ccc",
					"s2":"abc"
				})JSON" );

		REQUIRE( json_str == to_json( strings_t{ "ccc", "abc" } ) );
	}

	SECTION( "write 2" )
	{
		const std::string json_str = "{}";

		REQUIRE( json_str == to_json( strings_t{ "ddd", "123" } ) );
	}

	SECTION( "write 3" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"s1":null,
					"s2":null
				})JSON" );

		auto x = to_json( strings_t{} );
		REQUIRE( json_str == x );
	}
}

TEST_CASE( "str-nullable-opt-invalid" , "[invalid]" )
{
	SECTION( "s1 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s1":"abc"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s1 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"abc",
						"s2":"abc"
					})JSON" );

			const bool r = json_str != to_json( strings_t{ "abc", "abc" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 read" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"s2":"012345"
				})JSON";
			from_json< strings_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "s2 write" )
	{
		try
		{
			const std::string json_str =
				zip_json_str(
					R"JSON({
						"s1":"bbb",
						"s2":"321321321321"
					})JSON" );

			const bool r = json_str != to_json( strings_t{ "bbb", "321321321321" } );
			REQUIRE( r );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

} /* namespace optional_nullable_fields */

namespace vector_fields
{

// Numeric optional fields dto.
struct vector_fields_t
{
	std::vector< std::int32_t > m_nums_mandatory;
	std::vector< std::int32_t > m_nums_optional;

	nullable_t< std::vector< std::int32_t > > m_nums_mandatory_nullable;
	nullable_t< std::vector< std::int32_t > > m_nums_optional_nullable;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		static auto on_of_validator =
			one_of_constraint< std::int32_t >( { -10, -5, 0, 5, 10 } );

		io
			& mandatory(
				"nums_mandatory",
				m_nums_mandatory,
				min_max_constraint< std::int32_t >( -10, 10 ) )
			& optional_no_default(
				"nums_optional",
				m_nums_optional,
				min_max_constraint< std::int32_t >( -10, 10 ) )
			& mandatory(
				"nums_mandatory_nullable",
				m_nums_mandatory_nullable,
				on_of_validator )
			& optional_no_default(
				"nums_optional_nullable",
				m_nums_optional_nullable,
				on_of_validator )
			;
	}
};

TEST_CASE( "vector-valid" , "[valid]" )
{
	SECTION( "read 1" )
	{
		const std::string json_str =
			R"JSON({
				"nums_mandatory":[],
				"nums_mandatory_nullable":null
			})JSON";

		const auto dto = from_json< vector_fields_t >( json_str );

		REQUIRE( dto.m_nums_mandatory.empty() );
		REQUIRE( dto.m_nums_optional.empty() );
		REQUIRE_FALSE( dto.m_nums_mandatory_nullable );
		REQUIRE_FALSE( dto.m_nums_optional_nullable );
	}

	SECTION( "read 2" )
	{
		const std::string json_str =
			R"JSON({
				"nums_mandatory":[ -1, -2, 5, 10 ],
				"nums_optional":[ 1, 2, -5, -10 ],
				"nums_mandatory_nullable":[-10, 0, 10 ],
				"nums_optional_nullable":[5, 0, 5, 10 ]
			})JSON";

		const auto dto = from_json< vector_fields_t >( json_str );

		REQUIRE( 4 == dto.m_nums_mandatory.size() );
		REQUIRE( 4 == dto.m_nums_optional.size() );

		REQUIRE( -1 == dto.m_nums_mandatory[ 0 ] );
		REQUIRE( -2 == dto.m_nums_mandatory[ 1 ] );
		REQUIRE( 5 == dto.m_nums_mandatory[ 2 ] );
		REQUIRE( 10 == dto.m_nums_mandatory[ 3 ] );

		REQUIRE( 1 == dto.m_nums_optional[ 0 ] );
		REQUIRE( 2 == dto.m_nums_optional[ 1 ] );
		REQUIRE( -5 == dto.m_nums_optional[ 2 ] );
		REQUIRE( -10 == dto.m_nums_optional[ 3 ] );

		REQUIRE( dto.m_nums_mandatory_nullable );
		REQUIRE( dto.m_nums_optional_nullable );

		REQUIRE( 3 == dto.m_nums_mandatory_nullable->size() );
		REQUIRE( 4 == dto.m_nums_optional_nullable->size() );

		REQUIRE( -10 == dto.m_nums_mandatory_nullable->at( 0 ) );
		REQUIRE( 0 == dto.m_nums_mandatory_nullable->at( 1 ) );
		REQUIRE( 10 == dto.m_nums_mandatory_nullable->at( 2 ) );

		REQUIRE( 5 == dto.m_nums_optional_nullable->at( 0 ) );
		REQUIRE( 0 == dto.m_nums_optional_nullable->at( 1 ) );
		REQUIRE( 5 == dto.m_nums_optional_nullable->at( 2 ) );
		REQUIRE( 10 == dto.m_nums_optional_nullable->at( 3 ) );
	}


	SECTION( "write 1" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"nums_mandatory":[],
					"nums_optional":[],
					"nums_mandatory_nullable":null,
					"nums_optional_nullable":null
				})JSON" );

		REQUIRE( json_str == to_json( vector_fields_t{} ) );
	}

	SECTION( "write 2" )
	{

		const std::string json_str =
			zip_json_str(
				R"JSON({
					"nums_mandatory":[-2,-1,0,1,2,3,-2,-1,0,1,2,3],
					"nums_optional":[-2,-1,0,1,2,3,5,6,7,8,9,10,5,6,-10],
					"nums_mandatory_nullable":[-10,-5,-10,0,-5],
					"nums_optional_nullable":[-10,-5,-10,0]
				})JSON" );

		vector_fields_t vf{};
		vf.m_nums_mandatory = {-2,-1,0,1,2,3,-2,-1,0,1,2,3};
		vf.m_nums_optional = {-2,-1,0,1,2,3,5,6,7,8,9,10,5,6,-10};

		vf.m_nums_mandatory_nullable.emplace( {-10,-5,-10,0,-5} );
		vf.m_nums_optional_nullable.emplace( {-10,-5,-10,0} );
		REQUIRE( json_str == to_json( vf ) );
	}

	SECTION( "write 3" )
	{

		const std::string json_str =
			zip_json_str(
				R"JSON({
					"nums_mandatory":[-2,-1,0,1,2,3,-2,-1,0,1,2,3],
					"nums_optional":[-2,-1,0,1,2,3,5,6,7,8,9,10,5,6,-10],
					"nums_mandatory_nullable":null,
					"nums_optional_nullable":null
				})JSON" );

		vector_fields_t vf{};
		vf.m_nums_mandatory = {-2,-1,0,1,2,3,-2,-1,0,1,2,3};
		vf.m_nums_optional = {-2,-1,0,1,2,3,5,6,7,8,9,10,5,6,-10};

		vf.m_nums_mandatory_nullable = nullptr;
		vf.m_nums_optional_nullable = nullptr;
		REQUIRE( json_str == to_json( vf ) );
	}
}

TEST_CASE( "vector-invalid" , "[invalid]" )
{
	SECTION( "read 1" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"nums_mandatory":[-2,-1,0,1,2,3,-2,-1,0,1,2,3,-11],
					"nums_optional":[-2,-1,0,1,2,3,5,6,7,8,9,10,5,6,-10],
					"nums_mandatory_nullable":null,
					"nums_optional_nullable":null
				})JSON";

			from_json< vector_fields_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "write 1" )
	{
		try
		{
			vector_fields_t vf{};
			vf.m_nums_mandatory = {-2,-1,0,1,2,3,-2,-1,0,1,2,3,11};
			vf.m_nums_optional = {-2,-1,0,1,2,3,5,6,7,8,9,10,5,6,-10};

			vf.m_nums_mandatory_nullable = nullptr;
			vf.m_nums_optional_nullable = nullptr;

			to_json( vf );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "read 2" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"nums_mandatory":[-2,-1,0,1,2,3,-2,-1,0,1,2,3,10],
					"nums_optional":[-2,-1,0,1,2,3,5,6,123654,8,9,10,5,6,10],
					"nums_mandatory_nullable":null,
					"nums_optional_nullable":null}
				)JSON";

			from_json< vector_fields_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "write 2" )
	{
		try
		{
			vector_fields_t vf{};
			vf.m_nums_mandatory = {-2,-1,0,1,2,3,-2,-1,0,1,2,3,11};
			vf.m_nums_optional = {-2,-1,0,1,2,3,999,6,7,8,9,10,5,6,-10};

			vf.m_nums_mandatory_nullable = nullptr;
			vf.m_nums_optional_nullable = nullptr;

			to_json( vf );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "read 3" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"nums_mandatory":[],
					"nums_optional":[],
					"nums_mandatory_nullable":[3],
					"nums_optional_nullable":[5,-10]
				})JSON";

			from_json< vector_fields_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "write 3" )
	{
		try
		{
			vector_fields_t vf{};
			vf.m_nums_mandatory_nullable.emplace( {-10,-5,9,-10,0} );
			vf.m_nums_optional_nullable.emplace( {-10,-5,0,-10,0} );

			to_json( vf );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "read 4" )
	{
		try
		{
			const std::string json_str =
				R"JSON({
					"nums_mandatory":[],
					"nums_optional":[],
					"nums_mandatory_nullable":[5,10],
					"nums_optional_nullable":[-4]}
				)JSON";

			from_json< vector_fields_t >( json_str );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}

	SECTION( "write 4" )
	{
		try
		{
			vector_fields_t vf{};
			vf.m_nums_mandatory_nullable.emplace( {-10,-5,0,-10,0} );
			vf.m_nums_optional_nullable.emplace( {-10,-5,0,-10,0, 9999} );

			to_json( vf );

			REQUIRE( false /* validator didn't work*/ );
		}
		catch( const ex_t & )
		{
			REQUIRE( true );
		}
	}
}

} /* namespace vector_fields */
