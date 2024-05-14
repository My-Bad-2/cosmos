project(
    'cosmos',
    ['c', 'cpp'],
    default_options: [
        'warning_level=2',
        'debug=false',
        'optimization=2',
        'c_std=gnu2x',
        'build.c_std=gnu2x',
        'cpp_std=gnu++23',
        'build.cpp_std=gnu++23',
        'b_staticpic=false',
    ],
    meson_version: '>=0.64.0',
    version: '0.0.1',
)

# Project Options and Settings
subdir('meson/compiler')
subdir('meson/compiler/c')
subdir('meson/compiler/cpp')

if get_option('debug')
    add_project_arguments('-DDEBUG', language: ['c', 'cpp'])
endif

if get_option('kernel-arch') == 'x86_64'
    desired_common_compile_flags += [
        '-march=x86-64',
        '-msoft-float',
        '-mno-mmx',
        '-mno-sse',
        '-mno-sse2',
        '-mno-red-zone',
        '-mcmodel=kernel',
    ]

    desired_common_link_flags += [
        '-Wl,-z,noexecstack',
        '-Wl,-T,' + meson.current_source_dir() / 'scripts/link-kernel.x86_64.ld',
    ]

    if (cpp_compiler_id == 'clang')
        add_global_arguments('-target', 'x86_64-pc-elf', language: ['c', 'cpp'])
    endif
endif

compile_settings_list = [
    {'lang': 'c', 'compiler': c_compiler, 'flags': desired_c_compile_flags},
    {
        'lang': 'cpp',
        'compiler': cpp_compiler,
        'flags': desired_cpp_compile_flags,
    },
]

link_settings_list = [
    {'lang': 'c', 'compiler': c_compiler, 'flags': []},
    {'lang': 'cpp', 'compiler': cpp_compiler, 'flags': []},
]

subdir('meson/compiler/check-and-apply-flags')

# Subprojects
limine_subproject = subproject(
    'limine',
    default_options: [
        'limine-arch=' + get_option('kernel-arch'),
        'limine-compiler=' + c_compiler_id,
    ],
    required: true,
)

limine_subproject_include = limine_subproject.get_variable('limine_include')

ovmf_subproject = subproject('ovmf')

# Process Source Tree
clangtidy_files = []

subdir('libc')
subdir('kernel')

# Run kernel
package_files = [meson.current_source_dir() / 'scripts/limine.cfg']
package_files += limine_subproject.get_variable('limine_binaries')

subdir('meson/package')
subdir('meson/qemu')

# Tooling Modules
subdir('meson/analysis/clang-tidy')
subdir('meson/format')
