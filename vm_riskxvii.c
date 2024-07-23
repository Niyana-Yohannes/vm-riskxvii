#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "LINKED_LIST.h"
#include "HELPER.h"

#define OPCODEMASK 0x7F 
#define RDMASK 0x1F 
#define FUNCT3MASK 0x7
#define FUNCT7MASK 0x7F
#define RS1MASK 0x1F
#define RS2MASK 0x1F

#define RTYPE 0b0110011
#define ITYPE 0b0010011
#define LOADTYPE 0b0000011
#define JALRTYPE 0b1100111
#define STYPE 0b0100011
#define SBTYPE 0b1100011
#define UTYPE 0b0110111
#define UJTYPE 0b1101111


struct vm;
struct decoded_inst{
    uint32_t instruction;
    uint32_t opcode; 
    uint32_t rd;
    uint32_t funct3;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t funct7;
    uint32_t unsigned_i_imm;
    uint32_t unsigned_s_imm;
    uint32_t unsigned_sb_imm;
    int32_t u_imm;
    uint32_t unsigned_uj_imm;
};

//R Type
void rtype(struct vm *vm, struct decoded_inst *decoded){
    switch(decoded->funct7){
        case 0b0: 
            switch(decoded->funct3){
                case 0b000:
                    //add
                    vm->reg[decoded->rd] = vm->reg[decoded->rs1] + vm->reg[decoded->rs2]; 
                    break;
                case 0b001:
                    //SLL
                    vm->reg[decoded->rd] = vm->reg[decoded->rs1] << vm->reg[decoded->rs2]; 
                    break;
                case 0b010:
                    //SLT
                    vm->reg[decoded->rd] = (vm->reg[decoded->rs1] < vm->reg[decoded->rs2]) ? 1 : 0;
                    break;
                case 0b011:
                    //SLTU
                    vm->reg[decoded->rd] = ((uint32_t)vm->reg[decoded->rs1] < (uint32_t)vm->reg[decoded->rs2]) ? 1 : 0;
                    break;
                case 0b100:
                    //XOR
                    vm->reg[decoded->rd] = vm->reg[decoded->rs1] ^ vm->reg[decoded->rs2];
                    break;
                case 0b101:
                    //SRL
                    vm->reg[decoded->rd] = vm->reg[decoded->rs1] >> vm->reg[decoded->rs2]; 
                    break;
                case 0b110:
                    //OR
                    vm->reg[decoded->rd] = vm->reg[decoded->rs1] | vm->reg[decoded->rs2];
                    break;
                case 0b111:
                    //AND
                    vm->reg[decoded->rd] = vm->reg[decoded->rs1] & vm->reg[decoded->rs2];
                    break;
                default:
                    //Wrong instruction
                    not_implemented(decoded->instruction, vm->reg, vm->pc, vm->headp);
            }
            break;
        case 0b0100000:  
            switch(decoded->funct3){
                case 0b000: 
                    //SUB
                    vm->reg[decoded->rd] = vm->reg[decoded->rs1] - vm->reg[decoded->rs2];
                    break;
                case 0b101:
                    //SRA
                    vm->reg[decoded->rd] = (vm->reg[decoded->rs1]>>vm->reg[decoded->rs2]) | (vm->reg[decoded->rs1] << (32-vm->reg[decoded->rs2]));
                    break;
                default:
                    //Wrong instruction
                    not_implemented(decoded->instruction, vm->reg, vm->pc, vm->headp);
            }
            break;
        default:
            //Wrong instruction
            not_implemented(decoded->instruction, vm->reg, vm->pc, vm->headp);
    }
}

//I Type
void itype(struct vm *vm, struct decoded_inst *decoded){
    int32_t imm = sign_extend_nbits(decoded->unsigned_i_imm,12); 
    switch(decoded->funct3){
        case 0b000:
            //addi
            vm->reg[decoded->rd] = vm->reg[decoded->rs1] + imm;
            break;
        case 0b010:
            //slti
            vm->reg[decoded->rd] = (vm->reg[decoded->rs1] < imm) ? 1 : 0;
            break;
        case 0b011:
            //sltiu
            vm->reg[decoded->rd] = ((uint32_t)vm->reg[decoded->rs1] < (uint32_t)imm) ? 1 : 0; //according to riscv specs
            break;
        case 0b100:
            //xori
            vm->reg[decoded->rd] = vm->reg[decoded->rs1] ^ imm;
            break;
        case 0b110:
            //ori
            vm->reg[decoded->rd] = vm->reg[decoded->rs1] | imm;
            break;
        case 0b111:
            //andi
            vm->reg[decoded->rd] = vm->reg[decoded->rs1] & imm;
            break;
        default:
            //Wrong instruction
            not_implemented(decoded->instruction, vm->reg, vm->pc, vm->headp);
    }
}

