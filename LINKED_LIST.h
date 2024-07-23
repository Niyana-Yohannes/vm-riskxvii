#ifndef LINKED_LIST
#define LINKED_LIST

struct Node {
	uint32_t start_address;
	uint32_t size_of_chunk;
	uint32_t num_of_banks;
	struct Node *next;
};

struct Node* list_init(uint32_t size_of_chunk, uint32_t num_of_banks);
uint32_t list_add(struct Node** head, uint32_t size_of_chunk, uint32_t num_of_banks);
void list_free(struct Node** head);
void mallocBank(struct Node** headp, uint32_t size, uint32_t *reg);
void freeBank(struct Node** headp, uint32_t free_address, unsigned char *heap_bank, uint32_t *reg, uint32_t pc, uint32_t instruction);
int heap_bank_access_check(struct Node** headp, int b_h_w, uint32_t data_mem_address);

#endif