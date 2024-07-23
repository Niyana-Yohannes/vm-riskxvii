inline int scan_char() {
    int result;
    int addr = 0x0816;
    
    asm volatile("lw %[res], 0(%[adr])": [res]"=r"(result): [adr]"r"(addr));

    return result;
}

int volatile *const ConsoleReadSInt = (int *)0x0816;
int volatile *const ConsoleWriteSInt = (int *)0x0804;

int main() {

    int a = scan_char();
    int b = scan_char();

    *ConsoleWriteSInt = a - b;

    int c = scan_char();
    int d = scan_char();

    *ConsoleWriteSInt = c & d;

    int e = scan_char();
    int f = scan_char();

    *ConsoleWriteSInt = e | f;

    int g = scan_char();
    int h = scan_char();

    *ConsoleWriteSInt = g ^ h;

    int i = scan_char();
    int j = scan_char();

    *ConsoleWriteSInt = (i < j) ? 1 : 0;

    int k = scan_char();
    int l = scan_char();

    *ConsoleWriteSInt = (k >> l) | (k << (32 - l));

    return 0;
}