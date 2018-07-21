

Bell Laboratories


# A Tour through the UNIX- C	      
date: November 2, 1997
from: D. M. Ritchie

## The Intermediate Language

Communication between the two phases of the  compiler  proper  is
carried  out  by  means  of  a pair of intermediate files.  These
files are treated as having  identical	structure,  although  the
second	file contains only the code generated for strings.  It is
convenient to write strings out separately to reduce the need for
multiple location counters in a later assembly phase.

The  intermediate	language is not machine-independent;
its structure in a number of ways reflects the fact  that  C
was  originally a one-pass compiler chopped in two to reduce
the maximum memory requirement.  In fact,  only  the  latest
version of the compiler has a complete intermediate language
at all.  Until recently, the first  phase  of  the  compiler
generated  assembly  code  for	those constructions it could
deal with, and passed expression parse	trees,	in  absolute
binary	form, to the second phase for code generation.	Now,
at least, all inter-phase information is  passed  in  a  de-
scribable form, and there are no absolute pointers involved,
so the coupling between the phases is not so strong.

The areas in which the machine (and  system)  dependen-
cies are most noticeable are

1.   Storage  allocation  for  automatic variables and argu-
     ments has already been performed, and  nodes  for	such
     variables refer to them by offset from a display point-
     er.  Type conversion  (for  example,  from  integer  to
     pointer)  has  already occurred using the assumption of
     byte addressing and 2-byte words.

2.   Data representations suitable to  the  PDP-11  are  as-
     sumed;  in  particular,  floating	point  constants are
     passed as four words in the machine representation.

As it happens, each intermediate file is represented as
a  sequence  of binary numbers without any explicit demarca-
tions.	It consists of a sequence of conceptual lines,	each
headed	by  an	operator,  and	possibly  containing various
operands.

The operators are small numbers; to  assist  in  recognizing
failure  in synchronization, the high-order byte of each op-
erator word is always the octal number	376.   Operands  are
either 16-bit binary numbers or strings of characters repre-
senting names.	Each name is terminated by a null character.
There is no alignment requirement for numerical operands and
so there is no padding after a name string.

The binary representation was chosen to avoid  the  ne-
cessity of converting to and from character form and to min-
imize the size of the files.  It would be very easy to	make
each  operator-operand	`line'	in  the  file  be a genuine,
printable line, with the numbers in octal or  decimal;	this
in fact was the representation originally used.

The  operators  fall  naturally into two classes: those
which represent part of an expression, and all others.	 Ex-
pressions  are	transmitted in a reverse-Polish notation; as
they are being read, a tree is built which is isomorphic  to
the  tree  constructed	in the first phase.  Expressions are
passed as a whole, with no non-expression  operators  inter-
vening.   The reader maintains a stack; each leaf of the ex-
pression tree (name, constant) is pushed on the stack;	each
unary operator replaces the top of the stack by a node whose
operand is the old top-of-stack; each  binary  operator  re-
places	the top pair on the stack with a single entry.	When
the expression is complete there is exactly one item on  the
stack.	 Following  each  expression  is  a special operator
which passes the unique previous expression to the `optimiz-
er' described below and then to the code generator.

Here  is  the  list of operators not themselves part of
expressions.


### EOF

     marks the end of an input file.

### BDATA flag data ...

     specifies a sequence of bytes to be assembled as static
     data.  It is followed by pairs of words; the first mem-
     ber of the pair is non-zero to indicate that  the	data
     continue;	a zero flag is not followed by data and ter-
     minates the operator.  The data bytes occupy  the	low-
     order part of a word.

### WDATA flag data ...

     specifies a sequence of words to be assembled as static
     data; it is identical to the BDATA operator except that
     entire words, not just bytes, are passed.

### PROG

     means  that subsequent information is to be compiled as
     program text.

### DATA

     means that subsequent information is to be compiled  as
     static data.

### BSS

     means  that subsequent information is to be compiled as
     unitialized static data.

### SYMDEF name

     means that the symbol name is an external name  defined
     in the current program.  It is produced for each exter-
     nal data or function definition.

### CSPACE name size

     indicates that the name refers to	a  data  area  whose
     size  is the specified number of bytes.  It is produced
     for external data definitions without explicit initial-
     ization.

### SSPACE size

     indicates	that size bytes should be set aside for data
     storage.  It is used to pad out  short  initializations
     of  external  data and to reserve space for static (in-
     ternal) data.  It will be preceded  by  an  appropriate
     label.

### EVEN

     is  produced  after each external data definition whose
     size is not an integral number of	words.	 It  is  not
     produced  after  strings  except when they initialize a
     character array.

### NLABEL name

     is produced just before a BDATA or  WDATA	initializing
     external data, and serves as a label for the data.

### RLABEL name

     is  produced  just before each function definition, and
     labels its entry point.

