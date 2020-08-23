#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "expression_evaluator.h"

int degree_mode = 0; //determines whether the trignometric functions will treat the unit of input angles as 'degree' OR 'radian'.

const struct symbol_struct symbol_list[] = {
	{'-', 1, 2, 1},//---|
	{'+', 1, 2, 2},//	|		
	{'*', 1, 2, 3},//	|--> Binary Operators	
	{'/', 1, 2, 4},//	|	
	{'^', 1, 2, 5},//---|
	
	{'(', -1, 2, 0},//--|
	{')', -2, 2, 0},//	|--> Special symbols used in various phases of
	{'$', -3, 2, 0},//	|	 expression of evaluation
	{'#', -4, 3, 0}//---|
};

struct defined_name constants[] = {
	{"e", 1},
	{"pi",2},
	{NULL,0} //to mark the end of list of constants
};

struct defined_name functions[] = {
	{"cos", 1},
	{"log2", 2},
	{"loge", 3},
	{"log10", 4},
	{"sin", 5},
	{"tan", 6},
	{NULL, 0} //to mark the end of list of functions
};

int eval_expr(char* expression, struct number* result)
{
	int expr_length = strlen(expression);
	int infix_eval_status = 0, postfix_eval_status = 0;
	//Create a new pointer and let it point to a memory location which will hold the space free expression
	char *space_free_expr = malloc(expr_length*sizeof(char));  
	
	//printf("[info] Infix Expression : %s\n", expression);
		
	struct element *postfix_expr = NULL;
		
	if( !create_element_list(&postfix_expr, expr_length) || space_free_expr == NULL )
		fprintf(stderr, "[error] Couldn't allocate memory for storing the postfix expression due to lack of memory.\n");
	else
	{	
		//Remove the white spaces from the expression
		int i, j = 0;
		//Scan the input expression and copy every charachter from it to the memory location pointed by 'space_free_expr' except the whitespaces. 
		for (i = 0; i < expr_length; i += 1)
		{
			if(expression[i] == ' ')
				continue;
				
			space_free_expr[j++] = expression[i];
		}
		//Put the end of string charachter at the end of the string
		space_free_expr[j] = '\0';
		//Give the space free expression for evaluation
		infix_eval_status = infix_to_postfix(space_free_expr, postfix_expr, expr_length);
		switch( infix_eval_status )
		{
			case	1		: //resulting postfix expression
				//printf("[info] Postfix expression : ");
				//display_elements(postfix_expr, expr_length);
				//evaluate the postfix expression
				postfix_eval_status = postfix_eval(postfix_expr, expr_length, result);
				switch( postfix_eval_status )
				{
					case	1	:	// flawless result
						//printf("[info] Evaluated value : %f\n", result);
						break;
					
					case	0	:	// lack of memory
						fprintf(stderr, "[error] Couldn't allocate memory for evaluation of the postfix expression due to lack of memory\n");
						break;
					
					case	-1	:	// incorrect postfix expression
						fprintf(stderr, "[error] Incorrect postfix expression was given for evaluation.\n");
						break;
					
					case	-2	:	//	not defined result
						fprintf(stderr, "[error] Result is not defined for such expressions.\n");
						break;
						
					default		:
						fprintf(stderr, "[error] Unexpected behaviour detected from 'postfix_eval()' funxn.\n");
				}
				break;
				
			case	0		:	//lack of memory
				fprintf(stderr, "[error] Couldn't allocate memory for conversion of the infix expression to postfix expression due to lack of memory\n");
				break;
					
			case	-1	:	//incorrect expression
				fprintf(stderr, "[error] Incorrect infix expression was given for evaluation.\n");
				break;
				
			default		:	//unexpected behaviour of infix_to_postfix()
				fprintf(stderr, "[error] Unexpected behaviour detected from 'infix_to_postfix()' funxn.\n");
		}						
	}
	
	free_element_list(postfix_expr);
		
	return infix_eval_status + postfix_eval_status; 
	//iff the sum of (infix_eval_status + postfix_eval_status) is 2, 
	//can we say that the result is flawless. Otherwise, there might have been some issue in the expression OR a possible lack of memory
}

