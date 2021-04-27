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

Penulis menggunakan [WSL2](https://docs.microsoft.com/en-us/windows/wsl/about) dalam host OS Windows 10
dalam pengerjaan tugas ini, troubleshooting masalah instalasi dan cara instalasi WSL2 dapat dilihat pada markdown
[WSL-OS-Troubleshooting.md](other/WSL-OS-Troubleshooting.md).

Untuk pengguna selain distribusi linux dan tidak ingin memasang sistem operasi baru pada mesin yang dimiliki
dapat menggunakan [virtual machine](https://en.wikipedia.org/wiki/Virtual_machine) seperti
[Oracle VirtualBox](https://www.virtualbox.org/) atau [VMWare](https://www.vmware.com/).
Instruksi untuk memasang virtual machine sudah cukup banyak dan mudah diakses dengan mencari
`Cara install virtualbox dan ubuntu` pada search engine.



### 2. Pemasangan alat-alat
![Install tools embedded](other/markdown-img/milestone-1/installing-tools-command.jpg)

![Installing update](other/markdown-img/milestone-1/apt-update.jpg)

Jalankan command diatas pada terminal, jika menggunakan distribusi non-Ubuntu yang tidak menggunakan
package manager `apt`, gunakanlah package manager yang sesuai (contoh `apk` untuk Alpine).
Instalasi alat-alat dapat dimasukkan kedalam script jika ingin mempermudah pengguna lain,
cek pembuatan script pada section [tambahan script tools](#tambahan)

Setelah alat telah didownload dan install, buka dan unzip
[kit-1.zip](original-milestone/other/rules and guidelines/kit-1.zip) pada suatu lokasi.



### 3. Persiapan disk image
**TBA**








---

## Tambahan
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
<!--
Add later
https://wiki.osdev.org/Real_Mode#Information
-->
