project('gitedit', 'c')
cc = meson.get_compiler('c')

curses = dependency('curses')
miniz = dependency('miniz')

add_project_arguments([
  '-D_POSIX_C_SOURCE=200809L',
  '-Wundef',
  '-Wno-unused-parameter',
  '-Wno-missing-braces',
  '-Wno-cpp',
  cc.get_supported_arguments([
    '-Wno-builtin-declaration-mismatch',      # gcc
    '-Wno-incompatible-library-redeclaration' # clang
  ])
], language: 'c')

executable('gitedit', [
  'src/main.c',
  'src/process.c',
  'src/tui.c',
  'src/utils.c',
  'src/editor.c',
  'src/git_objects.c',
  'src/backup.c',
],
  install: true,
  dependencies: [
    curses,
    miniz,
  ],
)