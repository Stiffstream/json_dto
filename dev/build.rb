#!/usr/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target( MxxRu::BUILD_ROOT ) {

  toolset.force_cpp14
  global_include_path "."

  # If there is local options file then use it.
  if FileTest.exist?( "local-build.rb" )
    required_prj "local-build.rb"
  else
    if 'gcc' == toolset.name || 'clang' == toolset.name
      global_linker_option "-Wl,-rpath='$ORIGIN'"
    end

    if 'gcc' == toolset.name
      global_compiler_option '-Wextra'
      global_compiler_option '-Wall'
    end

    default_runtime_mode( MxxRu::Cpp::RUNTIME_RELEASE )
    MxxRu::enable_show_brief
    global_obj_placement MxxRu::Cpp::RuntimeSubdirObjPlacement.new( 'target' )
  end

  required_prj( "test/build_tests.rb" )

  required_prj( "sample/build_samples.rb" )
}

# vim:ts=2:sts=2:sw=2:expandtab
