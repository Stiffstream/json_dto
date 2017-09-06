gem 'Mxx_ru', '>= 1.3.0'

require 'mxx_ru/cpp'

MxxRu::Cpp::exe_target {

  # Define your target name here.
  target 'sample.tutorial15'

  required_prj 'rapidjson_mxxru/prj.rb'

  cpp_source 'main.cpp'
}

