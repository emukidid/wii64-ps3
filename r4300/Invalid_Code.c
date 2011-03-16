/* Invalid_Code.c - Uses 1/8th the memory as the char hash table
   by Mike Slegeir for Mupen64-GC / MEM2 ver by emu_kidid
 */

#include "Invalid_Code.h"

static unsigned char invalid_code[0x100000];

int inline invalid_code_get(int block_num){
	return invalid_code[block_num];
}

void inline invalid_code_set(int block_num, int value){
	invalid_code[block_num] = value;
}