//Load I Type
void loadtype(struct vm *vm, struct decoded_inst *decoded){

    int32_t imm = sign_extend_nbits(decoded->unsigned_i_imm,12);
    int32_t data_mem_address = vm->reg[decoded->rs1] + imm;
    int load_val_in_reg = load_virtual_routine(data_mem_address, vm->reg, decoded->rd);
    uint32_t rd_value;

    switch(decoded->funct3){
        case 0b000:
            //lb
            invalid_operation(data_mem_address, LOADINST, load_val_in_reg, LSBYTE, decoded->instruction, vm->pc, vm->reg, vm->headp);
            if(load_val_in_reg){
                if(data_mem_address >= 0xB700)
                    rd_value = vm->heap_bank[data_mem_address - 0xb700];
                else
                    rd_value = *(*(vm->memoryp)+data_mem_address);
                vm->reg[decoded->rd] = sign_extend_nbits(rd_value, 8);
            }
            break;
        case 0b001:
            //lh
            invalid_operation(data_mem_address, LOADINST, load_val_in_reg, LSHALF, decoded->instruction, vm->pc, vm->reg, vm->headp);
            if(load_val_in_reg){
                if(data_mem_address >= 0xB700)
                    memcpy(&rd_value, &(vm->heap_bank[data_mem_address - 0xb700]), sizeof(uint16_t));
                else
                    memcpy(&rd_value, &(*(*(vm->memoryp)+data_mem_address)), sizeof(uint16_t)); //To load 16bits from mem which is 1byte
                vm->reg[decoded->rd] = sign_extend_nbits(rd_value, 16);
            }
            break;
        case 0b010:
            //lw
            invalid_operation(data_mem_address, LOADINST, load_val_in_reg, LSWORD, decoded->instruction, vm->pc, vm->reg, vm->headp); 
            if(load_val_in_reg){
                if(data_mem_address >= 0xB700)
                    memcpy(&rd_value, &(vm->heap_bank[data_mem_address - 0xb700]), sizeof(uint32_t));
                else
                    memcpy(&rd_value, &(*(*(vm->memoryp)+data_mem_address)), sizeof(uint32_t));
                vm->reg[decoded->rd] = rd_value; 
            }
            break;
        case 0b100:
            //lbu
            invalid_operation(data_mem_address, LOADINST, load_val_in_reg, LSBYTE, decoded->instruction, vm->pc, vm->reg, vm->headp);
            if(load_val_in_reg){
                if(data_mem_address >= 0xB700)
                    rd_value = (uint32_t)vm->heap_bank[data_mem_address - 0xb700];
                else
                    vm->reg[decoded->rd] = (uint32_t)(*(*(vm->memoryp)+data_mem_address));
            }
            break;
        case 0b101:
            //lhu
            invalid_operation(data_mem_address, LOADINST, load_val_in_reg, LSHALF, decoded->instruction, vm->pc, vm->reg, vm->headp);
            if(load_val_in_reg){
                if(data_mem_address >= 0xB700)
                    memcpy(&rd_value, &(vm->heap_bank[data_mem_address - 0xb700]), sizeof(uint16_t));
                else
                    memcpy(&rd_value, &(*(*(vm->memoryp)+data_mem_address)), sizeof(uint16_t));
                vm->reg[decoded->rd] = (uint32_t) rd_value; 
            }
            break;
        default:
            //Wrong instruction
            not_implemented(decoded->instruction, vm->reg, vm->pc, vm->headp);
    }
}

//JALR I Type
int32_t jalrtype(struct vm *vm, struct decoded_inst *decoded){
    int32_t imm = sign_extend_nbits(decoded->unsigned_i_imm,12);

    vm->reg[decoded->rd] = vm->pc + 4;
    int32_t ret_pc = vm->reg[decoded->rs1] + imm;

    return ret_pc;
}

