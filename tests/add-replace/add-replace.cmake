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

test("csv-add-replace -c name -F ' i' -r 'STRING' -n new-col" data/3-columns-3-rows.csv add-replace/string.csv data/empty.txt 0
	add-replace_string)

test("csv-add-replace -c name -e '\\(.*\\)\\( i\\)\\(.*\\)' -r '%3 STRING %1' -n new-col" data/3-columns-3-rows.csv add-replace/regex.csv data/empty.txt 0
	add-replace_regex)

test("csv-add-replace -c name -E '(.*)( i)(.*)' -r '%3 STRING %1' -n new-col" data/3-columns-3-rows.csv add-replace/regex.csv data/empty.txt 0
	add-replace_eregex)

test("csv-add-replace -c name -E '(.*)( i)(.*)' -r '%3 STRING %1' -n new-col -s" data/3-columns-3-rows.csv add-replace/regex.txt data/empty.txt 0
	add-replace_eregex_-s)

test("csv-add-replace -T t1 -c name -F ' i' -r STRING -n new-col" data/2-tables.csv add-replace/2-tables-add1.csv data/empty.txt 0
	add-replace_2tables-add1)

test("csv-add-replace --help" data/empty.csv add-replace/help.txt data/empty.txt 2
	add-replace_help)

test("csv-add-replace --version" data/empty.csv data/git-version.txt data/empty.txt 0
	add-replace_version)
