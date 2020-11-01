#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ = 257, UEQ = 258, AND = 259, OR = 260,
        NOT = 261, REGISTER = 262, NUMBER = 263, HEXNUM = 264, MARK = 265, NEG, POINTOR

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
        int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE, 1},				// spaces
	{"\\+", '+', 6},				// plus
        {"\\*", '*', 5},                                //multiplication
	{"==", EQ, 9},		   			// equal
        {"!=", UEQ, 9},                                 // not equal
        {"&&", AND, 13},                                // AND
        {"\\|\\|", OR, 14},                             // OR
        {"!", NOT, 2},                                  // NOT
        {"\\(", '(', 1},                                // (
        {"\\)", ')', 1},                                // )
        {"/", '/', 5},                                  // division
        {"-", '-', 6},                                  // subtraction
        {"\\b[0-9]+\\b", NUMBER, 1},                    // number
        {"\\b0[xX][0-9a-fA-F]+\\b", HEXNUM, 1},         // 16 number
        {"\\$[a-zA-Z]+", REGISTER, 1},                  // register
        {"\\b[a-zA-Z_0-9]+", MARK, 1}                   // MARK
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
        int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;
                                
                                memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
                                        case 43:
                                                tokens[nr_token].type = 43;
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 42:
                                                tokens[nr_token].type = 42;
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 257:
                                                tokens[nr_token].type = 257;
                                                strcpy(tokens[nr_token].str, "==");
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 258:
                                                tokens[nr_token].type = 258;
                                                strcpy(tokens[nr_token].str, "!=");
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 259:
                                                tokens[nr_token].type = 259;
                                                strcpy(tokens[nr_token].str, "&&");
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 260:
                                                tokens[nr_token].type = 260;
                                                strcpy(tokens[nr_token].str, "||");
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 261:
                                                tokens[nr_token].type = 261;
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 40:
                                                tokens[nr_token].type = 40;
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 41:
                                                tokens[nr_token].type = 41;
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 47:
                                                tokens[nr_token].type = 47;
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 45:
                                                tokens[nr_token].type = 45;
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 263:
                                                tokens[nr_token].type = 263;
                                                strncpy(tokens[nr_token].str, &e[position-substr_len],substr_len);
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 264:
                                                tokens[nr_token].type = 264;
                                                strncpy(tokens[nr_token].str, &e[position-substr_len],substr_len);
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 262:
                                                tokens[nr_token].type = 262;
                                                strncpy(tokens[nr_token].str, &e[position-substr_len],substr_len);
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
                                        case 265:
                                                tokens[nr_token].type = 265;
                                                strncpy(tokens[nr_token].str, &e[position-substr_len],substr_len);
                                                tokens[nr_token].priority = rules[i].priority;
                                                break;
					default: 
                                                nr_token--;
                                                break;
				}
                                nr_token++;
                                break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
        //nr_token--;
	return true; 
}

bool check_parentheses(int p, int q) {
        int i;
        if(tokens[p].type == '(' && tokens[q].type == ')') {
                int left = 0, right = 0;
                for(i = p + 1; i < q; i++) {
                        if(tokens[i].type == '(')    left++;
                        if(tokens[i].type == ')')    right++;
                        if(right > left)   return false;
                }
                if(left == right)   return true;                         
        }
        return false;
}

int dominant_operator(int p, int q) {
        int pos = p, lowwest_priority = 1;
        int i, j;
        for(i = p; i <= q; i++) {
                int cnt = 0;
                bool key = true;
                if(tokens[i].type == NUMBER || tokens[i].type == HEXNUM || tokens[i].type == REGISTER || tokens[i].type == MARK)    continue;
                /*if(tokens[i].type == '(') {
                        for(j = i; j <= q; j++) {
                                if(tokens[j].type == '(')    cnt++;
                                if(tokens[j].type == ')')    cnt--;
                                if(cnt == 0)  break;
                        }*/
                for(j = i - 1; j >= p; j--) {
                        if(tokens[j].type == '(' && !cnt) {
                                key = false;
                                break;
                        }
                        if(tokens[j].type == '(')  cnt--;
                        if(tokens[j].type == ')')  cnt++;
                }
                if(!key) continue;
                //printf("%d\n", j);
                //i = j;
                
                if(tokens[i].priority >= lowwest_priority) {
                        lowwest_priority = tokens[i].priority;
                        pos = i;
                }
        }
        //printf("%d\n", pos);
        return pos;
}

