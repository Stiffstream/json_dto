/*
	json_dto
*/

/*!
	Helper lib to deal with DTO packed in json.
*/

#pragma once

#include <rapidjson/document.h>
#include <rapidjson/error/error.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

#include <cstdint>
#include <vector>
#include <memory>
#include <limits>
#include <type_traits>

#if defined( __has_include )
	//
	// Check for std::optional or std::experimental::optional
	//
	#define JSON_DTO_CHECK_FOR_STD_OPTIONAL
	#if defined( _MSC_VER ) && !_HAS_CXX17
		// Visual C++ 14.* allows to include <optional> only in c++17 mode.
		#undef JSON_DTO_CHECK_FOR_STD_OPTIONAL
	#endif

	#if defined( JSON_DTO_CHECK_FOR_STD_OPTIONAL )
		#if __has_include(<experimental/optional>)
			#include <experimental/optional>
			#define JSON_DTO_HAS_EXPERIMENTAL_OPTIONAL
		#elif __has_include(<optional>)
			#include <optional>
			#define JSON_DTO_HAS_STD_OPTIONAL
		#endif
		#if defined(JSON_DTO_HAS_STD_OPTIONAL) || \
				defined(JSON_DTO_HAS_EXPERIMENTAL_OPTIONAL)
			#define JSON_DTO_SUPPORTS_STD_OPTIONAL
		#endif
	#endif
#endif

namespace json_dto
{

namespace cpp17
{
#if defined(JSON_DTO_SUPPORTS_STD_OPTIONAL)
	#if defined(JSON_DTO_HAS_STD_OPTIONAL)
		template<typename T>
		using optional = std::optional<T>;
		inline constexpr auto nullopt() { return std::nullopt; }
	#elif defined(JSON_DTO_HAS_EXPERIMENTAL_OPTIONAL)
		template<typename T>
		using optional = std::experimental::optional<T>;
		inline constexpr auto nullopt() { return std::experimental::nullopt; }
	#endif
#endif
} /* namespace cpp17 */

//
// ex_t
//

//! Errors reading json data.
class ex_t
	:	public std::runtime_error
{
		typedef std::runtime_error base_type_t;

	public:
		ex_t( const std::string & error_desc )
			:	base_type_t{ error_desc }
		{}
};


//
// json_input_t
//

//! Input object for building DTO out of JSON.
class json_input_t
{
	public:
		json_input_t( const rapidjson::Value & object )
			:	m_object{ object }
		{}

		template< typename Binder >
		json_input_t &
		operator & ( const Binder & b )
		{
			b.read_from( m_object );
			return *this;
		}

	private:
		const rapidjson::Value & m_object;
};

//
// json_output_t
//

//! Input object for building JSON out of DTO.
class json_output_t
{
	public:
		json_output_t(
			rapidjson::Value & object,
			rapidjson::MemoryPoolAllocator<> & allocator )
			:	m_object{ object }
			,	m_allocator{ allocator }
		{
			m_object.SetObject();
		}

		template< typename Binder >
		json_output_t &
		operator & ( const Binder & b )
		{
			b.write_to( m_object, m_allocator );
			return *this;
		}