int infix_to_postfix(char *infix_expr, struct element *postfix_expr, int length)
{
	int i = 0, j = 0, next_index = 0, return_value = 1;
	struct stack item_stack;
	struct element data, temp;
	
	//Create a stack for the conversion procedure
	if( create_stack(&item_stack) )
	{
		//Scan the infix expression
		while(infix_expr[i] != '\0')
		{
			//printf("[info] i = %d, j = %d, next_index = %d\n", i, j, next_index);
			//Push every '(' encountered in the infix expression to the stack
			if(infix_expr[i] == '(')
			{	
				/*
				Prior to pushing '(' onto the stack check whether the symbol present 
				prior to ')' is an operator OR not. If it ain't an operator, then it's
				an invalid expression. For instance, consider the following:
				Index	i-1	i ---\ This is an incorrect expression since it contains
				Symbol	5	( ---/ '5('. Why incorrect? Because there's no operator
									present in b/w. '5' and '(', so basically the expression
									present after '(' cannot be operated with the expression
									that is present prior to '(' and whose last symbol is '5'.
				However, if the paranthesis is present right in the beginning of the expression
				then there's no meaning in checking the symbol present right before it. 
				Therefore, whenever an '(' is discovered then it should be present after the index 1
				along with an operator present before it OR it should be the first symbol in the expression.
				*/
				bool operator_present = (is_valid_operator(infix_expr[i-1]) && i > 1);
				bool nested_paranthesis = (infix_expr[i-1] == '(' && i > 0); 
				if( operator_present || nested_paranthesis || i == 0 ) 
				{
					//Try to push '(' onto the stack. If the stack is full 
					//then stack overflow is imminent. Otherwise, no issues.
					temp.type.operator = '(';
					temp.is_operator = -1;
					
					if( !push(&item_stack, temp) )
					{
						fprintf(stderr, "[error] Stack Overflow! Can't push anymore.\n");
						return_value = 0;
						break;
					}
				}
				else
				{
					fprintf(stderr, "[error] Detected lack of operator prior to the opening paranthesis\n");
					return_value = -1;
					break;
				}
			}
			//Every time a ')' is dicovered then keep on popping until 
			//either '(' OR '$' is dicovered 
			else if(infix_expr[i] == ')')
			{
				//To check if the user has entered empty paranthesis '()' in the expression 
				if(infix_expr[i-1] == '(')
				{
					//If the user has entered empty paranthesis than stop evaluation
					fprintf(stderr, "[error] Detected presence of empty paranthesis i.e. '()' in the infix expression. Empty paranthesis can't be evaluated\n");
					return_value = -1;
					break;
				}
				
				//To check if there's a valid symbol present after ')'. Note that after ')',
				//either there'll be an operator OR a ')' OR '\0'.
				bool end_of_expression = infix_expr[i+1] == '\0';
				bool operator_present = is_valid_operator(infix_expr[i+1]) && !end_of_expression;
				bool nested_paranthesis = infix_expr[i+1] == ')' && !end_of_expression;
				
				if( operator_present || nested_paranthesis || end_of_expression) 
				{
					/*
					Discovery of :
					'(' implies correct paranthesis usage in infix expression 
					'$' implies incorrect paranthesis usage in infix expression
					Hence, if incorrect paranthesis usage is detected then return
					'-1', in order to indicate the existence of this issue.
					*/
					//Weirdest for loop created by us
					//	Intialise 'temp'---------->Check contents of 'temp'---->Update 'temp' at the end of every iteration 
					for( temp = pop(&item_stack) ; temp.type.operator != '('; temp = pop(&item_stack) )
					{
						if(temp.type.operator == '$')
						{
							fprintf(stderr, "[error] Detected incorrect usage of paranthesis in the infix expression.\n");
							return_value = -1;
							break;
						}
						else
						{
							//printf("[info] Popped item : %c\n", temp);
							postfix_expr[j] = temp;
							j++;
						}
					}
				}
				else 
				{
					fprintf(stderr, "[error] Detected incorrect symbol after the closing paranthesis\n");
					return_value = -1;
					break;
				}
			}
			else
			{
				next_index = parse_element(infix_expr, i, &data);
		
				if( next_index == 0 )
				{
					fprintf(stderr, "[error] Detected illegal symbol in the infix expression.\n");
					return_value = -1; //the detected illegal symbol : '\0'
					break;
				}
				else if( next_index > 0 ) //discovered an operator OR an operand
				{
					if(data.is_operator) //if discovered an operator OR $ ( )
					{
						//All the operators chosen over here are binary in nature.
						//Hence, every operator needs to have exactly 2 inputs,
						//otherwise it's an incorrect expression. Therefore, before
						//and after every operator there should be either a no.
						//or a closing OR opening paranthesis. Read the description
						//of the 'check_safety()' fn. below to understand more.
						if(check_safety(infix_expr, i))
						{
							switch( compare(item_stack.element_list[item_stack.top].type.operator, infix_expr[i]) )
							{
								//&item_stack.element[&item_stack.top].type.operator < infix_expr[i])
								case -1	:	
									if(	!push(&item_stack, data) )
									{
										fprintf(stderr, "[error] Stack Overflow! Can't push anymore.\n");
										return_value = 0;
										break;
									}	
									break;
										
								//&item_stack.element[&item_stack.top].type.operator > infix_expr[i])
								case 1	:	
									if( item_stack.top < 0 )
									{	
										fprintf(stderr, "[error] Detected incorrect usage of operators/symbols in the infix expression.\n"); 
										return_value = -1; //incorrect usage of symbols
										break;
									}
									else
									{	
										postfix_expr[j] = pop(&item_stack);
										j++;
										continue;
										//Why 'continue' ?
										//Because we still haven't pushed the most recently scanned operator from the infix expression to the stack. And w/o 'continue' the last statement of the loop will increment the index value that represents the most recently scanned operator from infix expression. Hence usage of 'continue' directly forces the program to directly jump to the beginning of the loop, w/o incrementing the index variable. 
									}
									break;
										
								default	:	
									fprintf(stderr, "[error] Unexpected behaviour detected from the 'compare()' fn.\n");
									return 0;
							}
						}
						else //incorrect usage of operators
						{
							return_value = -1;
							break;
						}
					}
					else //if discovered a no.
					{
						//To check if '+' and '-' are used as a sign instead of depicting an operator
						
						//In this program, '+' and '-' will be treated as a sign representation instead of being an operator, iff they are either :
						//i.  Preceded by '(' and succeeded by a no.
						//ii. OR they are the first charachter in the expression
						if( i == 1 || infix_expr[i-2] == '(' )
						{
							//Once a symbol has been found out to be a representation of +ve/-ve sign. then assign that sign to the succeeding no. and then pop it from the 'item_stack'.
							//Why pop it from the 'item_stack'? 
							//Because the contents of the 'item_stack' will become part of the postfix expression. And in postfix expression, at any given index either there's an operator OR there's an operand. Hence, we can't differentiate in between a sign and an operator over there. Therefore, after assigning the sign to the succeeding no., we pop out the sign from the 'item_stack'. 
							//But why does the program put the sign on the 'item_stack'? 
							//Because initially every symbol is treated as a potential operator. However, once the charachter succeeding the symbol is scanned, only then we become sure of whether or not that symbol is acting as a sign OR an operator.
							if( infix_expr[i-1] == '+' ) //positive sign
							{	
								postfix_expr[j] = data;
								pop(&item_stack);						
							}
							else if( infix_expr[i-1] == '-' ) //negative sign
							{	
								postfix_expr[j].type.operand.value = (-1)*data.type.operand.value;
								postfix_expr[j].type.operand.precision = data.type.operand.precision;
								postfix_expr[j].is_operator = 0;
								pop(&item_stack);
							}
							else
							{
								return_value = -1; //illegal usage of operators detected
								break;
							}
						}
						else
							postfix_expr[j] = data;
							
						j++;
					}
					
					i = next_index;
					//Since 'next_index' was literally meant to tell 
					//the next index value of infix expression to work with, 
					//therefore 'i' is set to 'next_index' and immediately the 
					//next iteration is initiated by using 'continue' below.
					//(since, 'parse_element()' returns the next index for
					//processing and the 'next_index' was made to hold the
					//return value of 'parse_element()')
					continue;
				}
				else //discovered erroraneous expression OR lack of memory 
				{
					switch( next_index )
					{
						case -1 : //erroraneous expression
							fprintf(stderr, "[error] Detected incorrect usage of operators/symbols in the infix expression.\n");
							return_value = -1;
							break;
						
						case -2 :
							return_value = 0; //lack of memory
							break;
						
						default :
							fprintf(stderr, "[error] Unexprected behaviour detected from parse_element() function.\n");
							return_value = -2;
							break;	
					}
					
					break; //break the loop as soon as an issue is discovered
				}
			}
			
			i++;
		}
		
		//printf("[info] &item_stack.top = %d \n", &item_stack.top);
		if(item_stack.top > 0)
		{
			do
			{
				temp = pop(&item_stack);
				//printf("[info] %c, top = %d\n", temp, &item_stack.top);
				postfix_expr[j++] = temp;
			}while(item_stack.top > 0);
		}		
		
		release_stack(&item_stack);
		return return_value;
	}
	else
		return 0;
}

