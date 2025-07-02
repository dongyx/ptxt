PTXT
====

PTXT is a text-to-PDF converter aiming to:

- fast
- low memory consumption
- stream converting, line by line
- light weight

To reach these goals, PTXT uses an ad-hoc solution instead of depending on a full-feature PDF library or rendering engine.

Installation
------------

	$ make
	$ sudo make install

Usage
-----

- `ptxt file`

	Convert *file* to PDF.
	The result is printed to the standard output.
	If *file* is omitted, the standard input is used.

- `ptxt --help`

	Print the usage.

- `ptxt --version`

	Print version information.

Output Style
------------

Outputs are in the style of line printers:

- 66 lines per page
- 80 columns
- *Courier* font
- A4 page size
- proper margins

Credits
-------

PTXT was created by [DONG Yuxuan](https://www.dyx.name) in 2023.

