all: createbaseimage insertbootloader insertbasekernel

clean:
	rm -r out/*

createbaseimage:
	dd if=/dev/zero of=out/mangga.img bs=512 count=2880

insertbootloader:
	nasm src/asm/bootloader.asm -o out/bootloader
	dd if=out/bootloader of=out/mangga.img bs=512 count=1 conv=notrunc

insertbasekernel:
	if [ ! -d "out/obj" ]; then mkdir out/obj; fi
	bcc -ansi -c -o out/obj/kernel.o src/kernel.c
	nasm -f as86 src/asm/kernel.asm -o out/obj/kernel_asm.o
	ld86 -o out/kernel -d out/obj/*.o
	dd if=out/kernel of=out/mangga.img bs=512 conv=notrunc seek=1
