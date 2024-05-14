desired_common_warning_flags = [
    '-Wno-global-constructors',
    '-Wno-unknown-pragmas',
]

desired_common_compile_flags = [
    '-fdiagnostrics-show-option',
    '-fcolor-diagnostics',
    '-fno-pic',
    '-fno-pie',
    '-fno-omit-frame-pointer',
    '-fno-stack-protector',
    '-ffreestanding'
]

if meson.is_subproject() == false
    desired_common_compile_flags += desired_common_warning_flags
endif

desired_common_link_flags = [
    '-Wl,--gc-sections',
    '-Wl,-z,max-page-size=0x1000',
    '-nostdlib',
]