//Postfix evaluator
int postfix_eval(struct element* postfix_expr, int length, struct number *result)
{
	int i = 0, j = 0, return_value = 1;
	char operand, operator;
	struct stack item_stack;
	struct element temp;
	
	//try to create a stack of 'element's so that postfix evaluation can happen
	if( create_stack(&item_stack) )
	{
		//recall that if is_operator == -4 then, that particular 'element' is empty
		while( postfix_expr[i].is_operator != -4 && i < length )
		{
			//Check if the recently scanned 'element' from the postfix expression is an 'operator'
			if( postfix_expr[i].is_operator == 1 )
			{
				//If it's an 'operator', then check how many operands, does it need? For instance, if it's binary then it'll use 2 operands.
				int ary = obtain_operator_prop(postfix_expr[i].type.operator, 1); //1 signifies 'ary' property
				
				//Now check whether OR not the stack has at least 'ary'
				//items in it.
				if(item_stack.top - ary >= 0)  
				{
					struct number operands[ary]; //create an array for holding 'ary' operands
					//Pop 'ary' 'elements' from the stack and add it to the above array iff that 'element' is an operand.
					for (j = 0; j < ary; j += 1)
					{
						temp = pop(&item_stack);
						//Check if the popped 'element' is an operator
						if(temp.is_operator) 
						{
							//if it's an operator then the postfix expression is incorrect
							return_value = -1;
							break;
						}
						else//if it's not an operator then add it to the operand array
							operands[j] = temp.type.operand;
					}
					
					//If the postfix expression was not found to be faulty during the above scan, then evaluate the recently scanned 'operator' from the postfix expression by making use of the above operand array 
					if(return_value != -1)
					{
						struct number value = {0, 0};
						int operation_status = calculate(postfix_expr[i].type.operator, operands, ary, &value);
						//Check if the operation was performed successfully
						if( operation_status == 0 )
						{	
							return_value = -1; //incorrect postfix expression, since the operation cannot be performed
							break;
						}
						else if( operation_status == -1 )
						{
							return_value = -2; //evaluation cannot be performed, since the result for such expression is not defined
							break;
						}
						else
						{
							//if the operation went successfully, then push the 
							//result onto the stack
							temp.is_operator = 0;
							temp.type.operand = value;
							if( !push(&item_stack, temp) )
							{
								//If the push operation is unsuccessful, then the system
								//simply lacks memory
								fprintf(stderr, "[error] Stack Overflow! Can't push anymore.\n");
								return_value = 0;
								break;
							}
						}
					
					}
					else
						break;//since the postfix expression was found to be faulty, therefore simply break out of the loop
				}
				else
				{	
					return_value = -1; //if the stack doesn't contain atleast 'ary' elements then it implies that the postfix expression is incorrect
					break; //since it's an incorrect expression, therefore break out of the stack
				}
				
			}
			else
			{
				//if the recently scanned 'element' from the postfix expression 
				//ain't an operator, and instead it's an operand then, simply push 
				//it onto the stack
				if( !push(&item_stack, postfix_expr[i]) )
				{
					//If the push operation is unsuccessful, then the system
					//simply lacks sufficient memory to process this program
					fprintf(stderr, "[error] Stack Overflow! Can't push anymore.\n");
					return_value = 0;
					break;
				}
			}
			i++;
		}
		
		//After scanning the entire postfix expression, 
		//if the stack top holds an operand
		if( item_stack.element_list[item_stack.top].is_operator == 0 && return_value == 1 )
		{
			//then copy the stack top value onto the 'result' variable
			*result = item_stack.element_list[item_stack.top].type.operand;
			//return_value = 1;
		}
		/*else if( return_value != -2) //if it ain't mathematical error (not defined type of expressions) then enter here
		{	
			//otherwise, the postfix expression was incorrect
			return_value = -1;
		}*/
	}
	else
	{	
		//if the stack creation cannot happen, then it implies that the system 
		//lacks sufficient memory to process this program
		return_value = 0;
	}
	
	release_stack(&item_stack); //release the memory allocated to the '&item_stack'
	return return_value;
}