//S Type
void stype(struct vm *vm, struct decoded_inst *decoded){

    int32_t imm = sign_extend_nbits(decoded->unsigned_s_imm,12);
    int32_t data_mem_address = vm->reg[decoded->rs1] + imm;
    int store_val_in_mem;

    switch(decoded->funct3){
        case 0b000: //sb
            store_val_in_mem = store_virtual_routine(data_mem_address, vm, vm->reg[decoded->rs2] & 0xFF, decoded->instruction);
            invalid_operation(data_mem_address, STOREINST, store_val_in_mem, LSBYTE, decoded->instruction, vm->pc, vm->reg, vm->headp);
            if(store_val_in_mem){
                if(data_mem_address >= 0xB700)
                    vm->heap_bank[data_mem_address - 0xb700] = vm->reg[decoded->rs2] & 0xFF;
                else
                    *(*(vm->memoryp)+data_mem_address) = vm->reg[decoded->rs2] & 0xFF;
            }
            break;
        case 0b001:
            //sh (needs to take up 2 indexes in data mem)
            store_val_in_mem = store_virtual_routine(data_mem_address, vm, vm->reg[decoded->rs2] & 0xFFFF, decoded->instruction);
            invalid_operation(data_mem_address, STOREINST, store_val_in_mem, LSHALF, decoded->instruction, vm->pc, vm->reg, vm->headp);
            if(store_val_in_mem){
                uint16_t store_16b_num = vm->reg[decoded->rs2] & 0xFFFF;
                if(data_mem_address >= 0xB700)
                    memcpy(&(vm->heap_bank[data_mem_address - 0xb700]), &store_16b_num, sizeof(uint16_t));
                else
                    memcpy(&(*(*(vm->memoryp)+data_mem_address)), &store_16b_num, sizeof(uint16_t));
            }
            break;
        case 0b010:
            //sw (needs to take up 4 indexes in data mem)
            store_val_in_mem = store_virtual_routine(data_mem_address, vm, vm->reg[decoded->rs2], decoded->instruction);
            invalid_operation(data_mem_address, STOREINST, store_val_in_mem, LSWORD, decoded->instruction, vm->pc, vm->reg, vm->headp);
            if(store_val_in_mem){
                uint32_t store_32b_num = vm->reg[decoded->rs2];
                if(data_mem_address >= 0xB700)
                    memcpy(&(vm->heap_bank[data_mem_address - 0xb700]), &store_32b_num, sizeof(uint32_t));
                else
                memcpy(&(*(*(vm->memoryp)+data_mem_address)), &store_32b_num, sizeof(uint32_t));
            }
            break;
        default:
            //Wrong instruction
            not_implemented(decoded->instruction, vm->reg, vm->pc, vm->headp);
    }
}

//SB Type
int32_t sbtype(struct vm *vm, struct decoded_inst *decoded){

    int32_t imm = sign_extend_nbits(decoded->unsigned_sb_imm, 12);
    int32_t ret_pc = vm->pc + 4;

    switch(decoded->funct3){
        case 0b000:
            //beq
            if(vm->reg[decoded->rs1] == vm->reg[decoded->rs2])
                ret_pc = vm->pc + (imm<<1); 
            break;
        case 0b001:
            //bne
            if(vm->reg[decoded->rs1] != vm->reg[decoded->rs2])
               ret_pc = vm->pc + (imm<<1);
            break;
        case 0b100:
            //blt
            if(vm->reg[decoded->rs1] < vm->reg[decoded->rs2])
                ret_pc = vm->pc + (imm<<1);
            break;
        case 0b101:
            //bge
            if(vm->reg[decoded->rs1] >= vm->reg[decoded->rs2])
                ret_pc = vm->pc + (imm<<1);
            break;
        case 0b110:
            //bltu
            if((uint32_t)vm->reg[decoded->rs1] < (uint32_t)vm->reg[decoded->rs2])
                ret_pc = vm->pc + (imm << 1);
            break;
        case 0b111:
            //bgeu
            if((uint32_t)vm->reg[decoded->rs1] >= (uint32_t)vm->reg[decoded->rs2])
                ret_pc = vm->pc + (imm << 1);
            break;
        default:
            //Wrong instruction
            not_implemented(decoded->instruction, vm->reg, vm->pc, vm->headp);
    }
    return ret_pc;
}

//UType
void utype(struct vm *vm, struct decoded_inst *decoded){
    vm->reg[decoded->rd] = decoded->u_imm;
}

//UJ Type
int32_t ujtype(struct vm *vm, struct decoded_inst *decoded){
    int32_t imm = sign_extend_nbits(decoded->unsigned_uj_imm,20);

    vm->reg[decoded->rd] = vm->pc+4;
    int32_t ret_pc = vm->pc + (imm<<1);

    return ret_pc;
}

