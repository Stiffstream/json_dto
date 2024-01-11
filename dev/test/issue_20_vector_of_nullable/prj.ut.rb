require 'mxx_ru/binary_unittest'

path = 'test/issue_20_vector_of_nullable'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
