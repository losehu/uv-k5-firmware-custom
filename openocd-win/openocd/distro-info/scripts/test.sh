#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# DO NOT EDIT! Generated from xpacks/@xpack-dev-tools/xbb-helper/templates/*.
#
# This file is part of the xPack distribution.
#   (https://xpack.github.io)
# Copyright (c) 2020 Liviu Ionescu.
#
# Permission to use, copy, modify, and/or distribute this software
# for any purpose is hereby granted, under the terms of the MIT license.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Safety settings (see https://gist.github.com/ilg-ul/383869cbb01f61a51c4d).

if [[ ! -z ${DEBUG} ]]
then
  set ${DEBUG} # Activate the expand mode if DEBUG is anything but empty.
else
  DEBUG=""
fi

set -o errexit # Exit if command failed.
set -o pipefail # Exit if pipe failed.
set -o nounset # Exit if variable not set.

# Remove the initial space and instead use '\n'.
IFS=$'\n\t'

# -----------------------------------------------------------------------------
# Identify the script location, to reach, for example, the helper scripts.

script_path="$0"
if [[ "${script_path}" != /* ]]
then
  # Make relative path absolute.
  script_path="$(pwd)/$0"
fi

script_name="$(basename "${script_path}")"

script_folder_path="$(dirname "${script_path}")"
script_folder_name="$(basename "${script_folder_path}")"

# =============================================================================
# Run the application tests.

scripts_folder_path="${script_folder_path}"
project_folder_path="$(dirname ${script_folder_path})"
helper_folder_path="${project_folder_path}/xpacks/@xpack-dev-tools/xbb-helper"

tests_folder_path="$(dirname "${scripts_folder_path}")/tests"

# -----------------------------------------------------------------------------

source "${scripts_folder_path}/application.sh"

# Common definitions.
source "${helper_folder_path}/scripts/test-common.sh"

# Possibly override common definitions.
source "${scripts_folder_path}/tests/run.sh"
if [ -f "${scripts_folder_path}/tests/update.sh" ]
then
  source "${scripts_folder_path}/tests/update.sh"
fi

if [ ${#XBB_APPLICATION_COMMON_DEPENDENCIES[@]} -ne 0 ]
then
  for dependency in ${XBB_APPLICATION_COMMON_DEPENDENCIES[@]}
  do
    echo "Including ${helper_folder_path}/dependencies/${dependency}.sh..."
    source "${helper_folder_path}/dependencies/${dependency}.sh"
  done
fi

if [ ${#XBB_APPLICATION_DEPENDENCIES[@]} -ne 0 ]
then
  for dependency in ${XBB_APPLICATION_DEPENDENCIES[@]}
  do
    echo "Including ${scripts_folder_path}/dependencies/${dependency}.sh..."
    source "${scripts_folder_path}/dependencies/${dependency}.sh"
  done
fi

# -----------------------------------------------------------------------------

tests_parse_options "$@"

tests_perform_common

# Completed successfully.
exit 0

# -----------------------------------------------------------------------------