### SNAME name number

     is produced at the start  of  each  function  for	each
     static  variable or label declared therein.  Subsequent
     uses of the variable will be in terms of the given num-
     ber.   The  code  generator uses this only to produce a
     debugging symbol table.

### ANAME name number

     Likewise, each automatic variable's name and stack off-
     set  is specified by this operator.  Arguments count as
     automatics.

### RNAME name number

     Each register variable is	similarly  named,  with  its
     register number.

### SAVE number

     produces  a register-save sequence at the start of each
     function, just after its label (RLABEL).

### SETREG number

     is used to indicate the number of	registers  used  for
     register  variables.   It	actually  gives the register
     number of the lowest free register; it is redundant be-
     cause the RNAME operators could be counted instead.

### PROFIL

     is produced before the save sequence for functions when
     the profile option is turned on.  It produces  code  to
     count the number of times the function is called.

### SWIT deflab line label value ...

     is  produced for switches.  When control flows into it,
     the value being switched on is in the  register  forced
     by  RFORCE  (below).   The switch statement occurred on
     the indicated line of the source, and the label  number
     of  the  default location is deflab.  Then the operator
     is followed by a sequence	of  label-number  and  value
     pairs; the list is terminated by a 0 label.

### LABEL number

     generates	an  internal label.  It is referred to else-
     where using the given number.


### BRANCH number

     indicates an unconditional transfer to the internal la-
     bel number given.

### RETRN

     produces the return sequence for a function.  It occurs
     only once, at the end of each function.

### EXPR line

     causes the expression just preceding  to  be  compiled.
     The argument is the line number in the source where the
     expression occurred.

### NAME class type number

     indicates a name occurring in an expression.  The first
     form is used when the name is external; the second when
     the name is automatic, static, or a register.  Then the
     number indicates the stack offset, the label number, or
     the register number as appropriate.  Class and type en-
     coding is described elsewhere.

### CON type value

     transmits	an  integer constant.  This and the next two
     operators occur as part of expressions.

### FCON type 4-word-value

     transmits a floating constant as four words  in  PDP-11
     notation.

### SFCON type value

     transmits a floating-point constant whose value is cor-
     rectly represented by its high-order word in PDP-11 no-
     tation.

### NULL

     indicates a null argument list of a function call in an
     expression; call is  a  binary  operator  whose  second
     operand is the argument list.

### CBRANCH label cond

     produces a conditional branch.  It is an expression op-
     erator, and will be followed by an EXPR.  The branch to
     the  label number takes place if the expression's truth
     value is the same as that of cond.  That is, if  cond=1
     and  the  expression  evaluates  to true, the branch is
     taken.

### binary-operator type

     There are binary operators corresponding to  each	such
     source-language  operator;  the  type  of the result of
     each is passed as well.  Some  perhaps-unexpected	ones
     are:  COMMA,  which is a right-associative operator de-
     signed to simplify right-to-left evaluation of function
     arguments;  prefix  and postfix ++ and --, whose second
     operand is the increment amount, as a  CON;  QUEST  and
     COLON,   to   express  the  conditional  expression  as
     `a?(b:c)'; and a sequence of special operators for  ex-
     pressing  relations  between  pointers, in case pointer
     comparison is different from integer  comparison  (e.g.
     unsigned).

### unary-operator type

     There are also numerous unary operators.  These include
     ITOF, FTOI, FTOL, LTOF, ITOL, LTOI which convert  among
     floating,	long, and integer; JUMP which branches indi-
     rectly through a label expression; INIT, which compiles
     the  value of a constant expression used as an initial-
     izer; RFORCE, which is used before a return sequence or
     a switch to place a value in an agreed-upon register.

## Expression Optimization

Each expression tree, as it is read in, is subjected to
a fairly comprehensive analysis.  This is performed  by  the
optim  routine and a number of subroutines; the major things
done are

1.   Modifications and simplifications of the  tree  so  its
     value may be computed more efficiently and conveniently
     by the code generator.

2.   Marking each interior node with an estimate of the num-
     ber  of registers required to evaluate it.  This regis-
     ter count is needed to guide the code generation  algo-
     rithm.

One  thing  that is definitely not done is discovery or
exploitation of common subexpressions, nor is this done any-
where in the compiler.

The basic organization is simple: a depth-first scan of
the tree.  Optim does nothing for leaf nodes (except for au-
tomatics;  see below), and calls unoptim to handle unary op-
erators.  For binary operators, it calls itself  to  process
the operands, then treats each operator separately.  One im-
portant case is commutative and associative operators, which
are handled by acommute.

Here  is a brief catalog of the transformations carried
out by by optim itself.  It is not intended to be  complete.
Some  of the transformations are machine-dependent, although
they may well be useful on machines other than the PDP-11.

1.   As indicated in the discussion of	unoptim  below,  the
     optimizer	can  create a node type corresponding to the
     location addressed by a register plus a  constant	off-
     set.  Since this is precisely the implementation of au-
     tomatic variables and arguments, where the register  is
     fixed  by convention, such variables are changed to the
     new form to simplify later processing.

2.   Associative and commutative operators are processed  by
     the special routine acommute.

3.   After processing by acommute, the bitwise & operator is
     turned into a new andn operator; `a  &  b`  becomes  `a
     andn  ~b`.  This is done because the PDP-11 provides no
     and operator, but only andn.  A similar  transformation
     takes place for `=&`.

4.   Relationals  are  turned around so the more complicated
     expression is on the left.  (So that `2 > f(x)` becomes
     `f(x)  <  2`).  This improves code generation since the
     algorithm prefers to have	the  right  operand  require
     fewer registers than the left.

5.   An  expression  minus a constant is turned into the ex-
     pression plus the negative constant, and  the  acommute
     routine  is  called to take advantage of the properties
     of addition.

6.   Operators with constant operands are evaluated.

7.   Right shifts (unless by 1) are turned into left  shifts
     with  a negated right operand, since the PDP-11 lacks a
     general right-shift operator.

8.   A number of special cases are simplified, such as divi-
     sion or multiplication by 1, and shifts by 0.

The unoptim routine performs the same sort of processing for
unary operators.

1.   `*&x` and `&*x` are simplified to `x`.

2.   If r is a register and c is a constant or	the  address
     of  a  static  or	external  variable,  the expressions
     `*(r+c)` and `*r` are turned into	a  special  kind  of
     name  node which expresses the name itself and the off-
     set.  This  simplifies  subsequent  processing  because
     such  constructions  can appear as the the address of a
     PDP-11 instruction.

3.   When the unary `&` operator is applied to a  name	node
     of  the  special kind just discussed, it is reworked to
     make the addition explicit again; this is done  because
     the PDP-11 has no `load address' instruction.

