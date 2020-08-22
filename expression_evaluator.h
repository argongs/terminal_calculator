#define SYMBOL_COUNT 9
#define MAX_PRECISION 6

struct symbol_struct
{
	char symbol;
	int symbol_code, ary, precedence;
};

struct number
{
	double value; //to ensure higher precision in calculation 
	int precision;
};

struct element
{
	//to hold either an operator or an operand
	union data_type
	{
		struct number operand;
		char operator;
	}type;
	
	//supporting data to determine the type of 'symbol' (i.e. either operator OR operand)
	int is_operator;
	//is_operator == -4 => '#' (symbol to indicate empty 'element' type variable)
	//is_operator == -3 => '$' (starting symbol stored in stack)
	//is_operator == -2 => ')'
	//is_operator == -1 => '('
	//is_operator == 0 => an operand
	//is_operator == 1 => an operator - + * / ^
};

struct stack
{
	struct element *element_list;
	int max_len, top;
};

struct defined_name
{
	char *name;
	int code;
};

int eval_expr(char*, struct number*);

int infix_to_postfix(char*, struct element*, int);
int postfix_eval(struct element*, int, struct number*);

int compare(char, char);
int check_safety(char*, int);
int is_valid_operator(char);
int calculate(char, struct number*, int, struct number*);
int obtain_ary(char);
int obtain_precedence(char);
int obtain_operator_prop(char, int);
int adjust_precision(float);
int lookup_constant(int, char*, int, struct number*);
int lookup_function(char*, int, struct number*);
int fetch_name_code(char*, int);
void enable_degree_mode();
int is_degree_mode_set();

int create_element_list(struct element**, int);
void display_elements(struct element*, int);
void free_element_list(struct element*);
int parse_element(char*, int, struct element*);

int create_stack(struct stack*);
void release_stack(struct stack*);
int push(struct stack*, struct element);
struct element pop(struct stack*);
