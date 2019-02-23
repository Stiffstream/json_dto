/*
	An example of extracting DTO values from already parsed document.
*/

#include <iostream>
#include <string>
#include <ctime>

#include <json_dto/pub.hpp>

// Update period command.
struct update_period_t
{
	std::string m_units;
	int m_period;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "units", m_units )
			& json_dto::mandatory( "period", m_period );
	}
};
std::ostream & operator<<( std::ostream & to, const update_period_t & v )
{
	return (to << v.m_period << v.m_units);
}

// Read sensor command.
struct read_sensor_t
{
	std::string m_sensor_id;
	int m_priority;
	std::string m_publish_to;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "sensor_id", m_sensor_id )
			& json_dto::mandatory( "priority", m_priority )
			& json_dto::mandatory( "topic", m_publish_to );
	}
};
std::ostream & operator<<( std::ostream & to, const read_sensor_t & v )
{
	return (to << v.m_sensor_id << "(" << v.m_priority << ") -> "
			<< v.m_publish_to);
}

void parse_and_handle_message( const std::string & raw_msg )
{
	rapidjson::Document whole_msg;
	whole_msg.Parse< rapidjson::kParseDefaultFlags >( raw_msg );
	if( whole_msg.HasParseError() )
		throw std::runtime_error(
				std::string{ "unable to parse message: " } +
				rapidjson::GetParseError_En( whole_msg.GetParseError() ) );

	std::string msg_type = whole_msg[ "message_type" ].GetString();
	const auto & payload = whole_msg[ "payload" ];
	if( "Update-Period" == msg_type )
	{
		std::cout << "Update-Period command: "
				<< json_dto::from_json< update_period_t >( payload )
				<< std::endl;
	}
	else if( "Read-Sensor" == msg_type )
	{
		std::cout << "Read-Sensor command: "
				<< json_dto::from_json< read_sensor_t >( payload )
				<< std::endl;
	}
	else
		std::cout << "Unknown command: '" << msg_type << "'" << std::endl;
}

int
main( int , char *[] )
{
	try
	{
		parse_and_handle_message(
				R"JSON({
					"message_type" : "Read-Sensor",
					"payload" : {
						"sensor_id" : "a43-25",
						"priority" : 0,
						"topic" : "/meters/a/43/25"
					}
				})JSON" );

		parse_and_handle_message(
				R"JSON({
					"message_type" : "Update-Period",
					"payload" : {
						"units" : "ms",
						"period" : 350
					}
				})JSON" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