4.   Constructions  like `*r++` and `*--r` where r is a reg-
     ister are discovered and marked as being  implementable
     using the PDP-11 auto-increment and -decrement modes.

5.   If `!` is applied to a relational, the `!` is discarded
     and the sense of the relational is reversed.

6.   Special cases involving reflexive use of  negation  and
     complementation are discovered.

7.   Operations applying to constants are evaluated.

The acommute routine, called for associative and commu-
tative operators, discovers clusters of the same operator at
the  top  levels of the current tree, and arranges them in a
list: for `a+((b+c)+(d+f))` the list would  be`a,b,c,d,e,f`.
After  each  subtree is optimized, the list is sorted in de-
creasing difficulty of computation; as mentioned above,  the
code  generation algorithm works best when left operands are
the difficult ones.  The `degree of difficulty' computed  is
actually finer than the mere number of registers required; a
constant is considered simpler than the address of a  static
or  external, which is simpler than reference to a variable.
This makes it easy to fold all the constants  together,  and
also to merge together the sum of a constant and the address
of a static or external (since in such nodes there is  space
for  an `offset' value).  There are also special cases, like
multiplication by 1 and addition of 0.

A special routine is invoked to  handle  sums  of  products.
Distrib  is  based  on the fact that it is better to compute
`c1*c2*x + c1*y` as `c1*(c2*x + y)` and makes the divisibil-
ity  tests  required to assure the correctness of the trans-
formation.  This transformation is rarely possible with code
directly  written by the user, but it invariably occurs as a
result of the implementation of multi-dimensional arrays.

Finally, acommute reconstructs a tree from the list  of
expressions which result.

## Code Generation

The  grand  plan  for code-generation is independent of
any particular machine; it depends largely on a set  of  ta-
bles.	But this fact does not necessarily make it very easy
to modify the compiler to produce code for  other  machines,
both  because  there  is  a  good  deal of machine-dependent
structure in the tables, and because in any event  such  ta-
bles are non-trivial to prepare.

The  arguments to the basic code generation routine rc-
expr are a pointer to a tree representing an expression, the
name  of a code-generation table, and the number of a regis-
ter in which the value of the expression should  be  placed.
Rcexpr returns the number of the register in which the value
actually ended up; its caller may need to produce a mov  in-
struction  if the value really needs to be in the given reg-
ister.	There are four code generation tables.

Regtab is the basic one, which actually  does  the  job
described above: namely, compile code which places the value
represented by the expression tree in a register.

Cctab is used when the value of the expression  is  not
actually  needed,  but	instead  the  value of the condition
codes resulting from evaluation  of  the  expression.	This
table is used, for example, to evaluate the expression after
if.  It is clearly silly to calculate the value (0 or 1)  of
the expression `a==b` in the context `if (a==b) ... `

     The sptab table is used when the value of an expression
