/*
A major limitation : The -a and -m flags DO NOT accept negative nos.
*/

//C manually created libraries
#include "expression_evaluator.h"
//C POSIX libraries
#include <unistd.h> //for parsing the input options 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


//Macros
#define MAXLINE 1000 //maximum line length for a line read from a file

#define PROGRAM_NAME "compute"
#define VERSION "1.0.1"

//Funxn prototype area 

int eval_opt(int, char* [], int*, int*);
int calc_value(int, char* [], struct number*, int);
int obtain_precision(char*);
int display_usage(void);

//argc : argument count | argv : argument vector
int main(int argc, char *argv[])
{
	//Display the input given
	/*
	printf("[info] Input Command :\t'");
	for (int i = 1; i < argc; i += 1)
	{
		printf("%s ", argv[i]);
	}
	printf("'\n");
	*/
	
	//printf("[info] Initially argc = %d and argv[0] = %s\n", argc, argv[0]);
	
	int option_index = 0, user_opt = 0;
	
	if( eval_opt(argc, argv, &option_index, &user_opt) || argc < 2)
	{
		printf("[error] Incorrect usage detected.\n");
		printf("[.....] Lemme show you how to use me mate.\n");
		//printf("[info] Reached here with %d....\n", display_usage());
		
		if( display_usage() )
		{
			printf("[error] Can't fetch the help document due to lack of memory. Retry after sometime\n");
		}
		
		exit(2);
	}
	else
	{	
		if(user_opt)
		{
			if(user_opt <= 2) //add or multiply here
			{
				argc -= option_index;
				argv += option_index;	
				
				struct number result = {0,0};
				//Send the input expression for evaluation and print the results,
				//if the calculation has happened flawlessly
				if( calc_value(argc, argv, &result, user_opt) )
					printf("[info] Result : %.*f\n", result.precision, result.value);//);
			}
			else //for showing help, version etc. info
			{
				switch(user_opt)
				{
					case 3	:	
						printf("[info] Usage of 'compute':\n");
						if( display_usage() )
						{
							printf("[error] Can't fetch the help document due to lack of memory. Retry after sometime\n");
						}
						break;
						
					case 4	:	
						printf("[info] %s version %s\n", PROGRAM_NAME, VERSION);
						break;
						
					default	: 
						printf("[error] Obtained user option is beyond expectations. Can't process it. Evacuating with immediate effect.\n");
				}
			}
		}
		else
		{
			struct number result = {0, 0};
			int eval_status = eval_expr(argv[1], &result);
			if(eval_status == 2)
				printf("[info] Result : %.*f\n", result.precision, result.value);
				//Note that usage of '*' in '.*f' allows us to make use of variable
				//precision value. For instance, via this method we can display
				//5.653 as 5.6 OR 5.65 OR 5.653, depending upon the value of precision we provide.
				//How do we supply the value of precision?
				//By placing an integer value (which will hold the precision value)
				//which immdiately after the string provided to printf.
				//i.e. 				
				//printf("%.*f", precision, float_value);
				//					^			^
				//precision value---|			|--float value to display
			else
				printf("[info] Couldn't evaluate the expression\n");
		}
	}
		
	//printf("[info] Now argc = %d and argv[0] = %s\n", argc, argv[0]);
	
	return 0;
}

//Funxn. definition area

