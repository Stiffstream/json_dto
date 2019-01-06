#include <catch2/catch.hpp>

#include <iostream>
#include <limits>

#include <rapidjson/document.h>

#include <json_dto/pub.hpp>

#include <test/helper.hpp>

using namespace json_dto;

template < typename T >
bool
eq( const T & left, const T & right )
{
	return left == right;
}

template < typename T >
bool
eq( const std::vector< T > & left, const std::vector< T > & right )
{
	bool result{ true };

	if( left.size() == right.size() )
	{
		for( size_t i = 0, n = left.size(); i < n; ++i )
		{
			if( !eq( left[ i ], right[ i ] ) )
			{
				result = false;
				break;
			}
		}
	}
	else
		result = false;

	return result;
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
// simple_types_dto_t
//

struct simple_types_dto_t
{
	std::int32_t m_num_opt{ -1 };
	std::int32_t m_num_opt_no_default{-1};

	nullable_t< std::int32_t > m_num_opt_nullable{};
	nullable_t< std::int32_t > m_num_opt_no_default_nullable{};

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional( "num_opt", m_num_opt, 0 )
			& optional_no_default( "num_opt_no_default", m_num_opt_no_default )
			& optional( "num_opt_nullable", m_num_opt_nullable, nullptr )
			& optional_no_default( "num_opt_no_default_nullable", m_num_opt_no_default_nullable )
		;
	}

	bool
	operator == ( const simple_types_dto_t & other ) const
	{
		return
			eq( m_num_opt, other.m_num_opt ) &&
			eq( m_num_opt_no_default, other.m_num_opt_no_default ) &&
			eq( m_num_opt_nullable, other.m_num_opt_nullable ) &&
			eq( m_num_opt_no_default_nullable, other.m_num_opt_no_default_nullable );
	}
};

TEST_CASE("simple-types", "[simple]" )
{
	SECTION( "read empty" )
	{
		auto dto = json_dto::from_json< simple_types_dto_t >( "{}" );

		REQUIRE( dto.m_num_opt == 0 );
		REQUIRE( dto.m_num_opt_no_default == -1 );
		REQUIRE_FALSE( dto.m_num_opt_nullable );
		REQUIRE_FALSE( dto.m_num_opt_no_default_nullable );
	}

	SECTION( "write default constructed" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_opt":-1,
					"num_opt_no_default":-1,
					"num_opt_no_default_nullable":null
				})JSON" );

		simple_types_dto_t dto{};

		REQUIRE( json_str == to_json( dto ) );
	}

	SECTION( "write default values" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"num_opt_no_default":-1,
					"num_opt_no_default_nullable":null
				})JSON" );

		simple_types_dto_t dto{};
		dto.m_num_opt = 0;

		REQUIRE( json_str == to_json( dto ) );
	}

	const std::string all_defined_json =
		zip_json_str(
			R"JSON({
				"num_opt":42,
				"num_opt_no_default":-99999,
				"num_opt_nullable":999,
				"num_opt_no_default_nullable":2016
			})JSON" );

	SECTION( "read all defined" )
	{
		auto dto = json_dto::from_json< simple_types_dto_t >( all_defined_json );

		REQUIRE( dto.m_num_opt == 42 );
		REQUIRE( dto.m_num_opt_no_default == -99999 );
		REQUIRE( dto.m_num_opt_nullable );
		REQUIRE( *dto.m_num_opt_nullable == 999 );
		REQUIRE( dto.m_num_opt_no_default_nullable );
		REQUIRE( *dto.m_num_opt_no_default_nullable == 2016 );
	}

	SECTION( "write default values" )
	{
		simple_types_dto_t dto{};
		dto.m_num_opt = 42;
		dto.m_num_opt_no_default = -99999;
		dto.m_num_opt_nullable.emplace( 999 );
		dto.m_num_opt_no_default_nullable.emplace( 2016 );

		REQUIRE( all_defined_json == to_json( dto ) );
	}
}

//
// vector_simple_types_dto_t
//

struct vector_simple_types_dto_t
{
	std::vector< std::string > m_str_vec_opt;
	nullable_t< std::vector< std::string > > m_str_vec_opt_nullable;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional_no_default( "str_vec_opt", m_str_vec_opt )
			& optional_no_default( "str_vec_opt_nullable", m_str_vec_opt_nullable )
		;
	}

	bool
	operator == ( const vector_simple_types_dto_t & other ) const
	{
		return
			eq( m_str_vec_opt, other.m_str_vec_opt ) &&
			eq( m_str_vec_opt_nullable, other.m_str_vec_opt_nullable );
	}
};