is to be pushed on the stack, for example when it is an  ac-
tual  argument.   For example in the function call `f(a)' it
is a bad idea to load a into a register which is then pushed
on  the stack, when there is a single instruction which does
the job.

     The efftab table is used when an expression  is  to  be
evaluated  for its side effects, not its value.  This occurs
mostly for expressions which are statements, which  have  no
value.	Thus the code for the statement `a = b` need produce
only the approoriate mov instruction, and need not leave the
value  of b in a register, while in the expression 
`a + (b = c)` the value of `b = c` will appear in a register.

All of the tables besides regtab are rather small,  and
handle only a relatively few special cases.  If one of these
subsidiary tables does not contain an  entry  applicable  to
the  given  expression	tree,  rcexpr uses regtab to put the
value of the expression  into  a  register  and  then  fixes
things	up;  nothing need be done when the table was efftab,
but a tst instruction is produced when the table called  for
was  cctab,  and  a mov instruction, pushing the register on
the stack, when the table was sptab.

The rcexpr routine itself picks off some special cases,
then  calls  cexpr to do the real work.  Cexpr tries to find
an entry applicable to the given tree in  the  given  table,
and returns -1 if no such entry is found, letting rcexpr try
again with a different table.  A successful match  yields  a
string	containing both literal characters which are written
out and pseudo-operations, or macros,  which  are  expanded.
Before	studying  the contents of these strings we will con-
sider how table entries are matched against trees.

Recall that most non-leaf nodes in an  expression	tree
contain the name of the operator, the type of the value rep-
resented, and pointers to the subtrees (operands).  They al-
so  contain  an estimate of the number of registers required
to evaluate the expression, placed there by the  expression-
optimizer  routines.   The register counts are used to guide
the code generation process, which is based  on  the  Sethi-
Ullman algorithm.

The main code generation tables consist of entries each
containing an operator number and a pointer  to  a  subtable
for  the  corresponding  operator.  A subtable consists of a
sequence of entries, each  with  a  key  describing  certain
properties of the operands of the operator involved; associ-
ated with the key is a code string.  Once the subtable	cor-
responding  to	the  operator  is  found,  the	subtable  is
searched linearly until a key is found such that the proper-
ties demanded by the key are compatible with the operands of
the tree node.	A successful match returns the code  string;
an  unsuccessful search, either for the operator in the main
table or a compatble key in the subtable, returns a  failure
indication.

The  tables  are  all contained in a file which must be
processed to obtain an assembly language program.  Thus they
are written in a special-purpose language.  To provided def-
initeness to the following discussion, here is an example of
a subtable entry.

```
	%n,aw
	     F
	     add  A2,R
```

The  `%` indicates the key; the information following (up to
a blank line) specifies the code string.  Very briefly, this
entry  is  in the subtable for `+` of regtab; the key speci-
fies that the left operand is  any  integer,  character,  or
pointer  expression, and the right operand is any word quan-
tity which is directly addressible (e.g. a variable or	con-
stant).   The  code  string  calls for the generation of the
code to compile the left (first) operand  into	the  current
register  (`F`)  and  then  to	produce an `add` instruction
which adds the second operand (`A2`) to the register  (`R`).
All of the notation will be explained below.

     Only  three features of the operands are used in decid-
ing whether a match has occurred.  They are:

1.   Is the type of the operand compatible with that demand-
     ed?

2.   Is the `degree of difficulty` (in a sense described be-
     low) compatible?

3.   The table may demand that the operand have a `*` (indi-
     rection operator) as its highest operator.

As suggested above, the key for a subtable entry is in-
dicated by a `%,` and a comma-separated pair  of  specifica-
tions  for  the  operands.  (The second specification is ig-
nored for unary operators).   A  specification	indicates  a
type  requirement by including one of the following letters.
If no type letter is present,  any  integer,  character,  or
pointer  operand  will	satisfy  the requirement (not float,
double, or long).

* **b**    A byte (character) operand is required.

* **w**    A word (integer or pointer) operand is required.

* **f**    A float or double operand is required.

* **d**    A double operand is required.

* **l**    A long (32-bit integer) operand is required.

Before discussing the `degree of difficulty  specifica-
tion,  the  algorithm  has  to be explained more completely.
Rcexpr (and cexpr) are called  with  a	register  number  in
which  to  place their result.	Registers 0, 1, ... are used
during evaluation of expressions; the maximum register which
can  be  used  in this way depends on the number of register
variables, but in any event only registers 0 through  4  are
available  since  r5  is used as a stack frame header and r6
(sp) and r7 (pc) have special hardware properties.  The code
generation  routines assume that when called with register n
as argument, they may use n+1, ...  (up to the first  regis-
ter  variable)	as  temporaries.   Consider  the  expression
`X+Y`, where both X and Y are expressions.  As a  first  ap-
proximation,  there  are three ways of compiling code to put
this expression in register n.

1.   If Y is an addressible cell, (recursively) put  X	into
     register n and add Y to it.

2.   If  Y is an expression that can be calculated in k reg-
     isters, where k smaller than the  number  of  registers
     available,  compile  X into register n, Y into register
     n+1, and add register n+1 to n.

