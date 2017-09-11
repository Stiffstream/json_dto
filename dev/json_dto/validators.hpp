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
template < typename Number >
class min_max_validator_t
{
	public:
		min_max_validator_t( Number min_value, Number max_value )
			:	m_min_value{ min_value }
			,	m_max_value{ max_value }
		{
			if( m_min_value > m_max_value )
				validator_error(
					"invalid min-max validator: "
					"max_value cannot be less than min_value" );
		}


		void
		operator()( Number value ) const
		{
			if( m_min_value > value || m_max_value < value )
				validator_error(
					"invalid value: " + std::to_string( value ) +
					", must be in "
					"[ " + std::to_string( m_min_value ) +", " +
						std::to_string( m_max_value ) +" ]" );
		}

		void
		operator()( const std::vector< Number > & values ) const
		{
			for( auto v : values )
				(*this)( v );
		}

		template< typename Field_Inner_Type >
		void
		operator()( const nullable_t< Field_Inner_Type > & value ) const
		{
			if( value )
				(*this)( *value );
		}

	private:
		Number m_min_value{};
		Number m_max_value{};
};

template < typename Number >
auto
min_max_constraint( Number min_value, Number max_value )
{
	return min_max_validator_t< Number >{min_value, max_value };
}

//
// one_of_validator_t
//

//! Validate in some predefined set.
template < typename Field_Type >
class one_of_validator_t
{
	public:
		one_of_validator_t( std::vector< Field_Type > values )
			:	m_values{ std::move( values ) }
		{}

		one_of_validator_t( std::initializer_list< Field_Type > values )
			:	m_values{ values }
		{}

		void
		operator()( const Field_Type & value ) const
		{
			if( m_values.cend() ==
				std::find( m_values.cbegin(),  m_values.cend(), value ) )
			{
				validator_error( "invalid value, must be one of predefined values" );
			}
		}

		void
		operator()( const std::vector< Field_Type > & values ) const
		{
			for( auto v : values )
				(*this)( v );
		}

		template< typename Field_Inner_Type >
		void
		operator()( const nullable_t< Field_Inner_Type > & value ) const
		{
			if( value )
				(*this)( *value );
		}

	private:
		std::vector< Field_Type > m_values{};
};

template < typename Field_Type >
auto
one_of_constraint( std::initializer_list< Field_Type > values )
{
	return one_of_validator_t< Field_Type >{ values };
}

} /* namespace json_dto */

