project('ping_many', 'c', default_options: ['warning_level=3', 'werror=true'])

add_global_arguments('-Wno-unused-parameter', language : 'c')
add_global_arguments('-Wno-pedantic',         language : 'c')

ping_many_src = ['net.c', 'echo.c', 'addr.c']
main_src      = ping_many_src + ['main.c']

executable('ping_many', main_src)

add_languages('cpp')
gtest_dep = dependency('gtest', main : true, required : false)

test_src = ping_many_src + ['test.cc']
e = executable('testprog', test_src, dependencies : gtest_dep)

test('gtest test', e)
