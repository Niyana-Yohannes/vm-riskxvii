#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "LINKED_LIST.h"
#include "HELPER.h"

struct Node* list_init(uint32_t size_of_chunk, uint32_t num_of_banks){
	struct Node *head = (struct Node*)malloc(sizeof(struct Node));
	head->start_address = 0xB700;
	head->size_of_chunk = size_of_chunk;
	head->num_of_banks = num_of_banks;
	head->next = NULL;
	return head;
}

uint32_t list_add(struct Node** headp, uint32_t size_of_chunk, uint32_t num_of_banks){

	int total_blocks = 0;
	//Get head node
	struct Node* currentNode = *headp;
	total_blocks += currentNode->num_of_banks;
	//Traverse through linked list
	while(currentNode -> next != NULL){
		currentNode = currentNode -> next;
		total_blocks += currentNode->num_of_banks;
	}

	//If there is more heap banks available
	if((total_blocks + num_of_banks) <= 128){	
		//Check if can add to end
		uint32_t new_bank_start_address = currentNode->start_address + 64*currentNode->num_of_banks;
		uint32_t new_bank_end_adress = new_bank_start_address + 64*num_of_banks - 1; 
		if(new_bank_start_address <= 0xD6C0 && new_bank_end_adress <= 0xD6FF){
			//Creating the new node
			struct Node* addNode = (struct Node*)malloc(sizeof(struct Node));
			//Link new node to end node
			currentNode->next = addNode;
			//Setting new node value and next
			addNode->start_address = new_bank_start_address;
			addNode-> size_of_chunk = size_of_chunk;
			addNode->num_of_banks = num_of_banks;
			addNode->next = NULL;
			return addNode->start_address;
		}
		//Check if can add to front or any space available between nodes
		else{
			currentNode = *headp;
			struct Node* temp = NULL;
			//Add to front
			if(currentNode->start_address != 0xB700){
				if(currentNode->start_address - 0xB700 >= 64*num_of_banks){
					//Creating the new node
					struct Node* addNode = (struct Node*)malloc(sizeof(struct Node));
					temp = currentNode;
					*headp = addNode;
					addNode->start_address = 0xB700;
					addNode->size_of_chunk = size_of_chunk;
					addNode->num_of_banks = num_of_banks;
					addNode->next = temp;
					return addNode->start_address;
				}
			}
			//Add in between
			while(currentNode->next != NULL){
				if((currentNode->next->start_address - (currentNode->start_address+64*currentNode->num_of_banks) >= 64*num_of_banks)){
					//Creating the new node
					struct Node* addNode = (struct Node*)malloc(sizeof(struct Node));
					//Link new node to end node
					temp = currentNode->next;
					currentNode->next = addNode;
					//Setting new node value and next
					addNode->start_address = currentNode->start_address + 64*currentNode->num_of_banks;
					addNode-> size_of_chunk = size_of_chunk;
					addNode->num_of_banks = num_of_banks;
					addNode->next = temp;
					return addNode->start_address;
				}
				currentNode = currentNode-> next;
			}
		}
		//If reached here, not possible
		return 0;
	}
	//If reached here, not possible
	return 0;
}

void list_free(struct Node** headp){
	struct Node *currentNode = *headp;
	struct Node *next;
	//Traverse through linked list
	while(currentNode != NULL){
		next = currentNode-> next;
		free(currentNode);
		currentNode = next;
	}
	*headp = NULL;
}

void mallocBank(struct Node** headp, uint32_t size, uint32_t *reg){
    int banks = 0;
    struct Node* head = *headp;
    if((size % 64) == 0){
        banks = size / 64;
    }
    else if((size % 64) != 0){
        banks = (size / 64) + 1;
    }
    if(size == 0)
        reg[28] = 0; 
    else if(banks > 128)
    	reg[28] = 0;
    else if(head->size_of_chunk == 0 && head->num_of_banks == 0){
        head->size_of_chunk = size;
        head->num_of_banks = banks;
        reg[28] = 0xb700;
    }
    else
        reg[28] = list_add(headp, size, banks);
}

void freeBank(struct Node** headp, uint32_t free_address, unsigned char *heap_bank, uint32_t *reg, uint32_t pc, uint32_t instruction){
    struct Node* currentNode = *headp;
    struct Node* prev = NULL;

    while(currentNode != NULL && currentNode->start_address != free_address){
        prev = currentNode;
        currentNode = currentNode->next;
    }
    //Freeing the head node 
    if(currentNode == *headp){
        for(int i=0; i<currentNode->size_of_chunk; ++i){
            heap_bank[currentNode->start_address - 0xb700 + i] = 0;
        }
        //If only head node, set to 0   
        if(currentNode->next == NULL){
        	currentNode->num_of_banks = 0;
        	currentNode->size_of_chunk = 0;
        }
        else{
        	*headp = currentNode->next;
        	free(currentNode);
        }
    }
    else if(currentNode != NULL){
        prev->next = currentNode->next;
        for(int i=0; i<currentNode->size_of_chunk; ++i){
            heap_bank[currentNode->start_address - 0xb700 + i] = 0;
        }
        free(currentNode);
        
    }
    else{
    //If it reaches here, address not allocated
        printf("Invalid Operation");
        printf("Illegal Operation: %x\n", instruction);
        reg_dump(reg, pc);
        list_free(headp);
        exit(0);
    }
}

int heap_bank_access_check(struct Node** headp, int b_h_w, uint32_t data_mem_address){
    struct Node* currentNode = *headp;
    while(currentNode != NULL){
        //If data_mem_address is an address between start and end of bank should be good
        if(data_mem_address >= currentNode->start_address && data_mem_address < currentNode->start_address + currentNode->size_of_chunk){
            //From where address starts, check if enough to load/store
            uint32_t mem_space = currentNode->start_address+currentNode->size_of_chunk - data_mem_address;
            if((b_h_w == LSBYTE && mem_space >= 1) || (b_h_w == LSHALF && mem_space >= 2) || (b_h_w == LSWORD && mem_space >= 4)){
                return 0;
            }
        }
        currentNode = currentNode->next;
    }
    return 1;
}


