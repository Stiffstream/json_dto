require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/optional/prj.ut.rb",
		"test/optional/prj.rb" )
)
