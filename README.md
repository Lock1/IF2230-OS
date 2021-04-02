# Tugas Besar - IF2230 - Sistem Operasi
**TBA** \
Repository ini akan digunakan sebagai duplikat public dari repository private yang terdapat pada GitHub Classroom informatika19 \
Log : 3 April 2021, Upload v2.0.0

## Instruksi menjalankan
1. `chmod +x build-run.sh get-tools.sh run.sh`
2. Jika menjalankan untuk pertama kali, pastikan semua tools ada atau jalankan `./get-tools.sh`
3. Jika fscreate belum ada di folder other, jalankan `make filesystemcreator`
4. Jalankan command `./build-run.sh` untuk mengcompile & link seluruh file yang dibutuhkan dan akan menjalankan Bochs
5. Jika hanya ingin menjalankan gunakan `./run.sh`

## Program loadFile
Untuk program loadFile, gunakan `make fileloader` untuk membuat program, program akan terletak pada
folder other dengan nama **loadFile**.

Program menerima argumen pertama sebagai direktori target dan argumen kedua sebagai input file.
Status load file akan ditulis pada akhir program.

## Bonus
Semua bonus berhasil untuk diimplementasikan dalam milestone ini, meliputi:
1. Symbolic link (ln) dalam mode soft link dengan menggunakan flag "-s"
2. Autocomplete file yang tersedia untuk cd, ls, dan cat
3. Autocomplete folder saat melakukan cd
4. Shell memiliki history (4 command)

## Reference
1. Silberschatz, Galvin, Gagne. "Operating System Concepts", Chapter 10.
2. Asisten Sistem Terdistribusi. "Milestone 2 - 2021".
3. stanislavs.org/helppc/int_13.html
