# 13519214 - Makefile
all: basekernel shellpackage extrapackage createrecursiontest logoinsert

basekernel: diskimage bootloader kernel createfilesystem insertfilesystem

clean:
	# -- Cleaning output files --
	@rm out/fs/*
	@rm out/*
	@rm out/asm/*
	@rm out/shell/*

test: kernelgcc

cleantest: cleangcc

shellpackage: basekernel fileloader mash insertls insertcd insertmkdir \
			  insertcat insertcp insertmv insertln insertrm


extrapackage: shellpackage insertfile insertwc insertstrings insertmim \
			  insertwhereis insertsnok insertprintf

mash:
	if [ ! -d "out/shell" ]; then mkdir out/shell; fi
	@bcc -ansi -c -o out/shell/mash.o src/mash.c
	@bcc -ansi -c -o out/shell/std_stringio.o src/std_stringio.c
	@bcc -ansi -c -o out/shell/std_fileio.o src/std_fileio.c
	@bcc -ansi -c -o out/shell/shell_common.o src/shell_common.c
	@bcc -ansi -c -o out/shell/std_opr.o src/std_opr.c
	if [ ! -d "out/shell/asm" ]; then mkdir out/shell/asm; fi
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/mash -d out/shell/*.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img mash 0

insertls:
	if [ ! -d "out/shell/ls" ]; then mkdir out/shell/ls; fi
	@bcc -ansi -c -o out/shell/ls/ls.o src/ls.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/ls -d out/shell/ls/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img ls 0

insertcd:
	if [ ! -d "out/shell/cd" ]; then mkdir out/shell/cd; fi
	@bcc -ansi -c -o out/shell/cd/cd.o src/cd.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/cd -d out/shell/cd/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img cd 0

insertmkdir:
	if [ ! -d "out/shell/mkdir" ]; then mkdir out/shell/mkdir; fi
	@bcc -ansi -c -o out/shell/mkdir/mkdir.o src/mkdir.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/mkdir -d out/shell/mkdir/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img mkdir 0

insertcat:
	if [ ! -d "out/shell/cat" ]; then mkdir out/shell/cat; fi
	@bcc -ansi -c -o out/shell/cat/cat.o src/cat.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/cat -d out/shell/cat/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img cat 0

insertcp:
	if [ ! -d "out/shell/cp" ]; then mkdir out/shell/cp; fi
	@bcc -ansi -c -o out/shell/cp/cp.o src/cp.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/cp -d out/shell/cp/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img cp 0

insertmv:
	if [ ! -d "out/shell/mv" ]; then mkdir out/shell/mv; fi
	@bcc -ansi -c -o out/shell/mv/mv.o src/mv.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/mv -d out/shell/mv/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img mv 0

insertln:
	if [ ! -d "out/shell/ln" ]; then mkdir out/shell/ln; fi
	@bcc -ansi -c -o out/shell/ln/ln.o src/ln.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/ln -d out/shell/ln/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img ln 0

insertrm:
	if [ ! -d "out/shell/rm" ]; then mkdir out/shell/rm; fi
	@bcc -ansi -c -o out/shell/rm/rm.o src/rm.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/rm -d out/shell/rm/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img rm 0

insertfile:
	if [ ! -d "out/shell/file" ]; then mkdir out/shell/file; fi
	@bcc -ansi -c -o out/shell/file/file.o src/file.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/file -d out/shell/file/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img file 0

insertwc:
	if [ ! -d "out/shell/wc" ]; then mkdir out/shell/wc; fi
	@bcc -ansi -c -o out/shell/wc/wc.o src/wc.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/wc -d out/shell/wc/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img wc 0

insertstrings:
	if [ ! -d "out/shell/strings" ]; then mkdir out/shell/strings; fi
	@bcc -ansi -c -o out/shell/strings/strings.o src/strings.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/strings -d out/shell/strings/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img strings 0

insertmim:
	if [ ! -d "out/shell/mim" ]; then mkdir out/shell/mim; fi
	@bcc -ansi -c -o out/shell/mim/mim.o src/mim.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/mim -d out/shell/mim/*.o out/shell/std_fileio.o \
	out/shell/std_stringio.o out/shell/shell_common.o \
	out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img mim 0

insertsnok:
	if [ ! -d "out/shell/snok" ]; then mkdir out/shell/snok; fi
	@bcc -ansi -c -o out/shell/snok/snok.o src/snok.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/snok -d out/shell/snok/*.o out/shell/std_fileio.o \
	out/shell/std_stringio.o out/shell/shell_common.o \
	out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img snok 0

insertwhereis:
	if [ ! -d "out/shell/whereis" ]; then mkdir out/shell/whereis; fi
	@bcc -ansi -c -o out/shell/whereis/whereis.o src/whereis.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/whereis -d out/shell/whereis/*.o out/shell/std_fileio.o \
	out/shell/std_stringio.o out/shell/shell_common.o \
	out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img whereis 0

insertprintf:
	if [ ! -d "out/shell/printf" ]; then mkdir out/shell/printf; fi
	@bcc -ansi -c -o out/shell/printf/printf.o src/printf.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/printf -d out/shell/printf/*.o out/shell/std_fileio.o \
	out/shell/std_stringio.o out/shell/shell_common.o \
	out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img printf 0

createrecursiontest:
	if [ ! -d "out/shell/recursion_test" ]; then mkdir out/shell/recursion_test; fi
	@bcc -ansi -c -o out/shell/recursion_test/recursion_test.o src/recursion_test.c
	@nasm -f as86 src/asm/interrupt.asm -o out/shell/asm/interrupt.o
	@ld86 -o out/recursion_test -d out/shell/recursion_test/*.o out/shell/std_fileio.o \
			out/shell/std_stringio.o out/shell/shell_common.o \
	 		out/shell/std_opr.o out/shell/asm/interrupt.o
	@cd out; ./loadFile mangga.img recursion_test 0

logoinsert:
	@cp other/logo.hoho out/logo.hoho
	@cd out; ./loadFile mangga.img logo.hoho 255

# Main recipes
diskimage:
	# -- Initial mangga.img --
	if [ ! -d "out" ]; then mkdir out; fi
	@dd if=/dev/zero of=out/mangga.img bs=512 count=2880 status=noxfer

bootloader:
	# -- Bootloader insertion --
	@nasm src/asm/bootloader.asm -o out/bootloader;
	@dd if=out/bootloader of=out/mangga.img bs=512 count=1 conv=notrunc status=noxfer

kernel:
	# -- Source Compilation --
	@bcc -ansi -c -o out/kernel.o src/kernel.c
	@bcc -ansi -c -o out/std_stringio.o src/std_stringio.c
	@bcc -ansi -c -o out/std_fileio.o src/std_fileio.c
	@bcc -ansi -c -o out/screen.o src/screen.c
	@bcc -ansi -c -o out/output.o src/output.c
	@bcc -ansi -c -o out/std_opr.o src/std_opr.c
	@nasm -f as86 src/asm/kernel.asm -o out/kernel_asm.o
	if [ ! -d "out/asm" ]; then mkdir out/asm; fi
	@nasm -f as86 src/asm/interrupt.asm -o out/asm/interrupt.o
	ld86 -o out/kernel -d out/*.o out/asm/interrupt.o
	# ------------ Compiled kernel stat ------------
	# Max Kernel Size : 8192 bytes (16 sectors, 1 sector = 512 bytes)
	@stat --printf="Kernel Size : %s bytes\n" out/kernel
	# ----------------------------------------------
	@dd if=out/kernel of=out/mangga.img bs=512 conv=notrunc seek=1 status=noxfer

createfilesystem:
	if [ ! -d "out/fs" ]; then mkdir out/fs; fi
	@./other/fscreate out/fs/map.img out/fs/files.img out/fs/sectors.img

insertfilesystem:
	# -- Filesystem insertion --
	@dd if=out/fs/map.img of=out/mangga.img bs=512 count=1 seek=256 conv=notrunc status=noxfer
	@dd if=out/fs/files.img of=out/mangga.img bs=512 count=2 seek=257 conv=notrunc status=noxfer
	@dd if=out/fs/sectors.img of=out/mangga.img bs=512 count=1 seek=259 conv=notrunc status=noxfer

filesystemcreator:
	if [ ! -d "other" ]; then mkdir other; fi
	@gcc -Wall -Wextra -O3 -o other/fscreate other/filesystem_create.c
	chmod +x other/fscreate

fileloader:
	@gcc -Wall -Wextra -O3 -o out/loadFile other/fileloader.c


# Test recipes
kernelgcc:
	gcc -c -Wall -Wextra -o out/compiledgcc.o src/kernel.c

cleangcc:
	rm out/compiledgcc.o
