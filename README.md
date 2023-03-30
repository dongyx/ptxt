ptxt
====

`ptxt` is a text-to-PDF converter aiming to:

- fast
- low memory consumption
- stream converting, line by line
- light weight

To reach these goals, *ptxt* uses an ad-hoc solution instead of depending on a full-feature PDF library or rendering engine.

News about this project will be published at [Twitter](https://twitter.com/dongyx2).

Output Style
------------

Outputs are in the style of line printers:

- 66 lines per page
- 80 columns
- *Courier* font
- A4 page size
- proper margins

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

Installation
------------

	$ make
	$ sudo make install
