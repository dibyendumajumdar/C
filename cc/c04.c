/*
 * C compiler
 */

#include "c0.h"

#include <stdarg.h>
#include <stdlib.h>

/*
 * Reduce the degree-of-reference by one.
 * e.g. turn "ptr-to-int" into "int".
 */
int decref(int t)
{
	if ((t & ~TYPE) == 0) {
		error("Illegal indirection");
		return(t);
	}
	return((t>>TYLEN) & ~TYPE | t&TYPE);
}

/*
 * Increase the degree of reference by
 * one; e.g. turn "int" to "ptr-to-int".
 */
int incref(int t)
{
	return(((t&~TYPE)<<TYLEN) | (t&TYPE) | PTR);
}

/*
 * Make a tree that causes a branch to lbl
 * if the tree's value is non-zero together with the cond.
 */
void cbranch(union tree *t, int lbl, int cond)
{
	treeout(t, 0);
	outcode("BNNN", CBRANCH, (N_type) lbl, (N_type)cond, (N_type)line);
}

/*
 * Write out a tree.
 */
void rcexpr(union tree *tp)
{
	/*
	 * Special optimization
	 */
	if (tp->t.op==INIT && tp->t.tr1->t.op==CON) {
		if (tp->t.type==CHAR || tp->t.type==UNCHAR) {
			outcode("B1N0", BDATA, (N_type) tp->t.tr1->c.value);
			return;
		} else if (tp->t.type==INT || tp->t.type==UNSIGN) {
			outcode("BN", SINIT, (N_type) tp->t.tr1->c.value);
			return;
		}
	}
	treeout(tp, 0);
	outcode("BN", EXPR, (N_type) line);
}

void treeout(union tree *tp, int isstruct)
{
	register struct nmlist *hp;
	register nextisstruct;

	if (tp == NULL || tp->t.op==NULLOP) {
		outcode("B", XNULLOP);
		return;
	}
	nextisstruct = tp->t.type==STRUCT;
	switch(tp->t.op) {

	case NAME:
		hp = &tp->t.tr1->n;
		if (hp->hclass==TYPEDEF)
			error("Illegal use of type name");
		outcode("BNN", NAME, (N_type) (hp->hclass==0?STATIC:hp->hclass), (N_type) tp->t.type);
		if (hp->hclass==EXTERN)
			outcode("S", hp->name);
		else
			outcode("N", (N_type) hp->hoffset);
		break;

	case LCON:
		outcode("BNNN", tp->l.op, (N_type) tp->l.type, (N_type)(unsigned short)(tp->l.lvalue>>16),
		   (N_type)(unsigned short)tp->l.lvalue);
		break;

	case CON:
		outcode("BNN", tp->c.op, (N_type) tp->c.type, (N_type) tp->c.value);
		break;

	case FCON:
		outcode("BNF", tp->f.op, (N_type) tp->f.type, tp->f.cstr);
		break;

	case STRING:
		outcode("BNNN", NAME, (N_type) STATIC, (N_type) tp->t.type, (N_type) tp->t.tr1);
		break;

	case FSEL:
		treeout(tp->t.tr1, nextisstruct);
		outcode("BNNN", tp->t.op, tp->t.type,
		   tp->t.tr2->fld.bitoffs, tp->t.tr2->fld.flen);
		break;

	case ETYPE:
		error("Illegal use of type");
		break;

	case AMPER:
		treeout(tp->t.tr1, 1);
		outcode("BN", tp->t.op, (N_type) tp->t.type);
		break;


	case CALL:
		treeout(tp->t.tr1, 1);
		treeout(tp->t.tr2, 0);
		outcode("BN", CALL, (N_type) tp->t.type);
		break;

	default:
		treeout(tp->t.tr1, nextisstruct);
		if (opdope[tp->t.op]&BINARY)
			treeout(tp->t.tr2, nextisstruct);
		outcode("BN", tp->t.op, (N_type) tp->t.type);
		break;
	}
	if (nextisstruct && isstruct==0)
		outcode("BNN", STRASG, (N_type) STRUCT, (N_type) tp->t.strp->S.ssize);
}

/*
 * Generate a branch
 */
void branch(int lab)
{
	outcode("BN", BRANCH, (N_type) lab);
}

/*
 * Generate a label
 */
void label(int l)
{
	outcode("BN", LABEL, (N_type) l);
}

/*
 * ap is a tree node whose type
 * is some kind of pointer; return the size of the object
 * to which the pointer points.
 */
int plength(union tree *p)
{
	register t, l;

	if (p==0 || ((t=p->t.type)&~TYPE) == 0)		/* not a reference */
		return(1);
	p->t.type = decref(t);
	l = length(p);
	p->t.type = t;
	return(l);
}

/*
 * return the number of bytes in the object
 * whose tree node is acs.
 */
