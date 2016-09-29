#pragma once

#include <cmath>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

inline std::string
zip_json_str( const std::string & original )
{
	rapidjson::Document document;
	document.Parse( original.c_str() );

	rapidjson::StringBuffer buffer;
	rapidjson::Writer< rapidjson::StringBuffer > writer( buffer );
	document.Accept( writer );

	return buffer.GetString();
}

constexpr double EPSILON = 0.000001;

inline bool
equal( double a, double b )
{
	return fabs( a - b ) < EPSILON;
}
