require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/maybe_null/prj.ut.rb",
		"test/maybe_null/prj.rb" )
)
