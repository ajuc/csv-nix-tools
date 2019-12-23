---
title: csv-tail
section: 1
...

# NAME #

csv-tail - print on the standard output the end of CSV file from standard input

# SYNOPSIS #

**csv-tail** [OPTION]...

# DESCRIPTION #

Print the last 10 rows of CSV file from standard input to standard output.

-n, --lines=NUM
:   print the last NUM rows instead of the last 10, NUM must be >= 0

-s, --show
:   pipe output to **csv-show**(1)

--help
:   display this help and exit

--version
:   output version information and exit

# EXAMPLE #

csv-tail < file1.csv
:   output the last 10 rows of file1.csv

csv-tail -n 5 < file1.csv
:   output the last 5 rows of file1.csv

# SEE ALSO #

**tail**(1), **csv-head**(1), **csv-show**(1), **csv-nix-tools**(7)