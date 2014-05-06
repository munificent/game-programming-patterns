## Начало работы

После `git clone` добавьте оригинальный репозиторий командой:

    $ git remote add upstream git@github.com:munificent/game-programming-patterns.git

После этого командой `git merge --no-commit upstream/master` можно будет накатить изменения из оригинала. При этом переведенные места придется смержить, таким образом конфликты будут видны на ладони.

## Policy

В  [этом документе](https://docs.google.com/spreadsheets/d/1SqVZ-lTmtaQByOD17ZrEGPhBuTIM-IUfAtIcBE-D8d0) можно посмотреть текущий список глав и их статус, и контакты людей, работающих над проектов.
Словарик находится там же. Для критики и замечаний - можно использовать [issues](https://github.com/ol-loginov/game-programming-patterns/issues).

Изменения необходимо проводить через бранчи, которые затем попадут в master через pull request.

## Pull Request Policy

Для принятия pull request-a необходимо согласие людей, отмеченных в колонке Approval в [этом документе](https://docs.google.com/spreadsheets/d/1SqVZ-lTmtaQByOD17ZrEGPhBuTIM-IUfAtIcBE-D8d0).
В данном случае "согласие" - это комментарий в pull request-е.

## Разное

Было несколько правок в скрипт `script/format.py` связанных с русским текстом в файлах. А также странный баг с зависанием markdown-а, когда строчка начинается с "<a ...". Все это было поправлено в `master`
