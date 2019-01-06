require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/ensure_object/prj.ut.rb",
		"test/ensure_object/prj.rb" )
)
