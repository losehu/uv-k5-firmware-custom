# -----------------------------------------------------------------------------
# This file is part of the xPack distribution.
#   (https://xpack.github.io)
# Copyright (c) 2020 Liviu Ionescu.
#
# Permission to use, copy, modify, and/or distribute this software
# for any purpose is hereby granted, under the terms of the MIT license.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Included by `scrips/test.sh`.

function tests_run_all()
{
  local test_bin_path="$1"

  openocd_test "${test_bin_path}"
}

# -----------------------------------------------------------------------------