	private:
		rapidjson::Value & m_object;
		rapidjson::MemoryPoolAllocator<> & m_allocator;
};

//
// reader functions.
//

#define RW_JSON_VALUES( type, checker, getter, setter ) \
inline void \
read_json_value( type & v, const rapidjson::Value & object ) \
{ \
	if( object. checker () ) \
		v = object. getter (); \
	else \
		throw ex_t{ "value is not " #type }; \
} \
inline void \
write_json_value( type v, rapidjson::Value & object, rapidjson::MemoryPoolAllocator<> & ) \
{\
	object. setter ( v ); \
}

//
// BOOL
//

RW_JSON_VALUES( bool, IsBool, GetBool, SetBool )

//
// NUMBER
//

RW_JSON_VALUES( std::uint32_t, IsUint, GetUint, SetUint )
RW_JSON_VALUES( std::int32_t, IsInt, GetInt, SetInt )

RW_JSON_VALUES( std::uint64_t, IsUint64, GetUint64, SetUint64 )
RW_JSON_VALUES( std::int64_t, IsInt64, GetInt64, SetInt64 )

RW_JSON_VALUES( float, IsNumber, GetFloat, SetFloat )
RW_JSON_VALUES( double, IsNumber, GetDouble, SetDouble )

//
// uint16
//

inline void
read_json_value(
	std::uint16_t & v,
	const rapidjson::Value & object )
{
	std::uint32_t value;
	read_json_value( value, object );

	if( value <= std::numeric_limits< std::uint16_t >::max() )
		v = std::uint16_t( value );
	else
		throw ex_t{ "value is out of uint16: " + std::to_string( value ) };
}

inline void
write_json_value(
	std::uint16_t v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	const std::uint32_t value = v;
	write_json_value( value, object, allocator );
}

//
// int16
//

inline void
read_json_value( std::int16_t & v, const rapidjson::Value & object )
{
	std::int32_t value;
	read_json_value( value, object );

	if( value <= std::numeric_limits< std::int16_t >::max() &&
		value >= std::numeric_limits< std::int16_t >::min() )
		v = std::int16_t( value );
	else
		throw ex_t{ "value is out of int16: " + std::to_string( value ) };
}

inline void
write_json_value(
	std::int16_t v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	const std::int32_t value = v;
	write_json_value( value, object, allocator );
}

//
// STRING
//

inline void
read_json_value( std::string & s, const rapidjson::Value & object )
{
	if( object.IsString() )
		s = object.GetString();
	else
		throw ex_t{ "value is not std::string" };
}

inline void
write_json_value(
	const std::string & s,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	object.SetString( s, allocator );
}

//
// JSON
//

inline void
read_json_value( rapidjson::Document & d, const rapidjson::Value & object )
{
	d.CopyFrom( object, d.GetAllocator() );
}

inline void
write_json_value(
	const rapidjson::Document & d,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	object.CopyFrom( d, allocator );
}

#if defined( JSON_DTO_SUPPORTS_STD_OPTIONAL )
//
// std::optional
//
template< typename T >
inline void
read_json_value(
	cpp17::optional<T> & v,
	const rapidjson::Value & object );

template< typename T >
inline void
write_json_value(
	const cpp17::optional<T> & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator );
#endif

//
// ARRAY
//

template< typename T, typename A >
void
read_json_value(
	std::vector< T, A > & vec,
	const rapidjson::Value & object );

template< typename T, typename A >
void
write_json_value(
	const std::vector< T, A > & vec,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator );

//
// nullable_t
//

//! A wrapper for nullable fields.
template< typename Field_Type >
struct nullable_t
{
	nullable_t()
		:	m_has_value{ false }
	{}

	nullable_t( std::nullptr_t )
		:	m_has_value{ false }
	{}

	explicit nullable_t( Field_Type value )
		:	m_has_value{ true }
	{
		new( m_image_space ) Field_Type{ std::move( value ) };
	}

	nullable_t( const nullable_t & other )
		:	m_has_value{ other.m_has_value }
	{
		if( has_value() )
			new( m_image_space ) Field_Type{ other.field_ref() };
	}

	nullable_t( nullable_t && other )
		:	m_has_value{ other.m_has_value }
	{
		if( has_value() )
			new( m_image_space ) Field_Type{ std::move( other.field_ref() ) };
	}

	template< typename... Args >
	explicit nullable_t( Args &&... args )
		:	m_has_value{ true }
	{
		new( m_image_space ) Field_Type{ std::forward< Args >( args )... };
	}

	~nullable_t()
	{
		reset();
	}

	bool
	has_value() const
	{
		return m_has_value;
	}

	operator bool () const
	{
		return has_value();
	}

	void
	swap( nullable_t & other )
	{
		if( m_has_value && other.m_has_value )
		{
			std::swap( field_ref(), other.field_ref() );
		}
		else if( !m_has_value && other.m_has_value )
		{
			new( m_image_space ) Field_Type{ std::move( other.field_ref() ) };
			m_has_value = true;
			other.reset();
		}
		else if( m_has_value && !other.m_has_value )
		{
			other.swap( *this );
		}
		// Both objects has no value: do nothing.
	}

	nullable_t &
	operator = ( const nullable_t & other )
	{
		nullable_t temp{ other };
		swap( temp );

		return *this;
	}

	nullable_t &
	operator = ( nullable_t && other )
	{
		nullable_t temp{ std::move( other ) };
		swap( temp );

		return *this;
	}

	nullable_t &
	operator = ( const Field_Type & value )
	{
		nullable_t temp{ value };
		swap( temp );

		return *this;
	}

	nullable_t &
	operator = ( Field_Type && value )
	{
		nullable_t temp{ std::move( value ) };
		swap( temp );

		return *this;
	}

	nullable_t &
	operator = ( std::nullptr_t )
	{
		reset();
		return *this;
	}

	const Field_Type*
	operator -> () const
	{
		return field_ptr();
	}

	Field_Type*
	operator -> ()
	{
		return field_ptr();
	}

	const Field_Type &
	operator * () const
	{
		return field_ref();
	}

	Field_Type&
	operator * ()
	{
		return field_ref();
	}

	bool
	operator == ( const nullable_t & other ) const
	{
		return
			has_value() &&
			other.has_value() &&
			field_ref() == other.field_ref();
	}

	void
	emplace()
	{
		*this = Field_Type{};
	}

	void
	emplace( Field_Type value )
	{
		*this = std::move( value );
	}

	void
	reset()
	{
		if( has_value() )
		{
			m_has_value = false;
			field_ref().~Field_Type();
		}
	}

	private:
		alignas( alignof( Field_Type ) ) char m_image_space[ sizeof( Field_Type ) ];
		bool m_has_value{ false };

		Field_Type *
		field_ptr()
		{
			return reinterpret_cast< Field_Type * >( m_image_space );
		}

		const Field_Type *
		field_ptr() const
		{
			return reinterpret_cast< const Field_Type * >( m_image_space );
		}

		Field_Type &
		field_ref()
		{
			return *field_ptr();
		}

		const Field_Type &
		field_ref() const
		{
			return *field_ptr();
		}
};

//
// json_io
//

//! Standard io for DTO objects.
/*!
	It is possible to implement specifications for a concrete DTO type.
	For example it allows to write non intrusive json_io adapters.
*/
template< typename Io, typename Dto >
void
json_io( Io & io, Dto & dto )
{
	dto.json_io( io );
}

//
// Nested DTO helpers.
//

template< typename Dto >
void
read_json_value(
	Dto & v,
	const rapidjson::Value & object )
{
	json_input_t input( object );
	json_io( input, v );
}

template< typename Dto >
void
write_json_value(
	const Dto & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	json_output_t ouput( object, allocator );
	json_io( ouput, const_cast< Dto & >( v ) );
}

//
// RW specializations for nullable_t< T >
//

template< typename Field_Type >
void
read_json_value(
	nullable_t< Field_Type > & f,
	const rapidjson::Value & object )
{
	Field_Type value;
	read_json_value( value, object );
	f = std::move( value );
}

template< typename Field_Type >
void
write_json_value(
	nullable_t< Field_Type > & f,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	if( f )
		write_json_value( *f, object, allocator );
	else
		object.SetNull();
}

#if defined( JSON_DTO_SUPPORTS_STD_OPTIONAL )
//
// std::optional
//
template< typename T >
inline void
read_json_value(
	cpp17::optional<T> & v,
	const rapidjson::Value & object )
{
	T value_from_stream;
	read_json_value( value_from_stream, object );
	v = std::move(value_from_stream);
}

template< typename T >
inline void
write_json_value(
	const cpp17::optional<T> & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	if( v )
		write_json_value( *v, object, allocator );
}
#endif

//
// ARRAY
//

template< typename T, typename A  >
void
read_json_value(
	std::vector< T, A > & vec,
	const rapidjson::Value & object )
{
	if( object.IsArray() )
	{
		vec.clear();
		vec.reserve( object.Size() );
		for( rapidjson::SizeType i = 0; i < object.Size(); ++i )
		{
			T v;
			read_json_value( v, object[ i ] );
			vec.push_back( v );
		}
	}
	else
		throw ex_t{ "value is not an array" };
}

template< typename T, typename A  >
void
write_json_value(
	const std::vector< T, A > & vec,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	object.SetArray();
	for( const auto & v : vec )
	{
		rapidjson::Value o;
		write_json_value( v, o, allocator );
		object.PushBack( o, allocator );
	}
}

//
// Funcs for handling nullable property.
//

template< typename Field_Type >
void
set_value_null_attr( Field_Type & )
{
	throw ex_t{ "non nullable field is null" };
}

template< typename Field_Type >
void
set_value_null_attr( nullable_t< Field_Type > & f )
{
	f.reset();
}

template< typename Field_Type, typename Field_Default_Value_Type >
void
set_default_value( Field_Type & f, Field_Default_Value_Type && default_value )
{
	f = std::move( default_value );
}

template< typename Field_Type, typename Field_Default_Value_Type >
void
set_default_value(
	nullable_t< Field_Type > & f,
	Field_Default_Value_Type && default_value )
{
	f.emplace( std::move( default_value ) );
}

//
// mandtatory_attr_t
//

//! Field set/notset attribute ckecker for mandatory case.
struct mandatory_attr_t
{
	template< typename Field_Type >
	void
	on_field_not_defined( Field_Type & ) const
	{
		throw ex_t{ "mandatory field doesn't exist" };
	}

	template< typename Field_Type >
	constexpr bool
	is_default_value( Field_Type & ) const
	{
		return false;
	}
};

//
// optional_attr_t
//

//! Field set/notset attribute ckecker for optional case with default value.
template< typename Field_Default_Value_Type >
struct optional_attr_t
{
	optional_attr_t( Field_Default_Value_Type default_value )
		:	m_default_value{ std::move( default_value ) }
	{}

	template< typename Field_Type >
	void
	on_field_not_defined( Field_Type & f ) const
	{
		set_default_value( f, std::move( m_default_value ) );
	}

	template< typename Field_Type >
	bool
	is_default_value( nullable_t< Field_Type > & f ) const
	{
		return f && *f == m_default_value;
	}

	template< typename Field_Type >
	bool
	is_default_value( Field_Type & f ) const
	{
		return f == m_default_value;
	}

	Field_Default_Value_Type m_default_value;
};

//
// optional_attr_null_t
//

//! Field set/notset attribute ckecker for optional case with default value.
struct optional_attr_null_t
{
	template< typename Field_Type >
	void
	on_field_not_defined( nullable_t< Field_Type > & f ) const
	{
		f.reset();
	}

	template< typename Field_Type >
	bool
	is_default_value( nullable_t< Field_Type > & f ) const
	{
		return !f.has_value();
	}
};

//
// optional_nodefault_attr_t
//

//! Field set/notset attribute ckecker for optional case without default value.
struct optional_nodefault_attr_t
{
	template< typename Field_Type >
	constexpr void
	on_field_not_defined( Field_Type & ) const
	{}

	template< typename Field_Type >
	constexpr bool
	is_default_value( Field_Type & ) const
	{
		return false;
	}
};

using string_ref_t = rapidjson::Value::StringRefType;

//
// empty_validator_t
//

struct empty_validator_t
{
	template< typename Field_Type >
	constexpr void
	operator () ( const Field_Type & ) const
	{}
};

//
// binder_t
//

//! JSON IO binder_t for a field.
template<
		typename Field_Type,
		typename Manopt_Policy,
		typename Validator >
class binder_t
{
	public:
		binder_t(
			string_ref_t field_name,
			Field_Type & field,
			Manopt_Policy && manopt_policy,
			Validator && validator )
			:	m_field_name{ field_name }
			,	m_field{ field }
			,	m_manopt_policy{ std::move( manopt_policy ) }
			,	m_validator{ std::move( validator ) }
		{}

		//! Run read operation for value.
		void
		read_from( const rapidjson::Value & object ) const
		{
			try
			{
				read_from_impl( object );
			}
			catch( const std::exception & ex )
			{
				throw ex_t{
					"error reading field \"" + std::string{ m_field_name.s } + "\": " +
						ex.what() };
			}
		}

		//! Run write operation on object.
		void
		write_to(
			rapidjson::Value & object,
			rapidjson::MemoryPoolAllocator<> & allocator ) const
		{
			try
			{
				write_to_impl(
					object,
					allocator );
			}
			catch( const std::exception & ex )
			{
				throw ex_t{
					"error writing field \"" + std::string{ m_field_name.s } + "\": " +
						ex.what() };
			}
		}

	private:
		void
		read_from_impl(
			const rapidjson::Value & object ) const
		{
			const auto it = object.FindMember( m_field_name );

			if( object.MemberEnd() != it )
			{
				const auto & value = it->value;

				if( !value.IsNull() )
				{
					read_json_value( m_field, value );
				}
				else
				{
					set_value_null_attr( m_field );
				}
			}
			else
			{
				m_manopt_policy.on_field_not_defined( m_field );
			}

			m_validator( m_field ); // validate value.
		}

		void
		write_to_impl(
			rapidjson::Value & object,
			rapidjson::MemoryPoolAllocator<> & allocator ) const
		{
			m_validator( m_field ); // validate value.

			if( !m_manopt_policy.is_default_value( m_field ) )
			{
				rapidjson::Value value;

				write_json_value( m_field, value, allocator );

				object.AddMember(
					m_field_name,
					value,
					allocator );
			}
		}

		string_ref_t m_field_name;
		Field_Type & m_field;
		Manopt_Policy m_manopt_policy;
		Validator m_validator;
};

//
// mandatory
//

//! Create bind for a mandatory JSON field with validator.
template<
		typename Field_Type,
		typename Validator = empty_validator_t >
auto
mandatory(
	string_ref_t field_name,
	Field_Type & field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t< Field_Type, mandatory_attr_t, Validator >;
	return
		binder_type_t{
			field_name,
			field,
			mandatory_attr_t{},
			std::move( validator ) };
}

//
// optional
//

//! Create bind for an optional JSON field with default value and validator.
template<
		typename Field_Type,
		typename Field_Default_Value_Type,
		typename Validator = empty_validator_t >
auto
optional(
	string_ref_t field_name,
	Field_Type & field,
	Field_Default_Value_Type default_value,
	Validator validator = Validator{} )
{
	using opt_attr_t = optional_attr_t< Field_Default_Value_Type >;
	using binder_type_t = binder_t< Field_Type, opt_attr_t, Validator >;

	return
		binder_type_t{
			field_name,
			field,
			opt_attr_t{ std::move( default_value ) },
			std::move( validator ) };
}

//
// optional_null
//

//! Create bind for an optional JSON field with null default value .
template<
		typename Field_Type,
		typename Validator = empty_validator_t >
auto
optional_null(
	string_ref_t field_name,
	Field_Type & field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t< Field_Type, optional_attr_null_t, Validator >;
	return
		binder_type_t{
			field_name, field, optional_attr_null_t{}, std::move( validator ) };
}

//
// optional
//

//! Create bind for an optional JSON field with null default value.
template<
		typename Field_Type,
		typename Validator = empty_validator_t >
auto
optional(
	string_ref_t field_name,
	Field_Type & field,
	std::nullptr_t,
	Validator validator = Validator{} )
{
	return optional_null( field_name, field, std::move( validator ) );
}

//
// optional_no_default
//

//! Create bind for an optional JSON field without default value.
template<
		typename Field_Type,
		typename Validator = empty_validator_t >
auto
optional_no_default(
	string_ref_t field_name,
	Field_Type & field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t< Field_Type, optional_nodefault_attr_t, Validator >;
	return binder_type_t{
			field_name, field, optional_nodefault_attr_t{}, std::move( validator ) };
}

template< typename Dto >
json_input_t &
operator >> ( json_input_t & i, Dto & v )
{
	json_io( i, v );
	return i;
}

template< typename Dto >
inline json_output_t &
operator << ( json_output_t & o, const Dto & v )
{
	json_io( o, const_cast< Dto & >( v ) );
	return o;
}

//
// to_json
//

template< typename Dto >
std::string
to_json( const Dto & dto )
{
	rapidjson::Document output_doc;
	json_output_t jout{
		output_doc, output_doc.GetAllocator() };

	jout << dto;

	rapidjson::StringBuffer buffer;
	rapidjson::Writer< rapidjson::StringBuffer > writer( buffer );
	output_doc.Accept( writer );

	return buffer.GetString();
}

inline void
check_document_parse_status(
	const rapidjson::Document & document )
{
	if( document.HasParseError() )
	{
		throw ex_t{
			std::string{ "JSON parse error: '" } +
			rapidjson::GetParseError_En( document.GetParseError() ) +
			"' (offset: " + std::to_string( document.GetErrorOffset() ) + ")" };
	}
}

//
// from_json
//

//! Helper function to read DTO from json-string.
template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
Type
from_json( const std::string & json )
{
	rapidjson::Document document;
	json_input_t jin{ document };

	document.Parse< Rapidjson_Parseflags >( json.c_str() );

	check_document_parse_status( document );

	Type result;

	jin >> result;

	return result;
}

//! Helper function to read an already instantiated DTO.
template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
void
from_json( const std::string & json, Type & o )
{
	rapidjson::Document document;
	json_input_t jin{ document };

	document.Parse< Rapidjson_Parseflags >( json.c_str() );

	check_document_parse_status( document );

	jin >> o;
}

template< typename Type >
void
to_stream( std::ostream & to, const Type & type )
{
	rapidjson::Document output_doc;
	json_dto::json_output_t jout{
		output_doc, output_doc.GetAllocator() };

	jout << type;

	rapidjson::OStreamWrapper wrapper{ to };
	rapidjson::Writer< rapidjson::OStreamWrapper > writer{ wrapper };
	output_doc.Accept( writer );
}

template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
void
from_stream( std::istream & from, Type & o )
{
	rapidjson::IStreamWrapper wrapper{ from };

	rapidjson::Document document;
	json_dto::json_input_t jin{ document };

	document.ParseStream< Rapidjson_Parseflags >( wrapper );

	check_document_parse_status( document );

	jin >> o;
}

template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
Type
from_stream( std::istream & from )
{
	Type result;
	from_stream< Type, Rapidjson_Parseflags >( from, result );

	return result;
}

} /* namespace json_dto */

