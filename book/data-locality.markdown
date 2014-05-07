^title Data Locality
^section Низкоуровневая оптимизация

## Общая мысль

*Если расположить данные в памяти специальным образом, то это поможет кэшам процессора.*

## Предыстория

Нас обманули. Нам показывают графики, на которых скорость процессоров растёт все выше и выше, как будто закон Мура - это не просто историческое наблюдение, а какое-то непререкаемое правило. Не отрывая задниц от кресла, мы, программисты, наблюдаем, как наши программы волшебным образом ускоряются благодаря апгрейду.

Процессоры *ускорялись* (сейчас график больше похож на плато), но главные по железу забыли нам кое-что сказать. Конечно, мы можем *обработать* данные быстрее чем раньше, но мы не можем *получить* их быстрее.

<span name="legend"></span>

<img src="images/data-locality-chart.png" />

<aside name="legend">

Скорость процессора и доступа к памяти относительно уровня 1980-го года. Как видно, скорость процессоров растет как на дрожжах, а вот доступ к памяти плетётся далеко позади.

Данные взяты из "Computer architecture: a quantitative approach" John L. Hennessy, David A. Patterson, Andrea C. Arpaci-Dusseau by way of Tony Albrecht's "[Pitfalls of Object-Oriented Programming](http://research.scee.net/files/presentations/gcapaustralia09/Pitfalls_of_Object_Oriented_Programming_GCAP_09.pdf)".

</aside>

Для того, чтобы сверхбыстрый процессор провернул свои вычисления, ему, вообще-то, нужно выгрузить данные из главной памяти в свои регистры. И как видите, память не так быстра, как скорость процессора. Даже близко не так.

С современным уровнем развития железа, может понадобиться *сотни* тактов, чтобы вытащить байт из <span name="ram">RAM</span>. Если большинство операций связано с данными, и нужно сотни тактов, чтобы их получить - как так получается, что наши процессоры не бездельничают 99% времени, ожидая поступления данных?

На самом деле, они *действительно* ждут памать удивительно большую часть своего времени, но все не так плохо, как это могло бы быть. Чтобы объяснить почему, давайте совершим сделаем длинное лирическое отступление...

<aside name="ram">

Она называется RAM ("Random access memory") потому что можно получить доступ к любому участку памяти одинаково быстро (это в теории). То есть не нужно считывать все байты по очереди, чтоб добраться до нужного места.

Ну, *обычно* не нужно. Как будет видно дальше, RAM не всегда настолько гибка, как кажется.

</aside>

### Хранилище данных

Представьте, что вы -- бухгалтер в маленьком офисе. Ваша работа: получить коробку документов и сделать с ними что-нибудь <span name="accountant">бухгалтерское</span>. Например, сложить какие-то числа. Это нужно сделать только с документами из некоторых коробок, в соответствии с какими-то тайными правилами, которые понятны только другим бухгалтерам.

<aside name="accountant">

Наверное, не стоило здесь использовать бухгалтерию -- я ничего в ней не понимаю.

</aside>

Благодаря тяжелой работе, природной упорности и стимуляторам, можно обработать всю коробку, скажем, за минуту. Но есть одна проблема - все эти коробки хранятся на складе в отдельном здании. Чтобы получить коробку, вам нужно попросить парня со склада принести её вам. Он идет, берет подъемник и ездит вдоль стеллажей, пока не найдет коробку, которая вам нужна.

Все это займет целый день, без шуток. В отличие от вас, парень со склада ещё не скоро станет работников месяца. Это значит, что неважно насколько вы быстры -- вы все рано получите не больше одной коробки в день. Все остальное время вы будете просто сидеть и думать о жизни, которая привела вас к такой бессмысленной работе.

И вот однажды, появилась группа специалистов. Их работа -- увеличивать эффективность работы. Придумывать маленькие хаки, которые позволяют сборочной линии двигаться быстрее. Они понаблюдали за вами несколько дней, и сделали несколько замечаний:

* Довольно часто, следущая коробка, которую вы попросите, находится <span name="next">на одной полке</span> с предыдущей.

* Использовать подъемник, чтобы снять маленькую коробку с полки -- довольно неэффективная идея.

* И в вашем офисе есть немного места в углу.

<aside name="next">

Эта ситуация (когда вы используете вещь, которая лежала рядом с той, которую вы только что использовали) в техническом мире называется *компактность ссылок*.

</aside>

