
#include "lexical_analyzer.cpp"
int var_type;   //变量类型
int expr_type; //表达式类型
void deal_statement();
void deal_enmu_declaration();
void deal_function_declaration();
void deal_function_body();
void deal_expression(int level);
void deal_global_declaration()    //识别全局变量 最前置的词法解析（不支持#)  
{
    // printf("解析全局");
   int type;
   int i;
   var_type = INT;
   if(token==Enum)
   {
      token_match(Enum);  //枚举类型
   if(token=='{'){
       token_match('{');
       deal_enmu_declaration();
       token_match('}');
   }
   token_match(';');
   return;
   }

    if(token==Int){               //全局变量
        token_match(Int);
        // printf("declear int\n");
    }
    else if(token==Char){
        token_match(Char);
        var_type=CHAR;
    }

    while(token!=';'&&token!='}'){
        type=var_type;
        while(token==Mul){    //表示符号前面全部的* 地址  指针类型
            token_match(Mul);  
            type=type+PTR;
        }
        if(token!=Id){   //错误声明
            printf("%d:bad global declaration\n",line);
            execute_result+="%d:bad global declaration\n";
            exit(-1);
        }
        if(cur_id[Class]){  //重复声明
            printf("%d\n",token);
            execute_result+=std::to_string(token);

            printf("%d :duplicate global declaration\n",line);
            exit(-1);
        }
        token_match(Id);   //变量/函数 名称
       
        cur_id[Type]=type;
        
        if(token=='(')    //函数类型 
        {
            cur_id[Class] = Fun; 
            cur_id[Value]=(int)(text+1);
            deal_function_declaration();
        }
        else {
            // printf("before the id is %d\n",cur_id[Class]);
            cur_id[Class]=Glo;   //变量类型
            // printf("after -----the id is %d\n",cur_id[Class]);
            cur_id[Value]=(int)data;
            data=data+sizeof(int);
        
        }
        if(token==',')
        token_match(',');
        
    }
    lexical_analyzer();
}

void deal_enmu_declaration(){
   printf("解析枚举类型");
   int enmu_index=0;
   while(token!='}'){
       if(token!=Id){
           printf("%d:bad enum identifier %d\n",line,token);
           exit(-1);
       }
       lexical_analyzer();   //向内存中push 进这个id
       if(token==Assign){      //enmu初始化自己定义的下标
           lexical_analyzer();
           if(token!=Num){
           printf("%d:bad enum identifier %d\n",line,token);
           exit(-1);
           }
           enmu_index=token_val;   //初始化下标
          lexical_analyzer();
        }
        cur_id[Class]=Num;
        cur_id[Type]=INT;
        cur_id[Value]=enmu_index++;
        if(token==',')   
        lexical_analyzer();
   }
}
void deal_function_declaration(){
    // printf("a new function\n");
    token_match('(');


    //解析参数
    int type, parameter_index=0;
    while(token!=')'){         //每次while匹配一个参数
        // printf("push a new para");
        type=INT;
        if(token==Int)
        token_match(Int);
        else if(token==Char){
            type =CHAR;
            token_match(Char);
        }
        
        while(token==Mul){
            token_match(Mul);
            type=type+PTR;
        }
        if(token!=Id){
            printf("%d: bad parameter declaration\n", line);
            exit(-1);
        }
        if (cur_id[Class] == Loc) {  
            printf("%d: duplicate parameter declaration\n", line);
            exit(-1);
        }
       token_match(Id);
        //将全局变量临时保存到临时的BCLass中。再把这个变量初始化 
        cur_id[BClass] = cur_id[Class]; cur_id[Class]  = Loc;
        cur_id[BType]  = cur_id[Type];  cur_id[Type]   = type;
        
        cur_id[BValue] = cur_id[Value]; cur_id[Value]  = parameter_index++;   // i

        
        if(token==','){
            token_match(',');
        }
    }
    cur_bp=parameter_index+1; //ebp在最后一参数 下两个地址
    token_match(')');
    //参数解析结束

    token_match('{');
    //函数体解析

    // printf("function body\n");
      //局部变量定义 代码跟全局变量定义基本一致
    int p_localvar=cur_bp;
    while(token==Int||token==Char){   //变量定义
            var_type=(token == Int) ? INT : CHAR;
            token_match(token);
   
        while(token!=';'){  
            type=var_type;
            while(token==Mul){
                token_match(Mul);
                type=type+PTR;
            }
            if(token!=Id){   //错误声明
                printf("%d:bad local declaration\n",line);
                exit(-1);
            }
            if(cur_id[Class]==Loc){  //重复声明 
                
                printf("%d :duplicate local declaration\n",line);
                exit(-1);
            }
            token_match(Id);   
            
            cur_id[BClass] = cur_id[Class]; cur_id[Class]  = Loc;
            cur_id[BType]  = cur_id[Type];  cur_id[Type]   = type;
            cur_id[BValue] = cur_id[Value]; cur_id[Value]  = ++p_localvar;   // index of current parameter
            if (token == ',') {
                token_match(',');
            }
        }
        token_match(';');
    }

    *++text=ENT;
    *++text=p_localvar-cur_bp;
    
    while(token!='}'){
        deal_statement();
    }
    *++text=LEV;
    //把全局变量还原  这一段不加会导致两个函数的局部变量相串
    cur_id = symbols_tab;
    while (cur_id[Token]) {
        if (cur_id[Class] == Loc) {
            cur_id[Class] = cur_id[BClass];
            cur_id[Type]  = cur_id[BType];
            cur_id[Value] = cur_id[BValue];
        }
        cur_id = cur_id + IdSize;
    }
}



