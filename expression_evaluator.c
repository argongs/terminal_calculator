#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "expression_evaluator.h"


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

int eval_expr(char* expression, struct number* result)
{
	int expr_length = strlen(expression);
	int infix_eval_status = 0, postfix_eval_status = 0;
	
	//printf("[info] Infix Expression : %s\n", expression);
		
	struct element *postfix_expr = NULL;
		
	if( !create_element_list(&postfix_expr, expr_length) )
		fprintf(stderr, "[error] Couldn't allocate memory for storing the postfix expression due to lack of memory.\n");
	else
	{	
		infix_eval_status = infix_to_postfix(expression, postfix_expr, expr_length);
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
										fprintf(stderr, "[error] Detected incorrect usage of operators in the infix expression.\n"); 
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
						//postfix_expr[j].is_operator = 0;
						//postfix_expr[j].type.operand.value = data.type.operand.value;
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
				else //discovered erroraneous expression
				{
					fprintf(stderr, "[error] Detected incorrect usage of operators in the infix expression.\n");
					return_value = -1;
					break;
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
				int ary = obtain_ary(postfix_expr[i].type.operator);
				
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
	int a_precedence = obtain_precedence(a);
	int b_precedence = obtain_precedence(b);
	
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
			//-> a '.', then check whether the next element is a no.
			//If the next element is a no. then simply move on.
			//But if the next element is not a no. then, it indicates 
			//the presence of an erroraneous expression
			else if( infix_expr[i] == '.' )
			{	
				if( isdigit(infix_expr[i+1]) )
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
		sscanf(no_string, "%f", &(data->type.operand.value));
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
		switch( infix_expr[i] )
		{
			case	'e'	: //for handling euler's constant
				data->type.operand.value = M_E;
				data->type.operand.precision = MAX_PRECISION;
				data->is_operator = 0;
				return i+1; //return next_index to evaluate
				//break;
			case	'p' : //for handling the 'pi'
				if( infix_expr[i+1] == 'i')
				{
					data->type.operand.value = M_PI;
					data->type.operand.precision = MAX_PRECISION;
					data->is_operator = 0;
					return i+2; //return next_index to evaluate
					//break;
				}
			default	:
				return 0; //unable to parse due to incorrect 
		}
	}
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

//Check ary value of an operator
int obtain_ary(char operator)
{
	int i;
	
	for (i = 0; i < SYMBOL_COUNT; i += 1)
	{
		if( operator == symbol_list[i].symbol && symbol_list[i].symbol_code == 1)
		{
			return symbol_list[i].ary;
		}
	}
	
	return 0;
}

//Obtain precedence of an operator
int obtain_precedence(char operator)
{
	int i;
	
	for (i = 0; i < SYMBOL_COUNT; i += 1)
	{
		if( operator == symbol_list[i].symbol && symbol_list[i].symbol_code == 1)
		{
			return symbol_list[i].precedence;
		}
	}
	
	return -1;
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
	bool lhs = isalnum(infix_expr[i-1]) || infix_expr[i-1] == ')';
	bool rhs = isalnum(infix_expr[i+1]) || infix_expr[i+1] == '(';  
	if( lhs && rhs )
		return 1;
	else
		return 0;
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