/*
This function evaluates the options and the corresponding given by 
the user. If they're invalid then it will return a non zero value otherwise it will return 0.
*/
int eval_opt(int argc, char* argv[], int* option_index, int* user_opt)
{
	//Evaluate the input options and corresponding arguments
	int scanned_option = 0, option_no = 0, err_flg = 0;
	int return_array[2];
	//'scanned_option' will be used to hold the options obtained via 'getopt()'
	
	/*
	'option_no' will be used to represent the options. Why? 
	Note that we currently wish to use the 'a', 'm'
	(add, multiply) in a mutually exclusive fashion.
	That is, if option 'a' is chosen, then presence of 'm' 
	indicates error, since each option(a/m) is used to identify the
	unique operation that is to be performed with each the element of
	the input array.
	*/
	
	/*
	'err_flg' will be used to represent presence of an erroraneous 
	input. If it's set to 0, then it implies that no error has been 
	detected yet. And if it's >0 then it represents that error has 
	been detected.
	*/
	
	/*
	return_array is responsible for holding 2 values. One is the value
	of the 'err_flg' variable and the other will be the value of 
	'optind' (the variable from the 'unistd.h' library).  
	*/
	
	/*
	Presence of ':' charachter right in the beginning of the 'option'
	string ensures that if 'getopt()' function cannot find an argument 
	for an argument-based-option in the input option then it'll return
	':' charachter. 
	Now what happens if the first charachter ain't ':'?
	In it's absence, 'getopt()' will return '?' 
	charachter if it cannot find an argument for argument-based-option.
	Note that, if 'getopt()' encounters an option charachter that is 
	not present in the 'option' string then it will return '?'.
	(regardless of the presence of ':' as the first charachter in the 'option' string)
	*/
	  
	const char *options = ":amhv"; 
	//'getopt()' returns '-1' after it has scanned all the input 
	//options and thier corresponding arguments(if they exist) 
	while(err_flg == 0 && (scanned_option = getopt(argc, argv, options)) != -1)
	{
		/*
		Note that 'optind' is a variable that comes from 'unistd.h'
		library. If it contains value 'k' then it implies that 
		currently, 'k'th option is being evaluated 
		*/
		//printf("[info] Option no. %d is '%c', with opterr = %d\n", optind, scanned_option, opterr);
		
		/*
		if(scanned_option == '?' && isdigit(opterr))
		{
			printf("[info] -ve no. detected\n");
			continue;
		}
		*/
		
		switch(scanned_option)
		{
			case 'a':	
				if(option_no == 0)
				{	
					//printf("[info] Add!\n");
					option_no = 1; 
					//Setting option_no to '1' indicates that
					//'add' option has been selected
				}
				else
					err_flg++;
				/*
				If the option 'm' is detected along with 'a',
				then it's an error. Hence the error flag is incremented over here.
				*/
				break;
				
			case 'm':	
				if(option_no == 0)
				{	
					//printf("[info] Mul!\n");
					option_no = 2; 
					//Setting option_no to '1' indicates that
					//'multiply' option has been selected
				}
				else
					err_flg++;
				/*
				If the option 'a' is detected along with 'm',
				then it's an error. Hence the error flag is incremented over here.
				*/
				break;
				
			case 'h':	
				if(option_no == 0)
				{	
					//printf("[info] Help!\n");
					option_no = 3; 
					//Setting option_no to '3' indicates that
					//'help' option has been selected
				}
				else
					err_flg++;
				break;
				
			case 'v':	
				if(option_no == 0)
				{	
					//printf("[info] Version!\n");
					option_no = 4; 
					//Setting option_no to '4' indicates that
					//'version' option has been selected
				}
				else
					err_flg++;
				break;
				
			case '?':	
				if( !isdigit(optopt) )
				{	
					printf("[error] I can't recognise '%c' option\n", optopt);
					err_flg++;
				}
				break;
			
			default	:	
				printf("[info] Now this is some out of the world return value. Can't deal with this. Goodbye!\n");
		}
		
	}
	
	*option_index = optind;
	*user_opt = option_no;
		
	return err_flg;
}

int calc_value(int argc, char* argv[], struct number* result, int option_no)
{
	int i, return_value = 1;
	struct number temp = {0, 0};
	
	switch(option_no)
	{
		//add
		case 1	:	for (i = 0; i < argc; i += 1)
					{
						sscanf(argv[i], "%f", &temp.value);
											
						//record the precision of the scanned no. and
						//adjust the precision of the result accordingly
						temp.precision = obtain_precision(argv[i]);
						if( result->precision < temp.precision )
							result->precision = temp.precision;
						
						result->value += temp.value;
					}
					break;
		//multiply
		case 2	:	result->value = 1;
					int result_precision = 0;
					
					for (i = 0; i < argc; i += 1)
					{
						sscanf(argv[i], "%f", &temp.value);
						
						//record the precision of the scanned no. and
						//adjust the precision of the result accordingly
						temp.precision = obtain_precision(argv[i]);
						result_precision = result->precision + temp.precision;
						if( result_precision < MAX_PRECISION ) 
							//if the resulting precision is less than the MAX_PRECISION value,
							//then set the precision of the result as the sum of the 
							//precision of the recently scanned no. and the last result value
							result->precision = result_precision;
						else
							//otherwise, set the precision of the result to MAX_PRECISION value
							result->precision = MAX_PRECISION;
						
						result->value *= temp.value;
					}
					break;
		default	:	printf("[error] Incorrect option supplied!\n");	
					return_value = 0;
	}
	
	return return_value;
}

int obtain_precision(char* no_string)
{
	int precision = 0;
	
	//Locate the '.' in the 'no_string[]'
	char *location = strchr(no_string, '.');
	if( location != NULL )
	{	
		int index = location - no_string;
		precision = strlen(no_string) - (index+1);
	}
		
	return precision;
}

int display_usage(void)
{
	FILE *read_ptr = fopen("help_doc", "r");
	int return_value = 0;
	
	if(read_ptr == NULL)
	{
		return_value = 1;
	}	
	else
	{
		char *line = malloc(MAXLINE);
		/*
		malloc() allocates requested amt. of memory and hands over
		the address of the allocated memory to 'line' pointer.
		This allows the fgets() fn. below to easily fetch a line
		from the file it reads(via tha 'read_ptr') and copy it 
		to the memory location allocated to the 'line' pointer.
		This ultimately helps to access the contents present in the
		file (pointed to by 'read_ptr') in a line by line fashion.
		*/
		while(fgets(line, MAXLINE, read_ptr) != NULL)
			printf("%s", line);
		
		free(line); //Don't forget to free the memory allocated to 'line'
	}
	
	fclose(read_ptr);
	return return_value;
}
