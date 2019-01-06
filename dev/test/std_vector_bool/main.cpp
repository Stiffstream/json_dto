#include <catch2/catch.hpp>

#include <json_dto/pub.hpp>

struct data_t
{
	bool m_single;
	std::vector<bool> m_multi;

	template<typename I>
	void json_io(I & io)
	{
		io & json_dto::mandatory("single", m_single)
			& json_dto::mandatory("multi", m_multi);
	}
};

TEST_CASE( "read from json" , "read" )
{
	{
		const std::string json_data{
			R"JSON(
			{"single":true, "multi":[false, true]}
			)JSON" };
		auto obj = json_dto::from_json< data_t >( json_data );

		REQUIRE( obj.m_single );
		REQUIRE( 2 == obj.m_multi.size() );
		REQUIRE( !obj.m_multi[0] );
		REQUIRE( obj.m_multi[1] );
	}
}

TEST_CASE( "write to json" , "write" )
{
	{
		data_t obj{ true, {true, false, true}};
		const auto r = json_dto::to_json( obj );

		REQUIRE( R"({"single":true,"multi":[true,false,true]})" == r );
	}
}

