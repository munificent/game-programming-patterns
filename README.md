## Начало работы

После `git clone` добавьте оригинальный репозиторий командой:

    $ git remote add upstream git@github.com:munificent/game-programming-patterns.git

После этого командой `git merge --no-commit upstream/master` можно будет накатить изменения из оригинала. При этом переведенные места придется смержить, таким образом конфликты будут видны на ладони.

## Разное

Было несколько правок в скрипт `script/format.py` связанных с русским текстом в файлах. А также странный баг с зависанием markdown-а, когда строчка начинается с "<a ...". Все это было поправлено в `master`

## Другие документы

В  [этом документе](https://docs.google.com/spreadsheets/d/1SqVZ-lTmtaQByOD17ZrEGPhBuTIM-IUfAtIcBE-D8d0) можно посмотреть текущий список глав и их статус, и контакты людей, работающих над проектов.

Словарик находится там же. Для критики и замечаний - можно использовать [issues](https://github.com/ol-loginov/game-programming-patterns/issues).

