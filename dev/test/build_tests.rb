#!/usr/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	required_prj( "test/numeric_limits/prj.ut.rb" )
	required_prj( "test/floats/prj.ut.rb" )
	required_prj( "test/optional/prj.ut.rb" )
	required_prj( "test/custom_reader_writer/prj.ut.rb" )
	required_prj( "test/std_optional/prj.ut.rb" )
	required_prj( "test/stl_containers/prj.ut.rb" )
	required_prj( "test/stl_containers_top_level/prj.ut.rb" )
	required_prj( "test/std_vector_bool/prj.ut.rb" )
	required_prj( "test/std_vector_top_level/prj.ut.rb" )
	required_prj( "test/nullable/prj.ut.rb" )
	required_prj( "test/non_intrusive_io/prj.ut.rb" )
	required_prj( "test/user_defined_io/prj.ut.rb" )
	required_prj( "test/user_defined_io_2/prj.ut.rb" )
	required_prj( "test/validators/prj.ut.rb" )
	required_prj( "test/stream_io_helpers/prj.ut.rb" )
	required_prj( "test/ensure_object/prj.ut.rb" )
	required_prj( "test/from_string_ref/prj.ut.rb" )
}

