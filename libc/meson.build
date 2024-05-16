libc_include = [include_directories('include')]

libc_sources = files('ctype.c')

subdir('string')

clangtidy_files += libc_sources

libc_library = static_library(
    'libc_library',
    libc_sources,
    include_directories: libc_include,
)
