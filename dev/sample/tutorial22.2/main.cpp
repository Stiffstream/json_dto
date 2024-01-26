/*
	A sample using json_dto
*/

#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>

#include <json_dto/pub.hpp>

struct sizes
{
	int width;
	int height;

	template<typename Io> void json_io(Io & io)
	{
		io & json_dto::mandatory( "w", width )
			& json_dto::mandatory( "h", height )
			;
	}
};

struct outer
{
	std::tuple<int, std::string, sizes> x;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory(
				json_dto::inside_array::reader_writer(
					json_dto::inside_array::member( std::get<0>(x) ),
					json_dto::inside_array::member( std::get<1>(x) ),
					json_dto::inside_array::member( std::get<2>(x) ) ),
				"x", x )
			;
	}
};

std::ostream & operator<<( std::ostream & to, const outer & what )
{
	return (to << std::get<0>(what.x) << ", '" << std::get<1>(what.x)
			<< "', (" << std::get<2>(what.x).width
			<< ", " << std::get<2>(what.x).height << ")");
}

int
main( int , char *[] )
{
	try
	{
		const auto obj = json_dto::from_json<outer>(
				R"({"x":[1, "Hello, World", {"w":640, "h":480}]})" );
		std::cout << "deserialized: " << obj << std::endl;
		std::cout << "  serialized: " << json_dto::to_json( obj ) << std::endl;
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

