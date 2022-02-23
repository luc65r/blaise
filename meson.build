project('blaise', 'c', default_options : [
  'c_std=gnu11',
  'warning_level=2',
  'c_args=-Werror=switch'
])

cc = meson.get_compiler('c')

unistring_dep = cc.find_library('unistring')
gmp_dep = dependency('gmp')
jansson_dep = dependency('jansson')

srcs = []

subdir('src')

executable('blaise', srcs, dependencies : [
  unistring_dep,
  gmp_dep,
  jansson_dep,
], include_directories : include_directories('src'))
