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
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

#include <cstdint>
#include <vector>
#include <memory>
#include <limits>
#include <type_traits>
#include <iostream>

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
		#if __has_include(<optional>)
			#include <optional>
		#endif

		#if defined( __cpp_lib_optional )
			#define JSON_DTO_HAS_STD_OPTIONAL
		#elif __has_include(<experimental/optional>)
			#include <experimental/optional>
			#define JSON_DTO_HAS_EXPERIMENTAL_OPTIONAL
		#endif
		#if defined(JSON_DTO_HAS_STD_OPTIONAL) || \
				defined(JSON_DTO_HAS_EXPERIMENTAL_OPTIONAL)
			#define JSON_DTO_SUPPORTS_STD_OPTIONAL
		#endif
	#endif
#endif

namespace json_dto
{

namespace details
{

namespace meta
{

// See https://en.cppreference.com/w/cpp/types/void_t for details.
template<typename... Ts> struct make_void { typedef void type;};
template<typename... Ts> using void_t = typename make_void<Ts...>::type;

//
// has_value_type
//
template< typename, typename = void_t<> >
struct has_value_type : public std::false_type {};

template< typename T >
struct has_value_type< T, void_t<typename T::value_type> > : public std::true_type {};

//
// has_key_type
//
template< typename, typename = void_t<> >
struct has_key_type : public std::false_type {};

template< typename T >
struct has_key_type< T, void_t<typename T::key_type> > : public std::true_type {};

//
// has_mapped_type
//
template< typename, typename = void_t<> >
struct has_mapped_type : public std::false_type {};

template< typename T >
struct has_mapped_type< T, void_t<typename T::mapped_type> > : public std::true_type {};

//
// has_iterator_type
//
template< typename, typename = void_t<> >
struct has_iterator_type : public std::false_type {};

template< typename T >
struct has_iterator_type< T, void_t<typename T::iterator> > : public std::true_type {};

//
// has_const_iterator_type
//
template< typename, typename = void_t<> >
struct has_const_iterator_type : public std::false_type {};

template< typename T >
struct has_const_iterator_type< T, void_t<typename T::const_iterator> > : public std::true_type {};

//
// has_begin
//
template< typename, typename = void_t<> >
struct has_begin : public std::false_type {};

template< typename T >
struct has_begin<
		T,
		void_t<decltype(std::declval<const T &>().begin())> > : public std::true_type {};

//
// has_end
//
template< typename, typename = void_t<> >
struct has_end : public std::false_type {};

template< typename T >
struct has_end<
		T,
		void_t<decltype(std::declval<const T &>().end())> > : public std::true_type {};

//
// has_emplace
//
// emplace() method will be used for set-like structures.
template< typename, typename = void_t<> >
struct has_emplace : public std::false_type {};

template< typename T >
struct has_emplace<
		T,
		void_t<
			decltype(
					std::declval<T &>().emplace(
							std::declval<typename T::value_type>() )
			) >
		> : public std::true_type {};

//
// has_emplace_back
//
// emplace_back() method will be used for sequence containers
// except std::forward_list.
template< typename, typename = void_t<> >
struct has_emplace_back : public std::false_type {};

template< typename T >
struct has_emplace_back<
		T,
		void_t<
			decltype(
					std::declval<T &>().emplace_back(
							std::declval<typename T::value_type>() )
			) >
		> : public std::true_type {};

//
// has_emplace_after
//
// emplace_after() method will be used for std::forward_list.
template< typename, typename = void_t<> >
struct has_emplace_after : public std::false_type {};

template< typename T >
struct has_emplace_after<
		T,
		void_t<
			decltype(
					std::declval<T &>().emplace_after(
							std::declval<typename T::const_iterator>(),
							std::declval<typename T::value_type>() )
			) >
		> : public std::true_type {};

//
// has_before_begin
//
// If containers has emplace_after() and before_begin() methods then
// we assume that it is std::forward_list (or compatible container).
template< typename, typename = void_t<> >
struct has_before_begin : public std::false_type {};

template< typename T >
struct has_before_begin<
		T,
		void_t<
			std::enable_if_t<
					std::is_same<
							typename T::iterator,
							decltype(std::declval<T &>().before_begin()) >::value >
			>
		> : public std::true_type {};

//
// is_stl_like_sequence_container
//
template< typename T >
struct is_stl_like_sequence_container
	{
		static constexpr bool value = 
				has_value_type<T>::value &&
				!has_key_type<T>::value &&
				has_iterator_type<T>::value &&
				has_const_iterator_type<T>::value &&
				has_begin<T>::value &&
				has_end<T>::value &&
				( has_emplace_back<T>::value ||
				  	(has_before_begin<T>::value && has_emplace_after<T>::value) )
				;
	};

//
// is_stl_like_associative_container
//
template< typename T >
struct is_stl_like_associative_container
	{
		static constexpr bool value = 
				has_value_type<T>::value &&
				has_key_type<T>::value &&
				has_iterator_type<T>::value &&
				has_const_iterator_type<T>::value &&
				has_begin<T>::value &&
				has_end<T>::value &&
				has_emplace<T>::value
				;
	};

//
// is_stl_map_like_associative_container
//
template< typename T >
struct is_stl_map_like_associative_container
	{
		static constexpr bool value = 
				is_stl_like_associative_container<T>::value &&
				has_mapped_type<T>::value
				;
	};

//
// is_stl_set_like_associative_container
//
template< typename T >
struct is_stl_set_like_associative_container
	{
		static constexpr bool value = 
				is_stl_like_associative_container<T>::value &&
				!has_mapped_type<T>::value
				;
	};

//
// is_stl_like_container
//
template< typename T >
struct is_stl_like_container
	{
		static constexpr bool value =
				is_stl_like_sequence_container<T>::value ||
				is_stl_like_associative_container<T>::value;
	};

//
// field_type_from_reference_impl
//
// Since v.0.2.12.
// Helper metafunction for detection of type to be used as
// Field_Type template parameter for binder_t class template.
// If usual reference to a field is passed to
// mandatory/optional/optional_no_default helper functions then
// Field_Type should be a type of the field.
// If const- or rvalue reference is passed then Field_Type should
// be const type (e.g. const T instead of just T).
template< typename T >
struct field_type_from_reference_impl;

template< typename T >
struct field_type_from_reference_impl<T&>
{
	using type = T;
};

template< typename T >
struct field_type_from_reference_impl<const T&>
{
	using type = const T;
};

template< typename T >
struct field_type_from_reference_impl<T&&>
{
	using type = const T;
};

//
// field_type_from_reference_t
//
// Since v.0.2.12.
//
// Usage example:
//
// template<typename Field_Type>
// auto binder_maker(Field_Type && field) {
// 	using binder_type_t = binder_t{
// 		default_reader_writer_t,
// 		details::meta::field_type_from_reference_t<decltype(field)>,
// 		...
// 	};
// 	...
// }
//
template< typename T >
using field_type_from_reference_t =
	typename field_type_from_reference_impl<T>::type;

} /* namespace meta */

namespace sequence_containers
{

//
// Helper class for hiding details of emplacing values at the end
// of the sequence_container.
//
// In case of std::vector/list/deque there is emplace_back() method
// that should be called for each new item.
//
// But in the case of std::forward_list we should call to
// before_begin() at the very beginning and then we have to call
// emplace_after() for each new item with storing and reusing of
// the returned pointer.
//
template< typename C, bool is_forward_list >
class container_filler_impl_t;

template< typename C >
class container_filler_impl_t< C, false > final
{
	C & m_cnt;

public :
	container_filler_impl_t( C & cnt ) : m_cnt{ cnt } {}

