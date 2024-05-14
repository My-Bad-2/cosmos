#!/bin/bash

MESON_BUILD_ROOT=${MESON_BUILD_ROOT:-./build}

cd "${MESON_BUILD_ROOT}"
sed -i.bak 's/-pipe//g' compile_commands.json

clang-tidy -quiet -p ${MESON_BUILD_ROOT} $@

mv compile_commands.json.bak compile_commands.json