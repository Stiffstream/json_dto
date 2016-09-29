require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/validators/prj.ut.rb",
		"test/validators/prj.rb" )
)
