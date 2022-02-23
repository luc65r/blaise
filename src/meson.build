flex = find_program('flex')
bison = find_program('bison')

lgen = generator(flex,
    output : ['@BASENAME@.c', '@BASENAME@.h'],
    arguments : ['--outfile=@OUTPUT0@', '--header-file=@OUTPUT1@', '@INPUT@'])

ygen = generator(bison,
    output : ['@BASENAME@.c', '@BASENAME@.h'],
    arguments : ['--output=@OUTPUT0@', '--defines=@OUTPUT1@', '@INPUT@'])

srcs += files(
  'ast_free.c',
  'ast_json.c',
  'eval.c',
  'main.c',
  'map.c',
  'vec.c',
)

srcs += lgen.process('scan.l')
srcs += ygen.process('parse.y')
