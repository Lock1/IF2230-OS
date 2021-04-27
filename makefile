all: createbaseimage

clean:
	rm -r out/*

createbaseimage:
	dd if=/dev/zero of=out/mangga.img bs=512 count=2880
