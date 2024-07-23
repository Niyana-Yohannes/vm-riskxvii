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

    //store/load byte
    int addr = 0x0400;
    int a = scan_char();
    asm volatile("sb %[val], 0(%[adr])" : : [val]"r"(a), [adr]"r"(addr));
    int result;
    asm volatile("lb %[res], 0(%[adr])" : [res]"=r"(result) : [adr]"r"(addr));
    *ConsoleWriteSInt = result;

    //store/load half
    int addr2 = 0x0500;
    int b = scan_char();
    asm volatile("sh %[val], 0(%[adr])" : : [val]"r"(b), [adr]"r"(addr2));
    int result2;
    asm volatile("lh %[res], 0(%[adr])" : [res]"=r"(result) : [adr]"r"(addr2));
    *ConsoleWriteSInt = result2;

    //store/load word
    int addr3 = 0x0600;
    int c = scan_char();
    asm volatile("sw %[val], 0(%[adr])" : : [val]"r"(c), [adr]"r"(addr3));
    int result3;
    asm volatile("lw %[res], 0(%[adr])" : [res]"=r"(result) : [adr]"r"(addr3));
    *ConsoleWriteSInt = result3;

    //Invalid
    int addr4 = 0x0800;
    int d = scan_char();
    asm volatile("sw %[val], 0(%[adr])" : : [val]"r"(d), [adr]"r"(addr4));
    int result4;
    asm volatile("lw %[res], 0(%[adr])" : [res]"=r"(result) : [adr]"r"(addr4));
    *ConsoleWriteSInt = result4;

    return 0;
}