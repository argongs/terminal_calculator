terminal_calculator, version 1.1
==================================
This program is capable of handling simple mathematical expressions. The term *simple mathematical expressions* written above refers to mathematical expressions that can be created by using real nos. and the following operators : ` - + * / ^ `. Parentheses are also supported by this program.

Now let's see how you can make use of this program. The **compute** file is execution ready for debian-based systems. In case if you wish to compile it on your machine, then simply hit the `make` command on your terminal (for \*nix type systems) OR command line (for windows) :

`$make -C ~path/to/the/location/of/terminal_calculator`

Note that you need to have [gcc](https://gcc.gnu.org/) along with [make](https://www.gnu.org/software/make/) installed on your system in order to compile **terminal_calculator**. Note the presence of the `-C` argument in the `make` command. It tells the `make` command to look for the **makefile** in the specified directory.

Now let's see how to use the executable file (while you are in the `terminal_calculator` directory):

1. Direct operation mode :
   - Usage :

     `$ ./compute OPTIONS INPUT`

   - `OPTIONS`
     * `-a`    Perform addition.
     * `-m`    Perform multiplication.

   - `INPUT`
     * Real numbers separated by space.
     
   - Examples:
     * `$ ./compute -a 1 -2.3 3.45 -4.567 # equivalent to 1.1-2.3+3.45-4.567`
     * `$ ./compute -m 1 -2.3 4.5 # equivalent to 1*(-2.3)*4.5`

2. Expression mode :
   - Usage :
     
     `$./compute MODE EXPRESSION`

   - `MODE`
     * `-d` enable degree mode
     * This option will force the program to change the unit of the input given to the trigonometric functions to 'degrees'. Otherwise the default unit is 'radians' for the input of trigonometric functions.

   - `EXPRESSION`
     * Any mathematical expression that can be created by using the supported operators.
     * The supported operators are : `+ - \* / ^`

   - Examples :
     * `$ ./compute 1+2`
     * `$ ./compute 1+2*3`
     * `$ ./compute -d 'cos(45)' #equivalent to cos(45 degrees)`
     * `$ ./compute 'cos(45)' #equivalent to cos(45 radians)`

3. Information mode :
   - Usage :
     
     `$./compute ABOUT`
     
   - `ABOUT`
     * `-h`	Obtain the usage section and other helpful examples
     * `-v`	Obtain version of this program
     * `-b`	Obtain brief description of this program

Points to keep in mind :
------------------------
1. Parenthesis is allowed, but only in the expression mode. Use it as much as you want in that mode. However enclose the expression in quotes i.e. '', whenever you wish to make use of the parenthesis.
     * Correct usage - 
       - `$ ./compute '(1+2)'` 
       - `$ ./compute '(1+2)*3'`
     * Incorrect usage -
       - `$ ./compute (1+2)`
       - `$ ./compute (1+2)*3`

2. The operators are evaluated according to the following precedence rule (high to low):
   > Parenthesis \> Exponential \> Division \> Multiplication \> Addition \> Subtraction

3. Euler's constant and Pi are available in ready to use format. Euler's constant can be used by making use of `e` and Pi can be used by making use of `pi` in the expression as well as direct operation mode.
   - In expression mode :
     * `$ ./compute pi+e`
     * `$ ./compute '(e+pi)*90'`
  
   - In direct operation mode :
     * `$ ./compute -a pi ~e 1 2`
     * `$ ./compute -a ~pi e`
     * `$ ./compute -m ~pi 4 4`
     * `$ ./compute -m ~e 3 4`

     Notice the presence of `~` character in the direct operation mode. What does it indicate?    
     It indicates negative sign. It's meant to be used only with the constants and that too in the direct operation mode. It helps the program to capture negative constants. Note that this character is illegal in the expression mode. There's no need to use this sign in expression mode since regular representation of minus sign i.e. `-` character works perfectly over there for both numbers as well as constants.

4. If you wish to use negative nos. in the expression, then make sure to enclose each and every negative no. in parentheses to ensure correct evaluation. Otherwise, the program might consider your input as invalid.
   - Correct usage -
     * `$ ./compute '1*(-2)'`
     * `$ ./compute 'e^(-1)'`
   - Incorrect usage -
     * `$ ./compute 1*-3`
     * `$ ./compute e^-1`

5. The following functions can be directly made use of in the expression mode :

   i. Trigonometric functions -
      * `sin()` `cos()` `tan()`
      * Examples :   
        -  `./compute 'sin(pi)+cos(pi)'`
        -  `./compute 'tan(sin((cos(pi))))'`

   ii. Logarithmic functions -
       
      * `log2()` `loge()` `log10()` (representing log at base 2, e and 10 respectively) 
      * Examples :
        - `./compute log10(100)`
        - `./compute log2(1024)/loge(e^2)`
   
   Note that the trigonometric functions treat the unit of the input angles as 'radians'.  
