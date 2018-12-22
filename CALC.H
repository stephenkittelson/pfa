//error values
#define NO_ERROR        0 // no error
#define DIV_ZERO        1 // division by zero
#define MISSING_AFTER   2 // missing ')'
#define MISSING_BEFORE  3 // missing '('
#define INVALID_CHAR    4 // invalid char
#define INVALID_EXP     5 // exponent is <1 and !=0
#define BEFORE          6 // error is before op
#define AFTER           7 // error is after op
#define UNEXPECTED_END  8 // equation unexpectedly ended
#define STR_OVERFLOW    9 // too much equation for me to chew :P

struct CALC_RETURN
{
	char invalid_char;
	char op;
	char error;
};

void calculator(char *parse,double *answer,struct CALC_RETURN *return_val);
