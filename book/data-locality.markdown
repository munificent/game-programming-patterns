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

## Sample Code

If you really go down the rathole of optimizing for data locality, you'll discover countless ways to slice and dice your data structures into pieces your CPU can most easily digest. To get you started, I'll show an example for each of a few of the most common ways to organize your data. We'll cover them in the context of some specific part of a game engine, but (as with other patterns), keep in mind that the general technique can be applied anywhere it fits.

### Contiguous arrays

Let's start with a <a href="game-loop.html" class="pattern">Game Loop</a> that processes a bunch of game entities. Those entities are decomposed into different domains -- AI, physics, and rendering -- using the <a href="component.html" class="pattern">Component</a> pattern. Here's the game entity class:

^code game-entity

Each component has a relatively small amount of state, maybe little more than a few vectors or a matrix, and then a method to <span name="update">update</span> it. The details aren't important here, but imagine something roughly along the lines of:

<aside name="update">

As the name implies, these are examples of the <a href="update-method.html" class="pattern">Update Method</a> pattern. Even `render()` is this pattern, just by another name.

</aside>

^code components

The game maintains a big array of pointers to all of the entities in the world. Each spin of the game loop, we need to run the following, in this order:

1. Update the AI components for all of the entities.

2. Update the physics components for them.

3. Render them using their render components.

Lots of game engines implement that like so:

^code game-loop

Before you ever heard of a CPU cache, this looked totally innocuous. But by now you've got an inkling that something isn't right here. This code isn't just thrashing the cache, it's taking it around back and beating it to a pulp. Watch what it's doing:

1. The array of game entities is *pointers* to them, so for each element in the array, we have to traverse that pointer. That's a cache miss.

2. Then the game entity has a pointer to the component. Another cache miss.

3. Then we update the component.

4. Now we go back to step one for *every component of every entity in the game*.

The scary part is we have no idea how any of these objects are laid out in memory. We're completely at the mercy of the memory manager. As entities get allocated and freed over time, the heap is likely to become increasingly randomly organized.

<span name="lines"></span>

<img src="images/data-locality-pointer-chasing.png" />

<aside name="lines">

Every frame, the game loop has to follow all of those arrows to get to the data it cares about.

</aside>

If our goal was to take a whirlwind tour around the game's address space like some "256MB of RAM in Four Nights!" cheap vacation package, this would be a fantastic deal. But our goal is to run the game quickly, and <span name="chase">traipsing</span> all over main memory is *not* the way to do that. Remember that `sleepFor500Cycles()` function? Well this code is effectively calling that *all the time*.

<aside name="chase">

The term for wasting a bunch of time traversing pointers is "pointer chasing", which it turns out is nowhere near as fun as it sounds.

</aside>

Let's do something better. Our first observation is that the only reason we follow a pointer to get to the game entity is so we can immediately follow *another* pointer to get to a component. `GameEntity` itself has no interesting state and no useful methods. The *components* are what the game loop cares about.

Instead of a giant constellation of game entities and components scattered across the inky darkness of address space, we're going to get back down to Earth. We'll have a big array for each type of component: a flat array of AI components, another for physics, and another for rendering.

Like this:

<span name="long-name"></span>

^code component-arrays

<aside name="long-name">

My least favorite part about using components is how long the word "component" is.

</aside>

Let me just stress that these are arrays of *components* and not *pointers to components*. The data is all there, one byte after the other. The game loop can then walk these directly:

<span name="arrow"></span>

^code game-loop-arrays

<aside name="arrow">

One hint that we're doing better here is how few `->` operators there are in the new code. If you want to improve data locality, look for indirection operators you can get rid of.

</aside>

We've ditched all of that pointer chasing. Instead of skipping around in memory, we're doing a straight crawl through three contiguous arrays.

<img src="images/data-locality-component-arrays.png" />

This pumps a solid stream of bytes right into the hungry maw of the CPU. In my testing, this change made the update loop fifty *times* faster than the previous version.

Interestingly, we haven't lost much encapsulation here. Sure, the game loop is updating the components directly instead of going through the game entities, but it was doing that before to ensure they were processed in the right order. Even so, each component itself is still nicely encapsulated. It owns its own data and methods. We just changed the way it's used.

This doesn't mean we need to get rid of `GameEntity` either. We can leave it just as it is with pointers to its components. They'll just point into those arrays. This is still useful for other parts of the game where you want to pass around a conceptual "game entity" and everything that goes with it. The important part is that the performance critical game loop sidesteps that and goes straight to the data.

