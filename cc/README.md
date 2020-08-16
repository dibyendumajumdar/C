This is the work in progress C compiler based on the original C compiler written by Dennis Ritchie.
It is based on the `2.11BSD` version; you can view the other versions via the branch history.

## Status

Right now I am able to build and run the first pass `c0` on my Windows 10 machine. It compiles with Visual C++.
It should compile with any modern compiler but I have not yet tested this.

I have modified `c0` to generate output in text format. This makes it easy to read and understand the ouput.
A description of the intermediate language can be found in [A Tour through the Unix C Compiler](https://github.com/dibyendumajumdar/C/blob/master/docs/ctour.md).
For examples of the output produced by `c0`, please see the [tests](https://github.com/dibyendumajumdar/C/tree/master/cc/tests).

## Usage:

```
    c0 <C_source_file> <Main putput file> <String output file>
```

The `c0` command will process the `C_source_file` and produce two output files. Both output files contain intermediate
code. The `Main output file` will have the generated intermediate code. The `String output file` will contain the 
strings defined in the data segment.


## Copyrights 

Note that the copyrights to the code are as described in [Caldera License](https://github.com/dibyendumajumdar/C/blob/master/Caldera-license.pdf)
and [Alcatel-Lucent / Nokia Bell Labortatories](https://www.tuhs.org//Archive/Distributions/Research/Norman_v10/statement_regarding_Unix_3-7-17.pdf).

