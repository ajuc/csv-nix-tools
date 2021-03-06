#
# Copyright 2019, Marcin Ślusarz <marcin.slusarz@gmail.com>
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

if (LIBPROCPS_FOUND)

test("csv-ps | csv-count -c -R" data/empty.txt ps/columns.csv data/empty.txt 0
	ps_columns)

test("csv-ps -l | csv-count -c -R" data/empty.txt ps/columns-l.csv data/empty.txt 0
	ps_-l_columns)

test("csv-ps -ll | csv-count -c -R" data/empty.txt ps/columns-ll.csv data/empty.txt 0
	ps_-ll_columns)

test("csv-ps -lll | csv-count -c -R" data/empty.txt ps/columns-lll.csv data/empty.txt 0
	ps_-lll_columns)

test("csv-ps -llll | csv-count -c -R" data/empty.txt ps/columns-llll.csv data/empty.txt 0
	ps_-llll_columns)

test("csv-ps -M -c tid,cmd,age | csv-head -n 0" data/3-columns-3-rows-with-table.csv ps/columns-merged.csv data/empty.txt 0
	ps_merged)

test("csv-ps -M -c tid,cmd,age -N meh | csv-head -n 0" data/3-columns-3-rows-with-table.csv ps/columns-merged-table.csv data/empty.txt 0
	ps_merged_table)

test("csv-ps --help" data/empty.csv ps/help.txt data/empty.txt 2
	ps_help)

test("csv-ps --version" data/empty.csv data/git-version.txt data/empty.txt 0
	ps_version)

endif(LIBPROCPS_FOUND)
