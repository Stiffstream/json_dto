#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

#include <json_dto/pub.hpp>

#if defined( JSON_DTO_SUPPORTS_STD_OPTIONAL )

struct data_t
{
	json_dto::cpp17::optional< int > m_i;
	json_dto::cpp17::optional< float > m_f;
	json_dto::cpp17::optional< std::string > m_s;

	template< typename IO >
	void json_io( IO & io )
	{
		using namespace json_dto;

		io & optional( "int", m_i, json_dto::cpp17::nullopt() )
			& optional( "float", m_f, json_dto::cpp17::nullopt() )
			& optional( "str", m_s, json_dto::cpp17::nullopt() );
	}
};

TEST_CASE( "read from json" , "read" )
{
	{
		const std::string json_data{
			R"JSON(
			{}
			)JSON" };
		auto obj = json_dto::from_json< data_t >( json_data );

		REQUIRE( !obj.m_i );
		REQUIRE( !obj.m_f );
		REQUIRE( !obj.m_s );
	}
}

#endif

