cpp_compiler = meson.get_compiler('cpp', native: false)
cpp_compiler_id = cpp_compiler.get_id()

desired_cpp_compile_warning_flags = [
    '-Wold-style-cast',
    '-Wuseless-cast',
    '-Wextra-semi',
    '-Wstrict-null-sentinel',
    '-Wctor-dtor-privacy',
]

desired_cpp_compile_flags = [
    '-fno-exceptions',
    '-fno-rtti',
    '-fno-unwind-tables',
]

if meson.is_subproject() == false
    desired_cpp_compile_flags += desired_cpp_compile_warning_flags
endif