3.   Otherwise, compile Y into register n, save  the  result
     in  a temporary (actually, on the stack) compile X into
     register n, then add in the temporary.

The distinction between cases 2 and 3 therefore depends
on whether the right operand can be compiled in fewer than k
registers, where k is the number of free registers left  af-
ter  registers 0 through n are taken: 0 through n-1 are pre-
sumed to contain already computed temporary results; n will,
in  case  2, contain the value of the left operand while the
right is being evaluated.

These considerations should make clear  the  specifica-
tion  codes  for  the  degree of difficulty, bearing in mind
that a number of special cases are also present:

* **z**    is satisfied when the operand is zero, so that  special
     code can be produced for expressions like `x = 0'.

* **1**    is satisfied when the operand is the constant 1, to op-
     timize cases like left and right shift by 1, which  can
     be done efficiently on the PDP-11.

* **c**    is  satisfied  when  the operand is a positive (16-bit)
     constant; this takes care of some special cases in long
     arithmetic.

* **a**    is  satisfied when the operand is addressible; this oc-
     curs not only for variables and constants, but also for
     some  more  complicated constructions, such as indirec-
     tion through a simple variable, `*p++`  where  p  is  a
     register  variable (because of the PDP-11's auto-incre-
     ment address mode), and `*(p+c)` where p is a  register
     and  c  is  a  constant.  Precisely, the requirement is
     that the operand refers to a cell whose address can  be
     written as a source or destination of a PDP-11 instruc-
     tion.

* **e**    is satisfied by an operand whose value can be generated
     in  a  register using no more than k registers, where k
     is the number of registers left (not counting the	cur-
     rent register).  The `e` stands for `easy.`

n    is  satisfied by any operand.  The `n` stands for `any-thing.`

These degrees of difficulty are considered to lie in  a
linear	ordering and any operand which satisfies an earlier-
mentioned requirement will satisfy a later one.   Since  the
subtables  are	searched linearly, if a `1` specification is
included, almost certainly a `z` must be  written  first  to
prevent expressions containing the constant 0 to be compiled
as if the 0 were 1.

Finally, a key specification may contain  a  `*`  which
requires  the  operand to have an indirection as its leading
operator.  Examples below should clarify the utility of this
specification.

     Now let us consider the contents of the code string as-
sociated with each subtable entry.   Conventionally,  lower-
case  letters  in  this string represent literal information
which is copied directly to the output.  Upper-case  letters
generally introduce specific macro-operations, some of which
may be followed by modifying information.  The code  strings
in  the  tables  are  written  with  tabs and new-lines used
freely to suggest instructions which will be generated;  the
table-compiling  program compresses tabs (using the 0200 bit
of the next character) and throws  away  some  of  the	new-
lines.	For example the macro `F' is ordinarily written on a
line by itself; but since its expansion will end with a new-
line, the new-line after `F' itself is dispensable.  This is
all to reduce the size of the stored tables.


The first set of  macro-operations  is  concerned	with
compiling  subtrees.   Recall that this is done by the cexpr
routine.  In the following discussion the `current register'
is  generally  the  argument register to cexpr; that is, the
place where the result is desired.  The `next  register'  is
numbered one higher than the current register.	(This expla-
nation isn't fully true because of complications,  described
below,	involving operations which require even-odd register
pairs.)

F    causes a recursive call to the rcexpr routine  to	com-
     pile  code  which	places the value of the first (left)
     operand of the operator in the current register.

F1   generates code which places  the  value  of  the  first
     operand  in  the next register.  It is incorrectly used
     if there might be no next register; that is, if the de-
     gree  of difficulty of the first operand is not `easy;'
     if not, another register might not be available.

FS   generates code which pushes  the  value  of  the  first
     operand on the stack, by calling rcexpr specifying spt-
     ab as the table.

Analogously,

S, S1, SScompile the second (right) operand into the current
     register, the next register, or onto the stack.

To deal with registers, there are

R    which expands into the name of the current register.

R1   which expands into the name of the next register.

R+   which expands into the the name of the current register
     plus 1.  It was suggested above that this is  the	same
     as the next register, except for complications; here is
     one of them.  Long integer variables have 32  bits  and
     require 2 registers; in such cases the next register is
     the current register plus 2.  The code  would  like  to
     talk  about  both	halves	of  the  long quantity, so R
     refers to the register with the high-order part and  R+
     to the low-order part.

R-   This  is  another	complication, involving division and
     mod.  These operations involve a pair of  registers  of
     which the odd-numbered contains the left operand.	Cex-
     pr arranges that the current register is  odd;  the  R-
     notation  allows  the  code to refer to the next lower,
     even-numbered register.

To refer to addressible quantities, there are the notations:

A1   causes generation of the address specified by the first
     operand.  For this to be legal, the operand must be ad-
     dressible;  its  key  must contain an `a' or a more re-
     strictive specification.

A2   correspondingly generates the  address  of  the  second
     operand providing it has one.

We  now  have  enough  mechanism to show a complete, if
suboptimal, table  for	the  +	operator  on  word  or	byte
operands.

```
	%n,z
	     F

	%n,1
	     F
	     inc  R

	%n,aw
	     F
	     add  A2,R

	%n,e
	     F
	     S1
	     add  R1,R

	%n,n
	     SS
	     F
	     add  (sp)+,R
```

The first two sequences handle some special cases.  Actually
it turns out that handling a right operand of 0 is  unneces-
sary  since  the  expression-optimizer throws out adds of 0.
Adding 1 by using the `increment' instruction is done  next,
and  then  the	case where the right operand is addressible.
It must be a word quantity, since the PDP-11 lacks  an	`add
byte'  instruction.   Finally  the  cases  where  the  right
operand either can, or cannot, be done in the available reg-
isters are treated.

     The next macro-instructions are conveniently introduced
by noticing that the above table is suitable for subtraction
as well as addition, since no use is made of the commutativ-
ity of addition.  All that  is	needed	is  substitution  of
`sub` for `add` and `dec` for `inc`.  Considerable saving of
space is achieved by factoring out  several  similar  opera-
tions.


I    is  replaced  by a string from another table indexed by
     the operator in the node  being  expanded.   This	sec-
     ondary  table  actually contains two strings per opera-
     tor.

I'   is replaced by the second string in the side table  en-
     try for the current operator.

Thus,  given  that  the  entries for `+' and `-' in the
side table (which is called instab)  are  `add'  and  `inc,'
`sub' and `dec' respectively, the middle of of the above ad-
dition table can be written

```
	%n,1
	     F
	     I'   R

	%n,aw
	     F
	     I	  A2,R
```

and it will be suitable for subtraction, and  several  other
operators, as well.

Next,  there is the question of character and floating-
point operations.

B1   generates the letter `b' if  the  first  operand  is  a
     character,  `f'  if  it is float or double, and nothing
     otherwise.  It is used in a context like `movB1'  which
     generates	a  `mov',  `movb', or `movf' instruction ac-
     cording to the type of the operand.

B2   is just like B1 but applies to the second operand.

BE   generates `b' if either operand is a character and null
     otherwise.

BF   generates	`f'  if the type of the operator node itself
     is float or double, otherwise null.

For example, there is an entry in efftab  for  the  `='
operator

```
	%a,aw
	%ab,a
	     IBE  A2,A1
```

Note first that two key specifications can be applied to the
same code string.  Next, observe that when  a  word  is  as-
signed	to  a  byte or to a word, or a word is assigned to a
byte, a single instruction, a mov or  movb  as	appropriate,
does  the  job.  However, when a byte is assigned to a word,
it must pass through a register to implement the sign-exten-
sion rules:

```
	%a,n
	     S
	     IB1  R,A1
```

Next,  there  is  the  question of handling indirection
properly.  Consider the expression `X + *Y', where X  and  Y
are  expressions,  Assuming  that Y is more complicated than
just a variable, but on the other hand qualifies  as  `easy'
in  the context, the expression would be compiled by placing
the value of X in a register, that of *Y in the next  regis-
ter,  and  adding  the	registers.  It is easy to see that a
better job can be done by compiling X, then Y (into the next
register),  and producing the instruction symbolized by `add
(R1),R'.  This scheme avoids generating the instruction `mov
(R1),R1'  required  actually  to  place the value of *Y in a
register.  A related situation occurs with the expression `X
+  *(p+6)',  which  exemplifies  a  construction frequent in
structure and array references.  The  addition	table  shown
above would produce

```
	[put X in register R]
	mov  p,R1
	add  $6,R1
	mov  (R1),R1
	add  R1,R
```

when the best code is

```
	[put X in R]
	mov  p,R1
	add  6(R1),R
```

As we said above, a key specification for a code table entry
may require an operand to have an indirection as its highest
operator.   To	make  use  of the requirement, the following
macros are provided.

F*   the first operand must have the form *X.  If in partic-
     ular  it  has  the  form *(Y + c), for some constant c,
     then code is produced which places the value  of  Y  in
     the  current  register.   Otherwise,  code  is produced
     which loads X into the current register.

F1*  resembles F* except that the next register is loaded.

S*   resembles F* except that the second operand is  loaded.

S1*  resembles S* except that the next register is loaded.

FS*  The  first  operand  must have the form `*X'.  Push the
     value of X on the stack.

SS*  resembles FS* except that	it  applies  to  the  second
     operand.

To  capture  the constant that may have been skipped over in
the above macros, there are

#1   The first operand must have the form *X; if in particu-
     lar it has the form *(Y + c) for c a constant, then the
     constant is written out, otherwise a null string.

#2   is the same as #1 except that  the  second  operand  is
     used.