	template< typename V >
	void emplace_back( V && value )
	{
		m_cnt.emplace_back( std::forward<V>(value) );
	}
};

template< typename C >
class container_filler_impl_t< C, true > final
{
	C & m_cnt;
	typename C::iterator m_it;

public :
	container_filler_impl_t( C & cnt )
		: m_cnt{ cnt }, m_it{ cnt.before_begin() }
	{}

	template< typename V >
	void emplace_back( V && value )
	{
		m_it = m_cnt.emplace_after( m_it, std::forward<V>(value) );
	}
};

template< typename C >
using container_filler_t = container_filler_impl_t<
		C,
		meta::has_before_begin<C>::value && meta::has_emplace_after<C>::value >;

} /* namespace sequence_containers */

} /* namespace details */

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
// const_map_key_t
//
/*!
 * @brief A special indicator for the case when serialized value is a key
 * in map-like container.
 *
 * Since v.0.2.11 json_dto makes distinction between keys and values
 * in map-like containers. Values are serialized by calling write_json_value
 * for a reference of type T (where T is a type of a value). But keys are
 * now serialized by calling write_json_value for an instance of type
 * const_map_key_t<T> (where T is a type of a key).
 *
 * It allows to override write_json_value for const_map_key_t<T> for
 * the implementation of custom format for keys.
 *
 * @since v.0.2.11
 */
template< typename T >
struct const_map_key_t
{
	const T & v;
};

/*!
 * @brief Helper function for producing instances of const_map_key_t.
 *
 * @since v.0.2.11
 */
template< typename T >
const_map_key_t<T>
const_map_key( const T & v ) noexcept { return { v }; };

//
// mutable_map_key_t
//
/*!
 * @brief A special indicator for the case when deserialized value is a key
 * in map-like container.
 *
 * Since v.0.2.11 json_dto makes distinction between keys and values
 * in map-like containers. Values are deserialized by calling read_json_value
 * for a reference of type T (where T is a type of a value). But keys are
 * now deserialized by calling read_json_value for an instance of type
 * mutable_map_key_t<T> (where T is a type of a key).
 *
 * It allows to override mutable_json_value for mutable_map_key_t<T> for
 * the implementation of custom format for keys.
 *
 * @since v.0.2.11
 */
template< typename T >
struct mutable_map_key_t
{
	T & v;
};

/*!
 * @brief Helper function for producing instances of mutable_map_key_t.
 *
 * @since v.0.2.11
 */
template< typename T >
mutable_map_key_t<T>
mutable_map_key( T & v ) noexcept { return { v }; };

//
// default_reader_writer_t
//
// NOTE: implementation is going below.
/*!
 * @brief The default implementation of Reader_Writer.
 *
 * This implementation simply calls read_json_value and write_json_value
 * functions.
 *
 * @since v.0.2.10
 */
struct default_reader_writer_t
{
	template< typename Field_Type >
	void
	read( Field_Type & v, const rapidjson::Value & from ) const;

