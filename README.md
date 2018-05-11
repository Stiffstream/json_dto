Table of Contents
=================

   * [What Is json_dto?](#what-is-json_dto)
   * [What's new?](#whats-new)
      * [v.0.2.4](#v024)
      * [v.0.2.3](#v023)
      * [v.0.2.2](#v022)
      * [v.0.2.1](#v021)
      * [v.0.2.0](#v020)
   * [Obtain and build](#obtain-and-build)
      * [Prerequisites](#prerequisites)
      * [Obtaining](#obtaining)
         * [Cloning of Hg (Git) Repository](#cloning-of-hg-git-repository)
         * [MxxRu::externals recipe](#mxxruexternals-recipe)
      * [Build](#build)
   * [How to use it?](#how-to-use-it)
      * [Getting started](#getting-started)
      * [Non intrusive json_io()](#non-intrusive-json_io)
      * [Supported field types](#supported-field-types)
      * [Mandatory and optional fields](#mandatory-and-optional-fields)
         * [Mandatory fields](#mandatory-fields)
         * [Optional fields](#optional-fields)
            * [Optional fields and std::optional](#optional-fields-and-stdoptional)
      * [Array support](#array-support)
      * [Nullable fields](#nullable-fields)
      * [Complex types](#complex-types)
      * [Inheritance](#inheritance)
      * [Validators](#validators)
         * [Standard validators](#standard-validators)
      * [User defined IO](#user-defined-io)
   * [License](#license)

*TOC Created by [gh-md-toc](https://github.com/ekalinin/github-markdown-toc).*

# What Is json_dto?

*json_dto* library is a small header-only helper
for converting data between json representation
and c++ structs. DTO here stands for data transfer object.
It was made and used as a part of a larger project
inside [StiffStream](http://stiffstream.com).
And since Fall 2016 is ready for public. We are still using it for
working with JSON in various projects.

# What's new?

## v.0.2.5

Modify cmake-scripts for vcpkg port (target name `json-dto::json-dto`).

## v.0.2.4

Add cmake support.

Make string value setter independent to `RAPIDJSON_HAS_STDSTRING`.

## v.0.2.3

Bug fix in support of `std::vector<bool>`.

## v.0.2.2

Bug fix in implementation of `std::optional` support.

New example tutorial6.1 added.

## v.0.2.1

Some code style changes to meet expectations of some users.

## v.0.2.0

New format of `read_json_value` function. **NOTE: this is a breaking change!**

Support for `std::optional` (and `std::experimental::optional`) added. Note: this
may require to specify C++17 standard in compiler params (like `/std:c++17` for MSVC or
`-std=c++17` for GCC).

# Obtain and build

## Prerequisites

To use *json_dto* it is necessary to have:

* C++14 compiler (VC++14.0, GCC 5.2 or above, clang 3.6 or above)
* [rapidjson](https://github.com/miloyip/rapidjson)

And for building with mxxru:

* [rapidjson_mxxru](https://bitbucket.org/sobjectizerteam/rapidjson_mxxru-1.0)
	(v.1.0.0 or above)
* [Mxx_ru](https://sourceforge.net/projects/mxxru/) 1.6.13 or above

And for running test:

* [CATCH](https://github.com/philsquared/Catch) 1.10.0

## Obtaining

Assuming that *Mercurial* and *Mxx_ru* are already installed.

### Cloning of Hg (Git) Repository

```
hg clone https://bitbucket.org/sobjectizerteam/json_dto-0.2
git clone https://github.com/Stiffstream/json_dto.git
```

And then:
```
cd json_dto-0.2
mxxruexternals
```
to download and extract *json_dto*'s dependencies.

### MxxRu::externals recipe

For *json_dto* itself:
```ruby
MxxRu::arch_externals :json_dto do |e|
  e.url 'https://bitbucket.org/sobjectizerteam/json_dto-0.2/get/v.0.2.0.tar.bz2'

  e.map_dir 'dev/json_dto' => 'dev'
end
```

For *rapidjson* and *rapidjson_mxxru* dependencies:

```ruby
MxxRu::arch_externals :rapidjson do |e|
  e.url 'https://github.com/miloyip/rapidjson/archive/v1.1.0.zip'

  e.map_dir 'include/rapidjson' => 'dev/rapidjson/include'
end

MxxRu::arch_externals :rapidjson_mxxru do |e|
  e.url 'https://bitbucket.org/sobjectizerteam/rapidjson_mxxru-1.0/get/v.1.0.0.tar.bz2'

  e.map_dir 'dev/rapidjson_mxxru' => 'dev'
end
```

## Build

While *json_dto* is header-only library test and samples require a build.

Compiling with Mxx_ru:

```
hg clone https://bitbucket.org/sobjectizerteam/json_dto-0.2
cd json_dto
mxxruexternals
cd dev
ruby build.rb
```

*NOTE.* It might be necessary to set up `MXX_RU_CPP_TOOLSET` environment variable,
see Mxx_ru documentation for further details.

# How to use it?

**An important notice:** if you do not use Mxx_ru for building your project then
add the following defines for your project:

```C++
RAPIDJSON_HAS_STDSTRING
RAPIDJSON_HAS_CXX11_RVALUE_REFS
```

If you use Mxx_ru and `rapidjson_mxxru/prj.rb` then these definitions will be
added automatically.

## Getting started

To start using *json_dto* simply include `<json_dto/pub.hpp>` header.

The usage principle of *json_dto* is borrowed from
[Boost serialization](http://www.boost.org/doc/libs/1_61_0/libs/serialization/doc/tutorial.html)
where *rapidjson::Value* plays the role of archive.

Let's assume we have a c++ structure that must be serialized to JSON
and deserialized from JSON:

```C++
struct message_t
{
	std::string m_from;
	std::int64_t m_when;
	std::string m_text;
};
```

For integrating this struct with *json_dto* facilities the struct must be
modified as follows:

```C++
struct message_t
{
	std::string m_from;
	std::int64_t m_when;
	std::string m_text;

	// Entry point for json_dto.
	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io & json_dto::mandatory("from", m_from)
			& json_dto::mandatory("when", m_when)
			& json_dto::mandatory("text", m_text);
	}
};
```

Here `json_io()` function is an entry point for *json_dto* library.
It describes how to read the data from *rapidjson::Value*
(that is usualy parsed from string) and how to set the data
in *rapidjson::Value*.
`json_io()` is a template function. It allows to have a single
description for read and write operations.
The template is instantiated with `Json_Io=json_dto::json_input_t`
for reading dto from JSON-value and `Json_Io=json_dto::json_output_t` for writing
dto to JSON-value. Both `json_dto::json_input_t` and `json_dto::json_output_t`
override `operator&` for splitting io functionality.

There are also iostream-like overrides for `operator<<` and `operator>>`:

```C++
template<typename Dto>
json_input_t &
operator>>(json_input_t & i, Dto & v);

template<typename Dto>
inline json_output_t &
operator<<(json_output_t & o, const Dto & v);
```

But they are only helpful for top level read/write operations.

In general *json_dto* gets data from `rapidjson::Value` and puts
the data into `rapidjson::Value`. So read/write operations look like this:

```C++
// Read
rapidjson::Document document;

// ...

json_dto::json_input_t jin{ document };

message_t msg;
jin >> msg;

// If no exceptions were thrown DTO contains data received from JSON.
```

```C++
// Write
rapidjson::Document document;

// ...

json_dto::json_output_t jout{ document, document.GetAllocator() };

const message_t msg = get_message();
jout << msg;

// If no exceptions were thrown document contains data received from DTO.
```

But usually it is enough to work with `std::string` objects, so *json_dto*
comes with handy to/from string helpers:

```C++
template<typename Dto>
std::string
to_json(const Dto & dto);

template<typename Type>
Type
from_json(const std::string & json);
```

[See full example](./dev/sample/tutorial1/main.cpp).

[See full example without to/from string helpers](./dev/sample/tutorial1.1/main.cpp).

## Non intrusive `json_io()`

When it is unwanted to add an extra function to C++ structure
it is possible to use a non intrusive `json_io()` version.
In previous example dto part will look like this:

```C++
struct message_t
{
	std::string m_from;
	std::int64_t m_when;
	std::string m_text;
};

namespace json_dto
{

template<typename Json_Io>
void json_io(Json_Io & io, message_t & msg)
{
	io & json_dto::mandatory("from", msg.m_from)
		& json_dto::mandatory("when", msg.m_when)
		& json_dto::mandatory("text", msg.m_text);
}

} /* namespace json_dto */
```

[See full example](./dev/sample/tutorial2/main.cpp).

**Note** that it is necessary to define `json_io()` in namespace `json_dto`.

## Supported field types

Out of the box *json_dto* lib supports following types:

* Bool: bool;
* Numeric:
	std::int16_t, std::uint16_t,
	std::int32_t, std::uint32_t,
	std::int64_t, std::uint64_t,
	double;
* Strings: std::string
* C++17 specific: std::optional (or std::experimental::optional)

Example:

```C++
struct supported_types_t
{
	bool m_bool{ false };

	std::int16_t m_int16{};
	std::uint16_t m_uint16{};

	std::int32_t m_int32{};
	std::uint32_t m_uint32{};

	std::int64_t m_int64{};
	std::uint64_t m_uint64{};
	double m_double{};

	std::string m_string{};
};

namespace json_dto
{

template<typename Json_Io>
void json_io(Json_Io & io, supported_types_t & obj)
{
	io & json_dto::mandatory("bool", obj.m_bool)
		& json_dto::mandatory("int16", obj.m_int16)
		& json_dto::mandatory("uint16", obj.m_uint16)
		& json_dto::mandatory("int32", obj.m_int32)
		& json_dto::mandatory("uint32", obj.m_uint32)
		& json_dto::mandatory("int64", obj.m_int64)
		& json_dto::mandatory("uint64", obj.m_uint64)
		& json_dto::mandatory("double", obj.m_double)
		& json_dto::mandatory("string", obj.m_string);
}

} /* namespace json_dto */
```

[See full example](./dev/sample/tutorial3/main.cpp)

## Mandatory and optional fields

Each data member (at least those of them which are considered to be present in JSON)
in C++ struct binds to JSON field. Bind can be mandatory or optional.
Optional bind is extended with default value, but it is also possible
to set optional fields without defaults.
Also it is possible to add a value validator to the bind.

Binds are created by `mandatory()`, `optional()` and
`optional_no_default()` functions. These functions returns a *field binder*.
*Binder* is an instantiation of `binder_t` template class
which carries a part of internal logic
capable for handling field input/output operations.
With the help of *binders* `Json_Io` object understands how read, write and validate
the underlying field.

### Mandatory fields

Binders for mandatory fields are created via `mandatory()` function:

```C++
template<
		typename Field_Type,
		typename Validator = empty_validator_t>
auto mandatory(
	string_ref_t field_name,
	Field_Type & field,
	Validator validator = Validator{});
```

First parameter *field_name* is of type `string_ref_t`
which is an alias for `rapidjson::Value::StringRefType`.
Typically it is enough to pass `std::string` or `char *` args
(see *rapidjson*
[documentation](http://rapidjson.org/classrapidjson_1_1_generic_value.html)
for further details).
The second parameter is a reference to the instance of the field value.
The third parameter is optional and it sets validator on fields value.
Validators will be described later. By default `empty_validator_t`
is used, and as it says it does nothing.

### Optional fields

Binders for optional fields are created via `optional()` and
`optional_no_default()` functions:

```C++
template<
		typename Field_Type,
		typename Field_Default_Value_Type,
		typename Validator = empty_validator_t>
auto optional(
	string_ref_t field_name,
	Field_Type & field,
	Field_Default_Value_Type default_value,
	Validator validator = Validator{});

template<
		typename Field_Type,
		typename Validator = empty_validator_t>
auto optional_no_default(
	string_ref_t field_name,
	Field_Type & field,
	Validator validator = Validator{});
```

Parameters for functions are pretty much the same as for
`mandatory()` functon.

The only difference is the third parameter for `optional()` function,
it defines default value for a field if it is not defined in JSON.

In case of reading DTO, if optional field has default value
and JSON object doesn't define this field then default value is used.
In case of writing DTO, if value equals to default
then this field wouldn't be included in JSON.

For `optional()` there is a partial specification that accepts
`nullptr` argument as *default_value* parameter, it is usefull for
`nullable_t<T>` fields.

Example of using optional fields:

```C++
struct message_t
{
	std::string m_from;
	std::int64_t m_when;
	std::string m_text;
	std::string m_text_format;
	bool m_is_private{ false };
};

namespace json_dto
{

template<typename Json_Io>
void json_io(Json_Io & io, message_t & msg)
{
	io & json_dto::mandatory("from", msg.m_from)
		& json_dto::mandatory("when", msg.m_when)
		& json_dto::mandatory("text", msg.m_text)
		& json_dto::optional("text_format", msg.m_text_format, "text/plain")
		& json_dto::optional_no_default("is_private", msg.m_is_private);
}

} /* namespace json_dto */
```

[See full example](./dev/sample/tutorial4/main.cpp)

#### Optional fields and std::optional

Since v.0.2 it is possible to use C++17's `std::optional` template as a type
for field. In this case `std::nullopt` can be passed as third argument to
`json_dto::optional()` function:

```C++
struct email_data_t
{
	std::string m_from;
	std::string m_to;
	std::string m_subject;
	std::optional<std::vector<std::string>> m_cc;
	std::optional<std::vector<std::string>> m_bcc;
	...
	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io & json_dto::mandatory("from", m_from)
			& json_dto::mandatory("to", m_to)
			& json_dto::mandatory("subject", m_subject)
			& json_dto::optional("cc", m_cc, std::nullopt)
			& json_dto::optional("bcc", m_bcc, std::nullopt)
			...
	}
};
```

*Note.* If a compiler doesn't have `std::optional` but have
`std::experimental::optional` then `std::experimental::optional` and
`std::experimental::nullopt` can be used.


## Array support

JSON arrays are supported by *json_dto*, but there is one very important
limitation: all elements of the array must have the same type.
To set up an array simply use `std::vector<T>`.
If DTO member is of `std::vector<T>` type,
then corresponding JSON field is considered to be an array.
While for output the elements of the array-field will be automatically
of the same type, for successful input it is mandatory
that all elements of the array are convertible to vector value type.

Example for array-fields:

```C++
struct vector_types_t
{
	std::vector<bool> m_bool{};

	std::vector<std::int16_t> m_int16{};
	std::vector<std::uint16_t> m_uint16{};

	std::vector<std::int32_t> m_int32{};
	std::vector<std::uint32_t> m_uint32{};

	std::vector<std::int64_t> m_int64{};
	std::vector<std::uint64_t> m_uint64{};
	std::vector<double> m_double{};

	std::vector<std::string> m_string{};
};

namespace json_dto
{

template<typename Json_Io>
void json_io(Json_Io & io, vector_types_t & obj)
{
	io & json_dto::mandatory("bool", obj.m_bool)
		& json_dto::mandatory("int16", obj.m_int16)
		& json_dto::mandatory("uint16", obj.m_uint16)
		& json_dto::mandatory("int32", obj.m_int32)
		& json_dto::mandatory("uint32", obj.m_uint32)
		& json_dto::mandatory("int64", obj.m_int64)
		& json_dto::mandatory("uint64", obj.m_uint64)
		& json_dto::mandatory("double", obj.m_double)
		& json_dto::mandatory("string", obj.m_string);
}

} /* namespace json_dto */
```

[See full example](./dev/sample/tutorial5/main.cpp)

## Nullable fields

To support JSON null values, *json_dto* introduces `nullable_t<T>`.
It is required that nullable field is explicitly defined as
data member of type `nullable_t<T>`.

Interface of `nullable_t<T>` tries to mimic `std::optional` interface.

Example for `nullable_t<T>` field:

```C++
struct message_t
{
	message_t() {}

	message_t(
		std::string from,
		std::int64_t when,
		std::string text)
		:	m_from{ std::move(from) }
		,	m_when{ when }
		,	m_text{ std::move(text) }
	{}

	std::string m_from;
	std::int64_t m_when;
	std::string m_text;

	// Log level.
	// By default is constructed with null value.
	json_dto::nullable_t<std::int32_t> m_log_level{};
};

namespace json_dto
{

template<typename Json_Io>
void json_io(Json_Io & io, message_t & msg)
{
	io & json_dto::mandatory("from", msg.m_from)
		& json_dto::mandatory("when", msg.m_when)
		& json_dto::mandatory("text", msg.m_text)
		& json_dto::optional("log_level", msg.m_log_level, nullptr);
}

} /* namespace json_dto */

void
some_function( ... )
{
	// ...
	auto msg = json_dto::from_json<message_t>(json_data);

	// ...

	// If field is defined then its value can be obtained and used.
	if( msg.m_log_level )
		use_value(*msg.m_log_level);

	// ...

	msg.m_log_level = 1; // Set new value.

	// ...

	// equivalent to msg.m_log_level.reset();
	msg.m_log_level = nullptr; // Reset value.

	// ...
}
```

[See full example](./dev/sample/tutorial6/main.cpp)

Here default value for optional nullble field is `nullptr`.
And it means that absence of value is a default state for a field.
So when converting to JSON no-value nullable field
wouldn't be included in JSON as `"field":null` piece.

Nullable fields can be used with arrays:

```C++
struct message_t
{
	message_t() {}

	message_t(
		std::string from,
		std::int64_t when,
		std::string text)
		:	m_from{ std::move(from) }
		,	m_when{ when }
		,	m_text{ std::move(text) }
	{}

	// Who sent a message.
	std::string m_from;

	// When the message was sent (unixtime).
	std::int64_t m_when;

	// Message text.
	std::string m_text;

	// Log level.
	// By default is constructed with null value.
	json_dto::nullable_t<std::int32_t> m_log_level{};

	json_dto::nullable_t< std::vector<std::string> > m_tags{};
};

namespace json_dto
{

template<typename Json_Io>
void json_io(Json_Io & io, message_t & msg)
{
	io & json_dto::mandatory("from", msg.m_from)
		& json_dto::mandatory("when", msg.m_when)
		& json_dto::mandatory("text", msg.m_text)
		& json_dto::optional("log_level", msg.m_log_level, nullptr)
		& json_dto::optional("tags", msg.m_tags, nullptr);
}

} /* namespace json_dto */

void some_function( ... )
{
	// ...
	auto msg = json_dto::from_json<message_t>(json_data);

	// ...

	if( msg.m_tags )
		use_tags(*msg.m_tags);

	// ...
}

void some_other_function( ... )
{
	message_t msg{ ... };
	// ...

	// Add tags:
	msg.m_tags.emplace(); // equivalent to msg = std::vector<std::string>{};
	msg.m_tags->emplace_back("sample");
	msg.m_tags->emplace_back("tutorial");

	// ...
}
```

[See full example](./dev/sample/tutorial7/main.cpp)

## Complex types

*json_dto* allows to construct complex types with nested objects.
Using nested objects is pretty much the same as using data of a simple types.
Nested objects can be optional, nullable and be elements of array-fields.
However there are some constraints:

* nested type must be itself integrated with *json_dto*;
* type must be default-constructible (for input);
* for optional fields with default value equality operator must be defined
(more precisely an equality operator between nested type and type of passed default value).

Suppose there is a type which is already integrated with *json_dto*:

```C++
struct message_source_t
{
	std::int32_t m_thread_id{ 0 };
	std::string m_subsystem{};

	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io & json_dto::optional("thread_id", m_thread_id, 0)
			& json_dto::mandatory("subsystem", m_subsystem);
	}
};
```

Then it can be used as a nested object in other type:

```C++
struct message_t
{
	message_source_t m_from;
	std::int64_t m_when;
	std::string m_text;

	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io & json_dto::mandatory("from", m_from) // Exactly as with simple types.
			& json_dto::mandatory("when", m_when)
			& json_dto::mandatory("text", m_text);
	}
};
```

[See full example](./dev/sample/tutorial8/main.cpp)

[And see full example using nested objects as nullable and arrays](./dev/sample/tutorial9/main.cpp)

## Inheritance

*json_dto* works well with inheritance. It is possible to use
base implementation of `json_io()` function or completely override it.

For example derived class can use base class like this:

```C++
struct derived_t : public base_t
{
	//...

	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		base_t::json_io(io); // Run io on base class.

		// Run io on extra data:
		io & json_dto::mandatory("some_field", m_some_field)
			// ...
			;
	}
};
```

However for easier maintenance it is recommended to use non intrusive
`json_io()` function. Because if base class is integrated with *json_dto*
in non intrusive manner, then the following wouldn't work:

```C++
	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		// Base class doesn't provide such member function.
		base_t::json_io(io); // Run io on base class.
		// ...
	}
```

So it is preferred to put inheritance this way:

```C++
struct message_source_t
{
	std::int32_t m_thread_id{ 0 };
	std::string m_subsystem{};
};

namespace json_dto
{

template<typename Json_Io>
void json_io(Json_Io & io, message_source_t & m)
{
	io & json_dto::optional("thread_id", m.m_thread_id, 0)
		& json_dto::mandatory("subsystem", m.m_subsystem);
}

} /* namespace json_dto */

struct message_t : public message_source_t
{
	std::int64_t m_when;
	std::string m_text;

	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		json_dto::json_io(io, static_cast<message_source_t &>(*this));

		io & json_dto::mandatory("when", m_when)
			& json_dto::mandatory("text", m_text);
	}
};
```

[See full example](./dev/sample/tutorial10/main.cpp)

## Validators

*json_dto* allows to set validator on each field.
Validator is a function object (an object of a type supporting an
`operator()` member function) that receives a single parameter.

When handling input *json_dto* calls specified validator
and passes resulting field value as an argument.
If validator returns without throwing exception,
then field value considered to be valid, and execution continues.
Otherwise exception is catched and another will be thrown:
`json_dto::ex_t`. This exeption contains original exception description
supplemented with field name information.

When handling ouput *json_dto* calls specified validator before
trying to assign field value of JSON object. In all other respects
validation is the same as for input.

A simple example of using validators:

```C++
void check_all_7bit(
	const std::string & text)
{
	const auto it = std::find_if(std::begin(text), std::end(text),
			[](char c){ return c & 0x80; });

	if( std::end(text) != it )
	{
		throw std::runtime_error{
			"non 7bit char at pos " +
			std::to_string(std::distance(std::begin(text), it)) };
	}
}

struct message_t
{
	std::string m_from;
	std::int64_t m_when;

	// Message text. Must be 7bit ascii.
	std::string m_text;

	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io & json_dto::mandatory("from", m_from)
			& json_dto::mandatory("when", m_when)
			& json_dto::mandatory("text", m_text, check_all_7bit);
	}
};
```

[See full example](./dev/sample/tutorial11/main.cpp)

### Standard validators

*json_dto* comes with some useful ready to use validators for simple types.
They are defined in `<json_dto/pub.hpp>` header.

Standard validators curently available:

* `min_max_constraint_t<Num>` - range validator, targeted for numeric types;
* `one_of_validator_t<T>` - validator for set of values.

Standard validators are template classes with overloaded `operator()`.
And as they are template classes so for convenience
for each validator there is an auxiliary function that helps deduce
type of template instance from arguments:

```C++
template<typename Number>
auto min_max_constraint(Number min_value, Number max_value);

template<typename Field_Type>
auto one_of_constraint(std::initializer_list<Field_Type> values);
```

[See full example with standard validators](./dev/sample/tutorial12/main.cpp)

## User defined IO

It is possible to define custom IO logic for a specific type.
It might be useful for types when using object is an overkill,
for example time point that can be stored in format of 'YYYY.MM.DD hh:mm:ss'
or some token composed of several small items like '<item1>-<item1>-<item3>'.
But introducing custom IO logic for some type
requires to work with *rapidjson* API directly.

There are two way to introduce custom IO logic.

The first way uses C++'s Argument Dependent Lookup feature:
an user should define `read_json_value` and `write_json_value` in the same
namespace where types are defined. The right implementations of
`read_json_value` and `write_json_value` will be found by C++ compiler automatically.
For example:

```C++
namespace importance_levels
{

enum class level_t
	{
		low,
		normal,
		high
	};

// read_json_value and write_json_value for level_t are
// defined in importance_levels namespace.
// They will be found by argument dependent lookup.
void read_json_value(
	level_t & value,
	const rapidjson::Value & from)
{...}

void write_json_value(
	const level_t & value,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator)
{...}

} /* namespace importance_levels */
```

This approach also allows to define `read_json_value` and `write_json_value`
for user's template type. For example:

```C++
namespace demo
{

template<typename T>
class some_template
{...}

template<typename T>
void read_json_value(
	some_template<T> & value,
	const rapidjson::Value & from)
{...}

template<typename T>
void write_json_value(
	const some_template<T> & value,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator)
{...}

} /* namespace demo */

struct my_data_t
{
	demo::some_template<int> m_first;
	demo::some_template<double> m_second;
	...
	template<typename Json_Io>
	void json_io(Json_Io & io)
	{
		io & json_dto::mandatory("first", m_first)
			& json_dto::mandatory("second", m_second)
			...
	}
};
```

[See full example with custom IO and ADL](./dev/sample/tutorial15/main.cpp)

The second way uses explicit template specialization for 2 functons
inside `json_dto` namespace:

```C++
namespace json_dto
{

template<>
void read_json_value(
	Custom_Type & v,
	const rapidjson::Value & object)
{
	// ...
}

template<>
void write_json_value(
	const Custom_Type & v,
	rapidjson::Value & object,
	rapidjson::MemoryPoolAllocator<> & allocator)
{
	// ...
}

} /* namespace json_dto */
```

*json_dto* will consider these specializations for using with
specified `Custom_Type`. This way can be used when it is impossible
to place `read_json_value` and `write_json_value` into the namespace where
the type if defined (for example if it is standard type like `std::filesystem::path`).

[See full example with custom IO](./dev/sample/tutorial14/main.cpp)

# License

*json_dto* is distributed under
[BSD-3-Clause](http://spdx.org/licenses/BSD-3-Clause.html) license. See LICENSE
file for more information.

For the license of *rapidson* library see LICENSE file in *rapidson*
distributive.

For the license of *rapidson_mxxru* library see LICENSE file in *rapidson_mxxru*
distributive.

For the license of *CATCH* library see LICENSE file in *CATCH*
distributive.
