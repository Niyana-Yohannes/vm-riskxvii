inline int scan_char() {
    int result;
    int addr = 0x0816;
    
    asm volatile("lw %[res], 0(%[adr])": [res]"=r"(result): [adr]"r"(addr));

    return result;
}

int volatile *const ConsoleReadSInt = (int *)0x0816;
int volatile *const ConsoleWriteSInt = (int *)0x0804;
int volatile *const ConsoleWriteHexSInt = (int *)0x0808;
int main() {

    int a = scan_char();
    int imm = 5;

    //addi
    int result;
    asm volatile("addi %[res], %[a], %[imm]" : [res]"=r"(result) : [a]"r"(a), [imm]"i"(imm));
    *ConsoleWriteSInt = result;

    //xori
    int result2;
    asm volatile("xori %[res], %[a], %[imm]" : [res]"=r"(result2) : [a]"r"(a), [imm]"i"(imm));
    *ConsoleWriteSInt = result2;

    //ori
    int result3;
    asm volatile("ori %[res], %[a], %[imm]" : [res]"=r"(result3) : [a]"r"(a), [imm]"i"(imm));
    *ConsoleWriteSInt = result3;

    //andi
    int result4;
    asm volatile("andi %[res], %[a], %[imm]" : [res]"=r"(result4) : [a]"r"(a), [imm]"i"(imm));
    *ConsoleWriteSInt = result4;

    //lui
    int luiimm = 0x10000;
    int result5;
    asm volatile("lui %[res], %[luiimm]" : [res]"=r"(result5) : [luiimm]"i"(luiimm));
    *ConsoleWriteHexSInt = result5;

    return 0;
}