//Operator comparator
int compare(char a, char b)
{
	//The following comparisions will be based on the following precedence rule:
	// '$' < '-' < '+' < '*' < '/' < '^'
	/*
	Logic of this function :
	Precedence wise, 
	if			a < b, return -1
	else if a	>	b, return 1
	else if	a == b, then the operators will treated as right associative (i.e. similar to )
	But why make them right associative and not left associative?
	'^' is a right associative operator. And in order to just make the 
	code look uniform we will treat the rest of the operators :
	'-'  '+'  '*'  '/' as right associative.
	Note that we could have treated '-'  '+'  '*'  '/' as left 
	associative too. Why? Because unlike '^', the rest are of the 
	operators are both left as well right associative. 
	So it's up to us to use them in either left OR right associative
	fashion. And since '^' is right associative, therefore just for
	making the code look uniform, we'll use right associativity for 
	the rest of the operators.
	*/
	int a_precedence = obtain_operator_prop(a ,2); //2 indicates precedence property
	int b_precedence = obtain_operator_prop(b, 2);
	
	if( a == -1 || b == -1) //incorrect operators
	{
		fprintf(stderr, "[error] Illegal charachter detected in the expression!\n");
		return -2;	
	}
	else 
	{
	
		if( a_precedence == b_precedence )
		{
			return -1; 
			/*
			treating 'b' as a higher precedence operator as 
			compared to 'a' in order to forcefully make the operator
			stored by 'a' and 'b' as right associative
			*/
		}
		else if( a_precedence < b_precedence )
			return -1;
		else //a_precedence > b_precedence
			return 1;
	
	}
	
}

//Parse the element at index 'i' of the 'infix_expr'
int parse_element(char *infix_expr, int i, struct element* data)
{
	int initial_i = i, final_i, decimal_index = 0;
	//if the scanned charachter is:
	//-> a string terminator, then parsing can't be done 
	if(infix_expr[i] == '\0')
		return 0; //unable to parse
	//-> an operator, then record it at the address pointed by 'data' 
	else if( is_valid_operator(infix_expr[i]) )
	{
		data->type.operator = infix_expr[i];
		data->is_operator = 1;
		return i+1; //return next_index to evaluate
	}
	//-> a digit, then keep looking ahead 
	// The logic over here is that the initial digit might be no. 
	// itself or a part of a bigger no. Hence the scan of further 
	// elements of the 'infix_expr' will tell us how big this no. is.
	else if( isdigit(infix_expr[i]) )
	{
		i++;
		while(1)
		{
			//If the current index of 'infix_expr' holds :
			//-> a no. then move to the next index
			if( isdigit(infix_expr[i]) )
			{
				i++;
			}
			//-> a '.', then check whether the following conditions hold true :
			//i. next element is a no. 
			//ii.the 'decimal_index' is equal to 0. 
			//If both the conditions mentioned above hold true, then simply move on.
			//However, if atleast 1 condition isn't true, then it indicates 
			//the presence of an erroraneous expression.
			//The first condition is easy to understand. But what about the second one? What does it mean?
			//The second condition ensures that atmost one decimal point should exist in a no. Note that initially 'decimal_index' variable is set to 0. If it's modified, then it implies that a decimal point has been located. Now note that, the 'decimal_index' variable will always be modified in such a way that the it's resultant value won't be 0. Why? Because the variable that is used to update 'decimal_index' is 'i'. This variable comes from the argument of this function, and the smallest value that it can have is 0 (since it represents the index of an array.) And right before entering this while() contruct, the value of 'i' is incremented by 1. Implying that the prior to entering this loop, the minimum value of 'i' will be 1. So, the minimum value that can be stored in the 'decimal_index' is 1. Hence, we can say that if 'decimal_index' holds a value that is > 0, then it implies that a decimal point has been encountered.
			else if( infix_expr[i] == '.' )
			{	
				if( isdigit(infix_expr[i+1]) && decimal_index == 0 )
				{
					decimal_index = i; //storing the index at which '.' was found in order to calculate the precision of the no.
					i += 2;
				}
				else
					return -1; //erroraneous expression
			}
			//-> is neither a no. nor a '.' then it implies the end of
			// the no. we were scanning. In this scenario, simply break
			// out
			else
				break; 
		}
		//record the no. inside the memory location pointed by 'data'
		final_i = i;
		char *no_string = strndup(infix_expr + initial_i, (final_i - initial_i)*sizeof(char));
		//parse the scanned no. into a floating point no.
		sscanf(no_string, "%lf", &(data->type.operand.value));
		if( decimal_index )
			data->type.operand.precision = final_i - decimal_index - 1;
		//Why final_i - decimal_index - 1 and not final_i - decimal_index?
		//Consider the following example
		//Index		0 1 2 3 4 
		//Element   9 . 8 7 6
		//At the end of the loop, the 'final_i' will hold 5, based on the above logic. And the value of 'decimal_index' will be 1. Now, there are 3 digits after the decimal point in 9.876. However, computing 'final_i - decimal_index' will yield 4. Hence, we subtract 1 from this result in order to get the appropriate no. of digits after the decimal point. Hence, final_i - decimal_index - 1 is used
		else
			data->type.operand.precision = 0;
		//printf("[info] Extracted no. : %f\n", data->type.operand.value);
		data->is_operator = 0;
		
		return final_i;
	}
	//-> a letter of english alphabet, then it might be possibly a constant
	else if( isalpha(infix_expr[i]) )
	{
		//Check if the scanned charachter belongs to one of the defined constants
		int return_status = lookup_constant(1, infix_expr, i, &(data->type.operand));
		
		if( return_status == 0 )
			//Check if the scanned charachter belongs to one of the defined functions, iff the charachter was not found to be constant
			return_status = lookup_function(infix_expr, i, &(data->type.operand));
		
		data->is_operator = 0;
		return return_status;
	}
}

