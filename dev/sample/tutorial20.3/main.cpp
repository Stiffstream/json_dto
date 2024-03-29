/*
	A sample of creation own specialization of json_dto's
	binder_data_holder_t and binder_read_from_implementation.
*/

#include <cstdio>
#include <iostream>
#include <map>

#include <json_dto/pub.hpp>
#include <json_dto/validators.hpp>

namespace tutorial_20_3
{

template< typename F >
struct ignore_after_deserialization_proxy_t
{
	using field_type = const F;

	const F * m_field;
};

template< typename F >
ignore_after_deserialization_proxy_t<F>
ignore_after_deserialization( const F & field ) noexcept
{
	return { &field };
}

} /* namespace tutorial_20_3 */

namespace json_dto
{

template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
class binder_data_holder_t<
		Reader_Writer,
		const tutorial_20_3::ignore_after_deserialization_proxy_t<Field_Type>,
		Manopt_Policy,
		Validator >
	:	public binder_data_holder_t<
			Reader_Writer,
			typename tutorial_20_3::ignore_after_deserialization_proxy_t<Field_Type>::field_type,
			Manopt_Policy,
			Validator >
{
	using proxy_type =
			tutorial_20_3::ignore_after_deserialization_proxy_t<Field_Type>;

	using actual_field_type = typename proxy_type::field_type;

	using base_type = binder_data_holder_t<
			Reader_Writer,
			actual_field_type,
			Manopt_Policy,
			Validator >;

public:
	binder_data_holder_t(
		Reader_Writer && reader_writer,
		string_ref_t field_name,
		const proxy_type & proxy,
		Manopt_Policy && manopt_policy,
		Validator && validator )
		:	base_type{
				std::move(reader_writer),
				field_name,
				*(proxy.m_field),
				std::move(manopt_policy),
				std::move(validator)
			}
	{}
};

template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
struct binder_read_from_implementation_t<
		binder_data_holder_t<
			Reader_Writer,
			const tutorial_20_3::ignore_after_deserialization_proxy_t<Field_Type>,
			Manopt_Policy,
			Validator
		>
	>
{
	using proxy_type = tutorial_20_3::ignore_after_deserialization_proxy_t<Field_Type>;

	using data_holder_t = binder_data_holder_t<
			Reader_Writer,
			const proxy_type,
			Manopt_Policy,
			Validator >;

	static void
	read_from(
		const data_holder_t & binder_data,
		const rapidjson::Value & object )
	{
		if( !object.IsObject() )
		{
			throw ex_t{
				"unable to extract field \"" +
				std::string{ binder_data.field_name().s } + "\": "
				"parent json type must be object" };
		}

		// Temporary object for holding deserialized value.
		Field_Type tmp_object{};

		const auto it = object.FindMember( binder_data.field_name() );

		if( object.MemberEnd() != it )
		{
			const auto & value = it->value;

			if( !value.IsNull() )
			{
				binder_data.reader_writer().read( tmp_object, value );
			}
			else
			{
				binder_data.manopt_policy().on_null( tmp_object );
			}
		}
		else
		{
			binder_data.manopt_policy().on_field_not_defined( tmp_object );
		}

		binder_data.validator()( tmp_object ); // validate value.

		// NOTE: the value from tmp_object will be lost.
	}
};

} /* namespace json_dto */

struct example_data
{
	std::vector< std::uint32_t > ids() const { return { 1u, 2u, 3u, 4u }; }
	std::uint32_t m_payload{};
	std::uint32_t m_priority{};

	int m_version_base{ 18 };
	int version() const noexcept { return m_version_base + 2; }

	example_data() = default;
	example_data( std::uint32_t payload ) : m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "ids",
					tutorial_20_3::ignore_after_deserialization( ids() ) )
			& json_dto::mandatory( "payload", m_payload )
			& json_dto::mandatory( "priority",
					tutorial_20_3::ignore_after_deserialization( m_priority ),
					json_dto::min_max_constraint(
							std::uint32_t{0u},
							std::uint32_t{9u} ) )
			& json_dto::optional( "version",
					tutorial_20_3::ignore_after_deserialization( version() ),
					18 );
	}
};

std::ostream &
operator<<( std::ostream & to, const example_data & what )
{
	to << "(ids: (";
	const auto & ids = what.ids();
	for( const auto id : ids ) to << id << " ";
	to << "), payload: " << what.m_payload << ", priority: "
			<< what.m_priority << ", version: "
			<< what.version() << ")";
	return to;
}

int
main( int , char *[] )
{
	try
	{
		{
			const std::string json_data{
			R"JSON({
			  "ids" : [0, 2, 4, 8, 16, 32],
			  "payload" : 88,
			  "priority" : 3
			})JSON" };

			auto data = json_dto::from_json< example_data >( json_data );

			std::cout << "Deserialized from JSON: " << data << std::endl;
			std::cout << "Serialized again: " << json_dto::to_json( data )
					<< std::endl;
		}

		{
			const example_data data{ 42u };
			std::cout
				<< "\nSerialized to JSON:\n"
				<< json_dto::to_json( data ) << std::endl;
		}

		{
			example_data data{ 42u };
			data.m_priority = 15u;
			data.m_version_base = 16;

			std::cout << "\nTry to serialize object with invalid `priority` value"
					<< std::endl;
			try
			{
				const auto json_value = json_dto::to_json( data );
				std::cout << "\nSerialized to JSON:\n" << json_value << std::endl;
			}
			catch( const std::exception & x )
			{
				std::cout << "Expected exception: " << x.what() << std::endl;
			}
		}

		{
			std::cout << "\nTry to deserialize object with invalid `priority` value"
					<< std::endl;
			try
			{
				const std::string json_data{
				R"JSON({
				  "ids" : [0, 2, 4, 8, 16, 32],
				  "payload" : 88,
				  "priority" : 33
				})JSON" };

				auto data = json_dto::from_json< example_data >( json_data );

				std::cout << "Deserialized from JSON: " << data << std::endl;
			}
			catch( const std::exception & x )
			{
				std::cout << "Expected exception: " << x.what() << std::endl;
			}
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

