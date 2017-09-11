#include <iostream>
#include <cstdint>

#include <json_dto/pub.hpp>

struct supported_types_t
{
	supported_types_t() {}

	supported_types_t(
		bool val_bool,
		std::int16_t val_int16,
		std::uint16_t val_uint16,
		std::int32_t val_int32,
		std::uint32_t val_uint32,
		std::int64_t val_int64,
		std::uint64_t val_uint64,
		double val_double,
		std::string val_string )
		:	m_bool{ val_bool }
		,	m_int16{ val_int16 }
		,	m_uint16{ val_uint16 }
		,	m_int32{ val_int32 }
		,	m_uint32{ val_uint32 }
		,	m_int64{ val_int64 }
		,	m_uint64{ val_uint64 }
		,	m_double{ val_double }
		,	m_string{ std::move( val_string ) }
	{}

	bool m_bool{ false };

	std::int16_t m_int16{};
	std::uint16_t m_uint16{};

	std::int32_t m_int32{};
	std::uint32_t m_uint32{};

	std::int64_t m_int64{};
	std::uint64_t m_uint64{};
	double m_double{};

	std::string m_string{};
};

namespace json_dto
{

template< typename Json_Io >
void json_io( Json_Io & io, supported_types_t & obj )
{
	io & json_dto::mandatory( "bool", obj.m_bool )
		& json_dto::mandatory( "int16", obj.m_int16 )
		& json_dto::mandatory( "uint16", obj.m_uint16 )
		& json_dto::mandatory( "int32", obj.m_int32 )
		& json_dto::mandatory( "uint32", obj.m_uint32 )
		& json_dto::mandatory( "int64", obj.m_int64 )
		& json_dto::mandatory( "uint64", obj.m_uint64 )
		& json_dto::mandatory( "double", obj.m_double )
		& json_dto::mandatory( "string", obj.m_string );
}

} /* namespace json_dto */

const std::string json_data{
R"JSON({
	"bool" : true,
	"int16" : -1,
	"uint16" : 2,
	"int32" : -4,
	"uint32" : 8,
	"int64" : -16,
	"uint64" : 32,
	"double" : 2.718281828,
	"string" : "Sample string"
})JSON" };

int
main( int , char *[] )
{
	try
	{
		{
			auto obj = json_dto::from_json< supported_types_t >( json_data );

			std::cout
				<< "Deserialized from JSON:\n"
				<< "\tbool: " << obj.m_bool << "\n"
				<< "\tint16: " << obj.m_int16 << "\n"
				<< "\tuint16: " << obj.m_uint16 << "\n"
				<< "\tint32: " << obj.m_int32 << "\n"
				<< "\tuint32: " << obj.m_uint32 << "\n"
				<< "\tint64: " << obj.m_int64 << "\n"
				<< "\tuint64: " << obj.m_uint64 << "\n"
				<< "\tdouble: " << obj.m_double << "\n"
				<< "\tstring: " << obj.m_string << "\n"
				<< std::endl;
		}

		{
			const supported_types_t obj{
				false,
				-10,
				0xFFFF,
				-100,
				0xFFFFFFFF,
				~10000,
				0xFFFFFFFFFFFFFFFFul,
				3.14,
				"Another string" };

			std::cout
				<< "\nSerialized to JSON:\n"
				<< json_dto::to_json( obj ) << std::endl;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}