	template< typename Field_Type >
	void
	write(
		const Field_Type & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const;
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

	if( value <= std::uint32_t(std::numeric_limits< std::uint16_t >::max()) )
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

	if( value <= std::int32_t(std::numeric_limits< std::int16_t >::max()) &&
		value >= std::int32_t(std::numeric_limits< std::int16_t >::min()) )
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
// uint8
//

inline void
read_json_value(
	std::uint8_t & v,
	const rapidjson::Value & object )
{
	std::uint32_t value;
	read_json_value( value, object );

	if( value <= std::uint32_t(std::numeric_limits< std::uint8_t >::max()) )
		v = std::uint8_t( value );
	else
		throw ex_t{ "value is out of uint8: " + std::to_string( value ) };
}

inline void
write_json_value(
	std::uint8_t v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	const std::uint32_t value = v;
	write_json_value( value, object, allocator );
}

//
// int8
//

inline void
read_json_value( std::int8_t & v, const rapidjson::Value & object )
{
	std::int32_t value;
	read_json_value( value, object );

	if( value <= std::int32_t(std::numeric_limits< std::int8_t >::max()) &&
		value >= std::int32_t(std::numeric_limits< std::int8_t >::min()) )
		v = std::int8_t( value );
	else
		throw ex_t{ "value is out of int8: " + std::to_string( value ) };
}

inline void
write_json_value(
	std::int8_t v,
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
	constexpr std::string::size_type max_str_len = std::numeric_limits< rapidjson::SizeType >::max();

	if( max_str_len < s.size() )
	{
		throw ex_t{ "string length is too large: " + std::to_string( s.size() ) +
					" (max is " + std::to_string( max_str_len ) + ")" };
	}

	object.SetString( s.data(), static_cast< rapidjson::SizeType >( s.size() ), allocator );
}

// Since v.0.2.10.
inline void
write_json_value(
	const rapidjson::Value::StringRefType & s,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	// NOTE: there is no check for max_str_len as for std::string version
	// because s.length has type rapidjson::SizeType.
	object.SetString( s.s, s.length, allocator );
}

//
// const- and mutable map keys
//

// Since v.0.2.11.
template< typename T >
void
read_json_value(
	mutable_map_key_t<T> key,
	const rapidjson::Value & object )
{
	read_json_value( key.v, object );
}

// Since v.0.2.11.
template< typename T >
void
write_json_value(
	const_map_key_t<T> key,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator )
{
	write_json_value( key.v, object, allocator );
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
template< typename T, typename Reader_Writer = default_reader_writer_t >
inline void
read_json_value(
	cpp17::optional<T> & v,
	const rapidjson::Value & object,
	Reader_Writer reader_writer = Reader_Writer{} );

template< typename T, typename Reader_Writer = default_reader_writer_t >
inline void
write_json_value(
	const cpp17::optional<T> & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	Reader_Writer reader_writer = Reader_Writer{} );
#endif

//
// ARRAY
//

template< typename T, typename A, typename Reader_Writer = default_reader_writer_t >
void
read_json_value(
	std::vector< T, A > & vec,
	const rapidjson::Value & object,
	const Reader_Writer & reader_writer = Reader_Writer{} );

template< typename T, typename A, typename Reader_Writer = default_reader_writer_t >
void
write_json_value(
	const std::vector< T, A > & vec,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	const Reader_Writer & reader_writer = Reader_Writer{} );

//
// STL-like non-associative containers.
//
template< typename C, typename Reader_Writer = default_reader_writer_t >
std::enable_if_t<
		details::meta::is_stl_like_sequence_container<C>::value,
		void >
read_json_value(
	C & cnt,
	const rapidjson::Value & object,
	const Reader_Writer & reader_writer = Reader_Writer{} );

template< typename C, typename Reader_Writer = default_reader_writer_t >
std::enable_if_t<
		details::meta::is_stl_like_sequence_container<C>::value,
		void >
write_json_value(
	const C & cnt,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	const Reader_Writer & reader_writer = Reader_Writer{} );

//
// STL-set-like associative containers.
//
template< typename C, typename Reader_Writer = default_reader_writer_t >
std::enable_if_t<
		details::meta::is_stl_set_like_associative_container<C>::value,
		void >
read_json_value(
	C & cnt,
	const rapidjson::Value & object,
	const Reader_Writer & reader_writer = Reader_Writer{} );

template< typename C, typename Reader_Writer = default_reader_writer_t >
std::enable_if_t<
		details::meta::is_stl_set_like_associative_container<C>::value,
		void >
write_json_value(
	const C & cnt,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	const Reader_Writer & reader_writer = Reader_Writer{} );

//
// STL-map-like associative containers.
//
template< typename C, typename Reader_Writer = default_reader_writer_t >
std::enable_if_t<
		details::meta::is_stl_map_like_associative_container<C>::value,
		void >
read_json_value(
	C & cnt,
	const rapidjson::Value & object,
	const Reader_Writer & reader_writer = default_reader_writer_t{} );

template< typename C, typename Reader_Writer = default_reader_writer_t >
std::enable_if_t<
		details::meta::is_stl_map_like_associative_container<C>::value,
		void >
write_json_value(
	const C & cnt,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	const Reader_Writer & reader_writer = Reader_Writer{} );

//
// nullable_t
//

//! A wrapper for nullable fields.
template< typename Field_Type >
struct nullable_t
{
	nullable_t() noexcept
		:	m_has_value{ false }
	{}

	nullable_t( std::nullptr_t ) noexcept
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
	has_value() const noexcept
	{
		return m_has_value;
	}

	operator bool () const noexcept
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
	operator = ( std::nullptr_t ) noexcept
	{
		reset();
		return *this;
	}

	const Field_Type*
	operator -> () const noexcept
	{
		return field_ptr();
	}

	Field_Type*
	operator -> () noexcept
	{
		return field_ptr();
	}

	const Field_Type &
	operator * () const noexcept
	{
		return field_ref();
	}

	Field_Type&
	operator * () noexcept
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
	reset() noexcept
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
		field_ptr() noexcept
		{
			return reinterpret_cast< Field_Type * >( m_image_space );
		}

		const Field_Type *
		field_ptr() const noexcept
		{
			return reinterpret_cast< const Field_Type * >( m_image_space );
		}

		Field_Type &
		field_ref() noexcept
		{
			return *field_ptr();
		}

		const Field_Type &
		field_ref() const noexcept
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
std::enable_if_t<
		!details::meta::is_stl_like_container<Dto>::value,
		void >
read_json_value(
	Dto & v,
	const rapidjson::Value & object )
{
	json_input_t input( object );
	json_io( input, v );
}

template< typename Dto >
std::enable_if_t<
		!details::meta::is_stl_like_container<Dto>::value,
		void >
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

template<
	typename Field_Type,
	typename Reader_Writer = default_reader_writer_t >
void
read_json_value(
	nullable_t< Field_Type > & f,
	const rapidjson::Value & object,
	Reader_Writer reader_writer = Reader_Writer{} )
{
	Field_Type value;
	reader_writer.read( value, object );
	f = std::move( value );
}

template<
	typename Field_Type,
	typename Reader_Writer = default_reader_writer_t >
void
write_json_value(
	const nullable_t< Field_Type > & f,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	Reader_Writer reader_writer = Reader_Writer{} )
{
	if( f )
		reader_writer.write( *f, object, allocator );
	else
		object.SetNull();
}

#if defined( JSON_DTO_SUPPORTS_STD_OPTIONAL )
//
// std::optional
//
template< typename T, typename Reader_Writer >
inline void
read_json_value(
	cpp17::optional<T> & v,
	const rapidjson::Value & object,
	Reader_Writer reader_writer )
{
	T value_from_stream;
	reader_writer.read( value_from_stream, object );
	v = std::move(value_from_stream);
}

template< typename T, typename Reader_Writer >
inline void
write_json_value(
	const cpp17::optional<T> & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	Reader_Writer reader_writer )
{
	if( v )
		reader_writer.write( *v, object, allocator );
}
#endif

//
// ARRAY
//

template< typename T, typename A, typename Reader_Writer >
void
read_json_value(
	std::vector< T, A > & vec,
	const rapidjson::Value & object,
	const Reader_Writer & reader_writer )
{
	if( object.IsArray() )
	{
		vec.clear();
		vec.reserve( object.Size() );
		for( rapidjson::SizeType i = 0; i < object.Size(); ++i )
		{
			T v;
			reader_writer.read( v, object[ i ] );
			vec.push_back( std::move(v) );
		}
	}
	else
		throw ex_t{ "value is not an array" };
}

namespace details
{

template< typename T >
struct std_vector_item_read_access_type
{
	using type = const T&;
};

// since v.0.2.3
// std::vector<bool> must be processed different way.
template<>
struct std_vector_item_read_access_type<bool>
{
	using type = const bool;
};

} /* namespace details */

template< typename T, typename A, typename Reader_Writer >
void
write_json_value(
	const std::vector< T, A > & vec,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	const Reader_Writer & reader_writer )
{
	object.SetArray();
	for( typename details::std_vector_item_read_access_type<T>::type v : vec )
	{
		rapidjson::Value o;
		reader_writer.write( v, o, allocator );
		object.PushBack( o, allocator );
	}
}

//
// STL-like non-associative containers.
//
template< typename C, typename Reader_Writer >
std::enable_if_t<
		details::meta::is_stl_like_sequence_container<C>::value,
		void >
read_json_value(
	C & cnt,
	const rapidjson::Value & object,
	const Reader_Writer & reader_writer )
{
	if( object.IsArray() )
	{
		cnt.clear();
		details::sequence_containers::container_filler_t<C> filler{ cnt };

		for( rapidjson::SizeType i = 0; i < object.Size(); ++i )
		{
			typename C::value_type v;
			reader_writer.read( v, object[ i ] );
			filler.emplace_back( std::move(v) );
		}
	}
	else
		throw ex_t{ "value is not an array" };
}

template< typename C, typename Reader_Writer >
std::enable_if_t<
		details::meta::is_stl_like_sequence_container<C>::value,
		void >
write_json_value(
	const C & cnt,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	const Reader_Writer & reader_writer )
{
	object.SetArray();
	for( const auto & v : cnt )
	{
		rapidjson::Value o;
		reader_writer.write( v, o, allocator );
		object.PushBack( o, allocator );
	}
}

//
// STL-set-like associative containers.
//
template< typename C, typename Reader_Writer >
std::enable_if_t<
		details::meta::is_stl_set_like_associative_container<C>::value,
		void >
read_json_value(
	C & cnt,
	const rapidjson::Value & object,
	const Reader_Writer & reader_writer )
{
	if( !object.IsArray() )
		throw ex_t{ "value can't be deserialized into std::set-like container!" };

	cnt.clear();
	for( rapidjson::SizeType i = 0; i < object.Size(); ++i )
	{
		typename C::value_type v;
		reader_writer.read( v, object[ i ] );
		cnt.emplace( std::move(v) );
	}
}

/*
 * NOTE. There is no a special handling for multiset cases.
 * All values from the container are deserialized.
 *
 * It it possible to check for duplicates of keys in the containers.
 * But this check has performance penalty. So at v.0.2.8 there is no
 * such check.
 */
template< typename C, typename Reader_Writer >
std::enable_if_t<
		details::meta::is_stl_set_like_associative_container<C>::value,
		void >
write_json_value(
	const C & cnt,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	const Reader_Writer & reader_writer )
{
	const auto write_item =
			[&object, &allocator, &reader_writer]( const auto & v ) {
				rapidjson::Value o;
				reader_writer.write( v, o, allocator );
				object.PushBack( o, allocator );
			};

	object.SetArray();
	for( const auto & v : cnt )
		write_item( v );
}

//
// STL-map-like associative containers.
//
template< typename C, typename Reader_Writer >
std::enable_if_t<
		details::meta::is_stl_map_like_associative_container<C>::value,
		void >
read_json_value(
	C & cnt,
	const rapidjson::Value & object,
	const Reader_Writer & reader_writer )
{
	if( !object.IsObject() )
		throw ex_t{ "value can't be deserialized into std::map-like container!" };

	cnt.clear();
	for( auto it = object.MemberBegin(); it != object.MemberEnd(); ++it )
	{
		typename C::key_type key;
		typename C::mapped_type value;

		// It is necessary to have mutable_key_ref as a lvalue to pass
		// a non-const reference to it to read() method of the reader_writer.
		auto mutable_key_ref = mutable_map_key(key);
		reader_writer.read( mutable_key_ref, it->name );
		reader_writer.read( value, it->value );

		cnt.emplace( typename C::value_type{ std::move(key), std::move(value) } );
	}
}

/*
 * NOTE. There is no a special handling for multimap cases.
 * All values from the container are deserialized.
 *
 * It it possible to check for duplicates of keys in the containers.
 * But this check has performance penalty. So at v.0.2.8 there is no
 * such check.
 */
template< typename C, typename Reader_Writer >
std::enable_if_t<
		details::meta::is_stl_map_like_associative_container<C>::value,
		void >
write_json_value(
	const C & cnt,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator,
	const Reader_Writer & reader_writer )
{
	const auto write_item =
			[&object, &allocator, &reader_writer]( const auto & kv ) {
				rapidjson::Value key;
				rapidjson::Value value;

				// It is necessary to have const_key_ref as a lvalue to pass a
				// const reference to it to write() method of the reader_writer.
				auto const_key_ref = const_map_key(kv.first);
				reader_writer.write( const_key_ref, key, allocator );
				reader_writer.write( kv.second, value, allocator );

				object.AddMember( key, value, allocator );
			};

	object.SetObject();
	for( const auto & kv : cnt )
		write_item( kv );
}

//
// Support for to_json/from_json for STL-like containers.
// Since v.0.2.8
//
namespace details {

template< typename C >
struct stl_like_container_reader_t {
	C & m_dest;

	void
	read_from( const rapidjson::Value & from ) const
	{
		read_json_value( m_dest, from );
	}
};

template< typename C >
struct stl_like_container_writer_t {
	const C & m_src;

	void
	write_to(
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		write_json_value( m_src, to, allocator );
	}
};

} /* namespace details */

template< typename C >
std::enable_if_t<
		details::meta::is_stl_like_container<C>::value,
		void >
json_io( json_input_t & from, C & what )
{
	from & details::stl_like_container_reader_t<C>{ what };
}

// NOTE: argument 'what' is not const.
// It is required by implementation of operator<<() below.
template< typename C >
std::enable_if_t<
		details::meta::is_stl_like_container<C>::value,
		void >
json_io( json_output_t & from, C & what )
{
	from & details::stl_like_container_writer_t<C>{ what };
}

//
// Funcs for handling nullable property.
//

/*!
 * @brief Default handler of null value for non-nullable attribute.
 *
 * Throws an exception because non-nullable attribute can't receive null value.
 *
 * Is has name `default_on_null` since v.0.3.0.
 */
template< typename Field_Type >
void
default_on_null( Field_Type & )
{
	throw ex_t{ "non nullable field is null" };
}

/*!
 * @brief Default handler of null value for nullable attribute.
 *
 * Calls `reset` for @a f.
 *
 * Is has name `default_on_null` since v.0.3.0.
 */
template< typename Field_Type >
void
default_on_null( nullable_t< Field_Type > & f )
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
/*!
 * This class is intended to be used as Manopt_Policy trait
 * for field binders.
 */
struct mandatory_attr_t
{
	template< typename Field_Type >
	void
	on_field_not_defined( Field_Type & ) const
	{
		throw ex_t{ "mandatory field doesn't exist" };
	}

	/*!
	 * @note
	 * Since v.0.3.0.
	 */
	template< typename Field_Type >
	void
	on_null( Field_Type & f ) const
	{
		// NOTE: directly use 'default_on_null' from json_dto namespace
		// to avoid surprises with ADL.
		json_dto::default_on_null( f );
	}

	template< typename Field_Type >
	constexpr bool
	is_default_value( Field_Type & ) const noexcept
	{
		return false;
	}
};

//! Field set/notset attribute ckecker for mandatory field.
/*!
 * It field value is 'null' in JSON then field will receive
 * default value.
 *
 * This class is intended to be used as Manopt_Policy trait
 * for field binders.
 *
 * @note
 * Type \a Field_Type has to be DefaultConstructible.
 *
 * @since v.0.3.0
 */
struct mandatory_attr_with_null_as_default_t
{
	template< typename Field_Type >
	void
	on_field_not_defined( Field_Type & ) const
	{
		throw json_dto::ex_t{ "mandatory field doesn't exist" };
	}

	template< typename Field_Type >
	void
	on_null( Field_Type & f ) const
	{
		static_assert( std::is_default_constructible<Field_Type>::value,
				"type Field_Type has to be DefaultConstructible" );

		f = Field_Type{};
	}

	template< typename Field_Type >
	constexpr bool
	is_default_value( Field_Type & ) const noexcept
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

	/*!
	 * @note
	 * Since v.0.3.0.
	 */
	template< typename Field_Type >
	void
	on_null( Field_Type & f ) const
	{
		// NOTE: directly use 'default_on_null' from json_dto namespace
		// to avoid surprises with ADL.
		json_dto::default_on_null( f );
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

//! Field set/notset attribute checker for optional case with default value.
struct optional_attr_null_t
{
	template< typename Field_Type >
	void
	on_field_not_defined( nullable_t< Field_Type > & f ) const noexcept
	{
		f.reset();
	}

	/*!
	 * @note
	 * Since v.0.3.0.
	 */
	template< typename Field_Type >
	void
	on_null( Field_Type & f ) const
	{
		// NOTE: directly use 'default_on_null' from json_dto namespace
		// to avoid surprises with ADL.
		json_dto::default_on_null( f );
	}

	template< typename Field_Type >
	bool
	is_default_value( nullable_t< Field_Type > & f ) const noexcept
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
	on_field_not_defined( Field_Type & ) const noexcept
	{}

	/*!
	 * @note
	 * Since v.0.3.0.
	 */
	template< typename Field_Type >
	void
	on_null( Field_Type & f ) const
	{
		// NOTE: directly use 'default_on_null' from json_dto namespace
		// to avoid surprises with ADL.
		json_dto::default_on_null( f );
	}

	template< typename Field_Type >
	constexpr bool
	is_default_value( Field_Type & ) const noexcept
	{
		return false;
	}
};

using string_ref_t = rapidjson::Value::StringRefType;

//! Helper function to make a string_ref instance.
/*!
 * @since v.0.2.9
 */
inline string_ref_t
make_string_ref( const char * src, std::size_t length ) noexcept
{
	return { src, static_cast<rapidjson::SizeType>(length) };
}

//! Helper function to make a string_ref instance from std::string object.
/*!
 * @since v.0.2.9
 */
inline string_ref_t
make_string_ref( const std::string & src ) noexcept
{
	return make_string_ref( src.data(), src.size() );
}

//! Helper function to make a string_ref instance from raw pointer.
/*!
 * @since v.0.2.9
 */
inline string_ref_t
make_string_ref( const char * src ) noexcept
{
	return make_string_ref( src, std::strlen(src) );
}

//
// empty_validator_t
//

struct empty_validator_t
{
	template< typename Field_Type >
	constexpr void
	operator () ( const Field_Type & ) const noexcept
	{}
};

//
// the implementation of default_reader_writer_t
//

template< typename Field_Type >
void
default_reader_writer_t::read(
	Field_Type & v, const rapidjson::Value & from ) const
{
	read_json_value( v, from );
}

template< typename Field_Type >
void
default_reader_writer_t::write(
	const Field_Type & v,
	rapidjson::Value & to,
	rapidjson::MemoryPoolAllocator<> & allocator ) const
{
	write_json_value( v, to, allocator );
}

/*!
 * @brief A special proxy reader_writer that delegates actual
 * formatting actions to the nested reader_writer.
 *
 * When a custom reader_writer is specified for a field it's applied
 * to the field itself. For example:
 * @code
 * struct my_int_reader_writer {
 * 	void read(int & v, ...) const {...}
 * 	void write(const int & v, ...) const {...}
 * };
 *
 * struct my_data {
 * 	int field_;
 * 	...
 * 	template<typename Io> void json_io(Io & io) {
 * 		io & json_dto::mandatory(my_int_reader_writer{}, "field", field_)
 * 			...
 * 			;
 * 	}
 * };
 * @endcode
 * In that case a reference to integer field `field_` will be passed to
 * `my_int_reader_writer`. And this is intended behavior.
 *
 * But there could be cases when (de)serialized field is a container.
 * For example, in that case we'll get a compiler error:
 * @code
 * struct my_complex_data {
 * 	std::vector<int> field_;
 * 	...
 * 	template<typename Io> void json_io(Io & io) {
 * 		io & json_dto::mandatory(my_int_reader_writer{}, "field", field_)
 * 			...
 * 			;
 * 	}
 * };
 * @endcode
 * It is because a reference to `vector<int>` will be passed to
 * `my_int_reader_writer`, but `my_int_reader_writer` expects references
 * to integers.
 *
 * It seems that this problem can be solved by extending `my_int_reader_writer`:
 * @code
 * struct my_int_reader_writer {
 * 	void read(int & v, ...) const {...}
 * 	void read(std::vector<int> & v, ...) const {...}
 *
 * 	void write(const int & v, ...) const {...}
 * 	void write(const std::vector<int> & v, ...) const {...}
 * };
 * @endcode
 * But this solution doesn't scale well because there are a plenty of
 * container types. And it's not good to create an overload for every
 * of them.
 *
 * So the template `apply_to_content_t` gives another solution:
 * @code
 * struct my_complex_data {
 * 	std::vector<int> field_;
 * 	...
 * 	template<typename Io> void json_io(Io & io) {
 * 		io & json_dto::mandatory(
 * 				json_dto::apply_to_content_t<my_int_reader_writer>{},
 * 				"field", field_)
 * 			...
 * 			;
 * 	}
 * };
 * @endcode
 *
 * An instance of `apply_to_content_t` holds an instance of an actual
 * reader_writer type and applies it to every item of container.
 *
 * @note
 * The template `apply_to_content_t` should also be used will
 * `nullable_t` and `std::optional` if an actual reader_writer has
 * to be applied to the content of `nullable_t`/`std::optional`:
 * @code
 * struct my_data {
 * 	std::optional<int> weight_;
 * 	json_dto::nullable_t<int> priority_;
 * 	...
 * 	template<typename Io> void json_io(Io & io) {
 * 		io & json_dto::optional(
 * 				json_dto::apply_to_content_t<my_int_reader_writer>{},
 * 				"weight", weight_, std::nullopt)
 * 			& json_dto::mandatory(
 * 				json_dto::apply_to_content_t<my_int_reader_writer>{},
 * 				"priority", priority_)
 * 		...
 * 		;
 * 	}
 * }
 * @endcode
 *
 * @note
 * Sometimes `apply_to_content_t` should be nested:
 * @code
 * struct my_complex_data {
 * 	json_dto::nullable_t< std::vector<int> > params_;
 * 	...
 * 	template<typename Io> void json_io(Io & io) {
 * 		io & json_dto::mandatory(
 * 				// The first occurence of apply_to_content_t is for nullable_t.
 * 				json_dto::apply_to_content_t<
 * 					// The second occurence of apply_to_content_t is for std::vector.
 * 					json_dto::apply_to_content_t<
 * 						// This is for the content of std::vector.
 * 						my_int_reader_writer
 * 					>
 * 				>{},
 * 				"params", params_)
 * 			...
 * 			;
 * 	}
 * };
 * @endcode
 *
 * @since v.0.2.11
 */
template< typename Item_Reader_Writer >
struct apply_to_content_t
{
	Item_Reader_Writer m_reader_writer;

	apply_to_content_t() = default;

	apply_to_content_t( const Item_Reader_Writer & initial )
		:	m_reader_writer{ initial }
	{}

	apply_to_content_t( Item_Reader_Writer && initial )
		:	m_reader_writer{ std::move(initial) }
	{}

	template< typename Field_Type >
	void
	read(
		Field_Type & v, const rapidjson::Value & from ) const
	{
		read_json_value( v, from, m_reader_writer );
	}

	template< typename Field_Type >
	void
	write(
		Field_Type & v,
		rapidjson::Value & to,
		rapidjson::MemoryPoolAllocator<> & allocator ) const
	{
		write_json_value( v, to, allocator, m_reader_writer );
	}
};

//
// binder_data_holder_t
//
/*!
 * @brief Type of holder of data required for field binder.
 *
 * This type was introduced in v.0.2.12 as a part of the new customization
 * points for binder_t class. In previous versions of json-dto binder_t hold
 * all required data as direct members of binder_t template class. Since
 * v.0.2.12 all that data is moved into binder_data_holder_t template and
 * binder_t now just owns an instance of binder_data_holder_t. It allows a user
 * to make a specialization of binder_data_holder_t template for his/her own
 * data types.
 *
 * For example:
 * @code
namespace some_project
{

// A special proxy for field that can only be serialized, but not deserialized.
template< typename F >
struct serialize_only_proxy_t
{
	using field_type = const F;

	const F * m_field;
};

// Usage example:
//
// template< typename Json_Io >
// void my_type::json_io(Json_Io & io) {
// 	io & json_dto::mandatory("attr", some_project::serialize_only(attr_))
// 		& ...
// 		;
// }
template< typename F >
serialize_only_proxy_t<F> serialize_only( const F & field ) noexcept
{
	return { &field };
}

} // namespace some_project

// We have to specialize json_dto::binder_data_holder_t to store
// the content of some_project::serialize_only_proxy_t instead of
// a reference to serialize_only_proxy_t.
namespace json_dto
{

template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
class binder_data_holder_t<
		Reader_Writer,
		// NOTE the usage of `const serialize_only_proxy_t`.
		const some_project::serialize_only_proxy_t<Field_Type>,
		Manopt_Policy,
		Validator >
	:	public binder_data_holder_t<
			Reader_Writer,
			typename some_project::serialize_only_proxy_t<Field_Type>::field_type,
			Manopt_Policy,
			Validator >
{
	using serialize_only_proxy =
			some_project::serialize_only_proxy_t<Field_Type>;

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
				// Passing an actual reference to the field.
				*(proxy.m_field),
				std::move(manopt_policy),
				std::move(validator)
			}
	{} // The `proxy` parameter is no more needed and isn't used any more.
};

} // namespace json_dto

struct my_data
{
	const int version_{1};
	...
	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io
			// Now binder_t for `version_` field will contain
			// a specialized version of binder_data_holder_t.
			& json_dto::mandatory("version",
					some_project::serialize_only(version_))
			& ...
			;
	}
};
 * @endcode
 *
 * @attention
 * A specialization of binder_data_holder_t should define type name
 * `field_t` (see example in the code of binder_data_holder_t template).
 *
 * @note
 * All getters are const-methods because binder_t access them via
 * a const reference to binder_data_holder_t instance.
 *
 * @note
 * Methods field_for_serialization() and field_for_deserialization()
 * return just `Field_Type &` because the current versions of
 * Manopt_Policy implementations (like mandatory_attr_t, optional_attr_t,
 * and so on) accept `Field_Type &`. Some future versions of json-dto
 * can get different return types for field_for_serialization() and
 * field_for_deserialization().
 *
 * @tparam Reader_Writer type of reader_writer object to be used for
 * serializing/deserializing the field.
 *
 * @tparam Field_Type type of the field to be (de)serialized.
 *
 * @tparam Manopt_Policy type of object for handling mandatory/optional
 * policy for the field. It is expected to be mandatory_attr_t,
 * optional_attr_t, optional_attr_null_t, or optional_nodefault_attr_t.
 *
 * @tparam Validator type of object for checking the validity of
 * the field valued (check is performed before serialization and just
 * after deserialization).
 *
 * @since v.0.2.12
 */
template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
class binder_data_holder_t
{
		Reader_Writer m_reader_writer;
		string_ref_t m_field_name;
		Field_Type & m_field;
		Manopt_Policy m_manopt_policy;
		Validator m_validator;

	public:
		using field_t = Field_Type;

		binder_data_holder_t(
			Reader_Writer && reader_writer,
			string_ref_t field_name,
			Field_Type & field,
			Manopt_Policy && manopt_policy,
			Validator && validator )
			:	m_reader_writer{ std::move(reader_writer) }
			,	m_field_name{ field_name }
			,	m_field{ field }
			,	m_manopt_policy{ std::move( manopt_policy ) }
			,	m_validator{ std::move( validator ) }
		{}

		const Reader_Writer &
		reader_writer() const noexcept { return m_reader_writer; }

		const string_ref_t &
		field_name() const noexcept { return m_field_name; }

		Field_Type &
		field_for_serialization() const noexcept { return m_field; }

		Field_Type &
		field_for_deserialization() const noexcept { return m_field; }

		const Manopt_Policy &
		manopt_policy() const noexcept { return m_manopt_policy; }

		const Validator &
		validator() const noexcept { return m_validator; }
};

//
// binder_read_from_implementation_t
//
/*!
 * @brief Type that provides the default implementation of read_from
 * operation for a binder.
 *
 * This type was introduced in v.0.2.12 as a part of the new customization
 * points for binder_t class. In previous versions of json-dto binder_t
 * performed read_from operation by itself. Now it delegates this operation
 * to static method of binder_read_from_implementation_t template.
 * It allows a user to write own specialization of
 * binder_read_from_implementation_t template for his/her types.
 *
 * For example:
 * @code
namespace some_project
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

} // namespace some_project

// We have to specialize json_dto::binder_data_holder_t to store
// the content of some_project::ignore_after_deserialization_proxy_t instead of
// a reference to ignore_after_deserialization_proxy_t.
// We also have to specialize json_dto::binder_read_from_implementation_t
// for right handling of some_project::ignore_after_deserialization_proxy_t.
namespace json_dto
{
template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
class binder_data_holder_t<
		Reader_Writer,
		const some_project::ignore_after_deserialization_proxy_t<Field_Type>,
		Manopt_Policy,
		Validator >
{
	...
};

template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
struct binder_read_from_implementation_t<
		binder_data_holder_t<
			Reader_Writer,
			const some_project::ignore_after_deserialization_proxy_t<Field_Type>,
			Manopt_Policy,
			Validator
		>
	>
{
	using proxy_type = some_project::ignore_after_deserialization_proxy_t<Field_Type>;

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
				binder_data.manopt_policy.on_null( tmp_object );
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

} // namespace json_dto

// Now we can use some_project::ignore_after_deserialization() in json_io().
struct my_data
{
	const int version_{1};
	...
	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io
			// Now binder_t for `version_` field will contain
			// a specialized version of binder_data_holder_t.
			& json_dto::mandatory("version",
					some_project::ignore_after_deserialization(version_),
					// NOTE: validators will be applied to deserialized value.
					json_dto::min_max_constraint(1,3))
			& ...
			;
	}
};
 * @endcode
 *
 * @since v.0.2.12
 */
template< typename Binder_Data_Holder >
struct binder_read_from_implementation_t
{
	static void
	read_from(
		const Binder_Data_Holder & binder_data,
		const rapidjson::Value & object )
	{
		static_assert(
				!std::is_const<typename Binder_Data_Holder::field_t>::value,
				"const object can't be deserialized" );

		if( !object.IsObject() )
		{
			throw ex_t{
				"unable to extract field \"" +
				std::string{ binder_data.field_name().s } + "\": "
				"parent json type must be object" };
		}

		const auto it = object.FindMember( binder_data.field_name() );

		if( object.MemberEnd() != it )
		{
			const auto & value = it->value;

			if( !value.IsNull() )
			{
				binder_data.reader_writer().read(
						binder_data.field_for_deserialization(), value );
			}
			else
			{
				binder_data.manopt_policy().on_null(
						binder_data.field_for_deserialization() );
			}
		}
		else
		{
			binder_data.manopt_policy().on_field_not_defined(
					binder_data.field_for_deserialization() );
		}

		binder_data.validator()(
				binder_data.field_for_deserialization() ); // validate value.
	}
};

//
// binder_write_to_implementation_t
//
/*!
 * @brief Type that provides the default implementation of write_to
 * operation for a binder.
 *
 * This type was introduced in v.0.2.12 as a part of the new customization
 * points for binder_t class. In previous versions of json-dto binder_t
 * performed write_to operation by itself. Now it delegates this operation
 * to static method of binder_write_to_implementation_t template.
 * It allows a user to write own specialization of
 * binder_write_to_implementation_t template for his/her types.
 *
 * For example:
 * @code
namespace some_project
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

} // namespace some_project

// We have to specialize json_dto::binder_data_holder_t to store
// the content of some_project::deserialize_only_proxy_t instead of
// a reference to deserialize_only_proxy_t.
// We also have to specialize json_dto::binder_write_to_implementation_t
// for right handling of some_project::deserialize_only_proxy_t.
namespace json_dto
{

template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
class binder_data_holder_t<
		Reader_Writer,
		const some_project::deserialize_only_proxy_t<Field_Type>,
		Manopt_Policy,
		Validator >
{
	...
};

template<
	typename Reader_Writer,
	typename Field_Type,
	typename Manopt_Policy,
	typename Validator >
struct binder_write_to_implementation_t<
		binder_data_holder_t<
			Reader_Writer,
			const some_project::deserialize_only_proxy_t<Field_Type>,
			Manopt_Policy,
			Validator
		>
	>
{
	using data_holder_t = binder_data_holder_t<
			Reader_Writer,
			const some_project::deserialize_only_proxy_t<Field_Type>,
			Manopt_Policy,
			Validator >;

	static void
	write_to(
		const data_holder_t &,
		rapidjson::Value &,
		rapidjson::MemoryPoolAllocator<> & )
	{
		// Nothing to do.
	}

};

// Now we can use some_project::deserialize_only() in json_io().
struct my_data
{
	int obsolete_priority_{1};
	...
	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io
			// Now binder_t for `priority` field will contain
			// a specialized version of binder_data_holder_t.
			& json_dto::mandatory("priority",
					some_project::deserialize_only(obsolete_priority_),
					// NOTE: validators will be applied to deserialized value.
					json_dto::min_max_constraint(1,9))
			& ...
			;
	}
};
 * @endcode
 *
 * @since v.0.2.12
 */
template< typename Binder_Data_Holder >
struct binder_write_to_implementation_t
{
	static void
	write_to(
		const Binder_Data_Holder & binder_data,
		rapidjson::Value & object,
		rapidjson::MemoryPoolAllocator<> & allocator )
	{
		binder_data.validator()(
				binder_data.field_for_serialization() ); // validate value.

		if( !binder_data.manopt_policy().is_default_value(
				binder_data.field_for_serialization() ) )
		{
			rapidjson::Value value;

			binder_data.reader_writer().write(
					binder_data.field_for_serialization(),
					value,
					allocator );

			object.AddMember(
					binder_data.field_name(),
					value,
					allocator );
		}
	}
};

//
// binder_t
//

//! JSON IO binder_t for a field.
/*!
 * @attention
 * Since v.0.2.12, additional care should be taken when working with instances
 * of binder_t. An instance of binder_t can hold a reference to a temporary
 * object. For example, in such a case:
 * @code
 * struct demo {
 * 	std::vector<int> keys() const { return { 1, 2, 3 }; }
 * 	...
 * 	// NOTE: this code is intended for serializing only!
 * 	template< typename Json_Io >
 * 	void json_io(Json_Io & io) {
 * 		// NOTE: binder object returned by mandatory() function
 * 		// will contain a reference to a temporary std::vector<int>.
 * 		io & json_dto::mandatory("keys", keys());
 * 		...
 * 	}
 * };
 * @endcode
 * So it's safe to pass binder_t object directly to `operator&`, but
 * it can be unsafe to store binder_t object to be used later. Don't do
 * that:
 * @code
 * struct demo {
 * 	std::vector<int> keys() const { return { 1, 2, 3 }; }
 * 	...
 * 	// NOTE: this code is intended for serializing only!
 * 	template< typename Json_Io >
 * 	void json_io(Json_Io & io) {
 * 		// NOTE: binder object returned by mandatory() function
 * 		// will contain a reference to a temporary std::vector<int>.
 * 		// This reference become invalid just after the completion
 * 		// of line (1).
 * 		//
 * 		// DON'T USE binder_t THIS WAY!
 * 		const auto keys_binder = json_dto::mandatory("keys", keys()); // (1)
 * 		...
 * 		io & keys_binder; // An invalid reference will be used here!!!
 * 		...
 * 	}
 * };
 * @endcode
 *
 * @note
 * This template was extended in v.0.2.10 by a new template parameter
 * Reader_Writer. This parameter specifies a type of object with
 * two const methods `read` and `write`. Since v.0.2.10 those methods
 * are used for reading and writting a value of Field_Type from/to
 * rapidjson::Value. See default_reader_writer_t for an example
 * of Reader_Writer.
 */
template<
		typename Reader_Writer,
		typename Field_Type,
		typename Manopt_Policy,
		typename Validator >
class binder_t
{
		using data_holder_t = binder_data_holder_t<
				Reader_Writer,
				Field_Type,
				Manopt_Policy,
				Validator >;

		using read_from_impl_t = binder_read_from_implementation_t<
				data_holder_t >;

		using write_to_impl_t = binder_write_to_implementation_t<
				data_holder_t >;

	public:
		binder_t(
			Reader_Writer && reader_writer,
			string_ref_t field_name,
			Field_Type & field,
			Manopt_Policy && manopt_policy,
			Validator && validator )
			:	m_data_holder{
					std::move(reader_writer),
					field_name,
					field,
					std::move( manopt_policy ),
					std::move( validator )
				}
		{}

		//! Run read operation for value.
		void
		read_from( const rapidjson::Value & object ) const
		{
			try
			{
				read_from_impl_t::read_from( m_data_holder, object );
			}
			catch( const std::exception & ex )
			{
				throw ex_t{
						"error reading field \"" +
						std::string{ m_data_holder.field_name().s } +
						"\": " +
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
				write_to_impl_t::write_to(
						m_data_holder,
						object,
						allocator );
			}
			catch( const std::exception & ex )
			{
				throw ex_t{
						"error writing field \"" +
						std::string{ m_data_holder.field_name().s } +
						"\": " +
						ex.what() };
			}
		}

	private:
		data_holder_t m_data_holder;
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
	Field_Type && field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t<
			default_reader_writer_t,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			mandatory_attr_t,
			Validator >;

	return binder_type_t{
			default_reader_writer_t{},
			field_name,
			field,
			mandatory_attr_t{},
			std::move( validator ) };
}

//! Create bind for a mandatory JSON field with validator.
template<
		typename Reader_Writer,
		typename Field_Type,
		typename Validator = empty_validator_t >
auto
mandatory(
	Reader_Writer reader_writer,
	string_ref_t field_name,
	Field_Type && field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t<
			Reader_Writer,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			mandatory_attr_t,
			Validator >;

	return binder_type_t{
			std::move(reader_writer),
			field_name,
			field,
			mandatory_attr_t{},
			std::move( validator ) };
}

//! Create bind for a mandatory JSON field with validator.
/*!
 * This binder has to be used in case when 'null' is allowed
 * for mandatory attr and should be treated as default field value.
 *
 * For example:
 * @code
 * struct my_header_reader_writer
 * {
 * 	void read( std::string & v, ... ) const { ... }
 * 	void write( const std::string & v, ... ) const { ... }
 * };
 *
 * struct my_data
 * {
 * 	std::vector< std::string > headers_;
 * 	...
 * 	template< typename Json_Io >
 * 	void json_io( Json_Io & io )
 * 	{
 * 		// headers_ receives std::vector<std::string>{} in case of 'null'.
 * 		io & json_dto::mandatory_with_null_as_default(
 * 				// Use a custom reader-writer for that field.
 * 				json_dto::apply_to_content_t< my_header_reader_writer >{},
 * 				"headers", headers_ )
 * 			...
 * 			;
 * 	}
 * };
 * @endcode
 *
 * @note
 * Type \a Field_Type has to be DefaultConstructible.
 *
 * @since v.0.3.0
 */
template<
		typename Reader_Writer,
		typename Field_Type,
		typename Validator = json_dto::empty_validator_t >
auto
mandatory_with_null_as_default(
	Reader_Writer reader_writer,
	json_dto::string_ref_t field_name,
	Field_Type && field,
	Validator validator = Validator{} )
{
	using binder_type_t = json_dto::binder_t<
			Reader_Writer,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			mandatory_attr_with_null_as_default_t,
			Validator >;

	return binder_type_t{
			std::move(reader_writer),
			field_name,
			field,
			mandatory_attr_with_null_as_default_t{},
			std::move( validator ) };
}

//! Create bind for a mandatory JSON field with validator.
/*!
 * This binder has to be used in case when 'null' is allowed
 * for mandatory attr and should be treated as default field value.
 *
 * For example:
 * @code
 * struct my_data 
 * {
 * 	std::vector< std::string > headers_;
 * 	...
 * 	template< typename Json_Io >
 * 	void json_io( Json_Io & io )
 * 	{
 * 		// headers_ receives std::vector<std::string>{} in case of 'null'.
 * 		io & json_dto::mandatory_with_null_as_default(
 * 				"headers", headers_ )
 * 			...
 * 			;
 * 	}
 * };
 * @endcode
 *
 * @note
 * Type \a Field_Type has to be DefaultConstructible.
 *
 * @since v.0.3.0
 */
template<
		typename Field_Type,
		typename Validator = json_dto::empty_validator_t >
auto
mandatory_with_null_as_default(
	json_dto::string_ref_t field_name,
	Field_Type && field,
	Validator validator = Validator{} )
{
	return mandatory_with_null_as_default(
			default_reader_writer_t{},
			field_name,
			std::forward<Field_Type>(field),
			std::move(validator) );
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
	Field_Type && field,
	Field_Default_Value_Type default_value,
	Validator validator = Validator{} )
{
	using opt_attr_t = optional_attr_t< Field_Default_Value_Type >;
	using binder_type_t = binder_t<
			default_reader_writer_t,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			opt_attr_t,
			Validator >;

	return binder_type_t{
			default_reader_writer_t{},
			field_name,
			field,
			opt_attr_t{ std::move( default_value ) },
			std::move( validator ) };
}

//! Create bind for an optional JSON field with default value and validator.
template<
		typename Reader_Writer,
		typename Field_Type,
		typename Field_Default_Value_Type,
		typename Validator = empty_validator_t >
auto
optional(
	Reader_Writer reader_writer,
	string_ref_t field_name,
	Field_Type && field,
	Field_Default_Value_Type default_value,
	Validator validator = Validator{} )
{
	using opt_attr_t = optional_attr_t< Field_Default_Value_Type >;
	using binder_type_t = binder_t<
			Reader_Writer,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			opt_attr_t,
			Validator >;

	return binder_type_t{
			std::move(reader_writer),
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
	Field_Type && field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t<
			default_reader_writer_t,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			optional_attr_null_t,
			Validator >;

	return binder_type_t{
			default_reader_writer_t{},
			field_name,
			field,
			optional_attr_null_t{},
			std::move( validator ) };
}

//! Create bind for an optional JSON field with null default value .
template<
		typename Reader_Writer,
		typename Field_Type,
		typename Validator = empty_validator_t >
auto
optional_null(
	Reader_Writer reader_writer,
	string_ref_t field_name,
	Field_Type && field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t<
			Reader_Writer,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			optional_attr_null_t,
			Validator >;

	return binder_type_t{
			std::move(reader_writer),
			field_name,
			field,
			optional_attr_null_t{},
			std::move( validator ) };
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

//! Create bind for an optional JSON field with null default value.
template<
		typename Reader_Writer,
		typename Field_Type,
		typename Validator = empty_validator_t >
auto
optional(
	Reader_Writer reader_writer,
	string_ref_t field_name,
	Field_Type & field,
	std::nullptr_t,
	Validator validator = Validator{} )
{
	return optional_null(
			std::move(reader_writer),
			field_name,
			field,
			std::move( validator ) );
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
	Field_Type && field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t<
			default_reader_writer_t,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			optional_nodefault_attr_t,
			Validator >;

	return binder_type_t{
			default_reader_writer_t{},
			field_name,
			field,
			optional_nodefault_attr_t{},
			std::move( validator ) };
}

//! Create bind for an optional JSON field without default value.
template<
		typename Reader_Writer,
		typename Field_Type,
		typename Validator = empty_validator_t >
auto
optional_no_default(
	Reader_Writer reader_writer,
	string_ref_t field_name,
	Field_Type && field,
	Validator validator = Validator{} )
{
	using binder_type_t = binder_t<
			Reader_Writer,
			// NOTE: since v.0.2.12 this way of detection of Field_Type
			// for binder_t must be used.
			details::meta::field_type_from_reference_t< decltype(field) >,
			optional_nodefault_attr_t,
			Validator >;

	return binder_type_t{
			std::move(reader_writer),
			field_name,
			field,
			optional_nodefault_attr_t{},
			std::move( validator ) };
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
	const bool result = output_doc.Accept( writer );
	if( !result )
		throw ex_t{ "to_json: output_doc.Accept(writer) returns false" };

	return { buffer.GetString(), buffer.GetSize() };
}

//
// to_json with PrettyWriter
//

struct pretty_writer_params_t
{
	char m_indent_char{ ' ' };
	unsigned m_indent_char_count{ 4u };
	rapidjson::PrettyFormatOptions m_format_options{ rapidjson::kFormatDefault };

	//
	// Setters
	//
	pretty_writer_params_t &
	indent_char( char ch ) &
	{
		m_indent_char = ch;
		return *this;
	}

	pretty_writer_params_t &&
	indent_char( char ch ) &&
	{
		return std::move(this->indent_char(ch));
	}
	
	pretty_writer_params_t &
	indent_char_count( unsigned count ) &
	{
		if( !count ) count = 1u;

		m_indent_char_count = count;
		return *this;
	}

	pretty_writer_params_t &&
	indent_char_count( unsigned count ) &&
	{
		return std::move(this->indent_char_count(count));
	}

	pretty_writer_params_t &
	format_options( rapidjson::PrettyFormatOptions opt ) &
	{
		m_format_options = opt;
		return *this;
	}

	pretty_writer_params_t &&
	format_options( rapidjson::PrettyFormatOptions opt ) &&
	{
		return std::move(this->format_options(opt));
	}
};

template< typename Dto >
std::string
to_json( const Dto & dto, pretty_writer_params_t writer_params )
{
	rapidjson::Document output_doc;
	json_output_t jout{
		output_doc, output_doc.GetAllocator() };

	jout << dto;

	rapidjson::StringBuffer buffer;

	rapidjson::PrettyWriter< rapidjson::StringBuffer > writer( buffer );
	writer.SetIndent(
			writer_params.m_indent_char,
			writer_params.m_indent_char_count );
	writer.SetFormatOptions(
			writer_params.m_format_options );

	const bool result = output_doc.Accept( writer );
	if( !result )
		throw ex_t{ "to_json: output_doc.Accept(writer) returns false" };

	return { buffer.GetString(), buffer.GetSize() };
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

//! Helper function to read DTO from already parsed document.
template< typename Type >
Type
from_json( const rapidjson::Value & json )
{
	json_input_t jin{ json };

	Type result{};

	jin >> result;

	return result;
}

//! Helper function to read from already parsed document to already
//! constructed DTO.
template< typename Type >
void
from_json( const rapidjson::Value & json, Type & o )
{
	json_input_t jin{ json };

	jin >> o;
}

//! Helper function to read DTO from json-string in form of string_ref.
/*!
 * @since v.0.2.9
 */
template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
Type
from_json( const string_ref_t & json )
{
	rapidjson::Document document;

	document.Parse< Rapidjson_Parseflags >( json.s, json.length );

	check_document_parse_status( document );

	return from_json<Type>( document );
}

//! Helper function to read DTO from json-string.
template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
Type
from_json( const std::string & json )
{
	return from_json< Type, Rapidjson_Parseflags >( make_string_ref(json) );
}

//! Helper function to read DTO from json-string.
/*!
 * This version reads the JSON content from a raw char pointer
 * (it's assumed that it is a null-terminated string).
 *
 * @since v.0.2.9
 */
template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
Type
from_json( const char * json )
{
	return from_json< Type, Rapidjson_Parseflags >( make_string_ref(json) );
}

//! Helper function to read an already instantiated DTO.
/*!
 * This version reads the JSON content from a string_ref_t (aka
 * rapidjson::Value::StringRefType) object.
 *
 * @since v.0.2.9
 */
template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
void
from_json( const string_ref_t & json, Type & o )
{
	rapidjson::Document document;

	document.Parse< Rapidjson_Parseflags >( json.s, json.length );

	check_document_parse_status( document );

	from_json( document, o );
}

//! Helper function to read an already instantiated DTO.
template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
void
from_json( const std::string & json, Type & o )
{
	from_json< Type, Rapidjson_Parseflags >( make_string_ref(json), o );
}

//! Helper function to read an already instantiated DTO.
/*!
 * This version reads the JSON content from a raw char pointer
 * (it's assumed that it is a null-terminated string).
 *
 * @since v.0.2.9
 */
template< typename Type, unsigned Rapidjson_Parseflags = rapidjson::kParseDefaultFlags >
void
from_json( const char * json, Type & o )
{
	from_json< Type, Rapidjson_Parseflags >( make_string_ref(json), o );
}

/*!
 * @brief Serialize an object into specified stream.
 *
 * @note
 * Default formatting will be used. If one needs pretty-formatted
 * output then another overload has to be used.
 */
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

	const bool result = output_doc.Accept( writer );
	if( !result )
		throw ex_t{ "to_stream: output_doc.Accept(writer) returns false" };
}

/*!
 * @brief Serialize an object into specified stream with using pretty-writer.
 *
 * @since v.0.2.14
 */
template< typename Type >
void
to_stream(
	//! The target stream.
	std::ostream & to,
	//! Object to be serialized.
	const Type & type,
	//! Parameters for pretty-writer.
	pretty_writer_params_t writer_params )
{
	rapidjson::Document output_doc;
	json_dto::json_output_t jout{
		output_doc, output_doc.GetAllocator() };

	jout << type;

	rapidjson::OStreamWrapper wrapper{ to };
	rapidjson::PrettyWriter< rapidjson::OStreamWrapper > writer{ wrapper };
	writer.SetIndent(
			writer_params.m_indent_char,
			writer_params.m_indent_char_count );
	writer.SetFormatOptions(
			writer_params.m_format_options );

	const bool result = output_doc.Accept( writer );
	if( !result )
		throw ex_t{ "to_stream: output_doc.Accept(writer) returns false" };
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

