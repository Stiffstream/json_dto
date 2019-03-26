require 'mxx_ru/binary_unittest'

path = 'test/stl_containers_top_level'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
