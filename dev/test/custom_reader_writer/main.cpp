#include <catch2/catch.hpp>

#include <iostream>
#include <limits>
#include <sstream>

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
eq( const nullable_t< T > & left, const nullable_t< T > & right )
{
	if( !left && !right )
		return true;

	return left && right && eq( *left, *right );
}

//
// custom_reader_writer_t
//
struct custom_reader_writer_t
{
	template< typename Field_Type >
	void
	read( Field_Type & v, const rapidjson::Value & from ) const
	{
		using json_dto::read_json_value;
		read_json_value( v, from );
	}

	template< typename Field_Type >
	void
	write(
		Field_Type & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		using json_dto::write_json_value;
		write_json_value( v, to, allocator );
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
		std::sprintf( buf, "%x", v );

		write_json_value( json_dto::make_string_ref(buf), to, allocator );
	}
};

struct uint16_value_t
{
	std::uint16_t m_value;
};

struct uint16_value_reader_writer_t
{
	void
	read( uint16_value_t & v, const rapidjson::Value & from ) const
	{
		std::uint8_t hi = static_cast<std::uint8_t>(from["Hi"].GetUint());
		std::uint8_t lo = static_cast<std::uint8_t>(from["Lo"].GetUint());

		v.m_value = (static_cast<std::uint16_t>(hi) << 8) | lo;
	}

	void
	write(
		const uint16_value_t & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		std::uint8_t hi = static_cast<std::uint8_t>(v.m_value >> 8u);
		std::uint8_t lo = static_cast<std::uint8_t>(v.m_value & 0xffu);

		to.SetObject();
		rapidjson::Value hi_val(hi);
		rapidjson::Value lo_val(lo);
		to.AddMember("Hi", hi_val, allocator);
		to.AddMember("Lo", lo_val, allocator);
	}
};

//
// simple_types_dto_t
//

struct simple_types_dto_t
{
	std::int32_t m_num{ -1 };
	std::int32_t m_num_2{ -1 };
	std::int32_t m_num_opt{ -1 };
	std::int32_t m_num_opt_no_default{ -1 };

	nullable_t< std::int32_t > m_num_opt_nullable{};
	nullable_t< std::int32_t > m_num_opt_no_default_nullable{};

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& mandatory( custom_reader_writer_t{}, "num", m_num )
			& mandatory_with_null_as_default(
					custom_reader_writer_t{}, "num_2", m_num_2 )
			& optional(
					custom_reader_writer_t{},
					"num_opt", m_num_opt, 0 )
			& optional_no_default(
					custom_reader_writer_t{},
					"num_opt_no_default", m_num_opt_no_default )
			& optional(
					custom_reader_writer_t{},
					"num_opt_nullable", m_num_opt_nullable, nullptr )
			& optional_no_default(
					custom_reader_writer_t{},
					"num_opt_no_default_nullable", m_num_opt_no_default_nullable )
		;
	}

	bool
	operator == ( const simple_types_dto_t & other ) const
	{
		return
			eq( m_num, other.m_num ) &&
			eq( m_num_2, other.m_num_2 ) &&
			eq( m_num_opt, other.m_num_opt ) &&
			eq( m_num_opt_no_default, other.m_num_opt_no_default ) &&
			eq( m_num_opt_nullable, other.m_num_opt_nullable ) &&
			eq( m_num_opt_no_default_nullable, other.m_num_opt_no_default_nullable );
	}
};

struct vector_of_ints_t
{
	std::vector<int> m_values;

	template< typename Io >
	void json_io( Io & io )
	{
		io & json_dto::mandatory(
				json_dto::apply_to_content_t<hex_writer_t>{},
				"values",
				m_values );
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
				m_values );
	}
};

TEST_CASE("simple-types", "[simple]" )
{
	SECTION( "read empty" )
	{
		auto dto = json_dto::from_json< simple_types_dto_t >(
				R"({"num":1, "num_2":3})" );

		REQUIRE( dto.m_num == 1 );
		REQUIRE( dto.m_num_2 == 3 );
		REQUIRE( dto.m_num_opt == 0 );
		REQUIRE( dto.m_num_opt_no_default == -1 );
		REQUIRE_FALSE( dto.m_num_opt_nullable );
		REQUIRE_FALSE( dto.m_num_opt_no_default_nullable );
	}

	SECTION( "read with null" )
	{
		auto dto = json_dto::from_json< simple_types_dto_t >(
				R"({"num":1, "num_2":null})" );

		REQUIRE( dto.m_num == 1 );
		REQUIRE( dto.m_num_2 == 0 );
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
					"num":-1,
					"num_2":-1,
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
					"num":-1,
					"num_2":-1,
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
				"num":333,
				"num_2":777,
				"num_opt":42,
				"num_opt_no_default":-99999,
				"num_opt_nullable":999,
				"num_opt_no_default_nullable":2016
			})JSON" );

	SECTION( "read all defined" )
	{
		auto dto = json_dto::from_json< simple_types_dto_t >( all_defined_json );

		REQUIRE( dto.m_num == 333 );
		REQUIRE( dto.m_num_2 == 777 );
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
		dto.m_num = 333;
		dto.m_num_2 = 777;
		dto.m_num_opt = 42;
		dto.m_num_opt_no_default = -99999;
		dto.m_num_opt_nullable.emplace( 999 );
		dto.m_num_opt_no_default_nullable.emplace( 2016 );

		REQUIRE( all_defined_json == to_json( dto ) );
	}
}

