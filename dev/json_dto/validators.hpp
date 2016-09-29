/*
	json_dto
*/

/*!
	Some ready to use validators for typical cases.
*/

#pragma once

#include <json_dto/pub.hpp>

namespace json_dto
{

inline void
validator_error( const std::string & error_message )
{
	throw ex_t{ error_message };
}

//
// min_max_validator_t
//

//! Validate value in [ a, b].
template < typename NUMBER >
class min_max_validator_t
{
	public:
		min_max_validator_t( NUMBER min_value, NUMBER max_value )
			:	m_min_value{ min_value }
			,	m_max_value{ max_value }
		{
			if( m_min_value > m_max_value )
				validator_error(
					"invalid min-max validator: "
					"max_value cannot be less than min_value" );
		}


		void
		operator () ( NUMBER value ) const
		{
			if( m_min_value > value || m_max_value < value )
				validator_error(
					"invalid value: " + std::to_string( value ) +
					", must be in "
					"[ " + std::to_string( m_min_value ) +", " +
						std::to_string( m_max_value ) +" ]" );
		}

		void
		operator () ( const std::vector< NUMBER > & values ) const
		{
			for( auto v : values )
				(*this)( v );
		}

		template< typename FIELD_INNER_TYPE >
		void
		operator () ( const nullable_t< FIELD_INNER_TYPE > & value ) const
		{
			if( value )
				(*this)( *value );
		}

	private:
		NUMBER m_min_value{};
		NUMBER m_max_value{};
};

template < typename NUMBER >
auto
min_max_constraint( NUMBER min_value, NUMBER max_value )
{
	return min_max_validator_t< NUMBER >{min_value, max_value };
}

//
// one_of_validator_t
//

//! Validate in some predefined set.
template < typename FIELD_TYPE >
class one_of_validator_t
{
	public:
		one_of_validator_t( std::vector< FIELD_TYPE > values )
			:	m_values{ std::move( values ) }
		{}

		one_of_validator_t( std::initializer_list< FIELD_TYPE > values )
			:	m_values{ values }
		{}

		void
		operator () ( const FIELD_TYPE & value ) const
		{
			if( m_values.cend() ==
				std::find( m_values.cbegin(),  m_values.cend(), value ) )
			{
				validator_error( "invalid value, must be one of predefined values" );
			}
		}

		void
		operator () ( const std::vector< FIELD_TYPE > & values ) const
		{
			for( auto v : values )
				(*this)( v );
		}

		template< typename FIELD_INNER_TYPE >
		void
		operator () ( const nullable_t< FIELD_INNER_TYPE > & value ) const
		{
			if( value )
				(*this)( *value );
		}

	private:
		std::vector< FIELD_TYPE > m_values{};
};

template < typename FIELD_TYPE >
auto
one_of_constraint( std::initializer_list< FIELD_TYPE > values )
{
	return one_of_validator_t< FIELD_TYPE >{ values };
}

} /* namespace json_dto */
