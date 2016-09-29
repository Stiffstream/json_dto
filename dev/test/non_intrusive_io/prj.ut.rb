require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/non_intrusive_io/prj.ut.rb",
		"test/non_intrusive_io/prj.rb" )
)
