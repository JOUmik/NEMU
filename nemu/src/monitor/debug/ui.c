#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <elf.h>
void cpu_exec(uint32_t);

void GetFunctionAddr(swaddr_t EIP,char* name);
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
        char *arg = strtok(NULL, " ");
        int num = 0;
        if(arg == NULL) num = 1;
        else num = atoi(arg);
        cpu_exec(num);
        return 0;
}

static int cmd_info(char *args){
        char *arg = strtok(NULL, " ");
        if(strcmp(arg, "r") == 0)
        {
                printf("eax is %x\n",cpu.eax);
                printf("ecx is %x\n",cpu.ecx);
                printf("ebx is %x\n",cpu.ebx);
                printf("edx is %x\n",cpu.edx);
                printf("esp is %x\n",cpu.esp);
                printf("edi is %x\n",cpu.edi);
                printf("esi is %x\n",cpu.esi); 
        }
        else if(strcmp(arg, "w") == 0)
        {
                info_wp();
        }
        else    printf("Input is wrong, try info r");
        return 0;
}

static int cmd_p(char *args) {
	uint32_t num;
	bool success;
	num = expr(args, &success); 
	if(success)
		printf("%d\n", num);
	return 0;
}

static int cmd_x(char *args) {
        if(args == NULL)   printf("Input is wrong, try like x 10 0x100000");
        else
        {
                int num = atoi(strtok(NULL, " "));
                
                char *arg = strtok(NULL, " ");
                //bool success = true;
                char *str;
                swaddr_t start_addr = strtol(arg, &str, 16);   
                int i; 
                for(i = 0; i < num; i++)
                {
                        //printf("0x%08x ", start_addr);
                        printf("0x%08x ", swaddr_read(start_addr, 4));
                        start_addr += 4;  
                        printf("\n");
                }     
        }
        return 0;
}

static int cmd_w(char *args) {
        if(args == NULL)   printf("Input is wrong, try like w 7-2");
        else {
                WP *new;
                bool success;
                new = new_wp();
                new->val = expr(args, &success);
                strcpy(new->expr, args);
                if(!success) assert(0);
                printf("Watchpoint %d: %s\n", new->NO, args);
                printf("Value : %d\n", new->val);
        }         
        return 0;
}

static int cmd_d(char *args) {
        if(args == NULL)   printf("Input is wrong, try like d 1");
        else {
                int num;
                sscanf(args, "%d", &num);
                delete_wp(num);
        }
        return 0;
}

typedef struct {
	swaddr_t prev_ebp;
	swaddr_t ret_addr;
	uint32_t args[4];
}PartOfStackFrame ;
static int cmd_bt(char* args){
	if (args != NULL){
		printf("Wrong Command!");
		return 0;
	}
	PartOfStackFrame EBP;
	char name[32];
	int cnt = 0;
	EBP.ret_addr = cpu.eip;
	swaddr_t addr = cpu.ebp;
	// printf("%d\n",addr);
	int i;
	while (addr){
		GetFunctionAddr(EBP.ret_addr,name);
		if (name[0] == '\0') break;
		printf("#%d\t0x%08x\t",cnt++,EBP.ret_addr);
		printf("%s",name);
		EBP.prev_ebp = swaddr_read(addr,4);
		EBP.ret_addr = swaddr_read(addr + 4, 4);
		printf("(");
		for (i = 0;i < 4;i ++){
			EBP.args[i] = swaddr_read(addr + 8 + i * 4, 4);
			printf("0x%x",EBP.args[i]);
			if (i == 3) printf(")\n");else printf(", ");
		}
		addr = EBP.prev_ebp;
	}
	return 0;
}


static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
        { "si", "To make N times implementations, if N is not given, it will make 1 time", cmd_si },
        {"info", "Print register state", cmd_info},
        {"x", "Scane memory", cmd_x},
        {"p", "Expression evaluation", cmd_p},
        {"w", "monitoring point", cmd_w},
        {"d", "delete monitoring point", cmd_d},
        { "bt", "Print stack frame chain", cmd_bt}
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