vector_simple_types_dto_t
vector_simple_types_dto_sample()
{
	vector_simple_types_dto_t dto{};

	dto.m_str_vec_opt.push_back( "1" );
	dto.m_str_vec_opt.push_back( "23" );
	dto.m_str_vec_opt.push_back( "456" );

	dto.m_str_vec_opt_nullable.emplace();
	dto.m_str_vec_opt_nullable->push_back( "a" );
	dto.m_str_vec_opt_nullable->push_back( "b" );
	dto.m_str_vec_opt_nullable->push_back( "c" );
	dto.m_str_vec_opt_nullable->push_back( "d" );
	dto.m_str_vec_opt_nullable->push_back( "efgh" );

	return dto;
}

TEST_CASE("vector-simple-types", "[simple]" )
{
	SECTION( "read empty" )
	{
		auto dto = json_dto::from_json< vector_simple_types_dto_t >( "{}" );

		REQUIRE( dto.m_str_vec_opt.empty() );
		REQUIRE_FALSE( dto.m_str_vec_opt_nullable );
	}

	SECTION( "write default constructed" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"str_vec_opt":[],
					"str_vec_opt_nullable":null
				})JSON" );

		vector_simple_types_dto_t dto{};

		REQUIRE( json_str == to_json( dto ) );
	}


	const std::string all_defined_json =
		zip_json_str(
			R"JSON({
				"str_vec_opt":["1","23","456"],
				"str_vec_opt_nullable":["a","b","c","d","efgh"]
			})JSON" );

	SECTION( "read all defined" )
	{
		auto dto = json_dto::from_json< vector_simple_types_dto_t >( all_defined_json );

		REQUIRE( 3 == dto.m_str_vec_opt.size() );

		REQUIRE( dto.m_str_vec_opt.at( 0 ) == "1" );
		REQUIRE( dto.m_str_vec_opt.at( 1 ) == "23" );
		REQUIRE( dto.m_str_vec_opt.at( 2 ) == "456" );

		REQUIRE( dto.m_str_vec_opt_nullable );
		REQUIRE( 5 == dto.m_str_vec_opt_nullable->size() );
		REQUIRE( dto.m_str_vec_opt_nullable->at( 0 ) == "a" );
		REQUIRE( dto.m_str_vec_opt_nullable->at( 1 ) == "b" );
		REQUIRE( dto.m_str_vec_opt_nullable->at( 2 ) == "c" );
		REQUIRE( dto.m_str_vec_opt_nullable->at( 3 ) == "d" );
		REQUIRE( dto.m_str_vec_opt_nullable->at( 4 ) == "efgh" );
	}

	SECTION( "write all defined" )
	{
		vector_simple_types_dto_t dto = vector_simple_types_dto_sample();

		REQUIRE( all_defined_json == to_json( dto ) );
	}
}

//
// complex_type_t
//

struct complex_type_t
{
	simple_types_dto_t m_opt{};
	simple_types_dto_t m_opt_no_default{};

	nullable_t< simple_types_dto_t > m_opt_nullable{};
	nullable_t< simple_types_dto_t > m_opt_no_default_nullable{};
};

simple_types_dto_t
simple_types_dto_default()
{
	simple_types_dto_t dto{};

	dto.m_num_opt = 42;
	dto.m_num_opt_no_default = -99999;
	dto.m_num_opt_nullable.emplace( 999 );
	dto.m_num_opt_no_default_nullable.emplace( 2016 );

	return dto;
}

simple_types_dto_t
simple_types_dto_sample()
{
	simple_types_dto_t dto{};

	dto.m_num_opt = -4242;
	dto.m_num_opt_no_default = 789;
	dto.m_num_opt_nullable.emplace( 216 );
	dto.m_num_opt_no_default_nullable.emplace( 321321 );

	return dto;
}

auto
simple_types_dto_sample_str()
{
	return to_json( simple_types_dto_sample() );
}

namespace json_dto
{

template < typename Json_Io >
void
json_io( Json_Io & io, complex_type_t & dto )
{
	io
		& optional( "opt", dto.m_opt, simple_types_dto_default() )
		& optional_no_default( "opt_no_default", dto.m_opt_no_default )
		& optional( "opt_nullable", dto.m_opt_nullable, nullptr )
		& optional_no_default( "opt_no_default_nullable", dto.m_opt_no_default_nullable )
	;
}

} /* namespace json_dto */

