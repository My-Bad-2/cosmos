clangformat_excludes = get_variable('clangformat_excludes', [])

if clangformat_excludes.length() > 0
    clangformat_excludes_processed = ''
    foreach entry : clangformat_excludes
        clangformat_excludes_processed += entry + ','
    endforeach

    clangformat_excludes_processed = ['-e', clangformat_excludes_processed]
else
    clangformat_excludes_processed = []
endif

clangformat_default_targets = [meson.project_source_root() / 'src']

clangformat_targets = get_variable(
    'clangformat_targets',
    clangformat_default_targets,
)
clangformat_additional_targets = get_variable(
    'clangformat_additional_targets',
    [],
)

clangformat_targets_processed = ''
foreach entry : clangformat_targets + clangformat_additional_targets
    clangformat_targets_processed += entry + ','
endforeach

clangformat_default_file_types = ['*.c', '*.h', '*.cpp', '*.hpp']

clangformat_file_types = get_variable(
    'clangformat_file_types',
    clangformat_default_file_types,
)
clangformat_additional_file_types = get_variable(
    'clangformat_additional_file_types',
    [],
)

clangformat_file_types_processed = ''
foreach entry : clangformat_file_types + clangformat_additional_file_types
    clangformat_file_types_processed += entry + ','
endforeach

clangformat_default_args = [
    clangformat_excludes_processed,
    clangformat_targets_processed,
    clangformat_file_types_processed,
]

clangformat_args = get_variable('clangformat_args', clangformat_default_args)
clangformat = find_program('clang-format', required: false, disabler: true)
clangformat_script = files('format.sh')

if clangformat.found()
    run_target('format', command: [clangformat_script, clangformat_args])

    run_target(
        'format-patch',
        command: [clangformat_script, '-p', clangformat_args],
    )
endif

cp = find_program('cp')

run_target(
    'update-format-rules',
    command: [
        cp,
        meson.current_source_dir() + '/.clang-format',
        meson.project_source_root(),
    ],
)
