clangtidy = find_program('clang-tidy', required: false, disabler: true)
clangtidy_script = files('clang-tidy.sh')

if clangtidy.found()
    if get_variable('clangtidy_files', []).length() == 0
        warning(
            'clangtidy_files list not defined befored including module, so the target is disabled.',
        )
    else
        run_target(
            'clang-tidy',
            command: [
                clangtidy_script,
                get_variable('clangtidy_options', ''),
                clangtidy_files,
            ],
        )

        run_target(
            'clang-tidy-fix',
            command: [
                clangtidy_script,
                get_variable('clangtidy_options', ''),
                '--fix-errors',
                '--fix-notes', clangtidy_files,
            ],
        )
    endif
endif
