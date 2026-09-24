# inventory windows

***
###### 05-08-2026
 + добавил серийный номер материнской платы \

###### 09-07-2026
 - переделал makefile, теперь очищается экран, добавил цвета \
 - переделал обработчик удаления ненужной информации из строки get_prn()
```cpp
    // Удаляем всё начиная с '('
    size_t pos = id_prn.find('(');
    if (pos != std::string::npos) {
        id_prn.erase(pos);
    }

    // Удаляем всё начиная с " PCL" (если есть)
    pos = id_prn.find(" PCL");
    if (pos != std::string::npos) {
        id_prn.erase(pos);
    }
```
[+] в added.cpp добавил **полезную** функцию remove_substring(src, "..."), удаляет из строки подстроку

###### 08-07-2026
[~] переделал все, что касается цветов \
[~] дополнил вывод ошибочных данных красным цветом, правильных - ярко белым \
[+] добавился файл **colors.h**
[+] теперь дополнился определением принтера по умолчанию (работает!!!)


[x] [Права доступа ролей](https://docs.gitflic.ru/common/manage_roles)
[~] [Вебхуки](https://docs.gitflic.ru/common/webhook)
[-] [Вебхуки](https://docs.gitflic.ru/common/webhook)
[+] [Вебхуки](https://docs.gitflic.ru/common/webhook)
