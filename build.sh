if [[ $(uname) == "Darwin" ]]; then
    compiler_path="i686-elf-gcc"
    cxx_cmpl_path="i686-elf-g++"
    assemble_path="i686-elf-as"
    grub_iso_path="i686-elf-grub-mkrescue"
else
    compiler_path="/home/thewhynow/opt/cross/bin/i686-elf-gcc"
    cxx_cmpl_path="/home/thewhynow/opt/cross/bin/i686-elf-g++"
    assemble_path="/home/thewhynow/opt/cross/bin/i686-elf-as"
    grub_iso_path="grub-mkrescue"
fi

x_flags="-std=c++98 -ffreestanding -Wall -Wextra -D_KERNEL_LIBC -nostdlib"
c_flags="-std=gnu99 -ffreestanding -Wall -Wextra -D_KERNEL_LIBC -nostdlib"
s_flags=""
q_flags=" -m 512"
q_flopy=" -fda floppy.img"

if [[ "$1" == "debug" ]]; then
    c_flags+=" -g"
    s_flags+=" -g"
    x_flags+=" -g"
    q_flags+=" -s -S"
fi

if [[ "$1" == "log" || "$2" == "log" ]]; then
    q_flags+=" -d int -D qemu.log"
fi

$compiler_path -c kernel/kernel/kernel.c        -o kernel.o    $c_flags
$compiler_path -c kernel/arch/i386/tty.c        -o tty.o       $c_flags
$compiler_path -c kernel/arch/i386/gdt.c        -o gdt.o       $c_flags
$compiler_path -c kernel/arch/i386/irq.c        -o irq.o       $c_flags
$compiler_path -c kernel/arch/i386/idt.c        -o idt.o       $c_flags
$compiler_path -c kernel/arch/i386/isr.c        -o isr.o       $c_flags
$compiler_path -c kernel/arch/i386/io.c         -o io.o        $c_flags
$compiler_path -c kernel/arch/i386/pic.c        -o pic.o       $c_flags
$compiler_path -c kernel/arch/i386/pit.c        -o pit.o       $c_flags
$compiler_path -c kernel/arch/i386/ps2.c        -o ps2.o       $c_flags
$compiler_path -c kernel/arch/i386/multiboot.c  -o multiboot.o $c_flags
$compiler_path -c kernel/arch/i386/pmm.c        -o pmm.o       $c_flags
$compiler_path -c kernel/arch/i386/vmm.c        -o vmm.o       $c_flags
$compiler_path -c kernel/arch/i386/dma.c        -o dma.o       $c_flags
$compiler_path -c kernel/arch/i386/fdc.c        -o fdc.o       $c_flags

$compiler_path -c libc/stdio/printf.c           -o printf.o    $c_flags
$compiler_path -c libc/stdio/putchar.c          -o putchar.o   $c_flags
$compiler_path -c libc/stdio/puts.c             -o puts.o      $c_flags
$compiler_path -c libc/stdio/getchar.c          -o getchar.o   $c_flags
$compiler_path -c libc/stdio/gets.c             -o gets.o      $c_flags

$compiler_path -c libc/stdlib/abort.c           -o abort.o     $c_flags

$compiler_path -c libc/string/memcmp.c          -o memcmp.o    $c_flags
$compiler_path -c libc/string/memcpy.c          -o memcpy.o    $c_flags
$compiler_path -c libc/string/memmove.c         -o memmove.o   $c_flags
$compiler_path -c libc/string/memset.c          -o memset.o    $c_flags
$compiler_path -c libc/string/strlen.c          -o strlen.o    $c_flags
$compiler_path -c libc/string/strncat.c         -o strncat.o   $c_flags

$assemble_path kernel/arch/i386/asm/boot.s      -o boot.o      $s_flags
$assemble_path kernel/arch/i386/asm/crti.s      -o crti.o      $s_flags
$assemble_path kernel/arch/i386/asm/crtn.s      -o crtn.o      $s_flags
$assemble_path kernel/arch/i386/asm/gdt.s       -o _gdt.o      $s_flags
$assemble_path kernel/arch/i386/asm/idt.s       -o _idt.o      $s_flags
$assemble_path kernel/arch/i386/asm/isr.s       -o _isr.o      $s_flags
$assemble_path kernel/arch/i386/asm/irq.s       -o _irq.o      $s_flags

$compiler_path -T kernel/arch/i386/linker.ld -o iso/boot/lakeos.elf $c_flags -lgcc *.o

rm *.o

if [[ "$1" == "debug" ]]; then
    echo $q_flags $q_flopy
    qemu-system-i386 -kernel iso/boot/lakeos.elf $q_flags $q_flopy
else
    $grub_iso_path -o lakeos.iso iso
    qemu-system-i386 -cdrom lakeos.iso $q_flags $q_flopy
    rm lakeos.iso
fi

rm iso/boot/lakeos.elf