//Check if the input 'term' represents a defined constant (defined in the 'constants' variable in 'expression_evaluator.h') OR not. If it is, then simply lookup the corresponding value and store it in the provided data structure
int lookup_constant(int multiplier, char *term, int i, struct number* no) 
{
	//record the name of the constant
	int j = 0, max_length = 5;
	char const_name[max_length];
	
	//Keep recording the name of the constant as long as all of the following conditions hold true:
	//i. the name of the constant doesn't exceed the 'maxlength' in size 
	//ii. alphabetical letters are encountered
	//iii. the scanned charachter isn't the end of string charachter
	
	while( isalpha(term[i]) && term[i] != '\0' && j < max_length )
		const_name[j++] = term[i++];
	
	if( j == max_length )
		return 0; //not a valid constant
	
	const_name[j] = '\0'; //put end of string charahcter at the end of the array, which is used for storing the name of the constant. Why? Because the process of copying has been done manually, so the end of string charachter also has to be placed manually.
	//Obtain the code for the scanned constant
	int name_code = fetch_name_code( const_name, 1 );
	
	//Assign appropriate value to 'no' based on the code
	switch( name_code )
	{
		case 1 : //for handling euler's constant
			no->value = M_E;
			no->precision = MAX_PRECISION;
			return i; //return next_index to evaluate
			//break;
		
		case 2 : //for handling the 'pi'
			no->value = M_PI;
			no->precision = MAX_PRECISION;
			return i; //return next_index to evaluate
			//break;
			
		default	:
			return 0; //unable to parse due to presence of illegal charachter
	}
}

