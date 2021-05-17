require 'mxx_ru/binary_unittest'

path = "test/serialize_only_with_reader_writer"

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
