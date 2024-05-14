project('limine', 'c', license: 'BSD-2-Clause')

arch = get_option('limine-arch')

limine_binaries = [meson.current_source_dir() / 'limine-uefi-cd.bin']

limine_include = []
limine_include += include_directories('.')

limine_executable = meson.current_source_dir() / 'limine'

if arch == ''
    warning('No kernel architecture selected.')
elif arch == 'x86_64'
    limine_binaries += [
        meson.current_source_dir() / 'limine-bios.sys',
        meson.current_source_dir() / 'limine-bios-cd.bin',
        meson.current_source_dir() / 'BOOTX64.EFI',
        meson.current_source_dir() / 'BOOTIA32.EFI',
    ]
endif

run_command(
    [
        get_option('limine-compiler'),
        meson.current_source_dir() / 'limine.c',
        '-std=c99',
        '-Wall',
        '-Wextra',
        '-O2',
        '-o',
        meson.current_source_dir() / 'limine',
    ],
    check: true,
)

limine_dep = declare_dependency(include_directories: include_directories('.'))