require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/inside_array/prj.ut.rb",
		"test/inside_array/prj.rb" )
)