//Check if the input 'term' represents a defined function (defined in the 'constants' variable in 'expression_evaluator.h') OR not. If it is, then simply perform the function with the given input and store the corresponding result in the provided data structure
int lookup_function(char *term, int i, struct number* no) 
{
	int j = 0, expr_length = 0, max_length = 10;
	char func_name[5];
	
	char *input_expr = malloc(max_length*sizeof(int));
	if(input_expr == NULL)
	{
		printf("[error] Can't process the input expression due to lack of memory. Please retry after sometime\n");
		return -2;
	}
	
	//Obtain name of the function
	while( term[i] != '(' && term[i] != '\0' && j < 6)
		func_name[j++] = term[i++];
	
	func_name[j] = '\0';//put end of string charahcter at the end of the array, which is used for storing the name of the function. Why? Because the process of copying has been done manually, so the end of string charachter also has to be placed manually.
	
	//Check the length of the function name as well as the usage of the function
	//If the length exceeds the maximum function name length, then it's an incorrect function
	//If the end of string charachter is obtained while scanning the name of the function, then either the function does not exist OR it is used incorrectly. In either case, parsing becomes impossible.
	if(j == 6 || term[i] == '\0')
		return 0; //unable to parse since the function does not exist
	else if(term[i] == '(')
	{	
		j = 0;
		input_expr[j++] = '('; //store the scanned '(' into an array which will be resposnible for holding the input expression
		i++;//move on to the next index
		expr_length++;
		
		//extract the expression that is present as input to the function
		int counter = 1; //represents the no. of pairs of parantheses read
		
		//scan the expression given as input to the function for appropriate no. of parantheses 
		while( counter != 0 && term[i] != '\0' )
		{
			if( term[i] == '(' )
				counter++;
			else if( term[i] == ')' )
				counter--;
			
			input_expr[j++] = term[i++]; //copy the charachters scanned from the input expression into the array which is specially meant for storing the input expression
			expr_length++;
			
			//increase the maximum length limit for the input expression, everytime it becomes full
			if( j == max_length )
			{
				int increment_by = 10;
				max_length += increment_by; 
				char *allocation_status = realloc(input_expr, max_length*sizeof(char));
				if(allocation_status == NULL)
				{	
					printf("[error] Can't process the expression given input to %s function due to lack of memory. Please retry after sometime\n", func_name);
					return -2;
				}
			}
		}
		
		//If the input expression is incorrect then, counter will not be 0  
		if( counter != 0 )
		{
			printf("[error] The input expression provided to the %s function is incorrect.\n", func_name);
			return -1; //unable to parse since the input given to the function is incorrect
		}
		 
		input_expr[j] = '\0';//put end of string charahcter at the end of the array, which is used for storing the input expression. Why? Because the process of copying has been done manually, so the end of string charachter also has to be placed manually. 
		 
		//evaluate the input expression
		struct number eval_result = {0,0};
		int eval_status = eval_expr(input_expr, &eval_result);	
		
		free(input_expr); //free the space allocated to input_expr
		
		if(eval_status == 2)
		{
			//Obtain the code for the scanned function
			int name_code = fetch_name_code( func_name, 2 );
			
			//Check whether OR not the scanned function is log2, loge OR log10.
			//If it's one of them, then check if the input to that function is 0.
			//If it's 0, then report 'not defined' error and stop 
			if( name_code >= 2 &&  name_code <= 4 && eval_result.value <= 0 )
			{
				printf("[error] The input given to function %s evaluates to %.*f. And %s(%.*f) is not defined.\n", func_name, eval_result.precision, eval_result.value, func_name, eval_result.precision, eval_result.value);
				exit(EXIT_FAILURE);
			}
			
			if( degree_mode ) //if degree mode is enabled then, the input given to the trigonometric functions should be converted to radians, since the user has given the input to these functions in the form of degrees
			{
				eval_result.value *= (M_PI/180); //convert degree to radians
			}
			
			//Evaluate the function by using the evaluated result as input
			switch( name_code )
			{	
				case 1 : //cos(x)
					no->value = cosf(eval_result.value);
					no->precision = MAX_PRECISION; 
					break;
					
				case 2 : //log2(x)
					no->value = log2f(eval_result.value);
					no->precision = MAX_PRECISION;
					break;
					
				case 3 : //loge(x)
					no->value = logf(eval_result.value);
					no->precision = MAX_PRECISION;
					break;
					
				case 4 : //log10(x)
					no->value = log10f(eval_result.value);
					no->precision = MAX_PRECISION;
					break;
					
				case 5 : //sin(x)
					no->value = sinf(eval_result.value);
					no->precision = MAX_PRECISION;
					break;
					
				case 6 : //tan(x)
				//note that the usage of braces is necessary in order to create and initialise variables inside a 'case' section of a switch block. In absence of these braces, any sort of simultaneous creation of a variable and initialisation is declared incorrect (eg. int i = 0; simultaneously creates a variable 'i' and initialises it to 0). TO read more about this issue head over to : "https://stackoverflow.com/questions/92396/why-cant-variables-be-declared-in-a-switch-statement#92439"
				{	
					
					float multiplicity = eval_result.value/M_PI_2; //Obtain the multiplicity of the input w.r.t. pi/2
					int approx_multiplicity = roundf(multiplicity); //Round off the multiplicity to the nearest integer
					int is_diff_small = isless(fabsf(multiplicity - approx_multiplicity), powf(10, -MAX_PRECISION)); //Compare the difference b/w. the rounded off multiplicity and actual multiplicity w.r.t. 10^(-MAX_PRECISION). If the difference value is higher that 10^(-MAX_PRECISION), then it implies that the difference is significant b/w. the approximate and actual multiplicity. If the difference is less than 10^(-MAX_PRECISION) then it implies that the difference ain't significant thus, the input value represents an angle that is basically a multiple of pi/2.
					
					//If the difference is small and the approximated multiplitcity is odd, then it basically implies that the input that will be fed to tan() is nothing but an odd mulitple of pi/2. And at odd multiples of pi/2 the tan function yields a not defined result
					if(is_diff_small && approx_multiplicity%2)
					{
						printf("[error] The input given to function tan() evaluates to an odd multiple of pi/2. And result of applying tan() on such input is not defined.\n");
						exit(EXIT_FAILURE);
					}
					else
					{
						no->value = tanf(eval_result.value);
						no->precision = MAX_PRECISION;
					}
				}
					break;
					
				default :
					printf("[error] Unexpected return value recieved from fetch_name_code(). Looks like the code is corrupted.\n");
					exit(EXIT_FAILURE);
					//return -3;
			}
			return i; //on the account of successful evaluation of the function return the next index for processing
		}
		else
		{
			printf("[error] The input expression provided to the %s function is incorrect.\n", func_name);
			exit(EXIT_FAILURE);
			//return 0; //unable to parse since the input given to the function is incorrect
		}
	}
}

//Checks whether OR not the given 'input_name' is a member of the given 'type'. 
int fetch_name_code(char *input_name, int type)
{
	struct defined_name *ptr = NULL;
	
	//Check if the requested type of names exist
	switch( type )
	{
		//constants
		case 1 : 
			ptr = constants;
			break;
		//functions
		case 2 :
			ptr = functions;
			break;
			
		default :
			printf("[error] Incorrect type given as input");
			return 0;
	}

	//Loop through all the names within the given type in order to find a match for the 'input_type'
	while( ptr->name != NULL )
	{
		//Once a match has been found, then return the code corresponding to it
		if( strcmp(ptr->name, input_name) == 0 )
			return ptr->code;
		
		ptr++;
	}

	//If even after looping through all the possible names there ain't no name that matches with the 'input_name' then the 'input_name' doesn't exist in the records
	return -1; //incorrect name given as input
	
}


//Check if the input operator is legal
int is_valid_operator(char operator)
{
	int i;
	
	for (i = 0; i < SYMBOL_COUNT; i += 1)
	{
		if( operator == symbol_list[i].symbol && symbol_list[i].symbol_code == 1)
		{
			return 1;
		}
	}
	
	return 0;
}

//Obtain a specific property (symbol_code, ary, precdence) of the given input charachter
int obtain_operator_prop(char operator, int property_type)
{
	int i;
	
	//Check if the input 'operator' is a valid operator by matching it against the predefined collection of operators
	for (i = 0; i < SYMBOL_COUNT; i += 1)
	{
		//Once the match is found then return the requested property value
		if( operator == symbol_list[i].symbol && symbol_list[i].symbol_code >= 1)
		{
			switch( property_type )
			{
				case 1 : //ary
					return symbol_list[i].ary;
				case 2 : //precedence
					return symbol_list[i].precedence;
				case 3 : //symbol_code
					return symbol_list[i].symbol_code;
				default :
					return -1; //if the requested property is invalid, then return -1
			}
		}
	}
	
	return 0; //if the input 'operator' isn't present in the predefined collection of operators then, simply exit 
}