TEST_CASE("complex-types", "[complex]" )
{
	SECTION( "read empty" )
	{
		auto dto = json_dto::from_json< complex_type_t >( "{}" );

		REQUIRE( dto.m_opt == simple_types_dto_default() );
		REQUIRE( dto.m_opt_no_default == simple_types_dto_t{} );
		REQUIRE_FALSE( dto.m_opt_nullable );
		REQUIRE_FALSE( dto.m_opt_no_default_nullable );
	}

	SECTION( "write default constructed" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"opt":
						{
							"num_opt":-1,
							"num_opt_no_default":-1,
							"num_opt_no_default_nullable":null
						},
					"opt_no_default":
						{
							"num_opt":-1,
							"num_opt_no_default":-1,
							"num_opt_no_default_nullable":null
						},
					"opt_no_default_nullable":null
				})JSON" );

		complex_type_t dto{};

		REQUIRE( json_str == to_json( dto ) );
	}

	SECTION( "write default values" )
	{
		const std::string json_str =
			zip_json_str(
				R"JSON({
					"opt_no_default":
						{
							"num_opt":-1,
							"num_opt_no_default":-1,
							"num_opt_no_default_nullable":null
						},
					"opt_no_default_nullable":null
				})JSON" );

		complex_type_t dto{};
		dto.m_opt = simple_types_dto_default();

		REQUIRE( json_str == to_json( dto ) );
	}

	const std::string all_defined_json =
			R"JSON({"opt":)JSON"
				+ simple_types_dto_sample_str() +
			R"JSON(,"opt_no_default":)JSON"
				+ simple_types_dto_sample_str() +
			R"JSON(,"opt_nullable":)JSON"
				+ simple_types_dto_sample_str() +
			R"JSON(,"opt_no_default_nullable":)JSON"
				+ simple_types_dto_sample_str() +
			R"JSON(})JSON";

	SECTION( "read all defined" )
	{
		auto dto = json_dto::from_json< complex_type_t >( all_defined_json );

		REQUIRE( dto.m_opt == simple_types_dto_sample() );
		REQUIRE( dto.m_opt_no_default == simple_types_dto_sample() );
		REQUIRE( dto.m_opt_nullable );
		REQUIRE( *dto.m_opt_nullable == simple_types_dto_sample() );
		REQUIRE( dto.m_opt_no_default_nullable );
		REQUIRE( *dto.m_opt_no_default_nullable == simple_types_dto_sample() );
	}

	SECTION( "write all defined" )
	{
		complex_type_t dto{};
		dto.m_opt = simple_types_dto_sample();
		dto.m_opt_no_default = simple_types_dto_sample();
		dto.m_opt_nullable.emplace( simple_types_dto_sample() );
		dto.m_opt_no_default_nullable.emplace( simple_types_dto_sample() );

		REQUIRE( all_defined_json == to_json( dto ) );
	}
}

//
// vector_complex_types_dto_t
//

struct vector_complex_types_dto_t
{
	std::vector< vector_simple_types_dto_t > m_vec_opt;
	nullable_t< std::vector< vector_simple_types_dto_t > > m_vec_opt_nullable;

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& optional_no_default( "vec_opt", m_vec_opt )
			& optional_no_default( "vec_opt_nullable", m_vec_opt_nullable )
		;
	}
};

TEST_CASE("vector-complex-types", "[complex]" )
{
	SECTION( "read empty" )
	{
		auto dto = json_dto::from_json< vector_complex_types_dto_t >( "{}" );

		REQUIRE( dto.m_vec_opt.empty() );
		REQUIRE_FALSE( dto.m_vec_opt_nullable );
	}

	SECTION( "write default constructed" )
	{
		const std::string json_str =
			R"JSON({"vec_opt":[],"vec_opt_nullable":null})JSON";

		vector_complex_types_dto_t dto{};

		REQUIRE( json_str == to_json( dto ) );
	}

	const std::string all_defined_json =
		zip_json_str(
			R"JSON({
				"vec_opt":
					[
						{
							"str_vec_opt":[],
							"str_vec_opt_nullable":null
						},
						{
							"str_vec_opt":["1","23","456"],
							"str_vec_opt_nullable":["a","b","c","d","efgh"]
						}
					],
				"vec_opt_nullable":
					[
						{
							"str_vec_opt":["1","23","456"],
							"str_vec_opt_nullable":["a","b","c","d","efgh"]
						},
						{
							"str_vec_opt":[],
							"str_vec_opt_nullable":null
						}
					]
				})JSON" );

	SECTION( "read all defined" )
	{
		auto dto = json_dto::from_json< vector_complex_types_dto_t >( all_defined_json );

		REQUIRE( 2 == dto.m_vec_opt.size() );
		REQUIRE( dto.m_vec_opt.at( 0 ) == vector_simple_types_dto_t{} );
		REQUIRE( dto.m_vec_opt.at( 1 ) == vector_simple_types_dto_sample() );

		REQUIRE( dto.m_vec_opt_nullable );
		REQUIRE( 2 == dto.m_vec_opt_nullable->size() );
		REQUIRE( dto.m_vec_opt_nullable->at( 0 ) == vector_simple_types_dto_sample() );
		REQUIRE( dto.m_vec_opt_nullable->at( 1 ) == vector_simple_types_dto_t{} );
	}

	SECTION( "write all defined" )
	{
		vector_complex_types_dto_t dto{};
		dto.m_vec_opt.push_back( vector_simple_types_dto_t{} );
		dto.m_vec_opt.push_back( vector_simple_types_dto_sample() );

		dto.m_vec_opt_nullable.emplace();
		dto.m_vec_opt_nullable->push_back( vector_simple_types_dto_sample() );
		dto.m_vec_opt_nullable->push_back( vector_simple_types_dto_t{} );

		REQUIRE( all_defined_json == to_json( dto ) );
	}
}