Это дельный совет. Теперь, когда вы запрашиваете коробку у парня со склада, он приносит вам целую палету. Не только коробку, которая вам нужна, но и несколько соседних тоже. Он не знает, понадобятся ли они вам (вообще говоря, ему просто по барабану) -- он просто берет столько коробок, сколько может унести.

Итак, он загружает целую палету и несет её вам. Игнорируя правила безопасности, он въезжает на подъемнике прямо к вам в офис и сгружает все это прямо у вас в углу.

Когда вам нужна будет новая коробка, вы первым делом смотрите в углу -- возможно следущая коробка уже там. Если она действительно там, то вам повезло! Вам нужна всего секунда, чтобы взять новую коробку и занятся магией чисел. Если в куче 50 коробок, и вам так повезло, что *все* следующие коробки находятся в куче, то вы можете сделать в 50 раз больше работы чем раньше.

Но, если коробка, которая вам нужна, в куче *отсутствует*, вам все-таки придется её заказать. Поскольку в угол влезает только одна палета, то парень со склада приедет, заберет старую кучу и затем привезет вам совершенно новую кучу.

### Куча для процессора

Может показаться странным, но в современных компьютерах процессоры работают точно также. Офисный стол бухгалтера - это регистры процессора, а коробка с документами -- данные, которые можно положить в регистры. Склад -- это RAM вашего компьютера, а тот надоедливый парень со склада -- шина, которая наполняет регистры данными из памяти.

Если бы эта книга писалась тридцать лет назад, здесь история бы и закончилась. Но так как процессоры становятся быстрее, а RAM, грубо говоря, *нет*, инженеры начали искать решение. И они пришли к тому, что называется *кэшем процессора*.

В современных компьютеры если  <span name="caches">маленький объем</span> памяти прямо внутри процессора. Он может достать оттуда данные быстрее, чем из основной памяти. Этот объем достаточно мал, чтобы уместиться в процессоре, и довольно дорог, потому что используется особый, быстрый тип памяти (static RAM или "SRAM").

<aside name="caches">

