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
FILE *out;

int writepdf(char* data)
{
	fprintf(out, data);
}

int main(int argc, char **argv)
{
	FILE *in;
	if (argc > 3) {
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
		argv++;
		if(*argv)
		{
			if (!(out=fopen(*argv, "wb"))) {
				perror(PROGNAME);
				exit(-1);
			}
		} else {
			out = stdout;

		}
	} else {
		in = stdin;
	}
	genpdf(in);
	if(out != stdout)
	{
		fclose(out);
	}
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
	nbyte += fprintf(out, "%%PDF-1.4\n");
}

void gencl(void)
{
	cl_pos = nbyte;
	nbyte += fprintf(out, "1 0 obj\n");
	nbyte += fprintf(out, "<< /Type /Catalog /Pages 2 0 R >>\n");
	nbyte += fprintf(out, "endobj\n");
}

void genpg(void)
{
	int sl;

	pg_pos[npage] = nbyte;
	nbyte += fprintf(out, "%d 0 obj\n", PG_BASE+npage);
	nbyte += fprintf(out, "<< /Type /Page\n");
	nbyte += fprintf(out, "/Parent 2 0 R\n");
	nbyte += fprintf(out, "/Contents %d 0 R >>\n", PC_BASE+npage);
	nbyte += fprintf(out, "endobj\n");
}

/* return stream length */
int genpc(FILE *in)
{
	int gencs(FILE *in);
	int sl;

	pc_pos[npage] = nbyte;
	nbyte += fprintf(out, "%d 0 obj\n", PC_BASE+npage);
	nbyte += fprintf(out, "<< /Length %d 0 R >>\n", SL_BASE+npage);
	nbyte += fprintf(out, "stream\n");
	sl = gencs(in);
	nbyte += fprintf(out, "\nendstream\n");
	nbyte += fprintf(out, "endobj\n");
	return sl;
}

void gensl(int n)
{
	sl_pos[npage] = nbyte;
	nbyte += fprintf(out, "%d 0 obj\n", SL_BASE+npage);
	nbyte += fprintf(out, "%d\n", n);
	nbyte += fprintf(out, "endobj\n");
}

void genpr(void)
{
	int i;

	pr_pos = nbyte;
	nbyte += fprintf(out, "2 0 obj\n");
	nbyte += fprintf(out, "<< /Type /Pages\n");
	nbyte += fprintf(out, "/Kids [\n");
	for (i=0; i<npage; i++)
		nbyte += fprintf(out, "%d 0 R\n", PG_BASE+i);
	nbyte += fprintf(out, "]\n");
	nbyte += fprintf(out, "/Count %d\n", npage);
	nbyte += fprintf(out, "/MediaBox [0 0 595 842]\n");
	nbyte += fprintf(out, "/Resources << /Font << /F0 <<\n");
	nbyte += fprintf(out, "/Type /Font\n");
	nbyte += fprintf(out, "/Subtype /Type1\n");
	nbyte += fprintf(out, "/BaseFont /Courier\n");
	nbyte += fprintf(out, ">> >> >>\n");	/* end of resources dict */
	nbyte += fprintf(out, ">>\n");
	nbyte += fprintf(out, "endobj\n");
}

void genxref(void)
{
	int i;

	xref_pos = nbyte;
	fprintf(out, "xref\n");
	fprintf(out, "0 3\n");
	fprintf(out, "0000000000 65535 f \n");
	fprintf(out, "%010d 00000 n \n", cl_pos);
	fprintf(out, "%010d 00000 n \n", pr_pos);
	fprintf(out, "%d %d\n", PG_BASE, npage);
	for (i=0; i<npage; i++)
		fprintf(out, "%010d 00000 n \n", pg_pos[i]);
	fprintf(out, "%d %d\n", PC_BASE, npage);
	for (i=0; i<npage; i++)
		fprintf(out, "%010d 00000 n \n", pc_pos[i]);
	fprintf(out, "%d %d\n", SL_BASE, npage);
	for (i=0; i<npage; i++)
		fprintf(out, "%010d 00000 n \n", sl_pos[i]);
}

void gentail(void)
{
	fprintf(out, "trailer\n<< /Size %d /Root 1 0 R >>\n", SL_BASE+npage);
	fprintf(out, "startxref\n");
	fprintf(out, "%d\n", xref_pos);
	fprintf(out, "%%%%EOF\n");
}

/* return stream length */
int gencs(FILE *in)
{
	int escape(char *s);
	char line[NCOL+1];
	int n, nl, eol, ch;

	n = 0;
	n += fprintf(out, "BT\n");
	n += fprintf(out, "11 TL\n");
	n += fprintf(out, "34 784 Td\n");
	n += fprintf(out, "/F0 11 Tf\n");
	nl = 0;
	while (fgets(line, sizeof line, in)) {
		eol = strcspn(line, "\n");
		if (!line[eol] && (ch=fgetc(in)) != '\n')
			ungetc(ch, in);
		line[eol] = '\0';
		n += fprintf(out, "T* (");
		n += escape(line);
		n += fprintf(out, ") Tj\n");
		if (++nl == NROW)
			break;
	}
	n += fprintf(out, "ET");
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
			fputc('\\', out);
			n++;
		default:
			fputc(*s, out);
			n++;
		}
		s++;
	}
	return n;
}
