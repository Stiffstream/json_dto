require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/floats/prj.ut.rb",
		"test/floats/prj.rb" )
)