//Calculate and return operands[1] operator operands[0] 
int calculate(char operator, struct number *operands, int ary, struct number *result)
{
	int return_value = 1;
	
	switch( ary )
	{
		case	2	:
			switch( operator )
			{
				case	'-'	:	
					result->value = operands[1].value - operands[0].value;
					//Adjust precision of the result
					//In case of addition the precision, of the result will be the same as that of the operand with highest precision
					if( operands[1].precision > operands[0].precision )
						result->precision = operands[1].precision;
					else 
						result->precision = operands[0].precision;
					break;
					
				case	'+'	:
					result->value = operands[1].value + operands[0].value;
					//Adjust precision of the result
					//In case of subtraction, the precision of the result will be the same as that of the operand with highest precision
					if( operands[1].precision > operands[0].precision )
						result->precision = operands[1].precision;
					else 
						result->precision = operands[0].precision;
					break;
					
				case	'*'	:
					result->value = operands[1].value*operands[0].value;
					//Adjust precision of the result
					//In case of multiplication, the precision of the result will be set to the sum of precision of both the operands. i.e. If operand 1's precision is 'a' and operand 2's precision is 'b' then the precision of the result will be set to 'a+b' 
					result->precision = operands[1].precision + operands[0].precision;
					break;
					
				case	'/'	:
					
					if(operands[0].value == 0)
					{
						fprintf(stderr, "[error] Cannot divide by 0.\n");
						return_value = -1; //Not defined result
					}
					else
					{
					
						result->value = operands[1].value/operands[0].value;
						//Adjust precision of the result
						//In case of division, the precision of the result will be set to 0 if the resulting division operation leads to 0 remainder. Otherwise, the precision will be set to max possible value. 
						if( operands[0].precision == 0 && operands[1].precision == 0 )
						{	
							if( (int)operands[1].value %  (int)operands[0].value == 0)
								result->precision = 0;
							else
								result->precision = MAX_PRECISION;
						}
						else 
							result->precision = MAX_PRECISION;
					}
					
					break;
				
				case	'^'	:
				
					if(operands[0].value == 0 && operands[1].value == 0)
					{
						fprintf(stderr, "[error] Cannot perform 0^0.\n");
						return_value = -1; //Not defined result
					}
					else
					{
						result->value = powf(operands[1].value, operands[0].value); //operands[1]^operands[0]
					//Adjust precision of the result
						if( operands[0].precision == 0 && operands[1].precision == 0 )
							result->precision = 0;
						else
							result->precision = MAX_PRECISION;
					}
					break;
				
				default :
					return_value = 0;
			}
			if(result->precision > MAX_PRECISION)
				result->precision = MAX_PRECISION;
			break;
		
		default		:
			return_value = 0;
	}
	return return_value;
}

/*
Checks whether a operator has been appropriately used OR not in the 
infix expression. What does appropriate usage mean?
operand operator operand -> this is appropriate usage of operator
operator operator operator OR operator operator operand OR
operand operator operator -> these are all inappropriate
So how does the checking happen?
If the 'i'th index of the array (which holds the infix expression)
contains an operator, then the 'i-1'th & 'i+1'th index should contain 
an operand. Note that this operand can either be a no./constant (such as 'pi' OR 'e') 
OR it can be a part of bigger operand. Bigger operand?
Consider the following example :
Index   : 0 1 2 3 4 5 6
element: ( 4 + 3 ) * 6
Over here, the * operator has 2 operands : (4+3) and 6. Since we 
represent the infix expression via an array therefore, ')' is
the symbol that will be present in the immediate LHS of *.
And the presence of ')' indicates that a 'bigger operand' is present
in the LHS of '*'. So basically, a bigger operand is nothing but an 
operand composed of another expression and enclosed by parantheses.
And the only way we can understand that an operator is using a bigger 
operand by looking at those parantheses. Presence of a closing 
paranthesis on the LHS indicates the possible presence of a bigger 
operand on the LHS. Similarly, presence of '(' on the RHS indicates 
the presence of bigger operand on the RHS. But anything apart from
this cannot be considered as an appropriate representation of
big operand.
One may ask that based on the above logic, won't ')*(' also be 
considered as an expression?
Good question! The answer is NO. There are other checks put in place
to specifically take care of extra parantheses. 
*/
int check_safety(char *infix_expr, int i)
{
	int next_index = i+1, prev_index = i-1;
	//Conditions for 'safe' operator symbol 
	bool correct_lhs = isalnum(infix_expr[prev_index]) || infix_expr[prev_index] == ')';
	bool correct_rhs = isalnum(infix_expr[next_index]) || infix_expr[next_index] == '(';  
	//COnditions for 'safe' sign symbol
	bool exceptional_symbol = obtain_operator_prop(infix_expr[i], 2) == 1; //Obtain 'symbol_code' property of the input symbol to check whether OR not the input symbol can function as an unary operator by acting a 'sign' too.
	bool correct_preceding_char = ( i == 0 || (i>0 && infix_expr[prev_index] == '(') ); 
	
	if( correct_lhs && correct_rhs || exceptional_symbol && correct_preceding_char)
		return 1;
	else
		return 0;
}

