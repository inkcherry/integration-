// #include<stdio.h>
int token;          		//当前的记号
char *src,*old_src; 		//text的指针 token的下一个字符
int poolsize;                   //虚拟机内存开辟空间size的参数
int line;                       //行号
int *text,                 //代码段
 	*old_text,             
 	*stack;                //栈
char *data;                //数据段（只用与存放字符串）
int *pc,*bp,*sp,ax,cycle;  //虚拟机中的寄存器，类似于计算机中的汇编意义
int *symbols_tab,*cur_id;
int token_val;
int *idmain;  
int cur_bp;   //函数堆栈帧指针
int *watch_hash_change,watch_hash_flag;

enum {CHAR, INT, PTR };      // types of variable/function
                // the `main` function


//pc  存放下一条计算指令
//sp  指向栈顶 栈是从高地址到低地址增长 入栈是sp减小
//bp  指向栈的某些特定位置
//ax  通用寄存器 用于存放指令结果
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
enum{LEA,IMM,JMP,CALL,JZ,JNZ,ENT,ADJ,LEV,LI,LC,SI,SC,PUSH,
OR,XOR,AND,EQ,NE,LT,GT,LE,GE,SHL,SHR,ADD,SUB,MUL,DIV,MOD,
OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT};
//指令集 在虚拟机中相当与计算机汇编的指令 意义与汇编相同


enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize}; //symbols_tab




void  lexical_analyzer (){                    //词法分析(字符转token)
	char *st_pos,*en_pos;
	int hash;
		while(token=*src)
		{    
			// printf("[%d] ",token);
			if(token==209)
			printf("%s",src);
			// printf("token is %d %c \n ",token,*src);
			src++;
	
			if(token=='\n')  //换行;
			{
			   ++line;
			}
	
	
			else if(token =='#') //跳过
			{
					while (*src != 0 && *src != '\n') 
					src++;
			}    
	
	
			else if((token>='a'&&token<='z')||(token>='A'&&token<='Z')||token=='_') //标识符开头;
			{  
			   st_pos=src-1;
			   
			   hash=token;
			  
		
			   while((*src>='a'&&*src<='z')||(*src>='A'&&*src<='Z')||*src=='_'||(*src>= '0' && *src<= '9'))
					{hash=hash*147+*src;//hash映射
						// printf("||||||||||||||||||||||||||||||||||||||\n");
					 src++; 
					}
			   en_pos=src-1;
	
			   cur_id=symbols_tab;
			           
				while (cur_id[Token]) {  //遍历符号表
				//test symbol table
				// if(token=='a')
				// {
				// 	printf("symbol  %s | hash  %d | token  %d\n",cur_id[Name],cur_id[Hash],cur_id[Token]);						

				// }

					if (cur_id[Hash] == hash && !memcmp((char*)cur_id[Name], st_pos, en_pos-st_pos+1)) {
						token = cur_id[Token];  //如果是已经存在的字符
						return;
					}
					else cur_id=cur_id+IdSize;   //++
				}
				//如果没有return（跳出） ，下面这段相当于push
				// printf("push new  %s\n\n",st_pos);	
				    
			
				cur_id[Name]=(int)st_pos;
				cur_id[Hash]=hash;
				//test hash change
				// if(token=='a')
				// {// {   watch_hash_change=&cur_id[hash];
				// 	printf("%s %d $|||||||||||||||",cur_id[Name],cur_id[Hash]);
				// 	watch_hash_change=&cur_id[Hash];
				// 	// watch_hash_flag=cur_id[Hash];
				// 	// cur_id[Hash]=2;
				// 	// watch_hash_flag++;
				// }	
				token=cur_id[Token]=Id;

				return;
			}
	
	
			else if (token >= '0' && token <= '9') {   //数字
				// parse number, three kinds: dec(123) hex(0x123) oct(017)
				token_val = token - '0';
				if (token_val > 0) {                    //10 default
					// dec, starts with [1-9]
					while (*src >= '0' && *src <= '9') {
						token_val = token_val*10 + *src++ - '0';
					}
				} 
	
				else  if (*src == 'x' || *src == 'X')   //16
				{
					//hex
					token = *++src;
					while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
						token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
						token = *++src;
					}
				}
	
				else if (*src == 'o' || *src == 'O')
				{                                //8
					// oct
					while (*src >= '0' && *src <= '7') {
						token_val = token_val*8 + *src++ - '0';
					}
				}
			
				token = Num;
				return;
			}
	
			else if (token == '"' || token == '\'') {
				// parse string literal, currently, the only supported escape
				// character is '\n', store the string literal into data.
				st_pos = data;                      //用data存放字符串
				while (*src != 0 && *src != token) {
					token_val = *src++;
					if (token_val == '\\') {
						// escape character
						token_val = *src++;
						if (token_val == 'n') {
							token_val = '\n';
						}
					}
					if (token == '"') {
						*data++ = token_val;
					}
				}
				src++;
				// if it is a single character, return Num token
				if (token == '"') {
					token_val = (int)st_pos;
				} else {
					token = Num;
				}
				return;
			}
	
			else if (token == '/') {               //注释 仅仅//类型
			if (*src == '/') {
				// skip comments
				while (*src != 0 && *src != '\n') {
					++src;
				}
			} else {
				// divide operator
				token = Div;
				return;
			}
			}
	
	
			else if (token == '=') {
				// parse '==' and '='
				if (*src == '=') {
					src ++;
					token = Eq;
				} else {
					token = Assign;
				}
				return;
			}
			else if (token == '+') {
				// parse '+' and '++'
				if (*src == '+') {
					src ++;
					token = Inc;
				} else {
					token = Add;
				}
				return;
			}
			else if (token == '-') {
				// parse '-' and '--'
				if (*src == '-') {
					src ++;
					token = Dec;
				} else {
					token = Sub;
				}
				return;
			}
			else if (token == '!') {
				// parse '!='
				if (*src == '=') {
					src++;
					token = Ne;
				}
				return;
			}
			else if (token == '<') {
				// parse '<=', '<<' or '<'
				if (*src == '=') {
					src ++;
					token = Le;
				} else if (*src == '<') {
					src ++;
					token = Shl;
				} else {
					token = Lt;
				}
				return;
			}
			else if (token == '>') {
				// parse '>=', '>>' or '>'
				if (*src == '=') {
					src ++;
					token = Ge;
				} else if (*src == '>') {
					src ++;
					token = Shr;
				} else {
					token = Gt;
				}
				return;
			}
			else if (token == '|') {
				// parse '|' or '||'
				if (*src == '|') {
					src ++;
					token = Lor;
				} else {
					token = Or;
				}
				return;
			}
			else if (token == '&') {
				// parse '&' and '&&'
				if (*src == '&') {
					src ++;
					token = Lan;
				} else {
					token = And;
				}
				return;
			}
			else if (token == '^') {
				token = Xor;
				return;
			}
			else if (token == '%') {
				token = Mod;
				return;
			}
			else if (token == '*') {
				token = Mul;
				return;
			}
			else if (token == '[') {
				token = Brak;
				return;
			}
			else if (token == '?') {
				token = Cond;
				return;
			}
			else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
				// directly return the character as token;
				return;
			}
	
	
		}
	return;
	}
void token_match(int tk) {     //符号检测 检测当前
	if (token == tk) {
		lexical_analyzer();
	} else {
		printf("%d: expected token: %d\n", line, tk);
		exit(-1);
	}
}