TEST_CASE("vector with custom hex_writer", "[vector][hex_writer]")
{
	SECTION("not-empty vector")
	{
		const std::string json_str =
				R"JSON({"values":["0","1","a","f","10","20"]})JSON";

		vector_of_ints_t dto{ {0, 1, 10, 15, 16, 32} };

		REQUIRE( json_str == to_json( dto ) );
	}
}

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
	}
}

TEST_CASE("to_json and from_json with custom reader_writer",
		"[to_json][from_json][custom_reader_writer]")
{
	// to_json(writer, dto)
	{
		const std::string expected = R"JSON("Hello")JSON";

		std::string dto{ "Hello" };
		const std::string json_value = to_json(
				custom_reader_writer_t{}, dto );

		REQUIRE( expected == json_value );
	}

	// to_json(writer, dto, pretty_writter)
	{
		const std::string expected = R"JSON("Hello")JSON";

		std::string dto{ "Hello" };
		const std::string json_value = to_json(
				custom_reader_writer_t{},
				dto,
				pretty_writer_params_t{}.indent_char( '\t' ) );

		REQUIRE( expected == json_value );
	}

	// from_json(reader, json_value)
	{
		rapidjson::Value json_value;
		json_value.SetString("Hello");

		std::string extracted_value = from_json<std::string>(
				custom_reader_writer_t{}, json_value );

		REQUIRE( "Hello" == extracted_value );
	}

	// from_json(reader, json_value, dest)
	{
		rapidjson::Value json_value;
		json_value.SetString("Hello");

		std::string extracted_value;
		from_json(
				custom_reader_writer_t{},
				json_value,
				extracted_value );

		REQUIRE( "Hello" == extracted_value );
	}

	// from_json(reader, string-ref)
	{
		const string_ref_t what{ R"JSON("Hello")JSON" };
		std::string extracted_value = from_json<std::string>(
				custom_reader_writer_t{}, what );

		REQUIRE( "Hello" == extracted_value );
	}

	// from_json(reader, string-literal)
	{
		std::string extracted_value = from_json<std::string>(
				custom_reader_writer_t{},
				R"JSON("Hello")JSON" );

		REQUIRE( "Hello" == extracted_value );
	}

	// from_json(reader, std_string_object)
	{
		const std::string to_parse{ R"JSON("Hello")JSON" };
		std::string extracted_value = from_json<std::string>(
				custom_reader_writer_t{}, to_parse );

		REQUIRE( "Hello" == extracted_value );
	}

	// from_json(reader, string-ref, dest)
	{
		const string_ref_t what{ R"JSON("Hello")JSON" };

		std::string extracted_value;
		from_json(
				custom_reader_writer_t{},
				what,
				extracted_value );

		REQUIRE( "Hello" == extracted_value );
	}

	// from_json(reader, string-literal, dest)
	{
		std::string extracted_value;
		from_json(
				custom_reader_writer_t{},
				R"JSON("Hello")JSON",
				extracted_value );

		REQUIRE( "Hello" == extracted_value );
	}

	// from_json(reader, std_string_object, dest)
	{
		const std::string what{ R"JSON("Hello")JSON" };

		std::string extracted_value;
		from_json(
				custom_reader_writer_t{},
				what,
				extracted_value );

		REQUIRE( "Hello" == extracted_value );
	}
}

TEST_CASE("to_stream and from_stream with custom reader_writer",
		"[to_stream][from_stream][custom_reader_writer]")
{
	// to_stream(writer, stream, dto)
	{
		const std::string expected = R"JSON({"Hi":1,"Lo":1})JSON";

		uint16_value_t dto{ 0x0101 };
		std::ostringstream ss;
		to_stream( uint16_value_reader_writer_t{}, ss, dto );

		REQUIRE( expected == ss.str() );
	}

	// to_stream(writer, stream, dto, pretty_writer)
	{
		const std::string expected = R"JSON({
 "Hi": 1,
 "Lo": 1
})JSON";

		uint16_value_t dto{ 0x0101 };
		std::ostringstream ss;
		to_stream( uint16_value_reader_writer_t{}, ss, dto,
				pretty_writer_params_t{}
					.indent_char( ' ' )
					.indent_char_count( 1u ) );

		REQUIRE( expected == ss.str() );
	}

	// from_stream(reader, stream, dto)
	{
		std::istringstream ss{ R"JSON({"Lo":2, "Hi":3})JSON" };

		uint16_value_t dto{ 0x0 };
		from_stream( uint16_value_reader_writer_t{}, ss, dto );

		REQUIRE( 0x0302u == dto.m_value );
	}

	// from_stream(reader, stream)
	{
		std::istringstream ss{ R"JSON({"Lo":2, "Hi":3})JSON" };

		uint16_value_t dto = from_stream<uint16_value_t>(
				uint16_value_reader_writer_t{}, ss );

		REQUIRE( 0x0302u == dto.m_value );
	}
}

