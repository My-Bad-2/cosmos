package_files = get_variable('package_files', [])
package_name = get_variable(
    'package_name',
    meson.project_name() + get_option('kernel-arch'),
)
package_dir = get_variable('package_dir', '/tmp/' + package_name)

xorriso_args = []
if get_option('kernel-arch') == 'x86_64'
    xorriso_args += [
        '-b',
        'limine-bios-cd.bin',
        '-no-emul-boot',
        '-boot-load-size',
        '4',
        '-boot-info-table',
        '--efi-boot',
        'limine-uefi-cd.bin',
        '-efi-boot-part',
        '--efi-boot-image',
        '--protective-msdos-label',
    ]
endif

if find_program('xorriso', required: false).found()
    if package_files.length() == 0
        warning('No binary files variable defined: disabling package target.')
    else
        package_command = custom_target(
            'package',
            input: kernel_exec,
            output: package_name + '.iso',
            command: [
                'rm',
                '-rf', package_dir,
                '&&',
                'mkdir',
                '-p', package_dir,
                '&&',
                'cp',
                '@INPUT@',
                package_dir / 'kernel.elf',
                '&&',
                'cp',
                package_files,
                package_dir,
                '&&',
                'xorriso',
                '-as', 'mkisofs',
                xorriso_args,
                package_dir,
                '-o', '@OUTPUT@',
                '&&',
                limine_subproject.get_variable('limine_executable'),
                'bios-install',
                '@OUTPUT@',
                '&&',
                'rm',
                '-rf', package_dir,
            ],
            build_by_default: true,
            console: true,
            install: false,
            depends: kernel_exec,
        )
    endif
endif