uint32_t eval(int p, int q) {
        if(p > q) {
                printf("Something bad has happened, your input must be wrong!\n");
                return 0; 
        }
        else if(p == q) {
                uint32_t num = 0;
                if(tokens[p].type == NUMBER)
                        sscanf(tokens[p].str, "%d", &num);
                if(tokens[p].type == HEXNUM)
                        sscanf(tokens[p].str, "%x", &num);
                if(tokens[p].type == REGISTER) {
                        /*int test = strlen(tokens[p].str);
                        printf("%d\n", test);*/
                        if(strlen(tokens[p].str) == 4) {
                                int i;
                                for( i = R_EAX; i <= R_EDI; i++) {
                                        char sign[5] = "$";
                                        if(strcmp (tokens[p].str, strcat(sign,regsl[i])) == 0)  break;
                                }        
                                if(i > R_EDI) {
                                        if(strcmp(tokens[p].str, "$eip") == 0)      num = cpu.eip;
                                        else Assert(p, "do not exist this register\n");
                                } 
                                else num = reg_l(i); 
                        }
                        else if(strlen(tokens[p].str) == 3) {
                                if(tokens[p].str[2] == 'x' || tokens[p].str[2] == 'p' ||tokens[p].str[2] == 'i') {
                                        int i;
                                        for(i = R_AX; i <= R_DI; i++) {
                                                char sign[5] = "$";
                                                if(strcmp(tokens[p].str,strcat(sign, regsw[i])) == 0)  break;
                                        }  
                                        num = reg_w(i);
                                }
                                else if(tokens[p].str[2] == 'l' || tokens[p].str[2] == 'h') {
                                        int i;
                                        for(i = R_AL; i <= R_BH; i++) {
                                                char sign[5] = "$";
                                                if(strcmp(tokens[p].str,strcat(sign, regsb[i])) == 0)  break;
                                        }
                                        num = reg_b(i);
                                }
                        }
                        else assert(1);
                }
                //printf("%d\n", num);
                return num;
        }
        else if(check_parentheses(p, q) == true) {
                return eval(p+1, q-1);
        }
        else {
                int pos = dominant_operator(p,q);
                if(p == pos || tokens[pos].type == NEG || tokens[pos].type == POINTOR || tokens[pos].type == '!') {
                        uint32_t val = eval(p + 1, q);
                        //printf("val = %d\n", val);
                        switch (tokens[pos].type) {
                                case NEG: return -val;
                                case POINTOR: return swaddr_read(val, 4);
                                case NOT: return !val;
                                default: assert(0);
                        }
                }

                uint32_t val1 = eval(p, pos - 1);
                uint32_t val2 = eval(pos + 1, q);

                switch(tokens[pos].type) {
                        case '+': return val1 + val2;
                        case '-': return val1 - val2;
                        case '*': return val1 * val2;
                        case '/': return val1 / val2;
                        case EQ: return val1 == val2;
                        case UEQ: return val1 != val2;
                        case AND: return val1 && val2;
                        case OR: return val1 || val2;
                        default: assert(0);
                }
              
        }
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	int i;
        for(i = 0; i < nr_token; i++) {
                if(tokens[i].type == '-' && (i==0 ||(tokens[i-1].type != NUMBER && tokens[i-1].type != HEXNUM && tokens[i-1].type != REGISTER && tokens[i-1].type != MARK && tokens[i-1].type != ')'))){
                        tokens[i].type = NEG;
                        tokens[i].priority = 2;
                }
                if(tokens[i].type == '*' && (i==0 ||(tokens[i-1].type != NUMBER && tokens[i-1].type != HEXNUM && tokens[i-1].type != REGISTER && tokens[i-1].type != MARK && tokens[i-1].type != ')'))){
                        tokens[i].type = POINTOR;
                        tokens[i].priority = 2;
                }
        }
        *success = true;
        //printf("nr_token = %d\n", nr_token);
	return eval(0, nr_token - 1);
}

