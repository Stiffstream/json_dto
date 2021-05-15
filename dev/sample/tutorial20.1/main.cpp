/*
	A sample of creation own specialization of json_dto's
	binder_data_holder_t and binder_read_from_implementation.
*/

#include <cstdio>
#include <iostream>
#include <map>

#include <json_dto/pub.hpp>

namespace tutorial_20_1
{

template< typename F >
struct serialize_only_proxy_t
{
	using field_type = const F;

	const F * m_field;
};

template< typename F >
serialize_only_proxy_t<F> serialize_only( const F & field ) noexcept
{
	return { &field };
}

} /* namespace tutorial_20_1 */

namespace json_dto
{

template<
		typename Reader_Writer,
		typename Field_Type,
		typename Manopt_Policy,
		typename Validator >
class binder_data_holder_t<
		Reader_Writer,
		const tutorial_20_1::serialize_only_proxy_t<Field_Type>,
		Manopt_Policy,
		Validator >
	:	public binder_data_holder_t<
			Reader_Writer,
			typename tutorial_20_1::serialize_only_proxy_t<Field_Type>::field_type,
			Manopt_Policy,
			Validator >
{
	using serialize_only_proxy =
			tutorial_20_1::serialize_only_proxy_t<Field_Type>;

	using actual_field_type = typename serialize_only_proxy::field_type;

	using base_type = binder_data_holder_t<
			Reader_Writer,
			actual_field_type,
			Manopt_Policy,
			Validator >;

public:
	binder_data_holder_t(
		Reader_Writer && reader_writer,
		string_ref_t field_name,
		const serialize_only_proxy & proxy,
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
		Reader_Writer,
		const tutorial_20_1::serialize_only_proxy_t<Field_Type>,
		Manopt_Policy,
		Validator >
{
	using data_holder_t = binder_data_holder_t<
			Reader_Writer,
			const tutorial_20_1::serialize_only_proxy_t<Field_Type>,
			Manopt_Policy,
			Validator >;

	static void
	read_from(
		const data_holder_t & /*binder_data*/,
		const rapidjson::Value & /*object*/ )
	{
		// Nothing to do.
	}

};

} /* namespace json_dto */

struct example_data
{
	std::vector< std::uint32_t > ids() const { return { 1u, 2u, 3u, 4u }; }
	std::uint32_t m_payload{};

	example_data() = default;
	example_data( std::uint32_t payload ) : m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "ids",
					tutorial_20_1::serialize_only( ids() ) )
			& json_dto::mandatory( "payload", m_payload );
	}
};

std::ostream &
operator<<( std::ostream & to, const example_data & what )
{
	to << "(ids: (";
	const auto & ids = what.ids();
	for( const auto id : ids ) to << id << " ";
	to << "), payload: " << what.m_payload << ")";
	return to;
}

const std::string json_data{
R"JSON({
  "ids" : [0, 2, 4, 8, 16, 32],
  "payload" : 88
})JSON" };

int
main( int , char *[] )
{
	try
	{
		{
			auto data = json_dto::from_json< example_data >( json_data );

			std::cout << "Deserialized from JSON: " << data << std::endl;
			std::cout << "Serialized again: " << json_dto::to_json( data )
					<< std::endl;
		}

		{
			const example_data data{ 42 };
			std::cout
				<< "\nSerialized to JSON:\n"
				<< json_dto::to_json( data ) << std::endl;
		}
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << "." << std::endl;
		return 1;
	}

	return 0;
}