int adjust_precision(float value)
{
	char no_string[20];
	int i = 0, zero_streak = 0;
	
	//sprintf will convert the float type 'value' into a string
	sprintf(no_string, "%.*f", MAX_PRECISION+1,value);
	//Note that the precision of the 'value' is set to MAX_PRECISION+1
	//Why? 
	//Because in the following code, it will allow us to round off
	//the 'value' to have a precision value of MAX_PRECISION
	
	char *decimal_ptr = strchr(no_string, '.');
	int decimal_index = decimal_ptr - no_string;
	i = decimal_index + 1;
	int last_index = decimal_index + MAX_PRECISION;
	
	if( no_string[i] == '\0' )
		return -1; //incorrect input
	else
	{
		do
		{
			if( isdigit(no_string[i]) )
			{
				if( no_string[i] == '0' )
					zero_streak++;
				else
					zero_streak = 0;
				i++;
			}
			else
				return -1; //incorrect input
				
		}while (no_string[i] != '\0' && i <= last_index);
	
		if( zero_streak )
			return MAX_PRECISION - zero_streak;
			//Eg.
			//Index 0 1 2 3 4 5 6 7
			//Value	0 . 5 0 0 0 0 0
			//Based on above logic, the value of 'zero_streak' in
			//above case will be 5. Now, if MAX_PRECISION is set to 
			//6 then, simply doing MAX_PRECISION - zero_streak will
			//give us the no. of significant digits after the decimal
			//point and thus the most appropriate precision value.
		else
		{
			
			//Note that we don't need to round off the input value, since printf() takes care of it by default. Forceful roundoff over here will lead to incorrect result since by default printf will round off the results prior to display
			
			return MAX_PRECISION;
		}
	}
}

void enable_degree_mode()
{
	if( degree_mode == 0 )
		degree_mode++;
}

int is_degree_mode_set()
{
	return degree_mode;
}

int create_element_list(struct element **element_list, int length)
{
	*element_list = malloc(length*sizeof(struct element));
	if(*element_list == NULL)
		return 0;
	else
	{
		int i;
		for (i = 0; i < length; i += 1)
		{
			(*element_list)[i].is_operator = -4;
			(*element_list)[i].type.operator = '#';
		}
		return 1;
	}
}

void display_elements(struct element* element_list, int length)
{
	int i = 0; 
	//printf("\n[info] length = %d\n", length);
	//printf("\n[info] ");
	while( element_list[i].is_operator != -4 && i < length )
	{
		if(element_list[i].is_operator)
			printf("%c ", element_list[i].type.operator);
		else
		{
			/*
			What's logic behind the following statements?
			Note that by default our manually created datatype 'element'
			supports 'float' type operands. Now even an integer will
			be displayed in a floating point style due to this. For instance,
			4 will be displayed as 4.00000, which is unnnecessary. So, to avoid
			this the following statement try to find out whether a given operand
			is integer or not. The logic is simple, if it's an integer then 
			making it multiply by 10 and then modding it with 10 will yield 0.
			Simply put 4.0000 * 10 == 40.0000. And 40.0000 % 10 == 0.
			Otherwise, if it's not an integer then, the result won't be 0.
			Simply put 4.6250 * 10 == 46.2500. And 46.25 % 10 != 0.
			And note that '%' operation is performed only on integers. Hence,
			we need to type cast the result of product(no. to display * 10) 
			to integer, prior to applying '%' operation on it 
			*/
			if( ((int)element_list[i].type.operand.value*10)%10 == 0 )
				printf("%d ", (int)element_list[i].type.operand.value);
			else
				printf("%f ", element_list[i].type.operand.value);
		}
		i++;
	}
	printf("\n");
}

void free_element_list(struct element *element_list)
{
	free(element_list);
}

int create_stack(struct stack *item_stack)
{
	int max_length = 10;

	//Creating a stack, capable of holding atleast 10 charachters
	//item_stack->element_list = malloc(max_length*sizeof(struct element));
	create_element_list(&(item_stack->element_list), max_length);

	if(&item_stack->element_list == NULL)
		return 0;
	else
	{
		item_stack->max_len = max_length;
		item_stack->element_list[0].type.operator = '$';
		item_stack->element_list[0].is_operator = -3;
		item_stack->top = 0;
		return 1;
	}
}

void release_stack(struct stack *item_stack)
{
	free_element_list(item_stack->element_list);
}

int push(struct stack* item_stack, struct element data)
{
	if(item_stack->top == (item_stack->max_len - 1))
	{
		//If the stack is full, then expand the stack by adding 10 more
		//cells to the stack and then push the new charachter 
		//onto the stack.  
		//If the expansion is successfull then no issue, otherwise 
		//stack overflow is bound to occur 
		int new_size = item_stack->max_len + 10; 
		
		//adding 10 element_list worth of space
		/*
		Note the interesting datatype used over here : void*
		Why is it used? 
		The return type of 'realloc()' is void*. Now if, 
		'realloc()' won't be able to perform data allocation
		then, it will return NULL. And the best way to store 
		NULL is to hold it in a variable of type void*.
		Refer to "https://stackoverflow.com/questions/21559293/what-type-is-null" to understand this logic
		*/
		void* realloc_status = realloc(item_stack->element_list,(new_size)*sizeof(struct element));
		
		if( realloc_status == NULL )
			return 0;
		else
			item_stack->max_len = new_size;
	}
	
	item_stack->top++;
	
	if(data.is_operator)
	{
		item_stack->element_list[item_stack->top].type.operator = data.type.operator;
		item_stack->element_list[item_stack->top].is_operator = data.is_operator;
	}
	else
	{
		item_stack->element_list[item_stack->top].type.operand.value = data.type.operand.value;
		item_stack->element_list[item_stack->top].type.operand.precision = data.type.operand.precision;
		item_stack->element_list[item_stack->top].is_operator = 0;
	}
	
	return 1;
}

struct element pop(struct stack* item_stack)
{
	return item_stack->element_list[item_stack->top--];
}