int length(union tree *cs)
{
	register t, elsz;
	long n;
	int nd;

	t = cs->t.type;
	n = 1;
	nd = 0;
	while ((t&XTYPE) == ARRAY) {
		t = decref(t);
		n *= cs->t.subsp[nd++];
	}
	if ((t&~TYPE)==FUNC)
		return(0);
	if (t>=PTR)
		elsz = SZPTR;
	else switch(t&TYPE) {

	case VOID:
		error("Illegal use of void object");
		return(2);

	case INT:
	case UNSIGN:
		elsz = SZINT;
		break;

	case CHAR:
	case UNCHAR:
		elsz = 1;
		break;

	case FLOAT:
		elsz = SZFLOAT;
		break;

	case UNLONG:
	case LONG:
		elsz = SZLONG;
		break;

	case DOUBLE:
		elsz = SZDOUB;
		break;

	case STRUCT:
		if ((elsz = cs->t.strp->S.ssize) == 0)
			error("Undefined structure");
		break;
	default:
		error("Compiler error (length)");
		return(0);
	}
	n *= elsz;
	if (n >= (unsigned)50000)
		werror("very large data structure");
	return(n);
}

/*
 * The number of bytes in an object, rounded up to a word.
 */
int rlength(union tree *cs)
{
	return((length(cs)+ALIGN) & ~ALIGN);
}

/*
 * After an "if (...) goto", look to see if the transfer
 * is to a simple label.
 */
int simplegoto(void)
{
	register struct nmlist *csp;

	if ((peeksym=symbol())==NAME && nextchar()==';') {
		csp = csym;
		if (csp->hblklev == 0)
			csp = pushdecl(csp);
		if (csp->hclass==0 && csp->htype==0) {
			csp->htype = ARRAY;
			csp->hflag |= FLABL;
			if (csp->hoffset==0)
				csp->hoffset = isn++;
		}
		if ((csp->hclass==0||csp->hclass==STATIC)
		 &&  csp->htype==ARRAY) {
			peeksym = -1;
			return(csp->hoffset);
		}
	}
	return(0);
}

/*
 * Return the next non-white-space character
 */
int nextchar(void)
{
	while (spnextchar()==' ')
		peekc = 0;
	return(peekc);
}

/*
 * Return the next character, translating all white space
 * to blank and handling line-ends.
 */
int spnextchar(void)
{
	register int c;

	if ((c = peekc)==0)
		c = getc(infp);
	if (c=='\t' || c=='\014')	/* FF */
		c = ' ';
	else if (c=='\n') {
		c = ' ';
		line++;
	}
	peekc = c;
	return(c);
}

/*
 * is a break or continue legal?
 */
void chconbrk(int l)
{
	if (l==0)
		error("Break/continue error");
}

/*
 * The goto statement.
 */
void dogoto(void)
{
	register union tree *np;
	register char *st;

	st = starttree();
	*cp++ = tree(0);
	build(STAR);
	chkw(np = *--cp, -1);
	rcexpr(block(JUMP, 0, (int *)NULL, (union str *)NULL, np, TNULL));
	endtree(st);
}

/*
 * The return statement, which has to convert
 * the returned object to the function's type.
 */
void doret(void)
{
	if (nextchar() != ';') {
		register char *st;

		st = starttree();
		*cp++ = (union tree *)&funcblk;
		*cp++ = tree(0);
		build(ASSIGN);
		cp[-1] = cp[-1]->t.tr2;
		build(RFORCE);
		rcexpr(*--cp);
		endtree(st);
	}
	branch(retlab);
}

/*
 * Write a character on the error output.
 */
/*
 * Coded output:
 *   B: beginning of line; an operator
 *   N: a number
 *   S: a symbol (external)
 *   1: number 1
 *   0: number 0
 */
/* VARARGS1 */
void outcode(char *s, ...)
{
	va_list args;

	FILE *bufp;
	char *np;
	int n;

	bufp = outfp; /* Main output file */
	if (strflg)
		bufp = sbufp; /* String output file */
	va_start(args, s);
	for (;;) switch(*s++) {
	case 'B': {
		int op = va_arg(args, int);
		const char *opname = opnames[op];
		if (opname) 
			fprintf(bufp, "\n%s;", opname);
		else
			fprintf(bufp, "\n%d;", op);
		continue;
	}
	case 'N': {
		N_type n = va_arg(args, N_type);
		fprintf(bufp, "%lld;", n);
		continue;
	}
	case 'F':
		np = va_arg(args, char *);
		n = 1000;
		goto str;
	
	case 'S': 
		np = va_arg(args, char *);
		n = MAXCPS-1;
		fputc('"', bufp);
		if (*np)
			fputc('_', bufp);
	str:
		while(n-- && *np) {
			char ch = *np++ & 0177;
			if (ch == '"') {
				fputc('\\', bufp);
			}
			fputc(ch, bufp);
		}
		fputc('"', bufp);
		fputc(';', bufp);
		continue;

	case '1':
		fputc('1', bufp);
		fputc(';', bufp);
		continue;

	case '0':
		fputc('0', bufp);
		fputc(';', bufp);
		continue;

	case '\0':
		if (ferror(bufp)) {
			error("Write error on temp");
			exit(1);
		}
		va_end(args);
		return;

	default:
		error("Botch in outcode");
	}
}

unsigned int hash(char *sp)
{
	register unsigned int h;

	h = 0;
	for (; *sp; sp++) {
		h += h;
		h += *sp;
	}
	return(h%HSHSIZ);
}
