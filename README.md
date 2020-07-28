terminal_calculator, version 1.0.0
----------------------------------
This program is capable of handling simple mathematical expressions. The term 'simple mathematical expressions' written above refers to mathematical expressions 
that can be created by using real nos. and the following operators : " - + * / ^ ". Parantheses are also supported by this program.

Now let's see how you can make use of this program. The *compute* file is execution ready for debian-based systems. In case if you wish to compile it on your machine, then simply hit the 'make' command on your terminal(for \*nix type systems) OR command line (for windows) :

`$make -C ~path/to/the/location/of/terminal_calculator`

Note that you need to have [gcc](https://gcc.gnu.org/) along with [make](https://www.gnu.org/software/make/) installed on your system in order to compile the above mentioned program. Note the presence of the '-C' argument in the 'make' command. It tells the 'make' command to look for the 'makefile' in the specified directory.

Now let's see how to use the executable file :

- .\compute OPTIONS INPUT
- .\compute EXPRESSION
- .\compute ABOUT

OPTIONS

* -a    Perform addition.
* -m    Perform multiplication.

INPUT
- Numbers seperated by space.   
  Example : 1 2.3 3.45 4.567 5.678
- Do NOT enter negative numbers, as currently the program isn't capable of accepting negative numbers while performing direct addition/multiplication via the options.   
  Example : -4 -5.3
	
EXPRESSION

- Supported operators : + - \* / ^  
  Example : 1+2\*3 
- Paranthesis is allowed. Use it as much as you want. However
  enclose the expression in quotes i.e. \'\', if you wish to make use 
  of the paranthesis.  
  Examples:
  +  Allowed - '(1+2)\*3'
  +  Not allowed - (1+2)\*3
- Note that the operators will be evaluated according to the following 
  precedence rule (high to low):  
  Paranthesis > Exponential > Division > Multiplication > Addition > Subtraction
- Euler's constant and Pi are availible in ready to use format.
  Euler's constant can be used by making use of 'e' and Pi can be used by making use of 'pi' in the expression.  
  Example : pi+1
	
ABOUT

* -h	Obtain the usage section and other helpful examples
* -v	Obtain version of this program
* -b	Obtain brief description of this program
