require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/user_defined_io/prj.ut.rb",
		"test/user_defined_io/prj.rb" )
)