Now  we  can  improve the addition table above.  Just before
the `%n,e' entry, put

```
	%n,ew*
	     F
	     S1*
	     add  #2(R1),R
```

and just before the `%n,n' put

```
	%n,nw*
	     SS*
	     F
	     add  *(sp)+,R
```

When using the stacking macros there is no place to use  the
constant  as  an index word, so that particular special case
doesn't occur.

The constant mentioned above can actually be more	gen-
eral  than  a number.  Any quantity acceptable to the assem-
bler as an expression will do, in particular the address  of
a  static  cell,  perhaps with a numeric offset.  If x is an
external character array, the expression `x[i+5] =  0'	will
generate the code

```
	mov  i,r0
	clrb x+5(r0)
```

via the table entry (in the `=' part of efftab)

```
	%e*,z
	     F
	     I'B1 #1(R)
```

Some  machine operations place restrictions on the registers
used.  The divide instruction, used to implement the  divide
and  mod  operations,  requires the dividend to be placed in
the odd member of an even-odd pair; other  peculiarities  of
multiplication	make  it simplest to put the multiplicand in
an odd-numbered register.  There is no theory which optimal-
ly  accounts for this kind of requirement.  Cexpr handles it
by checking for a multiply, divide,  or  mod  operation;  in
these  cases, its argument register number is incremented by
one or two so that it is odd, and if the operation  was  di-
vide or mod, so that it is a member of a free even-odd pair.
The routine which determines the  number  of  registers  re-
quired	estimates,  conservatively, that at least two regis-
ters are required for a multiplication	and  three  for  the
other peculiar operators.  After the expression is compiled,
the register where the result actually ended up is returned.
(Divide  and  mod are actually the same operation except for
the location of the result).

These operations are the ones which  cause  results  to
end  up  in  unexpected  places, and this possibility adds a
further level of complexity.  The simplest way	of  handling
the  problem is always to move the result to the place where
the caller expected it, but this  will	produce  unnecessary
register  moves in many simple cases; `a = b*c' would gener-
ate

```
	mov  b,r1
	mul  c,r1
	mov  r1,r0
	mov  r0,a
```

The next thought is used the passed-back information  as  to
where  the result landed to change the notion of the current
register.  While compiling the `='  operation  above,  which
comes from a table entry like

```
	%a,e
	     S
	     mov  R,A1
```

it  is	sufficient to redefine the meaning of `R' after pro-
cessing the `S' which does the multiply.  This technique  is
in fact used; the tables are written in such a way that cor-
rect code is produced.	The trouble is	that  the  technique
cannot	be used in general, because it invalidates the count
of the number of registers required for an expression.	Con-
sider  just `a*b + X' where X is some expression.  The algo-
rithm assumes that the value of a*b, once computed, requires
just  one register.  If there are three registers available,
and X requires two registers to compute, then  this  expres-
sion will match a key specifying `%n,e'.  If a*b is computed
and left in register 1, then there are, contrary to expecta-
tions,	no  longer two registers available to compute X, but
only one, and bad code will be produced.  To  guard  against
this possibility, cexpr checks the result returned by recur-
sive calls which implement F, S and their relatives.  If the
result	is  not in the expected register, then the number of
registers required by the other operand is  checked;  if  it
can  be  done  using those registers which remain even after
making unavailable the unexpectedly-occupied register,	then
the notions of the `next register' and possibly the `current
register' are redefined.  Otherwise a register-copy instruc-
tion  is  produced.  A register-copy is also always produced
when the current operator is one of those  which  have	odd-
even requirements.

Finally, there are a few loose-end macro operations and
facts about the tables.  The operators:

V    is used for long operations.  It is written with an ad-
     dress like a machine instruction; it expands into `adc'
     (add carry) if the operation is an  additive  operator,
     `sbc'  (subtract  carry) if the operation is a subtrac-
     tive operator, and disappears, along with the  rest  of
     the  line,  otherwise.   Its purpose is to allow common
     treatment of logical operations, which have no carries,
     and additive and subtractive operations, which generate
     carries.

T    generates a `tst' instruction if the first  operand  of
     the  tree	does  not set the condition codes correctly.
     It is used with divide and mod  operations,  which  re-
     quire  a  sign-extended 32-bit operand.  The code table
     for the operations contains an `sxt' (sign-extend)  in-
     struction	to generate the high-order part of the divi-
     dend.

