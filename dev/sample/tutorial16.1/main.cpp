/*
	An example of extracting DTO values from already parsed document.
	It also shows to_json/from_json functions that accept reader-writer
	object.
*/

#include <iostream>
#include <string>
#include <ctime>

#include <json_dto/pub.hpp>

namespace tutorial_16_1
{

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

// Additional information.
struct extension_t
{
	std::string m_id;
	std::string m_payload;

	extension_t() = default;
	extension_t( std::string id, std::string payload )
		: m_id{ std::move(id) }
		, m_payload{ std::move(payload) }
	{}

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory("Id", m_id)
			& json_dto::mandatory("Payload", m_payload)
			;
	}
};

std::ostream & operator<<( std::ostream & to, const extension_t & ext )
{
	return (to << "('" << ext.m_id << "','" << ext.m_payload << "')");
}

std::ostream & operator<<( std::ostream & to, const std::vector<extension_t> & exts )
{
	bool comma_needed = false;
	for( const auto & e : exts )
	{
		if( comma_needed )
			to << ", ";
		to << e;
		comma_needed = true;
	}

	return to;
}

// Reader_Writer for vector of extension_t objects.
struct extension_reader_writer_t
{
	void
	read( std::vector< extension_t > & to, const rapidjson::Value & from ) const
	{
		using json_dto::read_json_value;

		to.clear();

		if( from.IsObject() )
		{
			extension_t single_value;
			read_json_value( single_value, from );
			to.push_back( std::move(single_value) );
		}
		else if( from.IsArray() )
		{
			read_json_value( to, from );
		}
		else
		{
			throw std::runtime_error{ "Unexpected format of extension_t value" };
		}
	}

	void
	write(
		const std::vector< extension_t > & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		using json_dto::write_json_value;
		if( 1u == v.size() )
			write_json_value( v.front(), to, allocator );
		else
			write_json_value( v, to, allocator );
	}
};

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

	if( whole_msg.HasMember( "extension" ) )
	{
		const auto extensions =
				json_dto::from_json< std::vector< extension_t > >(
						extension_reader_writer_t{},
						whole_msg[ "extension" ] );
		std::cout << "Extensions: " << extensions << std::endl;
	}
	else
		std::cout << "No extensions" << std::endl;

	std::cout << "--- completed ---" << std::endl;
}

} /* namespace tutorial_16_1 */

using namespace tutorial_16_1;

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
					"message_type" : "Read-Sensor",
					"payload" : {
						"sensor_id" : "a43-25",
						"priority" : 0,
						"topic" : "/meters/a/43/25"
					},
					"extension" : {"Id": "1", "Payload": "0000-0000-0001"}
				})JSON" );
		parse_and_handle_message(
				R"JSON({
					"message_type" : "Read-Sensor",
					"payload" : {
						"sensor_id" : "a43-25",
						"priority" : 0,
						"topic" : "/meters/a/43/25"
					},
					"extension" : [
						{"Id": "1", "Payload": "0000-0000-0001"},
						{"Id": "2", "Payload": "0000-0000-0002"}
					]
				})JSON" );

		parse_and_handle_message(
				R"JSON({
					"message_type" : "Update-Period",
					"payload" : {
						"units" : "ms",
						"period" : 350
					}
				})JSON" );

		parse_and_handle_message(
				R"JSON({
					"message_type" : "Update-Period",
					"payload" : {
						"units" : "ms",
						"period" : 350
					},
					"extension" : [
						{"Id": "3", "Payload": "0000-0000-0003"},
						{"Id": "4", "Payload": "0000-0000-0004"},
						{"Id": "5", "Payload": "0000-0000-0005"}
					]

				})JSON" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

