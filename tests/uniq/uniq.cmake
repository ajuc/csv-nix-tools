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

test("csv-uniq -c col1" uniq/input1.csv uniq/output1.csv data/empty.txt 0 uniq-1)

test("csv-uniq -c col3,col2,col1" uniq/input2.csv uniq/output2.csv data/empty.txt 0 uniq-2)

test("csv-uniq -c col3,col2,col1 -s" uniq/input2.csv uniq/output2.txt data/empty.txt 0 uniq-2-s)

test("csv-uniq -c col3" uniq/input2.csv uniq/output3.csv data/empty.txt 0 uniq-3)

test("csv-uniq -c _table,t1.str,t1.int,t2.string,t2.integer" uniq/input3.csv uniq/output4a.csv data/empty.txt 0 uniq-no-table)

test("csv-uniq -c str -T t1" uniq/input3.csv uniq/output4b.csv data/empty.txt 0 uniq-table1)

test("csv-uniq -c string -T t2" uniq/input3.csv uniq/output4c.csv data/empty.txt 0 uniq-table2)

test("csv-uniq --help" data/empty.csv uniq/help.txt data/empty.txt 2
	uniq_help)

test("csv-uniq --version" data/empty.csv data/git-version.txt data/empty.txt 0
	uniq_version)
