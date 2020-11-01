#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
bool su = true;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp() {
        if(su == true) {
                su = false;
                init_wp_pool();
                //printf("have initialled\n");
        }
        if(free_ == NULL) {
                printf("The free is empty!");
                assert(0);
        }
        WP *new, *tempt;
        new = free_;
        free_ = free_->next;
        new->next = NULL;
        tempt = head;
        if( tempt == NULL) {
                head = new;
                tempt = head;
        }
        else {
                while(tempt->next != NULL) {
                        tempt = tempt->next;
                        //printf("1\n");
                }
                tempt -> next = new;
        }
        return new;
}

void free_wp(WP *wp) {
        WP *free, *tempt;
        free = free_;
        if(free ==NULL) {
                free_ = wp;
                free = free_;
        }
        else {
                while(free->next != NULL)   free = free->next;
                free -> next = wp;
        }  
        tempt = head;
        if (head == NULL)  assert(0);
        else if (head->NO == wp->NO)  {
                head = head->next;
                wp->next = NULL;
                return;
        }
        else {
                while( tempt->next != NULL && tempt->next->NO != wp->NO)  tempt = tempt->next;
                if(tempt->next->NO == wp->NO) {
                        tempt->next = tempt->next->next;
                        wp->next = NULL;
                        return;
                }
                else assert(0);
        }
}

/*bool check_wp() {
        WP *tempt;
        tempt = head;
        bool key = true;
        bool success;
        while(tempt != NULL) {
                uint32_t tmp_expr = expr
        }
}*/

void delete_wp(int num) {
        WP *new;
        new = &wp_pool[num];
        free_wp(new);
}

void info_wp() {
        WP *new;
        new = head;
        while(new!=NULL) {
                printf("Watchpoint %d: %s = %d\n", new->NO, new->expr, new->val);
                new = new->next;
        }
}
