#include <catch2/catch.hpp>

#include <iostream>
#include <limits>

#include <rapidjson/document.h>
#include <json_dto/pub.hpp>

using namespace json_dto;

TEST_CASE("constructors", "[basic]" )
{
	SECTION( "default" )
	{
		nullable_t< std::string > str;

		REQUIRE_FALSE( str );
	}

	SECTION( "nullptr" )
	{
		{
			nullable_t< std::string > str{ nullptr };
			REQUIRE_FALSE( str );
		}

		{
			nullable_t< std::string > str= nullptr;
			REQUIRE_FALSE( str );
		}
	}

	SECTION( "value" )
	{
		{
			nullable_t< std::string > str{ std::string{ "abc" } };
			REQUIRE( str );
			REQUIRE( *str == "abc" );
		}
	}

	SECTION( "another nullable_t" )
	{
		{
			nullable_t< std::string > other{};
			const nullable_t< std::string > & o = other;
			nullable_t< std::string > str{ o };

			REQUIRE_FALSE( str );
		}

		{
			nullable_t< std::string > other{ std::string{ "123" } };
			const nullable_t< std::string > & o = other;
			nullable_t< std::string > str{ o };

			REQUIRE( str );
			REQUIRE( *str == "123" );
		}

		{
			nullable_t< std::string > other{};
			nullable_t< std::string > str{ std::move( other ) };

			REQUIRE_FALSE( str );
		}

		{
			nullable_t< std::string > other{ std::string{ "xyz" } };

			REQUIRE( other );
			REQUIRE( *other == "xyz" );

			nullable_t< std::string > str{ std::move( other ) };

			REQUIRE( str );
			REQUIRE( *str == "xyz" );

			REQUIRE( other );
			REQUIRE( *other == "" );
		}
	}


	SECTION( "delegate" )
	{
		// substring (3)
		// string (const string& str, size_t pos, size_t len = npos);
		{
			const std::string base{ "0123456789xyz" };
			size_t pos = 5;
			nullable_t< std::string > str{ base, pos };
			REQUIRE( str );
			REQUIRE( *str == "56789xyz" );
		}

		{
			const std::string base{ "0123456789xyz" };
			size_t pos = 5;
			size_t len = 4;
			nullable_t< std::string > str{ base, pos, len };
			REQUIRE( str );
			REQUIRE( *str == "5678" );
		}

		{
			const char * base = "0123456789xyz";
			nullable_t< std::string > str{ base };
			REQUIRE( str );
			REQUIRE( *str == "0123456789xyz" );
		}

		{
			const char * base = "0123456789xyz";
			size_t len = 4;
			nullable_t< std::string > str{ base, len };
			REQUIRE( str );
			REQUIRE( *str == "0123" );
		}
	}
}

struct dtor_tester_t
{
	dtor_tester_t( bool & dtor_trigger )
		:	m_dtor_trigger{ dtor_trigger }
	{
		m_dtor_trigger = false;
	}

	~dtor_tester_t()
	{
		m_dtor_trigger = true;
	}

	bool & m_dtor_trigger;
};

TEST_CASE("destructor", "[basic]" )
{
	bool trigger{ true };

	{
		nullable_t< dtor_tester_t > obj{ trigger };
		REQUIRE( obj );
		REQUIRE_FALSE( trigger );
	}
	REQUIRE( trigger );

	{
		nullable_t< dtor_tester_t > obj{};
		REQUIRE_FALSE( obj );
		REQUIRE( trigger );
		trigger = false;
	}
	REQUIRE_FALSE( trigger );

	{
		nullable_t< dtor_tester_t > obj{ trigger };
		REQUIRE( obj );
		REQUIRE_FALSE( trigger );
		obj.reset();
		REQUIRE( trigger );
	}
}

TEST_CASE("swap", "[basic]" )
{
	nullable_t< std::string > s1{ nullptr };
	nullable_t< std::string > s2{ "123456789" };

	REQUIRE_FALSE( s1 );
	REQUIRE( s2 );
	REQUIRE( *s2 == "123456789" );

	s1.swap( s2 );

	REQUIRE( s1 );
	REQUIRE( *s1 == "123456789" );
	REQUIRE_FALSE( s2 );
}

