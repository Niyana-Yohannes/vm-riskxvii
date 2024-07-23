#ifndef HELPER
#define HELPER

#define MEMORY_SIZE 1024
#define REGS 32
#define BANK_SIZE 8192

#define LOADINST 1
#define STOREINST 2
#define LSBYTE 1
#define LSHALF 2
#define LSWORD 3

struct vm{
    //Virtual Machine Memory
    unsigned char memory[2][MEMORY_SIZE];
    unsigned char heap_bank[BANK_SIZE];
    uint32_t reg[REGS];
    int32_t pc;
    //Pointers to Virtual Machine Memory
    unsigned char (*memoryp)[MEMORY_SIZE];
    struct Node** headp;
};

struct Node;

int32_t sign_extend_nbits(uint32_t combined, int n);
void reg_dump(uint32_t *reg, int32_t pc);
void not_implemented(uint32_t instruction, uint32_t *reg, int32_t pc, struct Node** headp);
void invalid_operation(int32_t data_mem_address, int load_or_store, int load_store_mem, int b_h_w, uint32_t instruction, int32_t pc, uint32_t *reg, struct Node** headp);
int store_virtual_routine(int32_t data_mem_address, struct vm *vm, uint32_t value, uint32_t instruction);
int load_virtual_routine(int32_t data_mem_address, uint32_t *reg,  uint32_t rd);

#endif