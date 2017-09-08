global _start                           ; making entry point visible to linker

extern kmain                            ; kmain is defined in kmain.cpp

extern start_ctors                      ; beginning and end
extern end_ctors                        ; of the respective
extern start_dtors                      ; ctors and dtors section,
extern end_dtors                        ; declared by the linker script

global BootPageDirectory

KERNEL_VIRTUAL_BASE equ 0xE0000000                  ; 3.5GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)

section .data
align 0x1000
BootPageDirectory:
    ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB.
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.
    ; This entry must be here -- otherwise the kernel will crash immediately after paging is
    ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
    dd 0x00000083
    times (KERNEL_PAGE_NUMBER - 1) dd 0                 ; Pages before kernel space.
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0  ; Pages after the kernel image.

section .text

; reserve initial kernel stack space
STACKSIZE equ 0x4000                    ; that's 16k.

_start:
    ; NOTE: Until paging is set up, the code must be position-independent and use physical
    ; addresses, not virtual ones!
    ;mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE)
    ;mov cr3, ecx                                        ; Load Page Directory Base Register.

    ;mov ecx, cr4
    ;or ecx, 0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
    ;mov cr4, ecx

    ;mov ecx, cr0
    ;or ecx, 0x80000000                          ; Set PG bit in CR0 to enable paging.
    ;mov cr0, ecx

    ; Start fetching instructions in kernel space.
    ; Since eip at this point holds the physical address of this command (approximately 0x00100000)
    ; we need to do a long jump to the correct virtual address of StartInHigherHalf which is
    ; approximately 0xE0100000.
    ;lea ecx, [higher_half_start]
    ;jmp ecx                                                     ; NOTE: Must be absolute jump!

higher_half_start:
    ; Unmap the identity-mapped first 4MB of physical address space. It should not be needed
    ; anymore.
    ;mov dword [BootPageDirectory], 0
    ;invlpg [0]

    mov  esp, stack + STACKSIZE         ; set up the stack

    mov [magic], eax
    ;add ebx, KERNEL_VIRTUAL_BASE ; make the address virtual
    mov [mbd_info], ebx

    mov  ebx, start_ctors               ; call the constructors
    jmp  .ctors_until_end
.call_constructor:
    call [ebx]
    add  ebx,4
.ctors_until_end:
    cmp  ebx, end_ctors
    jb   .call_constructor

    push dword [mbd_info]
    push dword [magic]

    call kmain                          ; call kernel proper

    mov  ebx, end_dtors                 ; call the destructors
    jmp  .dtors_until_end
.call_destructor:
    sub  ebx, 4
    call [ebx]
.dtors_until_end:
    cmp  ebx, start_dtors
    ja   .call_destructor

    cli
hang:
    hlt                                 ; halt machine should kernel return
    jmp  hang

section .bss

align 4
magic:      resd 1
mbd_info:   resd 1
stack:      resb STACKSIZE                   ; reserve 16k stack on a doubleword boundary
