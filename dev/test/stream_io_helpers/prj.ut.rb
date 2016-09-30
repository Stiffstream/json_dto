require 'mxx_ru/binary_unittest'

Mxx_ru::setup_target(
	Mxx_ru::Binary_unittest_target.new(
		"test/stream_io_helpers/prj.ut.rb",
		"test/stream_io_helpers/prj.rb" )
)