### Packed data

Say we're doing a particle system. Following the advice of the previous section, we've got all of our particles in a nice big contiguous array. Let's wrap it in a little <span name="pool">manager class</span> too:

<aside name="pool">

The `ParticleSystem` class is an example of an <a href="object-pool.html" class="pattern">Object Pool</a> custom built for a single type of object.

</aside>

^code particle-system

A rudimentary update method for the system just looks like this:

^code update-particle-system

But it turns out that we don't actually need to process *all* of the particles all the time. The particle system has a fixed-size pool of objects, but they aren't usually all actively twinkling across the screen. The easy answer is something like this:

^code particles-is-active

We give `Particle` a flag to track whether its in use or not. In the update loop, we <span name="branch">check</span> that for each particle. That loads the flag into the cache, along with all of that particle's other data. If the particle *isn't* active, then we just skip over that to the next one. The rest of the particle's data that we loaded into the cache is a waste.

The fewer active particles there are, the more we're skipping across memory. The more we do that, the more cache misses there are between actually doing useful work updating active particles. If the array is large and has *lots* of inactive particles in it, we're back to just thrashing the cache again.

Having objects in a contiguous array doesn't solve much if the objects we're actually processing aren't contiguous in it. If it's littered with inactive objects we have to dance around, we're right back to the original problem.

<aside name="branch">

Savvy low-level coders can see another problem here. Doing an `if` check for every particle can cause a *branch misprediction* and a *pipeline stall*. In modern CPUs a single "instruction" actually takes several clock cycles. To keep the CPU busy, instructions are *pipelined* so that the subsequent instructions start processing before the first one finishes.

To do that, the CPU has to guess which instructions it will be executing next. In straight line code, that's easy, but with flow control, it's harder. While it's executing the instructions for that `if`, does it guess that the particle is active and start executing the code for the `update()` call, or does it guess that it isn't?

To answer that, the chip does *branch prediction:* it sees which branches your code previously took and guesses that it will do that again. But when the loop is constantly toggling between particles that are and aren't active, that prediction fails.

When it does, the CPU has to ditch the instructions it had started speculatively processing (a *pipeline flush*) and start over. The performance impact of this varies widely by machine, but this is why you sometimes see developers avoid flow control in hot code.

</aside>

Given the title of this section, you can probably guess the answer. Instead of *checking* the active flag, we'll *sort* by it. We'll keep all of the active particles in the front of the list. If we know all of those particles are active, we don't have to check the flag at all.

We can also easily keep track of how many active particles there are. With this, our update loop turns into this thing of beauty:

^code update-particles

Now we aren't skipping over *any* data. Every byte that gets sucked into the cache is a piece of an active particle that we actually need to process.

Of course, I'm not saying you should actually quicksort the entire collection of particles every frame. That would more than eliminate the gains here. What we want to do is *keep* the array sorted.

Assuming the array is already sorted -- and it is at first when all particles are inactive -- the only time it can become *un*sorted is when a particle has been activated or deactivated. We can handle those two cases pretty easily. When a particle gets activated, we move it up to the end of the active particles by swapping it with the first *in*active one:

^code activate-particle

To deactivate a particle, we just do the opposite:

^code deactivate-particle

Lots of programmers (myself included) have developed allergies to moving things around in memory. Schlepping a bunch of bytes around *feels* heavyweight , compared to just assigning a pointer. But when you add in the cost of *traversing* that pointer, it turns out that our intuition is sometimes wrong. In <span name="profile">some cases</span>, it's cheaper to push things around in memory if it helps you keep the cache full.

<aside name="profile">

This is your friendly reminder to *profile* when making these kinds of decisions.

</aside>

There's a neat consequence of keeping the particles *sorted* by their active state: We don't need to store an active flag in each particle at all. It can be inferred by its position in the array and the `numActive_` counter. This makes our particle objects smaller, which means we can pack more in our cache lines, and that makes them even faster.

It's not all rosy, though. As you can see from the API, we've lost a bit of object-orientation here. The `Particle` class no longer controls its own active state. You can't just call some `activate()` method on it since it doesn't know it's index. Instead, any code that wants to activate particles needs access to the particle *system*.

In this case, I'm OK with `ParticleSystem` and `Particle` being tightly tied like this. I think of them as a single *concept* spread across two physical *classes*. It just means accepting the idea that particles are *only* meaningful in the context of some particle system. Also, in this case it's likely to be the particle system that will be spawning and killing particles anyway.

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
