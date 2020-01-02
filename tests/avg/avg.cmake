#
# Copyright 2019-2020, Marcin Ślusarz <marcin.slusarz@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#     * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

test("csv-avg -c id" data/id-column-3-rows.csv avg/id.csv data/empty.txt 0
	avg)

test("csv-avg -c col1,col2,col3" data/3-numeric-columns-4-rows.csv avg/3-columns.csv data/empty.txt 0
	avg-3-columns)

test("csv-avg -c col3,col1" data/3-numeric-columns-4-rows.csv avg/2-columns.csv data/empty.txt 0
	avg-2-columns)

test("csv-avg -c col3,col1 -s" data/3-numeric-columns-4-rows.csv avg/2-columns-s.txt data/empty.txt 0
	avg-2-columns-s)

test("csv-avg -T t1 -c id,something" avg/2-tables.csv avg/2-tables-avg1.csv data/empty.txt 0
	avg-2tables-avg1)

test("csv-avg -T t2 -c id" avg/2-tables.csv avg/2-tables-avg2.csv data/empty.txt 0
	avg-2tables-avg2)

test("csv-avg --help" data/empty.csv avg/help.txt data/empty.txt 2
	avg_help)

test("csv-avg --version" data/empty.csv data/git-version.txt data/empty.txt 0
	avg_version)
