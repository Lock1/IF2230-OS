# Milestone 1
## Deskripsi Pendek
Milestone 1 menugaskan untuk mendesain sistem operasi dalam C (dan sedikit assembly untuk fitur tambahan)
dan memperkenalkan cara kerja sederhana interrupt. Target sistem operasi yang didesain adalah sistem 16-bit
yang akan diemulasikan pada emulator `bochs`.

Pengerjaan milestone 1 secara umum sudah dijelaskan pada spesifikasi milestone 1 sendiri, namun pada markdown
ini akan ditulis step-by-step pengerjaan dan penjelasan tentang hal terkait.





## Pengerjaan
### 1. Persiapan instalasi alat-alat yang digunakan
![Install instruction embedded image](other/markdown-img/milestone-1/installing-tools.jpg)

Sebelum melakukan langkah pengerjaan 3.1, diperlukan untuk memasang alat-alat yang digunakan untuk
membuat sistem operasi. Spesifikasi milestone 1 menganjurkan pemrogram untuk menggunakan sistem operasi
**Ubuntu versi 18.04 atau 20.04**, jika sudah menggunakan **ubuntu** dapat melanjutkan
pengerjaan ke bagian [instalasi alat-alat yang dibutuhkan](#2-pemasangan-alat-alat).

---

Penulis menggunakan [WSL2](https://docs.microsoft.com/en-us/windows/wsl/about) dalam host OS Windows 10
dalam pengerjaan tugas ini, troubleshooting masalah instalasi dan cara instalasi WSL2 dapat dilihat pada markdown
[WSL-OS-Troubleshooting.md](other/WSL-OS-Troubleshooting.md).

Untuk pengguna selain distribusi linux dan tidak ingin memasang sistem operasi baru pada mesin yang dimiliki
dapat menggunakan [virtual machine](https://en.wikipedia.org/wiki/Virtual_machine) seperti
[Oracle VirtualBox](https://www.virtualbox.org/) atau [VMWare](https://www.vmware.com/).
Instruksi untuk memasang virtual machine sudah cukup banyak dan mudah diakses dengan mencari
`Cara install virtualbox dan ubuntu` pada search engine.

<br/>
<br/>

### 2. Pemasangan alat-alat
![Install tools embedded](other/markdown-img/milestone-1/installing-tools-command.jpg)

![Installing update](other/markdown-img/milestone-1/apt-update.jpg)

Jalankan command diatas pada terminal, jika menggunakan distribusi non-Ubuntu yang tidak menggunakan
package manager `apt`, gunakanlah package manager yang sesuai (contoh `apk` untuk Alpine).
Instalasi alat-alat dapat dimasukkan kedalam script jika ingin mempermudah pengguna lain,
cek pembuatan script pada section [tambahan script tools](#tambahan)

Setelah alat telah didownload dan install, buka dan unzip
[kit-1.zip](original-milestone/other) pada suatu lokasi.

---

Buatlah folder baru bernama `src` yang dalamnya berisi folder `asm` dan `bochs-config`.
Ketiga file `.asm` (`bootloader.asm`, `kernel.asm`, `lib.asm`) diletakkan pada folder `asm` dan
`if2230.config` pada folder `bochs-config`.

![Unzipping](other/markdown-img/milestone-1/unzip-kit.jpg)


<br/>
<br/>

### 3. Persiapan disk image
Setelah memindahkan file ke folder yang terkait, buatlah folder `out` dan file baru tak berekstensi bernama `makefile`.
Bukalah file tersebut menggunakan text editor dan tambahkan recipe baru `all`, `clean`, dan `createbaseimage`. Tambahkan
juga text file baru bernama `.gitignore` pada lokasi sekarang yang berisi tulisan `out/*` untuk mengabaikan isi
dari folder `out` pada version control `git`.

![Base image](other/markdown-img/milestone-1/makefile-baseimage.jpg)

`makefile` akan digunakan sebagai alat utama untuk build/membuat sistem operasi.
Nantinya dapat menggunakan perintah `make <recipe>` untuk membuat resep. Resep
`all` digunakan untuk membuat sistem operasi sepenuhnya, resep `clean` digunakan
untuk menghapus hasil output pada folder `out`, sedangkan `createbaseimage` digunakan
untuk membuat disk image.

---

![dd utility](other/markdown-img/milestone-1/disk-image-dd.jpg)

`dd` adalah command line utility pada distribusi Linux yang digunakan untuk melakukan operasi terhadap suatu *disc image*.

Argumen `if=/dev/zero` menyuruh `dd` untuk mengambil informasi pada `/dev/zero` sebagai source *image*, `/dev/zero` sendiri
merupakan file khusus pada distribusi Linux yang akan selalu menghasilkan nilai 0 jika dibaca. `if` kependekan dari `input file`.

Argumen `of=out/mangga.img` memberi tahu `dd` untuk meletakkan output ke `out/mangga.img`. Penulis menggunakan nama
disc image sistem operasi `mangga.img`, dapat diganti menjadi nama yang lain jika mau. `of` kependekan dari `output file`.

Argumen `bs=512` dan `count=2880` memberi informasi kepada `dd` berapa besar ukuran 1 sektor (`bs=512`) dan berapa banyak
sektor yang ada pada disc image (`count=2880`).

Singkatnya `if=/dev/zero` menyuruh `dd` untuk menulis nilai hex `0x00` sebanyak `bs=512` kali pada satu sektor dan terdapat
`count=2880` sektor ke file output yang bernama `of=out/mangga.img`.

---

Jalankan command `make createbaseimage` dan cek pada folder out.

![dd testing](other/markdown-img/milestone-1/disk-image-test.jpg)


<br/>
<br/>

### 4. Bootloader
Tambahkan kode pada spesifikasi ke `makefile` dengan recipe bernama `insertbootloader`.

![boot insert](other/markdown-img/milestone-1/bootloader-insertion.jpg)

Perintah `nasm` seperti compiling pada umumnya, menerima source file (dalam kasus ini `src/asm/bootloader.asm`) dan
mengoutputkan ke `out/bootloader` dengan flag `-o`.

`dd` digunakan untuk memasukan binary executable hasil kompilasi `nasm` ke `out/mangga.img` yang memiliki ukuran 1 sektor
`bs=512` bytes dan diulangi sebanyak `count=1`. Flag `conv=notrunc` menyuruh `dd` untuk tidak merubah apapun pada sektor
selain sektor target `if=out/bootloader` yaitu sektor 0.

Jalankan `make insertbootloader` dan hasilnya akan terlihat seperti berikut pada hex editor

![boot result](other/markdown-img/milestone-1/bootloader-hxd.jpg)

---

Pada tahap ini disarankan untuk mengecek [tambahan hex editor](#2-hex-editor) yang dapat digunakan mengecek hasil
build sistem operasi. Nantinya hex editor akan digunakan lagi secara ekstensif pada debugging dan pembuatan milestone 2
 filesystem.

<br/>
<br/>

### 5. Pembuatan kernel
Secara singkat bagian ini dapat mengikuti secara langsung spesifikasi pembuatan kernel dengan membuat file baru `kernel.c`
pada folder `src`. Isilah `kernel.c` dengan kode yang terdapat pada spesifikasi milestone 1 dan sedikit modifikasi untuk
mengikuti standar koding C.

![Kernel firststep](other/markdown-img/milestone-1/kernel-c-firststep.jpg)

Keyword `extern` digunakan untuk memberi tahu kepada `bcc` atau compiler lain untuk menganggap fungsi tersebut
akan disediakan pada file source code yang lain. Atau dapat dikatakan `extern` memperbolehkan pemrogram mendeklarasikan
fungsi yang tidak diberikan definisi pada file tersebut.

Setelah membuat file tersebut tambahkan kode berikut pada `makefile`

![Kernel base makefile](other/markdown-img/milestone-1/kernel-basemakefile.jpg)

Line `if [ ! -d "out/obj" ]; then mkdir out/obj; if` mengecek apakah ada folder `obj` pada folder `out`, jika tidak ada
maka buat folder tersebut dengan `mkdir out/obj`.

Seperti sebelumnya `bcc` dan `nasm` mencompile source code menjadi object file dengan flag yang sesuai. Flag `-ansi` dan
`-f as86` memberikan informasi terkait cara kompilasi `bcc` dan `nasm`.

`ld86` digunakan untuk melink semua object file dan mengeluarkan dengan nama `out/kernel`.
Flag `-d` sangat penting ketika proses linking, cek pada [catatan README.md](README.md#catatan-penting-ketika-melakukan-pengerjaan).

Parameter tambahan `seek=1` pada `dd` digunakan untuk memasukkan input file ke sektor 1.

---

Jalankan `make insertbasekernel` dan pada hex editor akan terlihat pada `mangga.img`

![Kernel insertion](other/markdown-img/milestone-1/kernel-base-insert.jpg)

<br/>
<br/>

### 6. Menjalankan sistem operasi
Setelah memastikan tahap sebelumnya telah berjalan dengan normal, sekarang sistem operasi dapat diuji dengan dijalankan.
Buatlah file baru bernama `run.sh` yang berisikan kode `bochs -f src/bochs-config/if2230.config`. Setelah file dibuat, buka
file `if2230.config` dengan text editor, carilah `floppya: 1_44=system.img`. Ganti konfigurasi tersebut ke nama disc image
yang dibuat (beserta relative pathing terhadap root). Contoh jika menggunakan `mangga.img` pada folder `out`, kode tersebut
menjadi `floppya: 1_44=out/mangga.img`.

![Bochs config](other/markdown-img/milestone-1/bochs-configurating.jpg)

---

Setelah itu kembali ke folder root dan jalankan `./run.sh` (Catatan kecil untuk WSL, jangan lupa menjalankan X server!).
Akan terlihat window baru bernama `Bochs x86-64 emulator` yang berisikan panel hitam.

![Bochs black](other/markdown-img/milestone-1/bochs-black.jpg)

Ketik `c` pada terminal dan tekan enter, jika kernel telah dimasukkan dengan normal akan keluar seperti berikut

![Bochs first run](other/markdown-img/milestone-1/bochs-first-test.jpg)

Perhatikan pada pojok kiri atas terdapat tulisan `Hai` dalam warna magenta seperti yang tertulis pada `main()`.

<br/>
<br/>

### 7. Pembuatan printString dan readString
**TBA**





<br/>
<br/>
<br/>
<br/>
<br/>

## Tambahan
Berikut adalah beberapa informasi tambahan yang digunakan untuk menjelaskan detail-detail, fungsionalitas tambahan, dan
alat-alat yang dapat digunakan untuk membantu proses pembuatan sistem operasi.

---
### 1. Script instalasi alat-alat
Untuk mempermudah pengguna lain dalam setup menjalankan sistem operasi yang dibuat,
dependencies installation dapat dimasukkan kedalam `bash` script sederhana.

![Simple bash script](other/markdown-img/milestone-1/extra-tools-script.jpg)

Tulislah kode tersebut pada file baru yang berekstensi `.sh`. Jika tidak menginginkan
menulis secara manual, gunakanlah pipe redirection seperti berikut

`printf "sudo apt update\nsudo apt install nasm bcc gcc bin86 bochs bochs-x make\n" > tools-install.sh`

Perintah tersebut akan menuliskan command untuk memasang dependencies dan mengarahkan output ke file baru
bernama `tools-install.sh`. Catatan, `bash` script umumnya perlu diberikan `chmod +x <filename>` terlebih dahulu
agar dapat dieksekusi seperti normal. Contoh eksekusi `bash` script `./tools-install.sh` jika sedang berada pada
lokasi direktori yang sama.


<br/>
<br/>

### 2. Hex editor
![HxD](other/markdown-img/milestone-1/hxd-sample.jpg)

**HxD hex editor**

![hexedit](other/markdown-img/milestone-1/hexedit-sample.jpg)

**hexedit hex editor**

Terdapat banyak hex editor yang dapat digunakan untuk mengedit dan membaca file binary secara hexadecimal. Kedua hex editor
diatas merupakan hex editor umum yang ada pada Windows dan Linux distribution. Hex editor nanti akan digunakan pada
milestone 2 pembuatan filesystem yang memerlukan pengecekan apakah disk I/O yang dioperasikan telah memenuhi keinginan atau
belum.

HxD hex editor dapat didownload pada link berikut [https://mh-nexus.de/en/hxd/](https://mh-nexus.de/en/hxd/).
HxD tersedia pada Windows 64-bit dan 32-bit.

`hexedit` merupakan command line utility yang umumnya dapat secara langsung didownload menggunakan package manager
masing-masing distro, contoh untuk instalasi `hexedit` pada Ubuntu `sudo apt-get install hexedit`.

<br/>
<br/>

### 3. Penjelasan assembly kernel.asm
Sebagian besar kode assembly sudah dijelaskan pada spesifikasi pembuatan kernel,
bagian ini hanya menjelaskan ulang dengan cara yang lain.

---

`void putInMemory (int segment, int address, char character)`

Fungsi tersebut menuliskan angka 1 byte yang bernama `character` kepada memori yang terletak pada `0x10*segment + address`.
Umumnya segment bernilai kelipatan dari `0x1000` hal ini dikarenakan biasanya segment hanya digunakan ketika ingin mengakses
memori diatas 16-bit address (`0x0000` hingga `0xFFFF`). Perhatikan bahwa nilai `segment` dikalikan `0x10` terlebih dahulu
sehingga dapat mengakses memori address `0x10000` hingga `0x1FFFF` jika nilai `segment` adalah `0x1000`
(Perkalian hexadecimal `0x10*0x1000` menghasilkan `0x10000`).

Pada sisi assembly, terdapat **segment register** untuk **setiap register data** yang ada. Secara implisit,
nilai yang terletak pada segment register akan dikalikan `0x10` ketika meng-fetch suatu data. Contoh register instruksi `ip`
 berhubungan dengan segment register `cs` yaitu data segment, secara implisit setiap kali CPU mengambil instruksi akan
mengambil pada address `0x10*cs + ip`. Misal nilai `cs` diset pada `0x1000` dan `ip` terletak pada `0xC400`, CPU akan
mengambil instruksi yang terletak pada `0x1000*0x10 + 0xC400` yaitu `0x1C400`, bukan instruksi pada `0xC400`. Segment
 register tidak dipergunakan lagi jika ingin memasuki **protected mode**. Segment register umumnya memiliki nama yang sama
 dengan **struktur segmentasi memori x86**, untuk lebih jelasnya cek
 [x86 memory segmentation](https://en.wikipedia.org/wiki/X86_memory_segmentation).


![putinmemory asm](other/markdown-img/milestone-1/put-in-memory.jpg)

Implementasi `putInMemory` pada `.asm` secara singkat mengganti sementara segment register `ds` ke target `segment`
yang terletak pada `[bp+4]` dan memindahkan 1 byte integer `character` ke lokasi `address` pada instruksi `mov [si], cl`.

---

`int interrupt (int number, int AX, int BX, int CX, int DX)`

Seperti namanya, fungsi `interrupt()` akan melakukan suatu interrupt `number` yang diberikan argumen vektor `AX`, `BX`, `CX`
, dan `DX`. Contohnya pemanggilan `interrupt(0x10, 0x0C0E, 0x0, 256, 128);` akan melakukan interrupt `0x10` yang
menyediakan fasilitas I/O. Jika belum familiar dengan cara kerja hexadecimal dan 16-bit register, dapat mengecek
[appendix hexadecimal dan register](Appendix.md#4-hexadecimal-dan-register).
Contoh diatas akan dijelaskan lebih lanjut pada appendix hexadecimal dan register.

Interrupt merupakan fasilitas pada BIOS yang disediakan oleh komputer yang berdasarkan IBM PC. Fitur ini umumnya tidak
digunakan lagi pada **protected mode**, sistem operasi yang sedang dibuat merupakan sistem operasi yang bekerja pada
**real mode**. Interrupt dipergunakan untuk melakukan fasilitas-fasilitas input-output dasar pada hardware yang digunakan.

Instruksi interrupt akan membuat CPU untuk mengakses
[interrupt vector table](https://en.wikipedia.org/wiki/Interrupt_vector_table) yang berisikan address-address fungsi atau
prosedur yang akan dijalankan ketika interrupt dipanggil. Fungsi dan prosedur tersebut dinamakan **interrupt handler**.
BIOS menyediakan beberapa interrupt handler yang dapat digunakan pemrogram, interrupt yang tersedia dapat dicek pada halaman
 [BIOS interrupt call](https://en.wikipedia.org/wiki/BIOS_interrupt_call).

Sistem operasi yang dibuat mencoba untuk mengikuti fitur-fitur yang tersedia pada sistem operasi **DOS**, penjelasan
menarik terkait hal ini dapat dicek pada bagian [fun fact DOS](#dos-dan-sistem-operasi). Fitur yang biasanya ada pada
DOS adalah interrupt `0x21` yang memiliki banyak sekali fitur yang dapat digunakan pengguna sistem operasi.
Penjelasan lebih detail terkait interrupt `0x21` akan dijelaskan pada `void makeInterrupt21()` dan
`void handleInterrupt21()`.


![interrupt asm](other/markdown-img/milestone-1/interrupt.jpg)

Implementasi assembly `interrupt()` secara singkat merupakan wrapper dari instruksi `INT` dan memindahkan
parameter-parameter ke register yang bersangkutan. Instruksi `INT` meminta sebuah angka tetap sehingga diperlukan
**self-modifying code**, informasi tersebut dapat dicek pada bagian [fun fact](#self-modifying-code). Hasil interrupt
yang dikembalikan pada register `AX` akan dihapus `AH` agar hanya mengembalikan nilai `AL` pada instruksi `mov ah, 0`.

---

`void makeInterrupt21()` dan `void interrupt21ServiceRoutine()`

Prosedur ini memiliki tujuan untuk memasang address `interrupt21ServiceRoutine()` pada **interrupt vector table**
 dengan lokasi relatif `0x21` terhadap tabel. Interrupt vector table `0x21` akan diisi dengan address
 `interrupt21ServiceRoutine()` yang akan dipanggil ketika `INT 0x21` terjadi.


![makeinterrupt asm](other/markdown-img/milestone-1/make-interrupt.jpg)

![interruptroutine asm](other/markdown-img/milestone-1/interrupt-21-routine.jpg)

Implementasi `makeInterrupt()` secara singkat akan menyiapkan address `interrupt21ServiceRoutine()` dan memasukkan pada
interrupt vector table. `interrupt21ServiceRoutine()` akan mengambil register-register dan memasukkannya sebagai argumen
pemanggilan fungsi `handleInterrupt21()`.



<br/>
<br/>
<br/>
<br/>
<br/>

## Fun fact
### Real mode dan protected mode
<!-- TODO : Add https://wiki.osdev.org/Real_Mode#Information -->


<br/>
<br/>

### DOS dan sistem operasi
Sistem operasi yang dibuat merupakan sebuah klon sederhana dari **DOS** (Disk operating system). Sistem operasi ini
dijalankan pada **1.44 MB floppy disk**. Pada pengkonfigurasian `bochs` sebelumnya mengganti `floppya: 1_44=out/mangga.img`
yang memberitahukan kepada emulator `bochs` untuk memasukkan 1.44 MB floppy disk `mangga.img` ke mesin.


<br/>
<br/>

### Self-modifying code
<!-- TODO : Add -->