Вообще, кэшей несколько. В зависимости от уровня они называются по-разному -- "L1", "L2", "L3" и так далее. Каждый уровень больше и медленнее предыдущего. В этой главе мы не будем углубляться в детали [иерархии памяти](http://ru.wikipedia.org/wiki/Иерархия_памяти), но это полезно знать.

</aside>

Этот маленький кусочек памяти называется *кэшем* (в частности, этот можно назвать *кэш L1*), и в нашей истории он эквивалентен куче коробок. Как только процессору понадобится байт памяти из RAM, он автоматически забирает целый кусок непрерывных данных -- обычно от 64 до 128 байт -- и кладет их в кэш. Эта порция памяти называется *кэш-линией*.

<img src="images/data-locality-cache-line.png" />

Если <span name="pallet">следующий байт</span> данных, который вам нужен, случайно окажется к этой линии, процессор прочитает его прямо из кэша, что *гораздо* быстрее чем обращение к RAM. Эту ситуацию, когда данные в кеше, называют *попаданием в кэш*. Если данных нет в кеше и приходится запрашивать главную память -- это *кэш-промах*.

<aside name="pallet">

Я приукрасил одну (по крайней мере) деталь в нашей истории. В офисе можно положить только одну кучу, то есть одну кэш-линию. Кэш процессора содержит несколько кэш-линий. Вы сможете утолить свое любопытство, поискав по слову "ассоциативность кэша".

</aside>

Когда случается кэш-промах, процессор *замирает*: он не может выполнить инструкцию, потому что ему нужны данные. Он просто стоит и скучает следующие несколько сотен тактов пока данные не появятся. Наша задача -- избежать этого. Допустим, мы хотит увеличить производительность важной части кода, который выглядит вот так:

^code do-nothing

Какой бы был ваш первый шаг? Правильно! Уберем этот бессмысленный и дорогой вызов функции. Вызов функции по стоимости эквивалентен кэш-промаху. Каждый раз вы выскакиваете в основную память -- это как вставить задержку в код.

### Разве данные -- это производительность?

Когда я начал работать над этой главой, я потратил время на примеры , которые показали бы хороший и плохой вариант использования кэша. Мне нужны были тесты, которые гоняли бы кэш, чтобы непосредственно увидеть все эффекты.

И когда я сделал часть работы, я был удивлен. Я знал, что кэш важен, но увидеть это своими глазами -- это нечто. <span name="ymmv">Я написал две программы</span>, которые делали *совершенно одинаковые* вещи. Единственное отличие было в том, как много кэш-промахов они получали. И худший вариант был в 50 *раз* медленнее, чем лучший.

<aside name="ymmv">

Здесь есть тонкости. В частности, разные компьютеры имеют разный кэш, поэтому мой комьютер может отличаться от вашего. А игровые консоли совсем отличны от дескторов, которые имеют много разного с мобильными устройствами.

Ваши результаты могут отличаться.

</aside>

Это по-настоящему открыло мне глаза. Я думал, на быстродействие влияет *код*, а не *данные*. Байт не бывает быстрым или медленным, он просто есть. Но из-за кэширования, *способ управления памятью прямо влияет на производительность*.

Теперь нужно все это применить к нашей теме. Оптимизация использования кэша -- довольно объемная тема. Я даже не упомянул о *кэшировании инструкций*. Вспомните -- код находится в памяти тоже, и его тоже нужно загрузить в процессор перед тем как выполнить. Кто-то более осведомленный может написать целую <span name="book">книгу</span> об этом.

<aside name="book">

На самом деле, кто-то *уже* написал книгу об этом:  [*Data-Oriented Design*](http://www.dataorienteddesign.com/dodmain/) от Ричарда Фабиана.

</aside>

Так как вы всё-таки читаете *эту* книгу, то я опишу несколько базовых вещей. Вместе с ними уже можно начать думать о том, как структуры данных влияют на производительность.

Все довольно просто: когда процессор считывать данные из памяти, он требует целую кэш-линию. Чем больше данных удасться <span name="line">положить в эту линию, тем быстрее все выполнится</span>. Так что надо *организовать ваши структуры так, чтобы вычисления оперировали с близко расположенными в памяти данными*.

<aside name="line">

Здесь есть ключевой момент:  контекст одного потока. Если вы модифицируете данные, используя несколько потоков, то лучше им находится в разных *кэш-линиях*. Если два потока попытаются изменить данные в одной и той же кэш-линии, обоим процессорам использовать синхронизацию кэшей, что довольно дорого.

</aside>

Другими словами, если ваш код использует `одно`, затем `другое`, потом `третье`, вам лучше расположить это в памяти как-то так:

<img src="images/data-locality-things.png" />

Заметьте, это не *указатели* на `одно`, `другое` и `третье`. Это настоящие данные, расположенные в одну линию. Как только процессор потребует `одно`, он сразу же получит и `другое`, и `третье` (вообще это зависит от того, насколько эти штуки большие, и от размера кэш-линии). Когда начнуться вычисления, данные уже будут в кэше. Ваш процессор счастлив, значит -- и вы счастливы.

## Общий подход

Современные **процессоры имеют кэш для ускорения доступа к памяти**. Доступ к соседним участкам памяти **осуществляется намного быстрее**. Этим можно воспользоваться **увеличивая компактность данных** -- хранить данные **непрерывно, в соответствии с порядком их обработки**.

## Когда этим пользоваться

Как и при большинстве оптимизаций, первым делом нужно убедится что *есть проблема с быстродействием*. Не тратьте время, пытаясь ускорить редко используемый участок кода. Преждевременная оптимизация усложнит вашу жизнь, так как результат почти всегда сложен и менее гибок.

Применительно к нашей теме, нужно убедиться, что быстродействие *действительно страдает от кэш-промахов*. Если код медленный по другим причинам, то лекарство не поможет.

В простом случае выследить проблему поможет инструментация. Она измерит время выполнения кода между двумя точками с использованием точного таймера. Для отлова кэш-промахов понадобиться что-то посложнее. Нужно увидеть, как много случилось промахов, и в каком месте.

К счастью, есть <span name="cachegrind">профайлеры</span>, которые могут помочь. Придется потратить время на настройку такого профайлера и научится воспринимать его (часто непростые) отчеты, перед тем как приступить к главному действу по оптимизации структур с данными.

<aside name="cachegrind">

К сожалению, такие профайлеры недешевы. Если вы в команде, которая работает с консолями, у вас скорее всего уже есть какая-нибудь лицензия.

Если нет, то если бесплатный [cachegrind](http://valgrind.org/docs/manual/cg-manual.html). Он выполняет программу в эмуляторе процессора и показывает все операции с кэшем.

</aside>

Как уже говорилось, кэш-промахи *повлияют* на быстродействие вашей игры. И все же не увлекайтесь тратой времени на преждевременную оптимизацию кэша. Просто помните при разработке архитектуры -- организовать структуры стоит так, чтобы они хорошо работали в кэше.

## Особенности

Отличительный признак архитектуры программы -- *абстрация*. Большая часть это книги рассказывает о том, как отвязать куски кода друг от друга так, чтобы они были легко изменяемы. В языках ООП это почти всегда означает интерфейсы.

В C++ использование интерфейсов предполагает доступ к объектам через <span name="virtual">указатели или ссылки</span>. Доступ через указатель означает прыжки в памяти, которые ведут к кэш-промахам -- а мы стараемся их избежать.

<aside name="virtual">

Другая особенность интерфейсов -- *виртуальные методы*. Для того чтобы вызвать такой метод, необходимо получить доступ к виртуальной таблице объекта и найти там указатель на функцию, которую необходимо вызвать. То есть, игра в указатели опять приводит к кэш-промахам.

</aside>

Для того, чтобы воспользоваться оптимизацией, придется пожертвовать частью абстракций. Чем больше вы концентрируете архитектуру программы вокруг компактности данных, тем меньше применения находится наследованию и интерфейсам -- и всем их сильным сторонам. Серебрянной пули здесь нет, угодить всем не удасться. Тем веселее!

## Примеры

Если заняться оптимизацией локальности данных, то можно придумать бесконечно много способов разложения ваши структур на кусочки, с которыми процессору будет удобно работать. Для общего понимания я покажу примеры основных реализаций этих идей. Мы рассмотрим их в контексте части игрового кода, но это общая техника, поэтому применить её возможно везде (как и все остальные паттерны).

### Непрерывные массивы

Начнем мы с <a href="game-loop.html" class="pattern">Game Loop</a>, который обрабатывает набор внутренних объектов. Эти объекты разделены на несколько областей -- AI, физика и рендеринг -- с использованием паттерна <a href="component.html" class="pattern">Component</a>. Вот класс объекта:

 ^code game-entity

Каждый компонент имеет относительный малый набор аттрибутов (не больше пары векторов или одной матрицы) и метод <span name="update">update</span>, который их обновляет. Детали не так важны здесь, просто представьте что-то вроде следующего:

<aside name="update">

Метод update -- это пример паттерна <a href="update-method.html" class="pattern">Update Method</a>.  Как и `render()`, просто с другим именем.

</aside>

^code components

Игра оперирует с большим массивом указателей на эти объекты. В каждой игровой итерации происходит следующее:

1. Обновляем AI для всех объектов.

2. Применяем к ним физику.

3. И отображаем все объекты с помощью рендера.

Большинство игр прямо так и делают:

^code game-loop

До того как вы узнали про кэш процессора, все выглядело абсолютно невинно. Но сейчас у вас возникло подозрение, что то-то делается неправильно. Код не использует кэш по полной, вместо этого он просто убивает его. Смотрите, что происходит:

1. Массив объектов -- это *указатели* на объекты. Когда мы образаемся к объекту, мы достаем данные по указателю. Это кэш-промах.

2. Затем мы достаем компонент из указателя в объекте. Ещё кэш-промах.

3. Затем мы вызываем метод компонента.

4. И возвращаемся к шагу 1, чтобы повторить его для *каждого компонента каждого игрового объекта*.

Страшная правда в том, что мы понятия не имеем как все эти объекты раскиданы по памяти. Этим распоряжается менеджер памяти. Если все время создавать и уничтожать объекты, то постепенно память заполнится данными в произвольном порядке.

<span name="lines"></span>

<img src="images/data-locality-pointer-chasing.png" />

<aside name="lines">

Движку приходится бегать по этим стрелками каждый игровой ход, чтобы достать данные.

</aside>

Это было бы здорово, если бы мы хотели "объехать всю память за 80 дней"! Но нам нужно сделать цикл быстро и <span name="chase">бродить</span> по всей памяти не наш метод. Помните функцию `sleepFor500Cycles()`? Ну так вот -- этот код использует её *постоянно*.

<aside name="chase">

Трата времени на разыменование указателей называется "блуждание по указателям", и это не так весело как кажется.

</aside>

Придумаем что-нибудь получше. С первого взгляда ясно, что указатель на объект нам нужен только для доступа к *другому* указателю на его компонент. Сам `GameEntity`нам не интересен и дает нам ничего полезного. *Компоненты* -- вот что нужно циклу.

Срубим под корень раскидистое дерево указателей на объекты! Нам нужно оформить по массиву на каждый тип компонента: один плоский массив для AI, один для физики и ещё один для рендера.

Как-то так:

<span name="long-name"></span>

^code component-arrays

<aside name="long-name">

Меньше всего в компонентах мне нравится длина самого слова "компонент".

</aside>

Подчеркну, что это массив *компонентов*, а не *указателей на компонент*. Все данные прямо здесь, один за другим. Игровой цикл может оперировать прямо с ними:

<span name="arrow"></span>

^code game-loop-arrays

<aside name="arrow">

Показатель того, что мы движемся в правильном направлении, это уменьшение количества "`->`" в коде. Если вам нужно улучшить компактность данных, ищите эти операторы и избавляйтесь от них.

</aside>

Итак, мы выбросили блуждание по указателям. Вместо метания по памяти, мы перебираем три непрерывных массива с прямыми данными.

<img src="images/data-locality-component-arrays.png" />

Теперь плотный поток байтов загружается прямо в жадное горло процессора. По результатам тестов, это изменение ускоряет цикл в тридцать раз по сравнению с предыдущим вариантом.

Интересно, что мы почти не потеряли в инкапсуляции. Конечно, игровой цикл оперирует с компонентами напрямую, не доставая их из объектов. Но мы можем достать их заранее, чтобы убедиться что мы все делаем правильно. Более того, каждый компонент все ещё надежно инкапсулирован. Мы просто изменили способ доступа к ним.

Это не значит, что `GameEntity` больше не нужен. Мы оставим его, так как он все ещё держит указатели на свои компоненты. Они просто будут указывать на элементы наших массивов. Сам концепт "игрового объекта" может пригодиться в других частях кода. Важно, что для производительности игровой цикл обходит этот момент и использует данные напрямую.

### Упаковка

Допустим, мы делаем систему частиц. Следуя совету из предыдущей секции, мы загрузили все частици в один большой непрерывный массив. Придумаем маленькую <span name="pool">систему частиц</span>:

<aside name="pool">

Класс `ParticleSystem` является примером паттерна <a href="object-pool.html" class="pattern">Object Pool</a> для одного типа объекта.

</aside>

^code particle-system

Его метод <code>update</code> выглядит следующим образом:

^code update-particle-system

Но реальность показала, что нам не нужно обрабатывать *все* частицы каждый раз. Система содержит фиксированный набор частиц, но не все они активны на экране:

^code particles-is-active

Мы дали `Particle` флажок, по которому определяем активна она или нет. В цикле мы <span name="branch">проверяем</span> его для каждой частицы. Флажок загружается в кэш вместе с целым объектом. И если частица *не активна*, мы просто переходим к следующей. Остальные аттрибуты частицы бесполезно загружаются в кэш.

Чем меньше активных частиц, тем больше частиц загружается в кэш вхолостую. Если массив большой и *большинство* частиц неактивно, то мы просто насилуем кэш без пользы.

Идея хранения объектов в непрерывном массиве не сильно помогает, так как набор частиц, с которым мы реально будем работать, не непрерывен. Массив замусорен неактивными частицами, которые нам надо пропускать, и мы возвращаемся к первоначальной проблеме.

<aside name="branch">

Смышленные программисты увидят здесь и другую проблему. Выполнение условия `if` для каждой частицы может привести к *сбою предсказания переходов* и *простою конвейера*.  В процессорах одна "инструкция" может занять несколько тактов. Чтобы процессор не простаивал, инструкции *распараллеливают*, чтобы следующая инструкция начала обрабатываться, не дожидаясь конца предыдущей.

Процессор пытается угадать, какая инструкция выполнится следующей. В линейном коде это просто, но условные переходы усложняют жизнь. Пока выполняется условие для `if`, какую часть кода брать следующей --  `update()` или ничего не делать?

 Для ответа на этот вопрос процессор использует *предсказание переходов*: он видит, по какому пути пошел в прошлый раз и предполагает, что пойдет по нему опять. И когда флажок постоянно переключается, это предсказание терпит сбой.

Когда происходит сбой, процессор выкидывает инструкции, которые он уже начал выполнять (*сброс конвейера*) и начинает заново. Влияние этого на производительность зависит от характеристик машины, но это та причина, по которой некоторые программисты пытаются избежать ветвлений в критических местах.

</aside>

Из названия секции, вы можете догадаться о решении этого вопроса. Вместо проверки *флага*, мы отсортируем по нему. Поместим все активные частицы в начало списка. Если мы будем знать, что эти частицы активны, проверка флага нам просто не нужна.

Мы можем хранить количество активных частиц. Тогда код превратится в следующую прекрасную штуку:

^code update-particles

Теперь мы не пропускаем *ни одного* лишнего байта. Каждый загружается в кэш для реальной работы с частицей.

Конечно, я не говорю, что вам нужно использовать быструю сортировку (quicksort) всего массива каждый раз. Это уничтожит все наши победы. Нам просто нужно *поддерживать* сортировку.

Предположим, что массив уже отсортирован -- и это действительно для случая, когда все частицы неактивны. Сортировка *теряется*, когда частица переходит из неактивного состояния в активное и обратно. Это легко можно отследить. Как только частица становится активной, мы передвинем её на место первой *неактивной* частицы:

^code activate-particle

При деактивации частицы, сделаем обратный ход:

^code deactivate-particle

Большинство (включая меня) выработали аллергию на копирование памяти. Перекидывание кучи байтов *выглядит* тяжелой операцией по сравнению с перемещением указателя. Но если вы прибавите к этому *разыменование* этого указателя, выяснится, что интуиция иногда обманывает. В <span name="profile">некоторых случаях</span>, дешевле перекинуть данные в памяти, если это поможет операциям с кэшем.

<aside name="profile">

Это тонкий намек на пользу *профайлера* при принятии решений подобного рода.

</aside>

Есть очевидное преимущество от хранения частиц *отсортированными* по активности: теперь не нужен флаг. Он заменяется позицией частицы в массиве и счетчиком `numActive_`. Из-за этого класс частицы становиться чуточку меньше, значит их больше поместится в кэш-линию, и все завертится ещё быстрее.

Ести и минусы, конечно. Мы потеряли немного объектно-ориентированности здесь. Класс `Particle` больше не управляет своим состоянием активности. Вы не сможете сделать у частицы метод `activate()`, так как она просто не знает свой индекс. Вместо этого придется иметь доступ к *системе частиц*.

В данном случае, я считаю нормальным, что `ParticleSystem` и `Particle` так жестко связаны между собой. Решая нашу главную задачу, пришлось вылезти за границы одного класса. И, скорее всего, именно система будет решать когда создавать частицы и когда их убивать.

### Hot/cold splitting

OK, this is the last example of a simple technique for making your cache happier. Say we've got an AI component for some game entity. It has some state in it: the animation it's currently playing, a goal position its heading towards, energy level, etc. -- stuff it checks and tweaks every single frame. Something like:

^code ai-component

But it also has some state for rarer eventualities. It stores some data describing what loot it drops when it has an unfortunate encounter with the noisy end of a shotgun. That drop data is only used once in the entity's lifetime, right at its bitter end.

^code loot-drop

Assuming we followed the earlier patterns, when we update these AI components, we walk through a nice packed, contiguous array of data. But that data includes all of the loot drop information. That makes each component bigger, which reduces the number of them we can fit in a cache line. We get more cache misses because the total memory we walk over is larger. The loot data gets pulled into the cache for every component, every frame, even though we aren't even touching it.

The solution for this is called "hot/cold splitting". The idea is to break our data structure into two separate pieces. The first holds the "hot" data: the state we need to touch every frame. The other piece is the "cold" data: everything else that gets used less frequently.

The hot piece is the *main* AI component. It's the one we need to use the most, so we don't want to chase a pointer to find it. The cold component can be off to the side, but we still need to get to it, so we give the hot component a pointer to it, like so:

^code hot-cold

Now when we're walking the AI components every frame, the only data that gets loaded into the cache is stuff we are actually processing (with the <span name="parallel">exception</span> of that one little pointer to the cold data).

<aside name="parallel">

We could conceivably ditch the pointer too by having parallel arrays for the hot and cold components. Then we can find the cold AI data for a component since both pieces will be at the same index in their respective arrays.

</aside>

You can see how this starts to get fuzzy, though. In my example here, it's pretty obvious which data should be hot and cold, but it's rarely so clear cut. What if you have fields that are used when an entity is in a certain mode but not in others? What if entities use a certain chunk of data only when they're in certain parts of the level?

Doing this kind of optimization is somewhere between a black art and a rathole. It's easy to get sucked in and spend endless time pushing data around to see what speed difference it makes. It will take practice to get a handle on where to spend your effort.

## Design Decisions

This pattern is really about a mindset: it's getting you to think about your data's arrangement in memory as a key part of your game's performance story. The actual concrete design space is wide open. You can let <span name="dod">data locality</span> affect your whole architecture, or maybe it's just a localized pattern you apply to a few core data structures.

The biggest question you'll need to answer is when and where you apply this pattern, but here are a couple of others that may come up.

<aside name="dod">

Noel Llopis' [famous article](http://gamesfromwithin.com/data-oriented-design) that got a lot more people thinking about designing games around cache usage calls this "data-oriented design".

</aside>

### How do you handle polymorphism?

Up to this point, we've avoided subclassing and virtual methods. We assumed we have nice packed arrays of *homogenous* objects. That way, we know they're all the exact same size. But polymorphism and dynamic dispatch are useful tools, too. How do we reconcile this?

 *  **Don't:**

    The <span name="type">simplest</span> answer is to just avoid subclassing, or at least avoid it in places where you're optimizing for cache usage. Software engineer culture is drifting away from heavy use of inheritance anyway.

    <aside name="type">

    One way to keep much of the flexibility of polymorphism without using subclassing is through the <a href="type-object.html" class="pattern">Type Object</a> pattern.

    </aside>

    * *It's safe and easy.* You know exactly what class you're dealing with and all objects are obviously the same size.

    * *It's faster.* Dynamic dispatch means looking up the method in the vtable and then traversing that pointer to get to the actual code. While the cost of this varies widely across different hardware, there is <span name="cpp">*some*</span> cost to dynamic dispatch.

    <aside name="cpp">

    As usual, the only absolute is that there are no absolutes. In most cases, a C++ compiler will require an indirection for a virtual method call. But in *some* cases, the compiler may be able to do *devirtualization* and statically call the right method if it knows what concrete type the receiver is. Devirtualization is more common in just-in-time compilers for languages like Java and JavaScript.

    </aside>

    * *It's inflexible.* Of course, the reason we use dynamic dispatch is because it gives us a powerful way to vary behavior between objects. If you want different entities in your game to have their own rendering styles, or their own special moves and attacks, virtual methods are a proven way to model that. Having to instead stuff all of that code into a single non-virtual method that does something like a big `switch` gets messy quickly.

 *  **Use separate arrays for each type:**

    We use polymorphism so that we can invoke behavior on an object whose type we don't know. In other words, we have a mixed bag of stuff and we want each object in there to do its own thing when we tell it to go.

    But that just raises the question of why mix the bag to begin with? Instead, why not just maintain separate homogenous collections for each type?

    * *It keeps objects tightly packed.* Since each array only contains objects of one class, there's no padding or other weirdness.

    * *You can statically dispatch.* Once you've got objects partitioned by type, you don't actually need polymorphism at all any more. You can use regular non-virtual method calls.

    * *You have to keep track of a bunch of collections.* If you have a lot of different object types, the overhead and complexity of maintaining separate arrays for each can be a chore.

    * *You have to be aware of every type*. Since you have to maintain separate collections for each type, you can't be decoupled from the *set* of classes. Part of the magic of polymorphism is that it's *open-ended*: code that works with an interface can be completely decoupled from the potentially large set of types that implement that interface.

 *  **Use a collection of pointers:**

    If you weren't worried about caching, this is the natural solution. Just have an array of pointers to some base class or interface type. All the polymorphism you could want, and objects can be whatever size they want.

    * *It's flexible.* The code that consumes the collection can work with objects of any type as long as it supports the interface you care about. It's completely open-ended.

    * *It's less cache-friendly.* Of course, the whole reason we're discussing other options here is because this means cache-unfriendly pointer indirection. But, remember, if this code isn't performance critical, that's probably OK.

### How are game entities defined?

If you use this pattern in tandem with the <a href="component.html" class="pattern">Component</a> pattern, you'll have nice contiguous arrays for all of the components that make up your game entities. The game loop will be iterating over those directly, so the object for the game entity itself is less important, but it's still useful in other parts of the codebase where you want to work with a single conceptual "entity".

The question then is how should it be represented? How does it keep track of its components?

 * **If game entities are classes with pointers to their components:**

    This is what our first example looked like. It's sort of the vanilla OOP solution. You've got a class for `GameEntity`, and it has pointers to the components it owns. Since they're just pointers, it's agnostic about where and how those components are actually organized in memory.

    * *You can store components in contiguous arrays.* Since the game entity doesn't care where its components are, you can organize them in a nice packed array to optimize iterating over them.

    * *Given an entity, you can easily get to its components.* They're just a pointer indirection away.

    * *Moving components in memory is hard.* When components get enabled or disabled, you may want to move them around in the array to keep the active ones up front and contiguous. If you move a component while the entity has a raw pointer to it, though, that pointer gets broken if you aren't careful. You'll have to make sure to update the entity's pointer at the same time.

 *  **If game entities are classes with IDs for their components:**

    The challenge with raw pointers to components is that it makes it harder to move them around in memory. You can address that by using something more abstract: an ID or index that can be used to *look up* a component.

    The actual semantics of the ID and lookup process are up to you. It could be as simple as storing a unique ID in each component and walking the array, or more complex like a hash table that maps IDs to their current index in the component array.

    * *It's more complex.* Your ID system doesn't have to be rocket science, but it's still more work than a basic pointer. You'll have to implement and debug it, and there will be memory overhead for bookkeeping.

    * *It's slower*. It's hard to beat traversing a raw pointer. There may be some actual searching or hashing involved to get from an entity to one of its components.

    * *You'll need access to the component "manager".* The basic idea is that you have some abstract ID that identifies a component. You can use it to get a reference to the actual component object. But to do that, you need to hand that ID to something that can actually find the component. That will be the class that wraps your raw contiguous array of component objects.

        With raw pointers, if you have a game entity, you can find its components. With this, you <span name="singleton">need</span> the game entity *and the component registry too*.

        <aside name="singleton">

        You may be thinking, "I'll just make it a singleton! Problem solved!" Well, sort of. You might want to check out <a href="singleton.html">the chapter</a> on those first.

        </aside>

 *  **If the game entity is *itself* just an ID:**

    This is a newer style that some game engines use. Once you've moved all of your entity's behavior and state out of the main class and into components, what's left? It turns out, not much. The only thing an entity does is bind a set of components together. It exists just to say *this* AI component and *this* physics component and *this* render component define one living entity in the world.

    That's important because components interact. The render component needs to know where the entity is, which may be a property of the physics component. The AI component wants to move the entity, so it needs to apply a force to the physics component. Each component needs a way to get the other sibling components of the entity it's a part of.

    Some smart people realized all you need for that is an ID. Instead of the entity knowing its components, the components know their entity. Each component knows the ID of the entity that owns it. When the AI component needs the physics component for its entity, it just asks for the physics component with the same entity ID that it holds.

    Your entity *classes* disappear entirely, replaced by a glorified wrapper around a number.

    * *Entities are tiny.* When you want to pass around a reference to a game entity, it's just a single value.

    * *Entities are empty.* Of course, the downside of moving everything out of entities is that you *have* to move everything out of entities. You no longer have a place to put non-component-specific state or behavior. This style
    doubles down on the component pattern.

    * *You don't have to manage their lifetime.* Since entities are just dumb value types, they don't need to be explicitly allocated and freed. An entity implicitly "dies" when all of its components are destroyed.

    * *Looking up a component for an entity may be slow.* This is the same problem as the previous answer, but in the opposite direction. To find a component for some entity, you have to map an ID to an object. That process may be costly.

        This time, though, it *is* performance critical. Components often interact with their siblings during update, so you will need to find components frequently. One solution is to make the "ID" of an entity just the index of the component in its array.

        If every entity has the same set of components, then your component arrays are completely parallel. The component in slot three of the AI component array will be for the same entity that the physics component in slot three of *its* array is associated with.

        Keep in mind, though, that this *forces* you to keep those arrays in parallel. That's hard if you want to start sorting or packing them by different criteria. You may have some entities with disabled physics and others that are invisible. There's no way to sort the physics and render component arrays optimally for both cases if they have to stay in sync with each other.

## See Also

* Much of this chapter revolves around the <a href="component.html" class="pattern">Component</a> pattern, and those are definitely one of the most common data structures that get optimized for cache usage. In fact, using the component pattern makes this optimization easier. Since entities are updated one "domain" (AI, physics, etc.) at a time, splitting them out into components lets you slice a bunch of entities into just the right pieces to be cache-friendly.

    But that doesn't mean you can *only* use this pattern with components! Any time you have performance critical code that touches a lot of data, it's important to think about locality.

* Tony Albrecht's <a href="http://research.scee.net/files/presentations/gcapaustralia09/Pitfalls_of_Object_Oriented_Programming_GCAP_09.pdf" class="pdf">"Pitfalls of Object-Oriented Programming"</a> is probably the most widely-read introduction to designing your game's data structures for cache-friendliness. It made a lot more people (including me!) aware of how big a deal this is for performance.

* Around the same time, Noel Llopis wrote a [very influential blog post](http://gamesfromwithin.com/data-oriented-design) on the same topic.

* This pattern almost invariably takes advantage of a contiguous array of homogenous objects. Over time, you'll very likely be adding and removing objects from that array. The <a href="object-pool.html" class="pattern">Object Pool</a> pattern is about exactly that.

* The [Artemis](http://gamadu.com/artemis/) game engine is one of the first and better-known frameworks that uses simple IDs for game entities.