H    is analogous to the `F' and `S' macros, except that  it
     calls  for  the generation of code for the current tree
     (not one of its operands) using regtab.  It is used  in
     cctab  for  all the operators which, when executed nor-
     mally, set the condition codes  properly  according  to
     the result.  It prevents a `tst' instruction from being
     generated for constructions like `if (a+b) ...'   since
     after  calculation  of the value of `a+b' a conditional
     branch can be written immediately.

All of the discussion above is in	terms  of  operators
with operands.	Leaves of the expression tree (variables and
constants), however, are  peculiar  in	that  they  have  no
operands.  In order to regularize the matching process, cex-
pr examines its operand to determine if it is a leaf; if so,
it  creates  a	special `load' operator whose operand is the
leaf, and substitutes it for the argument tree; this  allows
the table entry for the created operator to use the `A1' no-
tation to load the leaf into a register.

Purely to save space in the tables, pieces of subtables
can  be  labelled  and referred to later.  It turns out, for
example, that rather large portions of the the efftab  table
for  the `=' and `=+' operators are identical.	Thus `=' has
an entry

```
	%[move3:]
	%a,aw
	%ab,a
	     IBE  A2,A1
```

while part of the `=+' table is

```
	%aw,aw
	%    [move3]
```

Labels are written as `%[ ... : ]', before the key  specifi-
cations;  references are written with `%  [ ... ]' after the
key.  Peculiarities in the implementation make it  necessary
that labels appear before references to them.

The  example  illustrates the utility of allowing sepa-
rate keys to point to the same code string.  The  assignment
code  works  properly if either the right operand is a word,
or the left operand is a byte; but since there	is  no	`add
byte'  instruction the addition code has to be restricted to
word operands.

### Delaying and reordering

Intertwined with the code generation routines  are  two
other,	interrelated processes.  The first, implemented by a
routine called delay, is based on the observation that naive
code generation for the expression `a = b++' would produce

```
	mov  b,r0
	inc  b
	mov  r0,a
```

The  point  is that the table for postfix ++ has to preserve
the value of b before incrementing it; the general way to do
this  is  to  preserve	its value in a register.  A cleverer
scheme would generate

```
	mov  b,a
	inc  b
```

Delay is called for each expression input to rcexpr, and  it
searches  for  postfix ++ and -- operators.  If one is found
applied to a variable, the tree is patched to bypass the op-
erator	and  compiled  as  it  stands; then the increment or
decrement itself is done.  The effect is as if `a = b;	b++'
had been written.  In this example, of course, the user him-
self could have done the same job, but more complicated  ex-
amples	are  easily constructed, for example `switch (x++)'.
An essential restriction is that the condition codes not  be
required.   It	would be incorrect to compile `if (a++) ...'
as

```
	tst  a
	inc  a
	beq  ...
```

because the `inc' destroys the required setting of the	con-
dition codes.

Reordering  is  a	similar  sort of optimization.	Many
cases which it detects are useful mainly with register vari-
ables.	 If  r	is  a register variable, the expression `r =
x+y' is best compiled as

```
	mov  x,r
	add  y,r
```

but the codes tables would produce

```
	mov  x,r0
	add  y,r0
	mov  r0,r
```

which is in fact preferred if r is not a register.  (If r is
not a register, the two sequences are the same size, but the
second is slightly faster.)  The scheme is  to	compile  the
expression  as	if it had been written `r = x; r =+ y'.  The
reorder routine is called with a pointer to each  tree	that
rcexpr is about to compile; if it has the right characteris-
tics, the `r = x' tree is constructed and passed recursively
to  rcexpr; then the original tree is modified to read `r =+
y' and the calling instance of rcexpr compiles that instead.
Of  course  the  whole	business is itself recursive so that
more extended forms of the same phenomenon are handled, like
`r = x + y | z'.

Care does have to be taken to avoid `optimizing' an ex-
pression like `r = x + r' into `r = x; r =+ r'.  It  is  re-
quired that the right operand of the expression on the right
of the `=' be a ', distinct from the register variable.

The second case that reorder handles is expressions  of
the  form  `r = X' used as a subexpression.  Again, the code
out of the tables for `x = r = y' would be

```
	mov  y,r0
	mov  r0,r
	mov  r0,x
```

whereas if r were a register it would be better to produce

```
	mov  y,r
	mov  r,x
```

When reorder discovers that a register variable is being as-
signed to in a subexpression, it calls rcexpr recursively to
compile the subexpression, then fiddles the tree  passed  to
it  so	that  the  register  variable  itself appears as the
operand instead of the whole subexpression.  Here  care  has
to  be	taken  to avoid an infinite regress, with rcexpr and
reorder calling each other forever to handle assignments  to
registers.

A	third  set of cases treated by reorder comes up when
any name, not necessarily  a  register,  occurs  as  a	left
operand  of  an  assignment operator other than `=' or as an
operand of prefix `++' or `--'.  Unless condition-code tests
are  involved, when a subexpression like `(a =+ b)' is seen,
the assignment is performed and the argument  tree  modified
so that a is its operand; effectively `x + (y =+ z)' is com-
piled as `y =+ z; x + y'.  Similarly, prefix  increment  and
decrement  are	pulled out and performed first, then the re-
mainder of the expression.

Throughout code generation, the expression optimizer is
called whenever delay or reorder change the expression tree.
This allows some special cases to be  found  that  otherwise
would not be seen.
