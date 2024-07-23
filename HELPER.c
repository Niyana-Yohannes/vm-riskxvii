#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "HELPER.h"
#include "LINKED_LIST.h"


int32_t sign_extend_nbits(uint32_t combined, int n){
    int32_t result;
    //Imm[11:0] I & S & SB [1:12] or [11:0]
    if(n==12){
        if(combined>>11 & 1) //12th bit is 1
            result = combined | 0xFFFFF000;
        else
            result = combined; //Leave upper 20bits 0;
    }
    //Imm[20:1] or [19:0] UJ 
    else if(n==20){
        if(combined>>18 & 1) //19th bit is 1
            result = combined | 0xFFF00000; 
        else
            result = combined; //Leave upper 20bits 0;
    }
    //LB
    else if(n==8){
        if(combined>>7 & 1) //8th bit is 1
            result = combined | 0xFFFFFF00; 
        else
            result = combined;
    }
    //LH
    else if(n==16){
        if(combined>>15 & 1) //16th bit is 1
            result = combined | 0xFFFF0000; 
        else
            result  = combined;
    }
    return result;
}

void reg_dump(uint32_t *reg, int32_t pc){
    printf("PC = 0x%08x;\n", pc);
    for(int i=0; i<32; ++i)
        printf("R[%d] = 0x%08x;\n", i, reg[i]);
} 

void not_implemented(uint32_t instruction, uint32_t *reg, int32_t pc, struct Node** headp){
    printf("Instruction Not Implemented: 0x%08x\n", instruction);
    reg_dump(reg, pc);
    list_free(headp);
    exit(0);
}

void invalid_operation(int32_t data_mem_address, int load_or_store, int load_store_mem, int b_h_w, uint32_t instruction, int32_t pc, uint32_t *reg, struct Node** headp){
    int illegal_operation = 0;
    if(load_or_store == LOADINST){ //load instruction
        //Trying to load from virtual routine memory but not virtual routine
        if ((data_mem_address> 0x7FF) && (data_mem_address<0xB700) && load_store_mem) { 
            illegal_operation = 1;
        }
        //Trying to load from valid address but not enough bytes
        else if (data_mem_address >= 0x000 && data_mem_address <= 0x7FF){
            uint32_t mem_space = 0x0800 - data_mem_address;
            if((b_h_w == LSBYTE && mem_space < 1) || (b_h_w == LSHALF && mem_space < 2) || (b_h_w == LSWORD && mem_space < 4))
                illegal_operation = 1;
        }
        //Check if heap exists and enough in chunk
        else if(data_mem_address> 0x8FF){
            int illegal = heap_bank_access_check(headp, b_h_w, data_mem_address);
            if(illegal){
                illegal_operation = 1;
            }
        }
    }
    else if(load_or_store == STOREINST){ //Store instruction
        //Trying to store to virtual routine memory but not virtual routine
        if ((data_mem_address < 0x0400) || (((data_mem_address > 0x7ff) && (data_mem_address < 0xB700)) == load_store_mem)) {
            illegal_operation = 1;
        }
        //Trying to store into valid address but not enough bytes
        else if (data_mem_address >= 0x0400 && data_mem_address <= 0x7FF){
            uint32_t mem_space = 0x0800 - data_mem_address;
            if((b_h_w == LSBYTE && mem_space < 1) || (b_h_w == LSHALF && mem_space < 2) || (b_h_w == LSWORD && mem_space < 4))
                illegal_operation = 1;
        }
        //Check if heap exists and enough in chunk
        else if(data_mem_address > 0x8FF){
            int illegal = heap_bank_access_check(headp, b_h_w, data_mem_address);
            if(illegal){
                illegal_operation = 1;
            }
        }
    }
    if(illegal_operation){
        printf("Illegal Operation: 0x%08x\n", instruction);
        reg_dump(reg, pc);
        list_free(headp);
        exit(0);
    }
}

int store_virtual_routine(int32_t data_mem_address, struct vm *vm, uint32_t value, uint32_t instruction){
    if(data_mem_address == 0x0800) { //Console Write Charater
        printf("%c", value);
        return 0;
    }
    else if(data_mem_address == 0x0804) { //Console Write Signed Integer
        printf("%d", (int32_t)value);
        return 0;
    }
    else if(data_mem_address == 0x0808) { //Console Write Unsigned Integer
        printf("%x", value);
        return 0;
    }
    else if(data_mem_address == 0x080C) { //Halt
        printf("CPU Halt Requested\n");
        exit(0);
    }
    else if(data_mem_address == 0x0820) { //Dump PC
        printf("%x\n", vm->pc);
        return 0;
    }
    else if(data_mem_address == 0x0824){ //Dump Reg Banks
        reg_dump(vm->reg, vm->pc);
        return 0;
    }
    else if(data_mem_address == 0x0828){ //Dump Mem Word
        invalid_operation(value, LOADINST, 1, LSBYTE, instruction, vm->pc, vm->reg, vm->headp);
        printf("%x\n", *(*(vm->memoryp)+value));
        return 0;
    }
    else if(data_mem_address == 0x0830){ //Malloc
        mallocBank(vm->headp, value, vm->reg);
        return 0;
    }
    else if(data_mem_address == 0x0834) { //Free
        freeBank(vm->headp, value, vm->heap_bank, vm->reg, vm->pc, instruction);
        return 0;
    }
    else{
        return 1;
    }
}

int load_virtual_routine(int32_t data_mem_address, uint32_t *reg,  uint32_t rd){
    int32_t load_value;
    if(data_mem_address== 0x0812){ //Console Read Character
        scanf("%lc",&load_value);
        reg[rd] = load_value;
        return 0;
    }
    else if(data_mem_address== 0x0816){ //Console Read Signed Integer
        scanf("%d", &load_value);
        reg[rd] = load_value;
        return 0;
    }
    else
        return 1;
}