^title Грязный флаг
^section Низкоуровневая оптимизация

## Общая мысль

*Избегайте ненужной работы. Когда понадобиться -- тогда и выполните.*

## Предыстория

"Флаг" и "бит" являются практически синонимами в программировании: они оба могут находится только в оодном и двух состояний. Можно назвать их "правда" или "ложь", иногда "установлен" и "очищен". Я буду использовать оба варианта. Так что текущую главу можно было назвать <span name="specific">"Грязный бит"</span>, но я решил использовать более нейтральный вариант.

<aside name="specific">

Авторы википедии менее щепетильны, чем я, и выбрали [грязный бит](http://en.wikipedia.org/wiki/Dirty_bit).

</aside>

### Как найти лодку в море

Множество игр занимаются построение *графа сцены*. Это такая большая структура, в которой находятся все объекты игры. Рендерер использует его, чтобы определить что где рисовать.

В простом случае, граф -- это плоский список объектов. У каждого объекта есть модель или другой графический примитив, и <span name="transform">*матрицу трансформации*</span>. Трансформация описывает координаты объекта, поворот и масштабирование. Чтобы передвинуть или повернуть объект, мы просто меняем его трансформацию.

<aside name="transform">

Детали того, *как* хранить трансформацию, и как ей манипулировать, выходят за рамки разговора, к сожалению. Смешно сказать, но это действительно матрица 4 на 4. Чтобы применить две трансформации к объекту -- например, передвинуть и потом повернуть -- достаточно перемножить две матрицы. 

Почему и как это работает -- домашнее задание для читателей.

</aside>

Когда рендерер рисует объект, он берет модель объекта, накладывает на него транформацию, и затем рисует эту модель. И если не вспоминать про *граф* сцены, то на этом все заканчивается и жизнь становится проста, как два пальца.  

К сожалению, большинство графов будут иметь <span name="hierarchical">*иерарархию*</span>. У объекта в графе есть родитель, к котором он прикреплен. В этом случае, на трансформацию объекта надо ещё наложить трансформацию родителя, чтобы получить правильные координаты при рисовании.

Допустим, мы рисуем пиратский корабль в море. Пусть на верху мачты есть бочка, в ней сидит пират, и на плече у него -- попугай. Трансформация корабля описывает его положение в море. Трансформация бочки описывает её положение на корабле, и так далее. 

<span name="pirate"></span>
<img src="images/dirty-flag-pirate.png" />

<aside name="pirate">

Осторожно, рисовал программист!

</aside>

Когда родительский объект двигается, автоматически двигаются и все его дети. И если мы поменяем локальную трансформацию корабля, то бочка, старый пират и попугай должны передвинуться вместе с ним. И было бы <span name="slide">глупо</span>, если бы мы вручную меняли трансформации всех объектов, которые находятся на корабле, чтобы ничего не разъехалось.    

<aside name="slide">

Честно говоря, когда вы в море, то *приходится* самому двигаться, чтобы не вывалится за борт. Может быть стоит придумать более честный пример.

</aside>

Но когда мы непосредственно приступили к рисованию попугая, нам нужны его абсолютные координаты. Будем называть *локальной трансформацией* трансформацию относительно родительского объекта. А для того чтобы рисовать, будем вычислять абсолютную, или *мировую трансформацию*.
 
### Локальная и мировая трансформация

Сделать это несложно: надо подниматься по цепочке родителей вверх до самого конца, а затем спускаться обратно, накладывая трансформации друг на друга поочередно. Тогда мировая трансформация попугая будет:

<span name="degenerate"></span>
<img src="images/dirty-flag-multiply.png" />

<aside name="degenerate">

В частном случае, когда у объекта нет родителя, его локальная и мировая трансформация -- совпадают.

</aside>

Нам нужна эта мировая трансформация каждую отрисовку, так что операции с матрицами могут стать занозой в заднице. Поддерживать их в актуальном состоянии непросто, потому что когда родительский объект двигается, это влияет и на него, и на всех его детей рекурсивно. 

Очевидный вариант -- это вычислять трансформации прямо при рисовании. Каждую отрисовку мы будем спускаться по графу, начиная с самого верха, и вычислять мировую трансформацию для каждого объекта. А потом рисовать его.

Этот вариант ужасен до невозможности. На практике, большинство объектов не передвигаются -- вспомните про статические объекты, из которых состоит уровень. Нет необходимости делать лишнюю работу и вычислять их координаты каждый раз.

### Используем кэш

Очевидный подход -- кэшировать трансформации. В каждом объекте мы будем хранить локальную и мировую трансформацию. При отрисовке мы используем мировую трансформацию, которая уже вычислена. И если объект не двигается, то она всегда актуальна -- и все довольны.

Что делать, если объект *передвинули*? Можно сразу же пересчитать мировую трансофрмацию. Только у нас же иерархия! Когда двигается родитель, нужно пересчитать его трансформацию и *всех его детей, рекурсивно*.

Представьте, что у нас довольно сложная игра. В каждом цикле корабль толкают волны, бочка раскачивается на ветру, пирата постоянно тошнит, а попугай прыгает через голову. И тут мы меняем четыре локальных трансформации. Если мы сразу же начнем пересчитывать мировые, то во что это выльется?

<span name="stars"></span>
<img src="images/dirty-flag-update-bad.png" />

<aside name="stars">

Если посмотреть на линии, отмеченные &#x2605;, то можно увидеть, что мы *четыре* раза пересчитывали попугая, хотя будем использовать только результаты последнего пересчета.

</aside>

Мы всего лишь передвинули четыре объекта, а пришлось пересчитать *десять* мировых трансформаций. Шесть бесполезных вычислений будут выкинуты, потому что их результат нам не интересен. Мы четыре раза пересчитали попугая, хотя он был нарисован всего один раз.

Проблемы в том, что мировая трансформация зависит от нескольких локальных. Мы начинали перерасчет сразу после того, как изменилась хотя бы одна из них. В результат пришлось пересчитать мировую трансформацию для одного и того же объекта столько раз, сколько локальных трансформаций родителей изменилось.

### Отложенные вычисления

Мы можем решить эту проблему, если <span name="decoupling">разделим</span> обновление локальной и мировой трансформации. Это позволит изменять набор локальных трансформаций одних махом, и *затем* пересчитать те мировые трансформации, которые это затронуло, прямо перед отрисовкой.

<aside name="decoupling">

Занимательно -- столько архитектур основано на маленьких хитростях!

</aside>

Мы добавим флажок к каждому объекту в графе. Когда будет менятся локальная трансформация, мы его выставим. И когда нам понадобиться мировая, мы будем проверять этот флаг. Если он выставлен, то мы пересчитаем мировую трансформацию и очистим флаг. Этот флаг как бы говорит нам "нужно обновлять матрицы?" По причинам, которые не совсем ясны, обычное название для таких "уже неточных" данных -- это "грязные" (dirty). Вот и получился *грязный флаг*.

Если мы применим этот подход, и повторим пример с четырьмя объектами, то в результате увидим:

<img src="images/dirty-flag-update-good.png" />

Это лучшее, на что можно было бы надется: мировая трансформация для каждого тронутого объекта вычисляется только один раз. С помощью одного бита, это паттерн сделал несколько вещей:

1. Множество изменений локальных трансформаций он превратил в одно вычисление для каждого объекта.
2. Избавляет от необходимости пересчитывать статические объекты.
3. И мини-бонус: если объект удалили до того, как он был нарисован, то у него ничего не вычисляется вообще.

## Общий подход

Есть набор **основных данных**, который меняется постоянно. И есть **вторичные данные**, которые вычисляются из основных с помощью некоторого **дорогого процесса**. **Грязный флаг** обозначает актуальность вторичных данных и **их соответствие основным**. Когда запрашиваются вторичные данные -- проверяется флаг.  Если флаг поднят, необходимо **рассчитать вторичные данные заново и очистить флаг**. Если опущен -- можно использовать закэшированные вторичные данные, рассчитаные ранее.

## Когда это использовать

По сравнению с другими паттернами в этой книге, этот решает довольно специфическую задачу. Нужно понимать, что любая борьба за производительность выливается в усложнение кода. Так что проблема должна быть достаточно большой, чтобы компенсировать выросшую сложность.

"Грязный флаг" применим к двум процессам: *вычисления* и *синхронизация*. В обоих случаях, процесс перевод первичных данных во вторичные выполняется долго или дорог по другим причинам .

В нашем примере с графом сцены вычисления медленные, потому что там много математики. При задаче синхронизации проблема чаще заключается в том, что вторичные данные находятся *где-то далеко* -- на диске или в сети на другой машине -- и дорога сама доставка данных из пункта A в пункт B.

Вот ещё пара требований:

 *  **Первичные данные изменяются чаще, чем ипользуются вторичные.**
    Паттерн работает за счет избегания работы с вторичными данными до того как они понадобятся, если изменились первичные. Если есть жесткая необходимость обновить вторичные данные сразу же после изменения основных -- этот паттерн не для вас. 
   
 *  **Трудно реализовать инкрементальное обновление.** 
    Допустим, наш пиратский корабль может только возить товары. И нам нужно знать вес груза в трюмах. Мы *можем* использовать этот паттерн и указать флагом на общий вес. Каждый раз, когда мы добавляем или убираем сундук, мы поднимаем флажок. Когда нам нужен общий вес, мы складываем все сундуки вместе и опускаем флаг.

    Есть решение проще -- просто *обновлять сумму*. Когда мы добавляем или убираем вес, можно просто прибавлять или вычитать это число из общего веса. Если мы можем пренебречь затратами на подобную работу, и при этом поддержать актуальность первичных данных, то это выгодное решение. 

Из всего этого кажется, что техникой грязного флага тяжело воспользоваться. Но можно найти место, где её применить. Если <span name="hacks">поискать</span> слово "dirty" в коде  игры средней сложности, то подобные примеры легко найдутся.      

<aside name="hacks">

По опыту, там найдутся и комментарии с извинениями за "грязный" хак.

</aside>

## Особенности

Даже если вы решились на использование этого паттерна, надо быть готовым к моментам, который доставят некоторый дискомфорт.  

### Затраты на чрезмерно отложенные вычисления

Мы откладываем затратные вычисления до момента, когда результаты действительно понадобятся. А когда они становяться нужными, они нужны *практически сразу*. Но ведь из-за этого всё и завертелось -- потому что вычисления медленные!

В нашем примере, это не проблема, потому что перевод локальной трансформации в мировую быстр достаточно, чтобы не задерживать отрисовку. Однако, легко представить другие ситуации, когда вычисления вызовут паузу, заметную невооруженным взглядом. И если игрок не получит результат этих вычислений сразу же, это будет весьма неприятная <span name="gc">пауза</span>. 

Ещё одна трудность -- если что-то пойдет не так в процессе вычислений, то восстановиться после такой ошибки тяжело. В случае, если вы используете отложенные вычисления, чтобы сохранить состояние куда-нибудь, то это может добавить головной боли. 
  
Например, текстовые редакторы не сразу сохраняют файл на диск. Как правило, маленькая звездочка в заголовке окна наглядно показывает "грязный флаг". Здесь первичные данные -- это открытый в редакторе документ, а вторичные -- байты на диске.

<img src="images/dirty-flag-title-bar.png" />

Часть программ не сохраняют изменения на диск до тех пор, пока приложение или документ не закроются. Это нормально, но если неожиданно выдернуть питание, то шедевр не увидит своих фанатов. 

Функция автоматического бэкапа в редакторах компенсирует этот недостаток. Частота автосохранения -- это выбранный компромисс между "не потерять слишком много, если все накроется" и "экономить время и не утруждать винчестер лишней работой".  

<aside name="gc">

Это похоже на трудности, с которыми сталкиваются стратегии сборки мусора в системах с автоматическим управлением памятью. Счетчик ссылок поможет освободить память сразу же, как только она становиться не нужна, но тратит время на собственное обновление. 

Простейшие сборщики откладывают освобождение памяти до тех пор, пока она реально не понадобится. Но эта пауза на уборку может повесить всю игру, пока мусорщик не закончит ревизию памяти. 

Есть ещё варианты между этими крайностями. Например, отложенный подсчет ссылок или инкрементальный сборщик, который не ставит весь процесс на паузу, хотя собирает память менее интенсивно, чем прямые счетчики.

</aside>

### Не забывайте выставлять флажок *каждый раз*, когда состояние изменилось

Так как вторичные данные вычисляются из основных, это практически кэш. А где появляется кэш, то там появляется и <span name="cache">*инвалидация кэша*</span> -- внимательное слежение за актуальнойстью данных в кэше. В текущем паттерне, это означает своевременное выставление флажка сразу же, как *что-то* поменялось в основных данных.

<aside name="cache">

Фил Карлтон верно подметил, что "есть только два трудных момента в компьютерном искусстве: инвалидация кэша и придумывание имен".
 
</aside>

Стоит пропустить всего одно место, и программа начнет использовать устаревшие данные. Это ведет к конфузу у игроков и трудно отслеживаемым ошибкам. Когда используете этот паттерн, убедитесь, что грязный флаг выставляется после любого изменения в первичных данных. 

Одним из способов следить за этой моментом является применение интерфейса для модификаций первичных данных. Если все изменения проходят через одну и ту же точку, то можно выставлять этот флаг там и забыть об этой проблеме на долгое время.

### На хранение вторичных данных требуется память

<span name="sync"></span>

Когда вторичные данные понадобятся, и грязный флаг *не* поднят, то используются уже вычисленные результаты. Это очевидно, но это вынуждает хранить эти рузельтаты в памяти постоянно, так как они могут понадобиться позже.

<aside name="sync">

Это не относится к ситуации, когда вы синхронизируете данные в какое-то другое место. В этом случае, вторичные данные могут быть вообще не в памяти.
 
</aside>

Если бы мы не использовали грязный флаг, то просто вычисляли бы вторичные данные каждый раз, когда они понадобятся, и уничтожали сразу после использования. Это помогает избежать затрат на хранение их в памяти ценой постоянных вычислений.

Тут, как и везде, мы <span name="trade">платим</span> памятью за скорость. В обмен за хранение вычисленных результатов в памяти, вы получаете возможность не тратить время на повторые вычисления, если ничего не изменилось. Этот обмен имеет смысл, если вычисления дороги, а память -- дешева. Если у вас времени больше, чем памяти, то выгоднее  каждый раз пересчитывать. 

<aside name="trade">

Алгоритмы сжатия предлагают противоположный обмен: они оптимизируют *место* для хранения данных, цена -- время на их распаковку.   

</aside>

## Примеры

Let's assume we've met the surprisingly long list of requirements, and see how the pattern looks in code. As I mentioned before, the actual math behind transform matrices is beyond the humble aims of this book, so I'll just encapsulate that in a class whose implementation you can presume exists somewhere out in the æther:

^code transform

The only operation we need here is `combine()` so that we can get an object's world transform by combining all of the local transforms along its parent chain. It also has a method to get an "origin" transform -- basically an identity matrix that means no translation, rotation, or scaling at all.

Next, we'll sketch out the class for an object in the scene graph. This is the bare minimum we need *before* applying this pattern:

^code graph-node

Each node has a local transform which describes where it is relative to its parent. It has a mesh which is the actual graphic for the object. (We'll allow `mesh_` to be `NULL` too to handle non-visual nodes that are used just to group their children.) Finally, each node has a possibly empty collection of child nodes.

With this, a "scene graph" is really just a single root `GraphNode` whose children (and grandchildren, etc.) are all of the objects in the world:

^code scene-graph

In order to render a scene graph, all we need to do is traverse that tree of nodes starting at the root and call the following function for each node's mesh with the right world transform:

^code render

We won't implement this here, but if we did, it would do whatever magic the renderer needs to draw that mesh at the given location in the world. If we can call that correctly and efficiently on every node in the scene graph, we're happy.

### An unoptimized traversal

To get our hands dirty, let's throw together a basic traversal for rendering the scene graph that calculates the world positions on the fly. It won't be optimal, but it will be simple. We'll add a new method to `GraphNode`:

^code render-on-fly

We pass the world transform of the node's parent into this using `parentWorld`. With that, all that's left to get the correct world transform of *this* node is to combine that with its own local transform. We don't have to walk *up* the parent chain to calculate world transforms because we calculate as we go while walking *down* the chain.

We calculate the node's world transform and store it in `world`, then we render the mesh if we have one. Finally, we recurse into the child nodes, passing in *this* node's world transform. All in all, it's nice tight, simple recursive method.

To draw an entire scene graph, we kick off the process at the root node:

^code render-root

### Let's get dirty

So this code does the right thing -- renders all the meshes in the right place -- but it doesn't do it efficiently. It's calling `local_.combine(parentWorld)` on every node in the graph, every frame. Let's see how this pattern fixes that. First, we need to add two fields to `GraphNode`:

^code dirty-graph-node

The `world_` field caches the previously-calculated world transform, and `dirty_`, of course, is the dirty flag. Note that the flag starts out `true`. When we create a new node, we haven't calculated it's world transform yet, so at birth it's already out of sync with the local transform.

The only reason we need this pattern is because objects can *move*, so let's add support for that:

^code set-transform

The important part here is that it sets the dirty flag too. Are we forgetting anything? Right: the child nodes!

When a parent node moves, all of its children's world coordinates are invalidated too. But here we aren't setting their dirty flags. We *could* do that, but that's recursive and slow. Instead we'll do something clever when we go to render. Let's see:

<span name="branch"></span>

^code dirty-render

<aside name="branch">

There's a subtle assumption here that the `if` check is faster than a matrix multiply. Intuitively, you would think it is: surely testing a single bit is faster than a bunch of floating point arithmetic.

However, modern CPUs are fantastically complex. They rely heavily on *pipelining* -- queueing up a series of sequential instructions. A branch like our `if` here can cause a *branch misprediction* and force the CPU lose cycles refilling the pipeline.

The <a href="data-locality.html" class="pattern">Data Locality</a> chapter has more about how modern CPUs try to go faster and how you can avoid tripping them up like this.

</aside>

This is similar to the original naïve implementation. The key changes are that we check to see if the node is dirty before calculating the world transform, and we store the result in a field instead of a local variable. When the node is clean, we skip `combine()` completely and use the old but still correct `world_` value.

The <span name="clever">clever</span> bit is that `dirty` parameter. That will be `true` if any node above this node in the parent chain was dirty. In much the same way that `parentWorld` updates the world transform incrementally as we traverse down the hierarchy, `dirty` tracks the dirtiness of the parent chain.

This lets us avoid having to actually recursively set each child's `dirty_` flag in `setTransform()`. Instead, we just pass the parent's dirty flag down to its children when we render and look at that too to see if we need to recalculate the world transform.

The end result here is exactly what we want: changing a node's local transform is just a couple of assignments, and rendering the world calculates the exact minimum number of world transforms that have changed since the last frame.

<aside name="clever">

Note that this clever trick only works because `render()` is the *only* thing in `GraphNode` that needs an up-to-date world transform. If other things accessed it, we'd have to do something different.

</aside>

## Design Decisions

This pattern is fairly specific, so there are only a couple of knobs to twiddle:

### When is the dirty flag cleaned?

* **When the result is needed:**

    * *It avoids doing calculation entirely if the result is never used.* For
        primary data that changes much more frequently than the derived data is
        accessed, this can be a big win.

    * *If the calculation is time-consuming, it can cause a noticeable pause.*
        Postponing the work until the player is expecting to see the result can
        affect their gameplay experience. Often, it's fast enough that this
        isn't a problem, but if it is, you'll have to do the work earlier.

* **At well-defined checkpoints:**

    Sometimes there is a point in time or the progression of the game where it's
    natural to do the deferred processing. For example,
    we may want to save the game only when the pirate sails into port. Or the
    sync point may not be part of the game mechanics. We may just want to hide the
    work behind a loading screen or a cut scene.

    * *Doing the work doesn't impact the user experience.* Unlike the previous
      option, you can often give something to
        distract the player while the game is busy processing.

    * *You lose control over when the work happens.* This is sort of the
        opposite of the earlier point. You have micro-scale control over when you
        process, and can make sure the game handles it gracefully.

        What you *can't* do is ensure the player actually makes it to the
        checkpoint or meets whatever criteria you've defined. If they get lost
        or the game gets in a weird state, you can end up deferring
        longer than you expect.

* **In the background:**

    Usually, you start a fixed <span name="hysteresis">timer</span>
    on the first modification and then process all of the changes that happen
    between then and when the timer fires.

    <aside name="hysteresis">

    The term in human-computer interaction for in intentional delay between
    when a program receives user input and when it responds is [*hysteresis*](http://en.wikipedia.org/wiki/Hysteresis).

    </aside>

    * *You can tune how often the work is performed.* By adjusting the timer
        interval you can ensure it happens as frequently (or infrequently) as
        you want.

    * *You can do more redundant work.* If the primary state only changes a
        tiny amount during the timer's run, you can end up processing a large
        chunk of mostly unchanged data.

    * *You need support for doing work asynchronously.*
        Processing the data "in the background" implies that the player can
        keep doing whatever it is that they're doing at the same time. That
        means you'll likely need threading or some other kind of concurrency
        support so that the game can work on the data while it's still
        being played.

        Since the player is likely interacting with
        the same primary state that you're processing, you'll need to think
        about making that safe for concurrent modification too.

### How fine-grained is your dirty tracking?

Imagine our pirate game lets players build and customize their pirate ship. Ships are automatically saved online so the player can resume where they left off. We're using dirty flags to determine which decks of the ship have been fitted and need to be sent to the server. Each chunk of data we send to the server contains some modified ship data and a bit of metadata describing where on the ship this modification occurred.

* **If it's more fine-grained:**

    Say you slap a dirty flag on each tiny plank of each deck.

    * *You only process data that actually changed.* You'll send exactly the
        facets of the ship that were modified to the server.

* **If it's more coarse-grained:**

    Alternatively, we could associate a dirty bit with each deck.
    Changing anything on it marks the entire deck <span name="swab">dirty</span>.

    <aside name="swab">

    I could make some terrible joke about it needing to be swabbed here, but
    I'll refrain.

    </aside>

    * *You end up processing unchanged data.* Add a single barrel to a deck
        and you'll have to send the whole thing to the server.

    * *Less memory is used for storing dirty flags.*

    * *Less time is spent on fixed overhead.* When processing some changed data,
       there's often a bit of fixed work you have to do on top of handling the
       data itself. In the example here, that's the metadata required to
       identify where on the ship the changed data is. The bigger your
       processing chunks, the fewer of them there are, which means the less
       overhead you have.

## See Also

*   This pattern is common outside of games in browser-side web frameworks like
    [Angular](http://angularjs.org/). They use dirty
    flags to track which data has been changed in the browser and needs to
    be pushed up to the server.

* Physics engines track which objects are in motion and which are resting. Since a resting body won't move until an impulse is applied to it, they don't need processing until they get touched. This "is moving" bit is a dirty flag to note which objects have had forces applied and need to have their physics resolved.
