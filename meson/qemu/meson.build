ovmf_binary_id = ''
qemu_executable_id = 'qemu-system-' + get_option('kernel-arch')
qemu_logfile = get_variable(
    'qemu_logfile',
    meson.current_build_dir() / 'qemu-log.txt',
)

qemu_args = [
    '-cpu',
    'max',
    '-smp',
    '1',
    '-m',
    '512M',
    '-rtc',
    'base=localtime',
    '-serial',
    'stdio',
    '-boot',
    'order=d,menu=on,splash-time=100',
]

qemu_debug_args = [
    '-no-reboot',
    '-no-shutdown',
    '-d',
    'int',
    '-D',
    qemu_logfile,
    '-S',
    '-s',
]

if get_option('kernel-arch') == 'x86_64'
    ovmf_binary_id = 'X64'
    qemu_args += [
        '-M',
        'q35',
    ]

    if get_option('debug')
        qemu_debug_args += ['-M', 'smm=off']
    endif
endif

if get_option('debug')
    qemu_args += qemu_debug_args
else
    qemu_args += [
        '-accel',
        'kvm',
    ]
endif

qemu_executable = find_program(qemu_executable_id, required: false)

if qemu_executable.found()
    custom_target(
        'run-uefi',
        input: package_command,
        output: 'run-uefi',
        command: [
            qemu_executable,
            qemu_args,
            '-cdrom', '@INPUT@',
            '-bios', ovmf_subproject.get_variable(ovmf_binary_id),
        ],
        build_by_default: true,
        console: true,
        install: false,
        depends: package_command,
    )

    if get_option('kernel-arch') == 'x86_64'
        custom_target(
            'run-bios',
            input: package_command,
            output: 'run-bios',
            command: [
                qemu_executable,
                qemu_args,
                '-cdrom', '@INPUT@',
            ],
            build_by_default: false,
            console: true,
            install: false,
            depends: package_command,
        )
    endif
endif
