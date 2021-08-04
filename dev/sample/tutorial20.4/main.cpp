#include <json_dto/pub.hpp>

namespace tutorial_20_4
{

using actual_type_t = unsigned short;
using storage_type_t = std::uint32_t;

static_assert(
		!std::is_same<actual_type_t, storage_type_t>::value,
		"actual_type_t and storage_type_t should be different types" );
static_assert(
		sizeof(actual_type_t) < sizeof(storage_type_t),
		"actual_type_t should be smaller than storage_type_t" );

} /* namespace tutorial_20_4 */

namespace json_dto
{

namespace tutorial_20_4_addition
{

template<
	typename Actual_Field_Type,
	typename Target_Field_Type,
	typename Reader_Writer,
	typename Manopt_Policy,
	typename Validator >
class proxy_binder_data_holder_t
{
public:
	using source_data_holder_t = binder_data_holder_t<
			Reader_Writer,
			Actual_Field_Type,
			Manopt_Policy,
			Validator
	>;

	using field_t = Target_Field_Type;

private:
	field_t & m_target_field;
	const source_data_holder_t & m_source;

public:
	proxy_binder_data_holder_t(
		field_t & target_field,
		const source_data_holder_t & source )
		:	m_target_field{ target_field }
		,	m_source{ source }
	{}

	const Reader_Writer &
	reader_writer() const noexcept { return m_source.reader_writer(); }

	const string_ref_t &
	field_name() const noexcept { return m_source.field_name(); }

	field_t &
	field_for_serialization() const noexcept { return m_target_field; }

	field_t &
	field_for_deserialization() const noexcept { return m_target_field; }

	const Manopt_Policy &
	manopt_policy() const noexcept { return m_source.manopt_policy(); }

	const Validator &
	validator() const noexcept { return m_source.validator(); }
};

} /* namespace tutorial_20_4_addition */

template<
	typename Reader_Writer,
	typename Manopt_Policy,
	typename Validator >
struct binder_read_from_implementation_t<
		binder_data_holder_t<
			Reader_Writer,
			::tutorial_20_4::actual_type_t,
			Manopt_Policy,
			Validator
		>
	>
{
	using data_holder_t = binder_data_holder_t<
			Reader_Writer,
			::tutorial_20_4::actual_type_t,
			Manopt_Policy,
			Validator >;

	using actual_data_holder_to_be_used_t =
			tutorial_20_4_addition::proxy_binder_data_holder_t<
					::tutorial_20_4::actual_type_t,
					::tutorial_20_4::storage_type_t,
					Reader_Writer,
					Manopt_Policy,
					Validator
			>;

	using actual_binder_read_from_implementation_t =
			binder_read_from_implementation_t<
					actual_data_holder_to_be_used_t
			>;

	static void
	read_from(
		const data_holder_t & binder_data,
		const rapidjson::Value & object )
	{
//std::cout << "read_from!" << std::endl;
		::tutorial_20_4::storage_type_t actual_value{};
		actual_data_holder_to_be_used_t actual_binder{ actual_value, binder_data };
		actual_binder_read_from_implementation_t::read_from(
				actual_binder,
				object
		);

		binder_data.field_for_deserialization() =
				static_cast<::tutorial_20_4::actual_type_t>(
						actual_value );
	}
};

template<
	typename Reader_Writer,
	typename Manopt_Policy,
	typename Validator >
struct binder_write_to_implementation_t<
		binder_data_holder_t<
			Reader_Writer,
			::tutorial_20_4::actual_type_t,
			Manopt_Policy,
			Validator
		>
	>
{
	using data_holder_t = binder_data_holder_t<
			Reader_Writer,
			::tutorial_20_4::actual_type_t,
			Manopt_Policy,
			Validator >;

	using actual_data_holder_to_be_used_t =
			tutorial_20_4_addition::proxy_binder_data_holder_t<
					::tutorial_20_4::actual_type_t,
					::tutorial_20_4::storage_type_t,
					Reader_Writer,
					Manopt_Policy,
					Validator
			>;

	using actual_binder_write_to_implementation_t =
			binder_write_to_implementation_t<
					actual_data_holder_to_be_used_t
			>;

	static void
	write_to(
		const data_holder_t & binder_data,
		rapidjson::Value & object,
		rapidjson::MemoryPoolAllocator<> & allocator )
	{
//std::cout << "write_to!" << std::endl;
		::tutorial_20_4::storage_type_t actual_value{
				binder_data.field_for_serialization()
		};

		actual_data_holder_to_be_used_t actual_binder{ actual_value, binder_data };

		actual_binder_write_to_implementation_t::write_to(
				actual_binder,
				object,
				allocator
		);
	}
};

} /* namespace json_dto */

struct example_data
{
	tutorial_20_4::actual_type_t m_payload{};
	tutorial_20_4::actual_type_t m_priority{};

	example_data() = default;
	example_data( tutorial_20_4::actual_type_t payload )
		: m_payload{ payload } {}

	template < typename Json_Io >
	void
	json_io( Json_Io & io )
	{
		io
			& json_dto::mandatory( "payload", m_payload )
			& json_dto::optional( "priority", m_priority, 0u )
			;
	}
};

std::ostream &
operator<<( std::ostream & to, const example_data & what )
{
	to << "(payload: " << what.m_payload << ", priority: "
			<< what.m_priority << ")";
	return to;
}

const std::string json_data{
R"JSON({
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
			const example_data data{ 42u };
			std::cout
				<< "\nSerialized to JSON:\n"
				<< json_dto::to_json( data ) << std::endl;
		}

		{
			example_data data{ 42u };
			data.m_priority = 15u;
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

