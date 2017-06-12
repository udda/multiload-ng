# Copyright (C) 2017 Mario Cianciolo <mr.udda@gmail.com>
#
# This file is part of Multiload-ng.
#
# Multiload-ng is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Multiload-ng is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Multiload-ng.  If not, see <http://www.gnu.org/licenses/>.


# MULTILOAD_INIT(PACKAGE_NAME)
# Initialize common variables for use in configure.ac
AC_DEFUN([MULTILOAD_INIT],
[
	m4_define([MULTILOAD_PACKAGE],   [$1])
	m4_define([MULTILOAD_VERSION],   [2.0.0])
	m4_define([MULTILOAD_EMAIL],     [mr.udda@gmail.com])
	m4_define([MULTILOAD_URL],       [https://github.com/udda/multiload-ng])
	m4_define([MULTILOAD_COPYRIGHT], [Copyright © 2017 Mario Cianciolo])
])
