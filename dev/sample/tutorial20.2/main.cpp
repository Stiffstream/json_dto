/*
	A sample of creation own specialization of json_dto's
	binder_data_holder_t and binder_write_to_implementation.
*/

#include <cstdio>
#include <iostream>
#include <map>

#include <json_dto/pub.hpp>

namespace tutorial_20_2
{

template< typename F >
struct deserialize_only_proxy_t
{
	using field_type = F;

	F * m_field;
};

template< typename F >
deserialize_only_proxy_t<F> deserialize_only( F & field ) noexcept
{
	static_assert( !std::is_const<F>::value,
			"deserialize_only can't be used with const objects" );

	return { &field };
}

} /* namespace tutorial_20_2 */

namespace json_dto
{

template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
class binder_data_holder_t<
		Reader_Writer,
		const tutorial_20_2::deserialize_only_proxy_t<Field_Type>,
		Manopt_Policy,
		Validator >
	:	public binder_data_holder_t<
			Reader_Writer,
			typename tutorial_20_2::deserialize_only_proxy_t<Field_Type>::field_type,
			Manopt_Policy,
			Validator >
{
	using deserialize_only_proxy =
			tutorial_20_2::deserialize_only_proxy_t<Field_Type>;

	using actual_field_type = typename deserialize_only_proxy::field_type;

	using base_type = binder_data_holder_t<
			Reader_Writer,
			actual_field_type,
			Manopt_Policy,
			Validator >;

public:
	binder_data_holder_t(
		Reader_Writer && reader_writer,
		string_ref_t field_name,
		const deserialize_only_proxy & proxy,
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
struct binder_write_to_implementation_t<
		binder_data_holder_t<
			Reader_Writer,
			const tutorial_20_2::deserialize_only_proxy_t<Field_Type>,
			Manopt_Policy,
			Validator
		>
	>
{
	using data_holder_t = binder_data_holder_t<
			Reader_Writer,
			const tutorial_20_2::deserialize_only_proxy_t<Field_Type>,
			Manopt_Policy,
			Validator >;

	static void
	write_to(
		const data_holder_t & /*binder_data*/,
		rapidjson::Value & /*object*/,
		rapidjson::MemoryPoolAllocator<> & /*allocator*/ )
	{
		// Nothing to do.
	}

};

} /* namespace json_dto */

struct example_data
{
	std::uint32_t m_a{};
	std::uint32_t m_b{};
	std::uint32_t m_c{};

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "a",
					tutorial_20_2::deserialize_only( m_a ) )
			& json_dto::optional( "b",
					tutorial_20_2::deserialize_only( m_b ), 42u )
			& json_dto::optional_no_default( "c", m_c );
	}
};

std::ostream &
operator<<( std::ostream & to, const example_data & what )
{
	return (to << "(a: " << what.m_a << ", b: " << what.m_b
			<< ", c: " << what.m_c << ")");
}

int
main( int , char *[] )
{
	try
	{
		{
			const std::string json_data = R"json(
				{"a": 1, "b": 2}
			)json";

			auto data = json_dto::from_json< example_data >( json_data );

			std::cout << "Deserialized from JSON: " << data << std::endl;
			std::cout << "Serialized again: " << json_dto::to_json( data )
					<< std::endl;
		}

		{
			const std::string json_data = R"json(
				{"a": 1, "c": 2}
			)json";

			auto data = json_dto::from_json< example_data >( json_data );

			std::cout << "Deserialized from JSON: " << data << std::endl;
			std::cout << "Serialized again: " << json_dto::to_json( data )
					<< std::endl;
		}

		{
			const std::string json_data = R"json(
				{"b": 1, "c": 2}
			)json";

			try
			{
				auto data = json_dto::from_json< example_data >( json_data );

				std::cout << "We shouldn't be here, but: " << data << std::endl;
			}
			catch( const std::exception & x )
			{
				std::cout << "expected exception: " << x.what() << std::endl;
			}
		}

		{
			example_data data;
			data.m_a = 111u;
			data.m_b = 222u;
			data.m_c = 333u;

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

