all: createbaseimage

clean:
	rm -r out/*

createbaseimage:
	dd if=/dev/zero of=out/mangga.img bs=512 count=2880

insertbootloader:
	nasm src/asm/bootloader.asm -o out/bootloader
	dd if=out/bootloader of=out/mangga.img bs=512 count=1 conv=notrunc
