.data
    msg: .string "hello world\n"

.text
    .global _start

_start:
    mov $1, %rax;         # passing syscall write in %rax         
    mov $1, %rdi;         # set first parameter as 1 (stdout)
    mov $msg, %rsi;       # set second parameter as msg (hello world\n)  
    mov $12, %rdx;        # set third parameter as 12        
    SYSCALL               # make syscall
    
    mov %rax, %rdi;       # mov the rax value into the first paramter
    mov $60, %rax;        # passing syscall _exit in %rax       
    SYSCALL               # make syscall

