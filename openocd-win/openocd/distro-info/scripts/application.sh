# -----------------------------------------------------------------------------
# This file is part of the xPack distribution.
#   (https://xpack.github.io)
# Copyright (c) 2019 Liviu Ionescu.
#
# Permission to use, copy, modify, and/or distribute this software
# for any purpose is hereby granted, under the terms of the MIT license.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Application specific definitions. Included with source.

# Used to display the application name.
XBB_APPLICATION_NAME=${XBB_APPLICATION_NAME:-"OpenOCD"}

# Used as part of file/folder paths.
XBB_APPLICATION_LOWER_CASE_NAME=${XBB_APPLICATION_LOWER_CASE_NAME:-"openocd"}

XBB_APPLICATION_DISTRO_NAME=${XBB_APPLICATION_DISTRO_NAME:-"xPack"}
XBB_APPLICATION_DISTRO_LOWER_CASE_NAME=${XBB_APPLICATION_DISTRO_LOWER_CASE_NAME:-"xpack"}
XBB_APPLICATION_DISTRO_TOP_FOLDER=${XBB_APPLICATION_DISTRO_TOP_FOLDER:-"xPacks"}

XBB_APPLICATION_DESCRIPTION="${XBB_APPLICATION_DISTRO_NAME} ${XBB_APPLICATION_NAME}"

declare -a XBB_APPLICATION_DEPENDENCIES=( openocd )
declare -a XBB_APPLICATION_COMMON_DEPENDENCIES=( libusb1 libusb-w32 libusb0 libftdi libiconv hidapi autotools texinfo )

# -----------------------------------------------------------------------------

XBB_GITHUB_ORG="${XBB_GITHUB_ORG:-"xpack-dev-tools"}"
XBB_GITHUB_REPO="${XBB_GITHUB_REPO:-"${XBB_APPLICATION_LOWER_CASE_NAME}-xpack"}"
XBB_GITHUB_PRE_RELEASES="${XBB_GITHUB_PRE_RELEASES:-"pre-releases"}"

XBB_NPM_PACKAGE="${XBB_NPM_PACKAGE:-"@xpack-dev-tools/${XBB_APPLICATION_LOWER_CASE_NAME}@next"}"

# -----------------------------------------------------------------------------

# If you want to build OpenOCD from another repo then uncomment the
# following defines and tweak as needed.

# XBB_APPLICATION_OPENOCD_GIT_URL="https://github.com/openocd-org/openocd.git"
# XBB_APPLICATION_OPENOCD_GIT_BRANCH="master"
# XBB_APPLICATION_OPENOCD_GIT_COMMIT="HEAD"

# -----------------------------------------------------------------------------
