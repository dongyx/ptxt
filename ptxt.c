/* cl:	catalog object	(#0)
 * pr:	page tree root	(#1)
 * pg:	page object	(#PG_BASE ~ #PG_BASE+NPAGE-1)
 * pc:	page content	(#PC_BASE ~ #PC_BASE+NPAGE-1)
 * cs:	content stream
 * sl:	stream length	(#SL_BASE ~ #SL_BASE+NPAGE-1)
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define PROGNAME "ptxt"
#define VERSION "0.1.0"
#define AUTHOR "Written by DONG Yuxuan <https://www.dyx.name>"
#define USAGE "usage: ptxt [file]"
#define NPAGE 32768
#define PG_BASE 32768
#define PC_BASE (PG_BASE+NPAGE)
#define SL_BASE (PC_BASE+NPAGE)
#define NCOL 80
#define NROW 66

void genpdf(FILE *in);
int nbyte, npage;
int cl_pos, pr_pos, pg_pos[NPAGE], pc_pos[NPAGE], sl_pos[NPAGE];
int xref_pos;

int main(int argc, char **argv)
{
	FILE *in;

	if (argc > 2) {
		fprintf(stderr, "%s\n", USAGE);
		exit(-1);
	}
	argv++;
	if (*argv && !strcmp(*argv, "--help")) {
		printf("%s\n", USAGE);
		exit(0);
	}
	if (*argv && !strcmp(*argv, "--version")) {
		printf("%s %s\n%s\n", PROGNAME, VERSION, AUTHOR);
		exit(0);
	}
	if (*argv) {
		if (!(in=fopen(*argv, "r"))) {
			perror(PROGNAME);
			exit(-1);
		}
	} else
		in = stdin;
	genpdf(in);
	return 0;
}

void genhead(void);
void gencl(void);
void genpg(void);
int genpc(FILE *in);
void gensl(int n);
void genpr(void);
void genxref(void);
void gentail(void);

void genpdf(FILE *in)
{
	int sl;

	nbyte = npage = 0;
	genhead();
	gencl();
	do {
		genpg();
		sl = genpc(in);
		gensl(sl);
		npage++;
	} while (npage < NPAGE && !feof(in));
	if (!feof(in)) {
		fprintf(stderr, "%s: %s\n", PROGNAME, "Too many pages");
		exit(-1);
	}
	genpr();
	genxref();
	gentail();
}

void genhead(void)
{
	nbyte += printf("%%PDF-1.4\n");
}

void gencl(void)
{
	cl_pos = nbyte;
	nbyte += printf("1 0 obj\n");
	nbyte += printf("<< /Type /Catalog /Pages 2 0 R >>\n");
	nbyte += printf("endobj\n");
}

void genpg(void)
{
	int sl;

	pg_pos[npage] = nbyte;
	nbyte += printf("%d 0 obj\n", PG_BASE+npage);
	nbyte += printf("<< /Type /Page\n");
	nbyte += printf("/Parent 2 0 R\n");
	nbyte += printf("/Contents %d 0 R >>\n", PC_BASE+npage);
	nbyte += printf("endobj\n");
}

/* return stream length */
int genpc(FILE *in)
{
	int gencs(FILE *in);
	int sl;

	pc_pos[npage] = nbyte;
	nbyte += printf("%d 0 obj\n", PC_BASE+npage);
	nbyte += printf("<< /Length %d 0 R >>\n", SL_BASE+npage);
	nbyte += printf("stream\n");
	sl = gencs(in);
	nbyte += printf("\nendstream\n");
	nbyte += printf("endobj\n");
	return sl;
}

void gensl(int n)
{
	sl_pos[npage] = nbyte;
	nbyte += printf("%d 0 obj\n", SL_BASE+npage);
	nbyte += printf("%d\n", n);
	nbyte += printf("endobj\n");
}

void genpr(void)
{
	int i;

	pr_pos = nbyte;
	nbyte += printf("2 0 obj\n");
	nbyte += printf("<< /Type /Pages\n");
	nbyte += printf("/Kids [\n");
	for (i=0; i<npage; i++)
		nbyte += printf("%d 0 R\n", PG_BASE+i);
	nbyte += printf("]\n");
	nbyte += printf("/Count %d\n", npage);
	nbyte += printf("/MediaBox [0 0 595 842]\n");
	nbyte += printf("/Resources << /Font << /F0 <<\n");
	nbyte += printf("/Type /Font\n");
	nbyte += printf("/Subtype /Type1\n");
	nbyte += printf("/BaseFont /Courier\n");
	nbyte += printf(">> >> >>\n");	/* end of resources dict */
	nbyte += printf(">>\n");
	nbyte += printf("endobj\n");
}

void genxref(void)
{
	int i;

	xref_pos = nbyte;
	puts("xref");
	puts("0 3");
	puts("0000000000 65535 f ");
	printf("%010d 00000 n \n", cl_pos);
	printf("%010d 00000 n \n", pr_pos);
	printf("%d %d\n", PG_BASE, npage);
	for (i=0; i<npage; i++)
		printf("%010d 00000 n \n", pg_pos[i]);
	printf("%d %d\n", PC_BASE, npage);
	for (i=0; i<npage; i++)
		printf("%010d 00000 n \n", pc_pos[i]);
	printf("%d %d\n", SL_BASE, npage);
	for (i=0; i<npage; i++)
		printf("%010d 00000 n \n", sl_pos[i]);
}

void gentail(void)
{
	printf("trailer\n<< /Size %d /Root 1 0 R >>\n", SL_BASE+npage);
	puts("startxref");
	printf("%d\n", xref_pos);
	puts("%%EOF");
}

/* return stream length */
int gencs(FILE *in)
{
	int escape(char *s);
	char line[NCOL+1];
	int n, nl, eol, ch;

	n = 0;
	n += printf("BT\n");
	n += printf("11 TL\n");
	n += printf("34 784 Td\n");
	n += printf("/F0 11 Tf\n");
	nl = 0;
	while (fgets(line, sizeof line, in)) {
		eol = strcspn(line, "\n");
		if (!line[eol] && (ch=fgetc(in)) != '\n')
			ungetc(ch, in);
		line[eol] = '\0';
		n += printf("T* (");
		n += escape(line);
		n += printf(") Tj\n");
		if (++nl == NROW)
			break;
	}
	n += printf("ET");
	nbyte += n;
	return n;
}

int escape(char *s)
{
	int n;

	n = 0;
	while (*s) {
		switch (*s) {
		case '(':
		case ')':
		case '\\':
			putchar('\\');
			n++;
		default:
			putchar(*s);
			n++;
		}
		s++;
	}
	return n;
}