TEST_CASE("assign", "[basic]" )
{
	SECTION( "const nullable_t &" )
	{
		nullable_t< std::string > s1{ nullptr };
		nullable_t< std::string > s2{ "123456789" };
		const nullable_t< std::string > & n1 = s1;
		const nullable_t< std::string > & n2 = s2;

		REQUIRE_FALSE( s1 );
		REQUIRE( s2 );
		REQUIRE( *s2 == "123456789" );

		s1 = n1;
		REQUIRE_FALSE( s1 );

		s1 = n2;
		REQUIRE( s1 );
		REQUIRE( *s1 == "123456789" );

		s1 = n1;
		REQUIRE( s1 );
		REQUIRE( *s1 == "123456789" );

		s1.reset();
		REQUIRE_FALSE( s1 );

		s2 = n2;
		REQUIRE( s2 );
		REQUIRE( *s2 == "123456789" );

		s2 = n1;
		REQUIRE_FALSE( s2 );
	}

	SECTION( "nullable_t && " )
	{
		nullable_t< std::string > s1{ nullptr };
		nullable_t< std::string > s2{ "123456789" };

		REQUIRE_FALSE( s1 );
		REQUIRE( s2 );
		REQUIRE( *s2 == "123456789" );

		s1 = std::move( s2 );
		REQUIRE( s1 );
		REQUIRE( *s1 == "123456789" );
		REQUIRE( s2 );
		REQUIRE( *s2 == "" );
	}

	SECTION( "const FIELD_TYPE & " )
	{
		const std::string str1{ "0123456789" };
		const std::string str2{ "xyz" };

		nullable_t< std::string > s{ nullptr };
		REQUIRE_FALSE( s );

		s = str1;
		REQUIRE( s );
		REQUIRE( *s == str1 );

		s = str2;
		REQUIRE( s );
		REQUIRE( *s == str2 );
	}

	SECTION( "FIELD_TYPE &&  " )
	{
		std::string str1{ "0123456789" };
		std::string str2{ "xyz" };

		nullable_t< std::string > s{ nullptr };
		REQUIRE_FALSE( s );

		s = std::move( str1 );
		REQUIRE( s );
		REQUIRE( *s == "0123456789" );
		REQUIRE( str1.empty() );

		s = std::move( str2 );
		REQUIRE( s );
		REQUIRE( *s == "xyz" );
	}

	SECTION( "std::nullptr_t" )
	{
		nullable_t< std::string > s{ nullptr };
		REQUIRE_FALSE( s );

		const std::string str1{ "0123456789" };
		const std::string str2{ "xyz" };

		s = str1;

		REQUIRE( s );
		REQUIRE( *s == str1 );

		s = nullptr;
		REQUIRE_FALSE( s );

		s = str2;

		REQUIRE( s );
		REQUIRE( *s == str2 );

		s = nullptr;
		REQUIRE_FALSE( s );
	}
}

TEST_CASE("equals", "[basic]" )
{
	nullable_t< std::string > s1{ nullptr };
	nullable_t< std::string > s2{ "123456789" };

	REQUIRE_FALSE( s1 == s2 );
	REQUIRE_FALSE( s2 == s1 );

	REQUIRE_FALSE( s1 == s1 );
	REQUIRE( s2 == s2 );
	REQUIRE( *s2 == *s2 );

	s1 = s2;
	REQUIRE( s1 == s2 );
	REQUIRE( s2 == s1 );

	s2.reset();
	REQUIRE_FALSE( s1 == s2 );
	REQUIRE_FALSE( s2 == s1 );

	s1.reset();
	REQUIRE_FALSE( s1 == s2 );
	REQUIRE_FALSE( s2 == s1 );
}

TEST_CASE("emplace", "[basic]" )
{
	nullable_t< std::string > s{};
	nullable_t< std::vector< std::string > > sv{};

	REQUIRE_FALSE( s );
	REQUIRE_FALSE( sv );

	s.emplace();
	sv.emplace();

	REQUIRE( s );
	REQUIRE( sv );

	s.reset();
	sv.reset();

	REQUIRE_FALSE( s );
	REQUIRE_FALSE( sv );

	s.emplace( "abc" );
	sv.emplace( {"a", "bc", "def", "ghij", "klmnop" } );
	REQUIRE( s );
	REQUIRE( *s == "abc" );
	REQUIRE( sv );
	REQUIRE( sv->size() == 5 );
	REQUIRE( sv->at( 0 ) == "a" );
	REQUIRE( sv->at( 1 ) == "bc" );
	REQUIRE( sv->at( 2 ) == "def" );
	REQUIRE( sv->at( 3 ) == "ghij" );
	REQUIRE( sv->at( 4 ) == "klmnop" );
}
