
#include "aco.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "aco_assert_override.h"

int array[1000];

int next_val;

aco_t* main_co;

aco_share_stack_t* sstk2;


void co_fp0()
{
    

    aco_t* this_co = aco_get_co();
    


    
    printf("Found a prime: %d\r\n", (*((int*)(aco_get_arg()))));
    
    int the_arg =  *((int *)aco_get_arg());
    
    
    for (int i = 0; i < 1000; i++) {
        if (!(i % the_arg)) {
            array[i] = 0;
        }
    }
    
    
    int* next_val_ptr = (int *)aco_get_arg();
    next_val = *next_val_ptr;
    
    while (next_val < 1000) {
        next_val++;
        if (array[next_val] == 1) {
            aco_t* new_co = aco_create(this_co, sstk2, 0, co_fp0, &next_val);
            aco_resume(new_co);
            printf("resumed\r\n");
            break;
        }
    }
    
    
    
    
    aco_exit();
    
    printf("exit?\r\n");
    //assert(0);
}

int main() {
#ifdef ACO_USE_VALGRIND
    if(1){
        printf("%s doesn't have valgrind test yet, "
               "so bypass this test right now.\n",__FILE__
               );
        exit(0);
    }
#endif
    
    aco_thread_init(NULL);
    
    main_co = aco_create(NULL, NULL, 0, NULL, NULL);
    assertptr(main_co);
    
    sstk2 = aco_share_stack_new(0);
    
    aco_share_stack_t* sstk = aco_share_stack_new(0);
    assertptr(sstk);
    assertptr(sstk2);
    
    
    for (int i = 0; i < 1000; i++) {
        
        if (i == 0 || i == 1) {
            array[i] = 0;
        }
        
        
        array[i] = 1;
    }
    
    int co_ct_arg_point_to_me = 2;
    aco_t* co2 = aco_create(main_co, sstk2, 0, co_fp0, &co_ct_arg_point_to_me);
    assertptr(co2);
    
   
    
        
        assert(co2->is_end == 0);
        aco_resume(co2);
        
    
        
        printf("main_co:%p\n", main_co);
    
 
    
   
    
    
    aco_destroy(co2);
    co2 = NULL;
    
    aco_share_stack_destroy(sstk);
    sstk = NULL;
    //aco_share_stack_destroy(sstk2);
    //sstk2 = NULL;
    
    aco_destroy(main_co);
    main_co = NULL;
    
    return 0;
}

