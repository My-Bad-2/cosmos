kernel_sources = files('main.c')

kernel_include = [
    include_directories('include/'),
    include_directories('include/arch' / get_option('kernel-arch')),
]

kernel_include += limine_subproject_include

subdir('arch' / get_option('kernel-arch'))

clangtidy_files += kernel_sources

kernel_exec = executable(
    'kernel',
    kernel_sources,
    include_directories: [kernel_include, libc_include],
    link_with: [libc_library],
    install: false,
)
