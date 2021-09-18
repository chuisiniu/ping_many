project('ping_many', 'c', default_options: ['warning_level=3'])

add_global_arguments('-Wno-unused-parameter', language : 'c')
add_global_arguments('-Wno-pedantic', language : 'c')

addrmask_src = ['net.c', 'echo.c', 'main.c']

executable('ping_many', addrmask_src)