int main(int argc, char** argv){
    
    //Virtual Memory Space
    unsigned char memory[2][MEMORY_SIZE];
    unsigned char heap_bank[BANK_SIZE];
    uint32_t reg[REGS] = {0};
    int32_t pc = 0;
    struct Node* head = list_init(0, 0); 

    //Storing binary file
    size_t bytes_read;
    FILE* fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }
    while ((bytes_read = fread(memory, 1, sizeof(memory),fp)) >  0);
    fclose(fp);

    //Initializing VM in Struct  
    struct vm virtual_machine;
    struct vm *vm = &virtual_machine;
    memcpy(vm->memory, memory, sizeof(memory));
    memcpy(vm->heap_bank, heap_bank, sizeof(heap_bank));
    memcpy(vm->reg, reg, sizeof(reg));
    vm->pc = pc;
    vm->memoryp = memory;
    vm->headp = &head;

    //Last instruction has to be at pc=1020
    while(vm->pc < 1021){ 

        //Decoding current istruction and placing in struct (will be destroyed)
        uint32_t current_inst = vm->memory[0][vm->pc+3]<<24 | vm->memory[0][vm->pc+2]<<16 | vm->memory[0][vm->pc+1]<<8 | vm->memory[0][vm->pc];
        uint32_t opcode = OPCODEMASK & current_inst;
        uint32_t rd = RDMASK & (current_inst>>7);
        uint32_t funct3 = FUNCT3MASK & (current_inst>>12);
        uint32_t rs1 = RS1MASK & (current_inst>>15); 
        uint32_t rs2 = RS2MASK & (current_inst>>20); 
        uint32_t funct7 = FUNCT7MASK & (current_inst>>25);
        uint32_t unsigned_i_imm = current_inst >> 20;
        uint32_t unsigned_s_imm = ((current_inst>>7) & 0x1F) | (((current_inst>>25) & 0x7F)<<5);
        uint32_t sb_imm_11 = current_inst>>7 & 1;
        uint32_t sb_imm_4_1 = current_inst>>8 & 0xF;
        uint32_t sb_imm_10_5 = current_inst>>25 & 0x3F; 
        uint32_t sb_imm_12 = current_inst>>31 & 1;
        uint32_t unsigned_sb_imm = sb_imm_4_1 | (sb_imm_10_5 << 4) | (sb_imm_11<<10) | (sb_imm_12<<11);
        uint32_t u_imm = current_inst & 0xFFFFF000; 
        uint32_t uj_imm_12_19 = current_inst>>12 & 0xFF;
        uint32_t uj_imm_11 = current_inst>>20 & 1;
        uint32_t uj_imm_10_1 = current_inst>>21 & 0x3FF;
        uint32_t uj_imm_20 = current_inst>>31 & 1;
        uint32_t unsigned_uj_imm = (uj_imm_10_1<<0) | (uj_imm_11<<10) | (uj_imm_12_19<<11) | (uj_imm_20 <<19);

        struct decoded_inst current_inst_struct = {current_inst, opcode, rd, funct3, rs1, rs2, funct7, unsigned_i_imm, unsigned_s_imm, unsigned_sb_imm, u_imm, unsigned_uj_imm};
        struct decoded_inst *decoded = &current_inst_struct;


        //Branching to function based of type of instruction
        switch(decoded->opcode){
            case RTYPE: 
                rtype(vm, decoded);
                vm->reg[0] = 0;
                vm->pc += 4;
                break;
            case ITYPE: 
                itype(vm, decoded);
                vm->reg[0] = 0;
                vm->pc += 4;
                break;
            case LOADTYPE: 
                loadtype(vm, decoded);
                vm->reg[0] = 0;
                vm->pc += 4;
                break;
            case JALRTYPE: 
                vm->pc = jalrtype(vm, decoded);
                vm->reg[0] = 0;
                break;
            case STYPE: 
                stype(vm, decoded);
                vm->reg[0] = 0;
                vm->pc += 4;
                break;
            case SBTYPE: 
                vm->pc = sbtype(vm, decoded);
                vm->reg[0] = 0;
                break;
            case UTYPE: 
                utype(vm, decoded);
                vm->reg[0] = 0;
                vm->pc += 4;
                break;
            case UJTYPE: 
                vm->pc = ujtype(vm, decoded);
                vm->reg[0] = 0;
                break;
            default:
                //Invalid Instruction
                not_implemented(decoded->instruction, vm->reg, vm->pc, vm->headp); 
        }
    }

    list_free(vm->headp);

    return 0;
}