#include <stdio.h>
#include LIBDIR"std.h"
#include "calc.h"

B8 *_parser_id={"Calculator by Stephen Kittelson: Parser Build: 33: Last modification 2000-06-02"};

void calculator(char *parse,double *answer,struct CALC_RETURN *return_val)
{
/**************************************************************************
 *                                                                        *
 * ARGS:                                                                  *
 *  char *parse        -   equation to parse                              *
 *  double *answer     -   pointer to varible where the answer is inserted*
 *  RETURN *return_val - pointer to RETURN struct where return vals go    *
 *                                                                        *
 **************************************************************************/

	char    eq_ops[100]={0};
	double   eq_num[100]={0};
	char    temp_ops[100]={0};
	double   temp_num[100]={0};
	char    ops[100]={0};
	double   num[100]={0};
	char    str[100]={0};
	double   orig_num;
	int     cc;
	int     c3;
	int     place;
	int     c;
	int     pending;
	char    done;

	//error checker
	for(c=0,done=FALSE,pending=0;parse[c]!=0&&c<200;c++)
	{
		switch(parse[c])
		{
			case '0'://if its a number, dont worry about it
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '.':
				break;
			case '-'://after + or -, there should ONLY be numbers,open parenthesis,
			case '+'://if there is a newline or null, unexpected end returned
			{
				if
				(
					parse[c+1]<'0'&&parse[c+1]>'9'&&parse[c+1]!='.'&&parse[c+1]!='('&&
					parse[c+1]!='\n'&&parse[c+1]!=0
				)
				{
					return_val->invalid_char=parse[c+1];
					return_val->op=parse[c];
					return_val->error=AFTER;
					return;
				}
				else if(parse[c+1]=='\n'||parse[c+1]==0)
				{
					return_val->op=parse[c];
					return_val->error=UNEXPECTED_END;
					return;
				}
			}
			break;
			case '*'://after multiply and divide, ONLY numbers, open parenthesis,
			case '/'://and negative signs
			{
				if
				(
					parse[c+1]<'0'&&parse[c+1]>'9'&&parse[c+1]!='.'&&parse[c+1]!='('&&
					parse[c+1]!='-'&&parse[c+1]!='\n'&&parse[c+1]!=0
				)
				{
					return_val->invalid_char=parse[c+1];
					return_val->op=parse[c];
					return_val->error=AFTER;
					return;
				}
				else if(parse[c+1]=='\n'||parse[c+1]==0)
				{
					return_val->op=parse[c];
					return_val->error=UNEXPECTED_END;
					return;
				}
			}
			break;
			case '^'://after exponent, only numbers and open parenthesis
			{
				if
				(
					parse[c+1]<'0'&&parse[c+1]>'9'&&parse[c+1]!='('&&
					parse[c+1]!='\n'&&parse[c+1]!=0
				)
				{
					return_val->invalid_char=parse[c+1];
					return_val->op=parse[c];
					return_val->error=AFTER;
					return;
				}
				else if(parse[c+1]=='\n'||parse[c+1]==0)
				{
					return_val->op=parse[c];
					return_val->error=UNEXPECTED_END;
					return;
				}
			}
			break;
			case '('://after '(', ONLY numbers, negative signs, and more '('
			{
				pending++;//increment pending to make sure we have enough closing
				if              //parenthesis as there are opening
				(
					parse[c+1]<'0'&&parse[c+1]>'9'&&parse[c+1]!='.'&&
					parse[c+1]!='-'&&parse[c+1]!='\n'&&parse[c+1]!='('&&
					parse[c+1]!=0
				)
				{
					return_val->invalid_char=parse[c+1];
					return_val->op=parse[c];
					return_val->error=AFTER;
					return;
				}
				else if(parse[c+1]==')')//for some odd reason, the if above didn't
				{                               //catch this, so i added this else if
					return_val->invalid_char=parse[c+1];
					return_val->op=parse[c];
					return_val->error=AFTER;
					return;
				}
				else if//there has to be an operator before any '(' except if it
				(        //is the first char of the string
					parse[c-1]!='+'&&parse[c-1]!='-'&&parse[c-1]!='*'&&
					parse[c-1]!='/'&&parse[c-1]!='^'&&parse[c-1]!='('&&c>0
				)
				{
					return_val->invalid_char=parse[c-1];
					return_val->op=parse[c];
					return_val->error=BEFORE;
					return;
				}
				else if(parse[c+1]=='\n'||parse[c+1]==0)//unexpected end, '(' cannot
				{                                                    //end equations
					return_val->op=parse[c];
					return_val->error=UNEXPECTED_END;
					return;
				}
			}
			break;
			case ')'://after, only operators, more ')', and the end
			{
				pending--;//we have covered one '('
				if
				(
					parse[c+1]!='+'&&parse[c+1]!='-'&&parse[c+1]!='*'&&
					parse[c+1]!='/'&&parse[c+1]!='^'&&parse[c+1]!='\n'&&
					parse[c+1]!=')'&&parse[c+1]!=0
				)
				{
					return_val->invalid_char=parse[c+1];
					return_val->op=parse[c];
					return_val->error=AFTER;
					return;
				}
			}
			break;
			case '\n'://nuke this sucker
			{
				parse[c]=0;
			}
			break;
			case '\r':
			{
				parse[c]=0;
			}
			break;
			default://didn't fit anything else, must be invalid
			{
				return_val->invalid_char=parse[c];
				return_val->error=INVALID_CHAR;
				return;
			}
		}
	}
	if(c==200&&parse[c]!=0)//with 100 ops and 100 nums, the str should be
	{                                            //200 max chars, if someone passes that limit,
												 //increase all the sizes of the arrays and upgrade
												 //all the floats to a higher capacity data type
		return_val->error=STR_OVERFLOW;//too much equation for me to swallow :P
		return;
	}
	if(pending>0)//did we have more '(' than ')'?
	{
		return_val->error=MISSING_AFTER;
		return;
	}
	else if(pending<0)//did we have more ')' than '('?
	{
		return_val->error=MISSING_BEFORE;
		return;
	}
	//pull the operators and numbers out into eq_ops and eq_nums for easier
	//parsing and math
	for(c3=0,cc=0,c=0;parse[c]!=0;c++)
	{
		if((parse[c]>='0'&&parse[c]<='9')||parse[c]=='.'||parse[c]=='-')
		{
			if(parse[c]=='-')
			{
				if(cc!=0)
					goto chop1;//chop1 is on line 255
				eq_ops[c3]='*';
				eq_num[c3]=-1;
				c3++;
			}
			else
			{
				str[cc]=parse[c];//keep storing the number in str until there is no
				cc++;                        //more number
			}
		}
		else
		{
			chop1://goto on line 242
			str[cc]=0;
			sscanf(str,"%lf",&eq_num[c3]);//pull the complete number out of str
			str[0]=cc=0;
			if(parse[c]=='-')
			{
				eq_ops[c3]='+';//if there is a negative replace with '+ -1 * num'
				c3++;
				eq_num[c3]=-1;
				eq_ops[c3]='*';
				str[0]=cc=0;
				c3++;
				continue;
			}
			eq_ops[c3]=parse[c];//store the operators
			c3++;
		}
	}
	//pull the last number out
	str[cc]=0;
	sscanf(str,"%lf",&eq_num[c3]);
	eq_ops[c3]=0;
	str[0]=0;
	while(1)
	{
		//goto last '('
		place=0;
		for(c=0;eq_ops[c]!=0;c++)
		{
			if(eq_ops[c]=='(')
			{
				place=c+1;
			}
		}
		cc=place;
		//get the nums into num[] and ops into ops[]
		for(c=0;eq_ops[cc]!=')'&&eq_ops[cc]!=0;cc++,c++)
		{
			num[c]=eq_num[cc];
			ops[c]=eq_ops[cc];
		}
		num[c]=eq_num[cc];
		//order of operations
		while(1)
		{
			done=FALSE;
			for(c=0;ops[c]!=0;c++)
			{
				if(ops[c]=='^')
				{
	//----------------------------------------------------------------------
	//edit this code to include radicals
	//----------------------------------------------------------------------
					if(num[c+1]<1&&num[c+1]!=0)
					{
						*answer=num[c+1];
						return_val->error=INVALID_EXP;
						return;
					}
					orig_num=num[c];
					for(cc=1;cc<num[c+1];cc++)
					{
						num[c]*=orig_num;
					}
					if(num[c+1]==0)//if it was to the zero power
						num[c]=1;
	//----------------------------------------------------------------------
	//end of place to include radicals
	//----------------------------------------------------------------------
					//move all the values in num and ops back one to replace the
					//operation we just did
					for(cc=c+2;ops[cc-1]!=0;cc++)
					{
						num[cc-1]=num[cc];
						ops[cc-2]=ops[cc-1];
					}
					num[cc-1]=0;
					ops[cc-2]=0;
					done=TRUE;
					break;
				}
			}
			if(!done)//did we do ALL the ^'s in the whole equation we have?
				break;
		}
		while(1)
		{
			done=FALSE;
			for(c=0;ops[c]!=0;c++)
			{
				if(ops[c]=='*')
				{
					num[c]*=num[c+1];
					for(cc=c+2;ops[cc-1]!=0;cc++)
					{
						num[cc-1]=num[cc];
						ops[cc-2]=ops[cc-1];
					}
					num[cc-1]=0;
					ops[cc-2]=0;
					done=TRUE;
					break;
				}
				else if(ops[c]=='/')
				{
					if(num[c+1]==0)
					{
						return_val->error=DIV_ZERO;
						return;
					}
					num[c]/=num[c+1];
					for(cc=c+2;ops[cc-1]!=0;cc++)
					{
						num[cc-1]=num[cc];
						ops[cc-2]=ops[cc-1];
					}
					num[cc-1]=0;
					ops[cc-2]=0;
					done=TRUE;
					break;
				}
			}
			if(!done)
				break;
		}
		while(1)
		{
			done=FALSE;
			for(c=0;ops[c]!=0;c++)
			{
				if(ops[c]=='+')
				{
					num[c]+=num[c+1];
					for(cc=c+2;ops[cc-1]!=0;cc++)
					{
						num[cc-1]=num[cc];
						ops[cc-2]=ops[cc-1];
					}
					num[cc-1]=0;
					ops[cc-2]=0;
					done=TRUE;
					break;
				}
				else if(ops[c]=='-')
				{
					num[c]-=num[c+1];
					for(cc=c+2;ops[cc-1]!=0;cc++)
					{
						num[cc-1]=num[cc];
						ops[cc-2]=ops[cc-1];
					}
					num[cc-1]=0;
					ops[cc-2]=0;
					done=TRUE;
					break;
				}
			}
			if(!done)
				break;
		}
		temp_ops[0]=0;
		//goto last '('
		for(c=0;eq_ops[c]!=0;c++)
		{
			if(eq_ops[c]=='(')
			{
				place=c;
			}
		}
		//goto the next ')' after the last '('
		for(;eq_ops[place]!=')'&&eq_ops[place]!=0;place++);
		//if there was one, store all the ops and nums after ')'
		//so we can put them back in after we put the recently computed
		//answer in it's spot
		if(eq_ops[place]==')')
		{
			place++;
			c=0;
			if(eq_num[place]==-1)//if after ')', it is -1*blah, need to add
			{                                        //space for answer in temp and a '+'
				if(eq_ops[place]=='*')
				{
					c++;
					temp_ops[0]='+';
					temp_num[0]=0;
				}
			}
			for(cc=place;eq_ops[cc]!=0;c++,cc++)
			{
				temp_num[c]=eq_num[cc];
				temp_ops[c]=eq_ops[cc];
			}
			temp_num[c]=eq_num[cc];
			temp_ops[c]=0;
		}
		//find the last '(' again
		for(place=0,c=0;eq_ops[c]!=0;c++)
		{
			if(eq_ops[c]=='(')
			{
				place=c;
			}
		}
		//put all the nums and ops after the last '('s ')' into the main
		//eq_num and eq_ops
		for(c=0,cc=place;temp_ops[c]!=0;c++,cc++)
		{
			eq_num[cc]=temp_num[c];
			eq_ops[cc]=temp_ops[c];
		}
		if(cc!=0)
		{
			eq_num[cc]=temp_num[c];
			eq_ops[cc]=temp_ops[c];
		}
		else
		{
			eq_num[cc]=0;
			eq_ops[cc]=0;
		}
		//put the recently computed answer into it's spot
		eq_num[place]=num[0];

		done=TRUE;
		for(c=0;eq_ops[c]!=0;c++)//check to see if there are anymore operations
		{                                                //to perform...
			if
			(
				eq_ops[c]=='+'||
				eq_ops[c]=='/'||
				eq_ops[c]=='*'||
				eq_ops[c]=='^'||
				eq_ops[c]=='('||
				eq_ops[c]==')'
			)
			{
				done=FALSE;//we do, keep going
				break;
			}
			else if(eq_ops[c]=='-'&&c>0)
			{
				done=FALSE;//its not a negative sign on the answer, keep going
				break;
			}
		}
		if(done)//are we done?
			break;
	}
	*answer=num[0];//place final answer into pointer given to us
	return_val->error=NO_ERROR;//no errors if we got here
	return;
}
