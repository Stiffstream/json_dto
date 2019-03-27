/*
	A sample using json_dto
*/

#include <iostream>
#include <string>

#include <forward_list>
#include <list>
#include <map>
#include <unordered_map>

#include <tuple>

#include <json_dto/pub.hpp>

struct property_info_t
{
	// Is this property mandatory?
	bool m_mandatory;

	// Priority of this property.
	int m_priority;

	// Default value.
	std::string m_default;

	// Actual value.
	std::string m_value;

	friend bool
	operator==( const property_info_t & a, const property_info_t & b )
	{
		const auto tie = [](const auto & v) {
			return std::tie(v.m_mandatory, v.m_priority, v.m_default, v.m_value);
		};	

		return tie(a) == tie(b);
	}

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::optional( "mandatory", m_mandatory, false )
			& json_dto::mandatory( "priority", m_priority )
			& json_dto::optional( "default", m_default, std::string{} )
			& json_dto::mandatory( "value", m_value )
			;
	}
};

std::ostream &
operator<<( std::ostream & to, const property_info_t & info )
{
	return (to << "(" << (info.m_mandatory ? "mandatory" : "optional")
			<< ", p=" << info.m_priority << ", defaults='"
			<< info.m_default << "', value='" << info.m_value << "')");
}

struct message_t
{
	// Title of the message.
	std::string m_title;

	// Body of the message.
	std::string m_body;

	// List of hash-tags.
	std::list< std::string > m_hash_tags;

	// Map of message properties.
	std::map< std::string, property_info_t > m_properties;

	message_t() = default;

	template< typename Json_Io >
	void json_io( Json_Io & io )
	{
		io & json_dto::mandatory( "title", m_title )
			& json_dto::mandatory( "body", m_body )
			& json_dto::optional( "hash-tags", m_hash_tags,
					decltype(m_hash_tags){} )
			& json_dto::optional( "properties", m_properties,
					decltype(m_properties){} )
			;
	}
};

std::ostream &
operator<<( std::ostream & to, const message_t & what )
{
	to << "'" << what.m_title << "'\n";
	to << "==================================\n";
	to << what.m_body << "\n";
	to << "----------------------------------\n";
	if( !what.m_hash_tags.empty() )
	{
		for( const auto & v : what.m_hash_tags )
			to << "#" << v << ",";
		to << "\n";
	}
	if( !what.m_properties.empty() )
	{
		to << "----------------------------------\n";
		for( const auto & kv : what.m_properties )
			to << kv.first << " => " << kv.second << "\n";
	}
	to << "--- end ---\n\n";

	return to;
}

const std::string json_data{
R"JSON(
[
  {
    "title" : "Hello!",
    "body" : "Just a simple text",
	 "hash-tags" : ["demo"]
  },
  {
    "title" : "Bye!",
	 "body" : "Just an another simple text",
	 "properties" : {
	   "importance" : { "mandatory" : true, "priority" : 1, "value" : "high" },
		"access-control" : { "priority" : 0, "default" : "NONE", "value" : "simple" }
	 }
  },
  {
    "title" : "Welcome!",
	 "body" : "This is a demo for json_dto-0.2.8",
	 "hash-tags" : ["demo", "json_dto", "rapidjson", "moderncpp"],
	 "properties" : {
	   "importance" : { "mandatory" : true, "priority" : 1, "value" : "high" },
	   "difficulty" : {
		  "mandatory" : false,
		  "priority" : 10,
		  "default" : "high",
		  "value" : "low"
		}
	 }
  }
])JSON" };

int
main( int , char *[] )
{
	try
	{
		// Load all messages to signle-linked list.
		auto messages = json_dto::from_json<
				std::forward_list<message_t> >( json_data );

		std::cout << "Loaded messages:\n" << std::endl;
		for( const auto & m : messages )
			std::cout << m;

		std::cout << std::endl;

		// Transform messages from one container to another.
		std::unordered_map< std::string, message_t > tagged_messages;
		auto it = messages.begin();

		tagged_messages[ "hello" ] = *(it++);
		tagged_messages[ "bye" ] = *(it++);
		tagged_messages[ "welcome" ] = *it;

		// Serialize the tagged messages.
		std::cout << "Serialized tagged messages:\n"
				<< json_dto::to_json( tagged_messages )
				<< std::endl;
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