void deal_statement(){
    int *b,*a;
    if (token == If) {
       token_match(If);
       token_match('(');
       deal_expression(Assign);  // parse condition
       token_match(')');
        *++text = JZ;
        b = ++text;
        deal_statement();         // parsedeal_deal_statement
        if (token == Else) { // parse else
           token_match(Else);
            // emit code for JMP B
            *b = (int)(text + 3);
            *++text = JMP;
            b = ++text;
            deal_statement();
        }
        *b = (int)(text + 1);
    }

    else if (token == While) {
       token_match(While);
        a = text + 1;
       token_match('(');
       deal_expression(Assign);
       token_match(')');
        *++text = JZ;
        b = ++text;
        deal_statement();
        *++text = JMP;
        *++text = (int)a;
        *b = (int)(text + 1);
    }

    else if (token == Return) {
        // return [expression];
       token_match(Return);
        if (token != ';') {
           deal_expression(Assign);
        }
       token_match(';');
        // emit code for return
        *++text = LEV;
    }

    else if (token == '{') {
        // { <statement> ... }
       token_match('{');
        while (token != '}') {
            deal_statement();
        }
       token_match('}');
    }
    else if (token == ';') {
        // empty statement
       token_match(';');
    }
    else {
        // a = b; or function_call();
       deal_expression(Assign);
       token_match(';');
    }
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////





void deal_expression(int level) {

    // 1. unit_unary ::= unit | unit unary_op | unary_op unit
    // 2. expr ::= unit_unary (bin_op unit_unary ...)

    // unit_unary()
    int *id;
    int tmp;
    int *addr;
    {
        if (!token) {
            printf("%d: unexpected token EOF of expression\n", line);
            exit(-1);
        }
        if (token == Num) {
            token_match(Num);

            // emit code
            *++text = IMM;
            *++text = token_val;
            expr_type = INT;
        }
        else if (token == '"') {
            // continous string "abc" "abc"


            // emit code
            *++text = IMM;
            *++text = token_val;

            token_match('"');
            // store the rest strings
            while (token == '"') {
                token_match('"');
            }

            // append the end of string character '\0', all the data are default
            // to 0, so just move data one position forward.
            data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
            expr_type = PTR;
        }
        else if (token == Sizeof) {
            // sizeof is actually an unary operator
            // now only `sizeof(int)`, `sizeof(char)` and `sizeof(*...)` are
            // supported.
            token_match(Sizeof);
            token_match('(');
            expr_type = INT;

            if (token == Int) {
                token_match(Int);
            } else if (token == Char) {
                token_match(Char);
                expr_type = CHAR;
            }

            while (token == Mul) {
                token_match(Mul);
                expr_type = expr_type + PTR;
            }

            token_match(')');

            // emit code
            *++text = IMM;
            *++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);

            expr_type = INT;
        }
        else if (token == Id) {
            // there are several type when occurs to Id
            // but this is unit, so it can only be
            // 1. function call
            // 2. Enum variable
            // 3. global/local variable
            token_match(Id);

            id = cur_id;

            if (token == '(') {
                // function call
                token_match('(');

                // pass in arguments
                tmp = 0; // number of arguments
                while (token != ')') {
                    deal_expression(Assign);
                    *++text = PUSH;
                    tmp ++;

                    if (token == ',') {
                        token_match(',');
                    }

                }
                token_match(')');

                // emit code
                if (id[Class] == Sys) {
                    // system functions
                    *++text = id[Value];
                }
                else if (id[Class] == Fun) {
                    // function call
                    *++text = CALL;
                    *++text = id[Value];
                }
                else {
                    printf("%d: bad function call\n", line);
                    exit(-1);
                }

                // clean the stack for arguments
                if (tmp > 0) {
                    *++text = ADJ;
                    *++text = tmp;
                }
                expr_type = id[Type];
            }
            else if (id[Class] == Num) {
                // enum variable
                *++text = IMM;
                *++text = id[Value];
                expr_type = INT;
            }
            else {
                // variable
                if (id[Class] == Loc) {
                    *++text = LEA;
                    *++text = cur_bp - id[Value];
                }
                else if (id[Class] == Glo) {
                    *++text = IMM;
                    *++text = id[Value];
                }
                else {
                    printf("%d: undefined variable\n", line);
                    exit(-1);
                }

                // emit code, default behaviour is to load the value of the
                // address which is stored in `ax`
                expr_type = id[Type];
                *++text = (expr_type == Char) ? LC : LI;
            }
        }
        else if (token == '(') {
            // cast or parenthesis
            token_match('(');
            if (token == Int || token == Char) {
                tmp = (token == Char) ? CHAR : INT; // cast type
                token_match(token);
                while (token == Mul) {
                    token_match(Mul);
                    tmp = tmp + PTR;
                }

                token_match(')');

                deal_expression(Inc); // cast has precedence as Inc(++)

                expr_type  = tmp;
            } else {
                // normal parenthesis
                deal_expression(Assign);
                token_match(')');
            }
        }
        else if (token == Mul) {
            // dereference *<addr>
            token_match(Mul);
            deal_expression(Inc); // dereference has the same precedence as Inc(++)

            if (expr_type >= PTR) {
                expr_type = expr_type - PTR;
            } else {
                printf("%d: bad dereference\n", line);
                exit(-1);
            }

            *++text = (expr_type == CHAR) ? LC : LI;
        }
        else if (token == And) {
            // get the address of
            token_match(And);
            deal_expression(Inc); // get the address of
            if (*text == LC || *text == LI) {
                text --;
            } else {
                printf("%d: bad address of\n", line);
                exit(-1);
            }

            expr_type = expr_type + PTR;
        }
        else if (token == '!') {
            // not
            token_match('!');
            deal_expression(Inc);

            // emit code, use <expr> == 0
            *++text = PUSH;
            *++text = IMM;
            *++text = 0;
            *++text = EQ;

            expr_type = INT;
        }
        else if (token == '~') {
            // bitwise not
            token_match('~');
            deal_expression(Inc);

            // emit code, use <expr> XOR -1
            *++text = PUSH;
            *++text = IMM;
            *++text = -1;
            *++text = XOR;

            expr_type = INT;
        }
        else if (token == Add) {
            // +var, do nothing
            token_match(Add);
            deal_expression(Inc);

            expr_type = INT;
        }
        else if (token == Sub) {
            // -var
            token_match(Sub);

            if (token == Num) {
                *++text = IMM;
                *++text = -token_val;
                token_match(Num);
            } else {

                *++text = IMM;
                *++text = -1;
                *++text = PUSH;
                deal_expression(Inc);
                *++text = MUL;
            }

            expr_type = INT;
        }
        else if (token == Inc || token == Dec) {
            tmp = token;
            token_match(token);
            deal_expression(Inc);
            if (*text == LC) {
                *text = PUSH;  // to duplicate the address
                *++text = LC;
            } else if (*text == LI) {
                *text = PUSH;
                *++text = LI;
            } else {
                printf("%d: bad lvalue of pre-increment\n", line);
                exit(-1);
            }
            *++text = PUSH;
            *++text = IMM;
            *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
            *++text = (tmp == Inc) ? ADD : SUB;
            *++text = (expr_type == CHAR) ? SC : SI;
        }
        else {
            printf("%d: baddeal_expression\n", line);
            exit(-1);
        }
    }

    // binary operator and postfix operators.
    {
        while (token >= level) {
            // handle according to current operator's precedence
            tmp = expr_type;
            if (token == Assign) {
                // var = expr;
                token_match(Assign);
                if (*text == LC || *text == LI) {
                    *text = PUSH; // save the lvalue's pointer
                } else {
                    printf("%d: bad lvalue in assignment\n", line);
                    exit(-1);
                }
                deal_expression(Assign);

                expr_type = tmp;
                *++text = (expr_type == CHAR) ? SC : SI;
            }
            else if (token == Cond) {
                // expr ? a : b;
                token_match(Cond);
                *++text = JZ;
                addr = ++text;
                deal_expression(Assign);
                if (token == ':') {
                    token_match(':');
                } else {
                    printf("%d: missing colon in conditional\n", line);
                    exit(-1);
                }
                *addr = (int)(text + 3);
                *++text = JMP;
                addr = ++text;
                deal_expression(Cond);
                *addr = (int)(text + 1);
            }
            else if (token == Lor) {
                // logic or
                token_match(Lor);
                *++text = JNZ;
                addr = ++text;
                deal_expression(Lan);
                *addr = (int)(text + 1);
                expr_type = INT;
            }
            else if (token == Lan) {
                // logic and
                token_match(Lan);
                *++text = JZ;
                addr = ++text;
                deal_expression(Or);
                *addr = (int)(text + 1);
                expr_type = INT;
            }
            else if (token == Or) {
                // bitwise or
                token_match(Or);
                *++text = PUSH;
                deal_expression(Xor);
                *++text = OR;
                expr_type = INT;
            }
            else if (token == Xor) {
                // bitwise xor
                token_match(Xor);
                *++text = PUSH;
                deal_expression(And);
                *++text = XOR;
                expr_type = INT;
            }
            else if (token == And) {
                // bitwise and
                token_match(And);
                *++text = PUSH;
                deal_expression(Eq);
                *++text = AND;
                expr_type = INT;
            }
            else if (token == Eq) {
                // equal ==
                token_match(Eq);
                *++text = PUSH;
                deal_expression(Ne);
                *++text = EQ;
                expr_type = INT;
            }
            else if (token == Ne) {
                // not equal !=
                token_match(Ne);
                *++text = PUSH;
                deal_expression(Lt);
                *++text = NE;
                expr_type = INT;
            }
            else if (token == Lt) {
                // less than
                token_match(Lt);
                *++text = PUSH;
                deal_expression(Shl);
                *++text = LT;
                expr_type = INT;
            }
            else if (token == Gt) {
                // greater than
                token_match(Gt);
                *++text = PUSH;
                deal_expression(Shl);
                *++text = GT;
                expr_type = INT;
            }
            else if (token == Le) {
                // less than or equal to
                token_match(Le);
                *++text = PUSH;
                deal_expression(Shl);
                *++text = LE;
                expr_type = INT;
            }
            else if (token == Ge) {
                // greater than or equal to
                token_match(Ge);
                *++text = PUSH;
                deal_expression(Shl);
                *++text = GE;
                expr_type = INT;
            }
            else if (token == Shl) {
                // shift left
                token_match(Shl);
                *++text = PUSH;
                deal_expression(Add);
                *++text = SHL;
                expr_type = INT;
            }
            else if (token == Shr) {
                // shift right
                token_match(Shr);
                *++text = PUSH;
                deal_expression(Add);
                *++text = SHR;
                expr_type = INT;
            }
            else if (token == Add) {
                // add
                token_match(Add);
                *++text = PUSH;
                deal_expression(Mul);

                expr_type = tmp;
                if (expr_type > PTR) {
                    // pointer type, and not `char *`
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                *++text = ADD;
            }
            else if (token == Sub) {
                // sub
                token_match(Sub);
                *++text = PUSH;
                deal_expression(Mul);
                if (tmp > PTR && tmp == expr_type) {
                    // pointer subtraction
                    *++text = SUB;
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = DIV;
                    expr_type = INT;
                } else if (tmp > PTR) {
                    // pointer movement
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                    *++text = SUB;
                    expr_type = tmp;
                } else {
                    // numeral subtraction
                    *++text = SUB;
                    expr_type = tmp;
                }
            }
            else if (token == Mul) {
                // multiply
                token_match(Mul);
                *++text = PUSH;
                deal_expression(Inc);
                *++text = MUL;
                expr_type = tmp;
            }
            else if (token == Div) {
                // divide
                token_match(Div);
                *++text = PUSH;
                deal_expression(Inc);
                *++text = DIV;
                expr_type = tmp;
            }
            else if (token == Mod) {
                // Modulo
                token_match(Mod);
                *++text = PUSH;
                deal_expression(Inc);
                *++text = MOD;
                expr_type = tmp;
            }
            else if (token == Inc || token == Dec) {
                // postfix inc(++) and dec(--)
                // we will increase the value to the variable and decrease it
                // on `ax` to get its original value.
                if (*text == LI) {
                    *text = PUSH;
                    *++text = LI;
                }
                else if (*text == LC) {
                    *text = PUSH;
                    *++text = LC;
                }
                else {
                    printf("%d: bad value in increment\n", line);
                    exit(-1);
                }

                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? ADD : SUB;
                *++text = (expr_type == CHAR) ? SC : SI;
                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? SUB : ADD;
                token_match(token);
            }
            else if (token == Brak) {
                // array access var[xx]
                token_match(Brak);
                *++text = PUSH;
                deal_expression(Assign);
                token_match(']');

                if (tmp > PTR) {
                    // pointer, `not char *`
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                else if (tmp < PTR) {
                    printf("%d: pointer type expected\n", line);
                    exit(-1);
                }
                expr_type = tmp - PTR;
                *++text = ADD;
                *++text = (expr_type == CHAR) ? LC : LI;
            }
            else {
                printf("%d: compiler error, token = %d\n", line, token);
                exit(-1);
            }
        }
    }
}

