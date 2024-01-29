/*
	A sample using json_dto
*/

#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include <json_dto/pub.hpp>

struct inner
{
	int a;
	int b;
	int c;
	int d;

	// NOTE: there is no own json_io!
};

struct outer
{
	inner x;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory(
				json_dto::inside_array::reader_writer<
						json_dto::inside_array::at_least<2>
				>(
					json_dto::inside_array::member( x.a ),
					json_dto::inside_array::member( x.b ),
					// Will receive 0 if missed.
					json_dto::inside_array::member( x.c ),
					// Will receive 44 if missed.
					json_dto::inside_array::member_with_default_value( x.d, 44 )
				), "x", x )
			;
	}
};

std::ostream & operator<<( std::ostream & to, const outer & what )
{
	return (to << what.x.a << ", " << what.x.b << ", " << what.x.c << ", " << what.x.d);
}

int
main( int , char *[] )
{
	try
	{
		const auto obj = json_dto::from_json<outer>(
				R"({"x":[1, 2]})" );
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

