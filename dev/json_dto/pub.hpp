/*
	json_dto
*/

/*!
	Helper lib to deal with DTO packed in json.
*/

#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <limits>
#include <type_traits>

#include <rapidjson/document.h>
#include <rapidjson/error/error.h>
#include <rapidjson/error/en.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

namespace json_dto
{

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

		template < typename BINDER >
		json_input_t &
		operator & ( const BINDER & b )
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

		template < typename BINDER >
		json_output_t &
		operator & ( const BINDER & b )
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
read_json_value( const rapidjson::Value & object, type & v ) \
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
	const rapidjson::Value & object,
	std::uint16_t & v )
{
	std::uint32_t value;
	read_json_value( object, value );

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
read_json_value( const rapidjson::Value & object, std::int16_t & v )
{
	std::int32_t value;
	read_json_value( object, value );

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
read_json_value( const rapidjson::Value & object, std::string & s )
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
read_json_value( const rapidjson::Value & object, rapidjson::Document & d )
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

//
// ARRAY
//

template < typename T, typename A >
void
read_json_value(
	const rapidjson::Value & object,
	std::vector< T, A > & vec );

template < typename T, typename A >
void
write_json_value(
	const std::vector< T, A > & vec,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator );

//
// nullable_t
//

//! A wrapper for nullable fields.
template < typename FIELD_TYPE >
struct nullable_t
{
	nullable_t()
		:	m_has_value{ false }
	{}

	nullable_t( std::nullptr_t )
		:	m_has_value{ false }
	{}

	explicit nullable_t( FIELD_TYPE value )
		:	m_has_value{ true }
	{
		new( m_image_space ) FIELD_TYPE{ std::move( value ) };
	}

	nullable_t( const nullable_t & other )
		:	m_has_value{ other.m_has_value }
	{
		if( has_value() )
			new( m_image_space ) FIELD_TYPE{ other.field_ref() };
	}

	nullable_t( nullable_t && other )
		:	m_has_value{ other.m_has_value }
	{
		if( has_value() )
			new( m_image_space ) FIELD_TYPE{ std::move( other.field_ref() ) };
	}

	template < typename... ARGS >
	explicit nullable_t( ARGS &&... args )
		:	m_has_value{ true }
	{
		new( m_image_space ) FIELD_TYPE{ std::forward< ARGS >( args )... };
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
			new( m_image_space ) FIELD_TYPE{ std::move( other.field_ref() ) };
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
	operator = ( const FIELD_TYPE & value )
	{
		nullable_t temp{ value };
		swap( temp );

		return *this;
	}

	nullable_t &
	operator = ( FIELD_TYPE && value )
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

	const FIELD_TYPE*
	operator -> () const
	{
		return field_ptr();
	}

	FIELD_TYPE*
	operator -> ()
	{
		return field_ptr();
	}

	const FIELD_TYPE &
	operator * () const
	{
		return field_ref();
	}

	FIELD_TYPE&
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
		*this = FIELD_TYPE{};
	}

	void
	emplace( FIELD_TYPE value )
	{
		*this = std::move( value );
	}

	void
	reset()
	{
		if( has_value() )
		{
			m_has_value = false;
			field_ref().~FIELD_TYPE();
		}
	}

	private:
		alignas( alignof( FIELD_TYPE ) ) char m_image_space[ sizeof( FIELD_TYPE ) ];
		bool m_has_value{ false };

		FIELD_TYPE *
		field_ptr()
		{
			return reinterpret_cast< FIELD_TYPE * >( m_image_space );
		}

		const FIELD_TYPE *
		field_ptr() const
		{
			return reinterpret_cast< const FIELD_TYPE * >( m_image_space );
		}

		FIELD_TYPE &
		field_ref()
		{
			return *field_ptr();
		}

		const FIELD_TYPE &
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
template < typename IO, typename DTO >
void
json_io( IO & io, DTO & dto )
{
	dto.json_io( io );
}

//
// Nested DTO helpers.
//

template < typename DTO >
void
read_json_value(
	const rapidjson::Value & object,
	DTO & v )
{
	json_input_t input( object );
	json_io( input, v );
}

template < typename DTO >
void
write_json_value(
	const DTO & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	json_output_t ouput( object, allocator );
	json_io( ouput, const_cast< DTO & >( v ) );
}

//
// RW specializations for nullable_t< T >
//

template < typename FIELD_TYPE >
void
read_json_value(
	const rapidjson::Value & object,
	nullable_t< FIELD_TYPE > & f )
{
	FIELD_TYPE value;
	read_json_value( object, value );
	f = std::move( value );
}

template < typename FIELD_TYPE >
void
write_json_value(
	nullable_t< FIELD_TYPE > & f,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	if( f )
		write_json_value( *f, object, allocator );
	else
		object.SetNull();
}

//
// ARRAY
//

template < typename T, typename A  >
void
read_json_value(
	const rapidjson::Value & object,
	std::vector< T, A > & vec )
{
	if( object.IsArray() )
	{
		vec.clear();
		vec.reserve( object.Size() );
		for( rapidjson::SizeType i = 0; i < object.Size(); ++i )
		{
			T v;
			read_json_value( object[ i ], v );
			vec.push_back( v );
		}
	}
	else
		throw ex_t{ "value is not an array" };
}

template < typename T, typename A  >
void
write_json_value(
	const std::vector< T, A > & vec,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	object.SetArray();
	for( auto
			it = vec.cbegin(), it_end = vec.cend();
			it != it_end;
			++it )
	{
		rapidjson::Value o;
		write_json_value( *it, o, allocator );
		object.PushBack( o, allocator );
	}
}

//
// Funcs for handling nullable property.
//

template < typename FIELD_TYPE >
void
set_value_null_attr( FIELD_TYPE & )
{
	throw ex_t{ "non nullable field is null" };
}

template < typename FIELD_TYPE >
void
set_value_null_attr( nullable_t< FIELD_TYPE > & f )
{
	f.reset();
}

template < typename FIELD_TYPE, typename FIELD_DEFAULT_VALUE_TYPE >
void
set_default_value( FIELD_TYPE & f, FIELD_DEFAULT_VALUE_TYPE && default_value )
{
	f = std::move( default_value );
}

template < typename FIELD_TYPE, typename FIELD_DEFAULT_VALUE_TYPE >
void
set_default_value(
	nullable_t< FIELD_TYPE > & f,
	FIELD_DEFAULT_VALUE_TYPE && default_value )
{
	f.emplace( std::move( default_value ) );
}

//
// mandtatory_attr_t
//

//! Field set/notset attribute ckecker for mandatory case.
struct mandatory_attr_t
{
	template < typename FIELD_TYPE >
	void
	on_field_not_defined( FIELD_TYPE & ) const
	{
		throw ex_t{ "mandatory field doesn't exist" };
	}

	template < typename FIELD_TYPE >
	constexpr bool
	is_default_value( FIELD_TYPE & ) const
	{
		return false;
	}
};

//
// optional_attr_t
//

//! Field set/notset attribute ckecker for optional case with default value.
template < typename FIELD_DEFAULT_VALUE_TYPE >
struct optional_attr_t
{
	optional_attr_t( FIELD_DEFAULT_VALUE_TYPE default_value )
		:	m_default_value{ std::move( default_value ) }
	{}

	template < typename FIELD_TYPE >
	void
	on_field_not_defined( FIELD_TYPE & f ) const
	{
		set_default_value( f, std::move( m_default_value ) );
	}

	template < typename FIELD_TYPE >
	bool
	is_default_value( nullable_t< FIELD_TYPE > & f ) const
	{
		return f && *f == m_default_value;
	}

	template < typename FIELD_TYPE >
	bool
	is_default_value( FIELD_TYPE & f ) const
	{
		return f == m_default_value;
	}

	FIELD_DEFAULT_VALUE_TYPE m_default_value;
};

//
// optional_attr_null_t
//

//! Field set/notset attribute ckecker for optional case with default value.
struct optional_attr_null_t
{
	template < typename FIELD_TYPE >
	void
	on_field_not_defined( nullable_t< FIELD_TYPE > & f ) const
	{
		f.reset();
	}

	template < typename FIELD_TYPE >
	bool
	is_default_value( nullable_t< FIELD_TYPE > & f ) const
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
	template < typename FIELD_TYPE >
	constexpr void
	on_field_not_defined( FIELD_TYPE & ) const
	{}

	template < typename FIELD_TYPE >
	constexpr bool
	is_default_value( FIELD_TYPE & ) const
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
	template < typename FIELD_TYPE >
	constexpr void
	operator () ( const FIELD_TYPE & ) const
	{}
};

//
// binder_t
//

//! JSON IO binder_t for a field.
template <
		typename FIELD_TYPE,
		typename MANOPT_POLICY,
		typename VALIDATOR >
class binder_t
{
	public:
		binder_t(
			string_ref_t field_name,
			FIELD_TYPE & field,
			MANOPT_POLICY && manopt_policy,
			VALIDATOR && validator )
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
					json_dto::read_json_value( value, m_field );
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
		FIELD_TYPE & m_field;
		MANOPT_POLICY m_manopt_policy;
		VALIDATOR m_validator;
};

//
// mandatory
//

//! Create bind for a mandatory JSON field with validator.
template <
		typename FIELD_TYPE,
		typename VALIDATOR = empty_validator_t >
auto
mandatory(
	string_ref_t field_name,
	FIELD_TYPE & field,
	VALIDATOR validator = VALIDATOR{} )
{
	using binder_type_t = binder_t< FIELD_TYPE, mandatory_attr_t, VALIDATOR >;
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
template <
		typename FIELD_TYPE,
		typename FIELD_DEFAULT_VALUE_TYPE,
		typename VALIDATOR = empty_validator_t >
auto
optional(
	string_ref_t field_name,
	FIELD_TYPE & field,
	FIELD_DEFAULT_VALUE_TYPE default_value,
	VALIDATOR validator = VALIDATOR{} )
{
	using opt_attr_t = optional_attr_t< FIELD_DEFAULT_VALUE_TYPE >;
	using binder_type_t = binder_t< FIELD_TYPE, opt_attr_t, VALIDATOR >;

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
template <
		typename FIELD_TYPE,
		typename VALIDATOR = empty_validator_t >
auto
optional_null(
	string_ref_t field_name,
	FIELD_TYPE & field,
	VALIDATOR validator = VALIDATOR{} )
{
	using binder_type_t = binder_t< FIELD_TYPE, optional_attr_null_t, VALIDATOR >;
	return
		binder_type_t{
			field_name, field, optional_attr_null_t{}, std::move( validator ) };
}

//
// optional
//

//! Create bind for an optional JSON field with null default value.
template <
		typename FIELD_TYPE,
		typename VALIDATOR = empty_validator_t >
auto
optional(
	string_ref_t field_name,
	FIELD_TYPE & field,
	std::nullptr_t,
	VALIDATOR validator = VALIDATOR{} )
{
	return optional_null( field_name, field, std::move( validator ) );
}

//
// optional_no_default
//

//! Create bind for an optional JSON field without default value.
template <
		typename FIELD_TYPE,
		typename VALIDATOR = empty_validator_t >
auto
optional_no_default(
	string_ref_t field_name,
	FIELD_TYPE & field,
	VALIDATOR validator = VALIDATOR{} )
{
	using binder_type_t = binder_t< FIELD_TYPE, optional_nodefault_attr_t, VALIDATOR >;
	return
		binder_type_t{
			field_name, field, optional_nodefault_attr_t{}, std::move( validator ) };
}

template < typename DTO >
json_input_t &
operator >> ( json_input_t & i, DTO & v )
{
	json_io( i, v );
	return i;
}

template < typename DTO >
inline json_output_t &
operator << ( json_output_t & o, const DTO & v )
{
	json_io( o, const_cast< DTO & >( v ) );
	return o;
}

//
// to_json
//

template < typename DTO >
std::string
to_json( const DTO & dto )
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
template < typename TYPE, unsigned RAPIDJSON_PARSEFLAGS = rapidjson::kParseDefaultFlags >
TYPE
from_json( const std::string & json )
{
	rapidjson::Document document;
	json_input_t jin{ document };

	document.Parse< RAPIDJSON_PARSEFLAGS >( json.c_str() );

	check_document_parse_status( document );

	TYPE result;

	jin >> result;

	return result;
}

//! Helper function to read an already instantiated DTO.
template < typename TYPE, unsigned RAPIDJSON_PARSEFLAGS = rapidjson::kParseDefaultFlags >
void
from_json( const std::string & json, TYPE & o )
{
	rapidjson::Document document;
	json_input_t jin{ document };

	document.Parse< RAPIDJSON_PARSEFLAGS >( json.c_str() );

	check_document_parse_status( document );

	jin >> o;
}

template< typename TYPE >
void
to_stream( std::ostream & to, const TYPE & type )
{
	rapidjson::Document output_doc;
	json_dto::json_output_t jout{
		output_doc, output_doc.GetAllocator() };

	jout << type;

	rapidjson::OStreamWrapper wrapper{ to };
	rapidjson::Writer< rapidjson::OStreamWrapper > writer{ wrapper };
	output_doc.Accept( writer );
}

template< typename TYPE, unsigned RAPIDJSON_PARSEFLAGS = rapidjson::kParseDefaultFlags >
void
from_stream( std::istream & from, TYPE & o )
{
	rapidjson::IStreamWrapper wrapper{ from };

	rapidjson::Document document;
	json_dto::json_input_t jin{ document };

	document.ParseStream< RAPIDJSON_PARSEFLAGS >( wrapper );

	check_document_parse_status( document );

	jin >> o;
}

template< typename TYPE, unsigned RAPIDJSON_PARSEFLAGS = rapidjson::kParseDefaultFlags >
TYPE
from_stream( std::istream & from )
{
	TYPE result;
	from_stream< TYPE, RAPIDJSON_PARSEFLAGS >( from, result );

	return result;
}

} /* namespace json_dto */
