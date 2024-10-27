Программа, которая для системы FAT16

• Распечатывает список файлов в корневом каталоге,

• Напротив каждого пишет атрибуты и время создания/изменения,

• Читает файл, сохранённый в образе FAT16, и печатает его в stdout

Как создать образ ФС:

1) Создаём файл нужного размера на основной ФС

`dd if=/dev/zero of=floppy.img bs=1M count=32`

2) `mkfs.fat -F 16 floppy.img`

Затем монтируем

`sudo mount -o loop,fat=16 -t vfat floppy.img floppy`

Добавляем файлы: перейдя под рута (sudo -i), `touch file.txt`

Записываем в них что-то, например `echo "123" > file.txt`

Компилируем и запускаем программу: `gcc main.c -o main` и `./main`
