# 13519214 - Makefile
all: diskimage bootloader kernel createfilesystem insertfilesystem

clean:
	# -- Cleaning output files --
	@rm out/fs/*;
	@rm out/*

test: kernelgcc

cleantest: cleangcc



# Main recipes
diskimage:
	# -- Initial mangga.img --
	@dd if=/dev/zero of=out/mangga.img bs=512 count=2880 status=noxfer

bootloader:
	# -- Bootloader insertion --
	@nasm src/asm/bootloader.asm -o out/bootloader;
	@dd if=out/bootloader of=out/mangga.img bs=512 count=1 conv=notrunc status=noxfer

kernel:
	# -- Source Compilation --
	@bcc -ansi -c -o out/kernel.o src/kernel.c
	@bcc -ansi -c -o out/std.o src/std.c
	@bcc -ansi -c -o out/screen.o src/screen.c
	@bcc -ansi -c -o out/shell.o src/shell.c
	@bcc -ansi -c -o out/output.o src/output.c
	@bcc -ansi -c -o out/opr.o src/opr.c
	@nasm -f as86 src/asm/kernel.asm -o out/kernel_asm.o
	@ld86 -o out/kernel -d out/*.o
	# ------------ Compiled kernel stat ------------
	# Max Kernel Size : 15872 bytes (31 sectors, 1 sector = 512 bytes)
	@stat --printf="Kernel Size : %s bytes\n" out/kernel
	# ----------------------------------------------
	@dd if=out/kernel of=out/mangga.img bs=512 conv=notrunc seek=1 status=noxfer

createfilesystem:
	[ -d out/fs ] || mkdir out/fs;
	@./other/fscreate out/fs/map.img out/fs/files.img out/fs/sectors.img

insertfilesystem:
	# -- Filesystem insertion --
	@dd if=out/fs/map.img of=out/mangga.img bs=512 count=1 seek=256 conv=notrunc status=noxfer
	@dd if=out/fs/files.img of=out/mangga.img bs=512 count=2 seek=257 conv=notrunc status=noxfer
	@dd if=out/fs/sectors.img of=out/mangga.img bs=512 count=1 seek=259 conv=notrunc status=noxfer

filesystemcreator:
	@gcc -Wall -Wextra -O3 -o other/fscreate other/filesystem_create.c

fileloader:
	@gcc -Wall -Wextra -O3 -o other/loadFile other/fileloader.c


# Test recipes
kernelgcc:
	gcc -c -Wall -Wextra -o out/compiledgcc.o src/kernel.c

cleangcc:
	rm out/compiledgcc.o
