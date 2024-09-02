project(
  'gamma-controller',
  'c',
  version: '0.8.0',
  license: 'GPL-2.0-only',
  default_options: [
    'c_std=c11',
    'warning_level=2',
  ],
)

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
  '-Wundef',
  '-Wlogical-op',
  '-Wmissing-include-dirs',
  '-Wpointer-arith',
  '-Winit-self',
  '-Wimplicit-fallthrough=2',
  '-Wendif-labels',
  '-Wstrict-aliasing=2',
  '-Woverflow',
  '-Wmissing-prototypes',
  '-Walloca',
  '-Wunused-macros',

  '-Wno-unused-parameter',
]), language: 'c')

wayland_client = dependency('wayland-client')

subdir('protocols')

executable(
  meson.project_name(),
  files('main.c'),
  include_directories: [],
  dependencies: [
    protos,
    wayland_client,
  ],
  install: true,
)
