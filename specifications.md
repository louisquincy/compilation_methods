# Техническая спецификация транслятора-интерпретатора

Данный документ содержит полное описание структуры и логики транслятора-интерпретатора, разработанного согласно учебному заданию.

---

## 1. Лексический анализатор

Лексический анализатор реализован как табличный детерминированный конечный автомат (ДКА).

### 1.1. Список лексем (терминалов)

| Код | Имя лексемы | Описание |
| :--- | :--- | :--- |
| 1 | `L_ID` | Идентификатор (имя переменной) |
| 2 | `L_NUM_INT` | Целое число (константа) |
| 3 | `L_NUM_REAL` | Вещественное число (константа) |
| 4 | `read` | Ввод данных |
| 5 | `write` | Вывод данных |
| 6 | `if` | Условный оператор |
| 7 | `then` | Ключевое слово "тогда" |
| 8 | `else` | Ключевое слово "иначе" |
| 9 | `while` | Цикл с предусловием |
| 10 | `do` | Тело цикла |
| 11 | `begin` | Начало блока операторов |
| 12 | `end` | Конец блока операторов |
| 13 | `:=` | Оператор присваивания |
| 14 | `+` | Сложение |
| 15 | `-` | Вычитание / Унарный минус |
| 16 | `*` | Умножение |
| 17 | `/` | Деление |
| 18 | `(` | Левая круглая скобка |
| 19 | `)` | Правая круглая скобка |
| 20 | `[` | Левая квадратная скобка |
| 21 | `]` | Правая квадратная скобка |
| 22 | `;` | Разделитель операторов |
| 23 | `<` | Меньше |
| 24 | `>` | Больше |
| 25 | `=` | Равно |
| 26 | `<=` | Меньше или равно |
| 27 | `>=` | Больше или равно |
| 28 | `$` | Конец файла |

### 1.2. Таблица переходов лексического автомата

Работа автомата описывается таблицей переходов $\delta(State, Class) \to NewState$. 
Автомат корректно распознает составные лексемы (`:=`, `<=`, `>=`) за счет переходов между состояниями.

**Классы символов:**
- `Let`: буквы `a-z, A-Z, _`
- `Dig`: цифры `0-9`
- `Dot`: точка `.`
- `Col`: двоеточие `:`
- `Eq`: равно `=`
- `Lt`: меньше `<`
- `Gt`: больше `>`
- `Space`: пробельные символы
- `Sep`: `+ - * / ( ) [ ] ;`

```math
\begin{array}{|c|c|c|c|c|c|c|c|c|c|}
\hline
\text{State \ Class} & \text{Let} & \text{Dig} & \text{Dot} & \text{Col} & \text{Eq} & \text{Lt} & \text{Gt} & \text{Space} & \text{Sep} \\ \hline
\text{0 (Start)} & 1 & 2 & \text{err} & 5 & 25 & 6 & 7 & 0 & \text{fin} \\ \hline
\text{1 (ID)} & 1 & 1 & \text{fin} & \text{fin} & \text{fin} & \text{fin} & \text{fin} & \text{fin} & \text{fin} \\ \hline
\text{2 (Int)} & \text{fin} & 2 & 3 & \text{fin} & \text{fin} & \text{fin} & \text{fin} & \text{fin} & \text{fin} \\ \hline
\text{3 (Dot)} & \text{err} & 4 & \text{err} & \text{err} & \text{err} & \text{err} & \text{err} & \text{err} & \text{err} \\ \hline
\text{4 (Real)} & \text{fin} & 4 & \text{fin} & \text{fin} & \text{fin} & \text{fin} & \text{fin} & \text{fin} & \text{fin} \\ \hline
\text{5 (Colon)} & \text{err} & \text{err} & \text{err} & \text{err} & 13 & \text{err} & \text{err} & \text{err} & \text{err} \\ \hline
\text{6 (Less)} & \text{fin} & \text{fin} & \text{fin} & \text{fin} & 26 & \text{fin} & \text{fin} & \text{fin} & \text{fin} \\ \hline
\text{7 (Great)} & \text{fin} & \text{fin} & \text{fin} & \text{fin} & 27 & \text{fin} & \text{fin} & \text{fin} & \text{fin} \\ \hline
\end{array}
```
---

# 2. КС-грамматика языка (Исходная)

Грамматика определяется четверкой $G = (N, \Sigma, P, S)$, где $\Sigma$ — литералы из п. 1.1.

```math
\begin{aligned}
\text{Program}      &\to \text{StatementList} \\
\text{StatementList} &\to \text{Statement StatementList} \mid \epsilon \\
\text{Statement}    &\to \text{Assignment} \mid \text{IfStmt} \mid \text{WhileStmt} \mid \text{ReadStmt} \mid \text{WriteStmt} \mid \text{CompoundStmt} \\
\text{Assignment}   &\to \text{L\_ID [ Expression ] := Expression ;} \mid \text{L\_ID := Expression ;} \\
\text{IfStmt}       &\to \text{if Expression then Statement else Statement} \mid \text{if Expression then Statement} \\
\text{WhileStmt}    &\to \text{while Expression do Statement} \\
\text{ReadStmt}     &\to \text{read L\_ID [ Expression ] ;} \mid \text{read L\_ID ;} \\
\text{WriteStmt}    &\to \text{write Expression ;} \\
\text{CompoundStmt} &\to \text{begin StatementList end} \\
\text{Expression}   &\to \text{Addition < Addition} \mid \text{Addition > Addition} \mid \text{Addition = Addition} \\
                    &\mid \text{Addition <= Addition} \mid \text{Addition >= Addition} \mid \text{Addition} \\
\text{Addition}     &\to \text{Addition + Multiplication} \mid \text{Addition - Multiplication} \mid \text{Multiplication} \\
\text{Multiplication} &\to \text{Multiplication * Unary} \mid \text{Multiplication / Unary} \mid \text{Unary} \\
\text{Unary}        &\to \text{- Unary} \mid \text{Primary} \\
\text{Primary}      &\to \text{L\_NUM\_INT} \mid \text{L\_NUM\_REAL} \mid \text{L\_ID [ Expression ]} \mid \text{L\_ID} \mid \text{( Expression )} \\
\end{aligned}
```
---

# 3. КС-грамматика в нестрогой нормальной форме Грейбах (НФГ)

Для реализации нисходящего синтаксического анализатора (LL(1)) грамматика факторизована, устранена левая рекурсия. Каждое правило начинается с терминала или является пустым.

### 1. Операторы (Главный нетерминал S)

```math
\begin{aligned}
S            &\to \text{L\_ID ID\_Tail S} \\
             &\mid \text{if E then S ElsePart S} \\
             &\mid \text{while E do S S} \\
             &\mid \text{read L\_ID ArrayAccess ; S} \\
             &\mid \text{write E ; S} \\
             &\mid \text{begin SL end S} \\
             &\mid \epsilon \\
\text{ID\_Tail}    &\to \text{[ E ] := E ;} \mid \text{:= E ;} \\
\text{ElsePart}     &\to \text{else S} \mid \epsilon \\
\text{SL}           &\to \text{S StatementList} \\
\text{StatementList} &\to \text{S StatementList} \mid \epsilon \\
\end{aligned}
```
### 2. Выражения (E)

```math
\begin{aligned}
E            &\to \text{T E\_Tail} \\
\text{E\_Tail}       &\to \text{+ T E\_Tail} \mid \text{- T E\_Tail} \\
             &\mid \text{< T} \mid \text{> T} \mid \text{= T} \mid \text{<= T} \mid \text{>= T} \mid \epsilon \\
\end{aligned}
```
### 3. Термы (T)

```math
\begin{aligned}
T            &\to \text{F T\_Tail} \\
\text{T\_Tail}       &\to \text{* F T\_Tail} \mid \text{/ F T\_Tail} \mid \epsilon \\
\end{aligned}
```
### 4. Первичные элементы и унарные операции (F)

```math
\begin{aligned}
F            &\to \text{- F} \\
             &\mid \text{L\_NUM\_INT} \\
             &\mid \text{L\_NUM\_REAL} \\
             &\mid \text{L\_ID F\_ID\_Tail} \\
             &\mid \text{( E )} \\
\text{F\_ID\_Tail}    &\to \text{[ E ]} \mid \epsilon \\
\text{ArrayAccess}   &\to \text{[ E ]} \mid \epsilon \\
\end{aligned}
```
---

# 4. Семантические действия для генерации ОПС

В процессе прохождения правил КС-грамматики LL(1)-анализатором параллельно формируется Обратная Польская Строка (ОПС).

### Обозначения семантических действий (атомов):
- **□** – пустое действие (ничего не генерируется).
- **v** – записать переменную (или базовый адрес массива) в ОПС.
- **k** – записать константу в ОПС.
- **+ , - , * , /** – записать соответствующую арифметическую операцию в ОПС.
- **< , > , = , <= , >=** – записать операцию сравнения в ОПС.
- **:=** – записать операцию присваивания в ОПС.
- **NEG** – записать унарный минус в ОПС.
- **i** – индексация массива (снимает со стека индекс и базовый адрес, возвращает физический адрес элемента).
- **r** – оператор чтения (read).
- **w** – оператор вывода (write).
- **j** – безусловный переход (БП).
- **jf** – условный переход по лжи (УПЛ).
- **m** – создать/поставить метку (адрес в ОПС).

### Таблица правил и действий

| Правило КС-грамматики (НФГ) | Семантические действия (генерация ОПС) |
| :--- | :--- |
| `S → L_ID ID_Tail S1` | Записать **v(L_ID)**, сгенерировать **ID_Tail**, сгенерировать **S1** |
| `ID_Tail → := E ;` | Сгенерировать **E**, затем записать **:=** |
| `ID_Tail → [ E1 ] := E2 ;` | Сгенерировать **E1**, записать **i**, сгенерировать **E2**, записать **:=** |
| `S → if E then S1 ElsePart S2` | **m1**, сгенерировать **E**, **jf m2**, сгенерировать **S1**, **j m3**, **m2**, сгенерировать **ElsePart**, **m3**, сгенерировать **S2** |
| `ElsePart → else S` | Сгенерировать **S** |
| `ElsePart → ε` | **□** |
| `S → while E do S1 S2` | **m1**, сгенерировать **E**, **jf m2**, сгенерировать **S1**, **j m1**, **m2**, сгенерировать **S2** |
| `S → read L_ID ArrayAccess ; S1` | Записать **v(L_ID)**, сгенерировать **ArrayAccess**, записать **r**, сгенерировать **S1** |
| `S → write E ; S1` | Сгенерировать **E**, записать **w**, сгенерировать **S1** |
| `S → begin SL end S1` | Сгенерировать **SL**, сгенерировать **S1** |
| `S → ε` | **□** |
| `SL → S StatementList` | Сгенерировать **S**, сгенерировать **StatementList** |
| `StatementList → S StatementList1`| Сгенерировать **S**, сгенерировать **StatementList1** |
| `StatementList → ε` | **□** |
| `E → T E_Tail` | Сгенерировать **T**, сгенерировать **E_Tail** |
| `E_Tail → + T E_Tail1` | Сгенерировать **T**, записать **+**, сгенерировать **E_Tail1** |
| `E_Tail → - T E_Tail1` | Сгенерировать **T**, записать **-**, сгенерировать **E_Tail1** |
| `E_Tail → < T` | Сгенерировать **T**, записать **<** |
| `E_Tail → > T` | Сгенерировать **T**, записать **>** |
| `E_Tail → = T` | Сгенерировать **T**, записать **=** |
| `E_Tail → <= T` | Сгенерировать **T**, записать **<=** |
| `E_Tail → >= T` | Сгенерировать **T**, записать **>=** |
| `E_Tail → ε` | **□** |
| `T → F T_Tail` | Сгенерировать **F**, сгенерировать **T_Tail** |
| `T_Tail → * F T_Tail1` | Сгенерировать **F**, записать `*`, сгенерировать **T_Tail1** |
| `T_Tail → / F T_Tail1` | Сгенерировать **F**, записать `/`, сгенерировать **T_Tail1** |
| `T_Tail → ε` | **□** |
| `F → L_NUM_INT` | Записать **k(INT)** |
| `F → L_NUM_REAL` | Записать **k(REAL)** |
| `F → L_ID F_ID_Tail` | Записать **v(L_ID)**, сгенерировать **F_ID_Tail** |
| `F → - F1` | Сгенерировать **F1**, записать **NEG** |
| `F → ( E )` | Сгенерировать **E** |
| `F_ID_Tail → [ E ]` | Сгенерировать **E**, записать **i** |
| `F_ID_Tail → ε` | **□** |
| `ArrayAccess → [ E ]` | Сгенерировать **E**, записать **i** |
| `ArrayAccess → ε` | **□** |

---

# 5. Список операций ОПС

Каждая операция является уникальной и обрабатывается интерпретатором как отдельная команда.

### Бинарные операции:
- **`+`** (сложение), **`-`** (вычитание), **`*`** (умножение), **`/`** (деление).
- **`:=`** (присваивание): извлекает из стека значение и целевой адрес, производит запись в память.
- **`i`** (индекс массива): извлекает из стека индекс и базовый адрес массива, вычисляет и кладет в стек физический адрес элемента в памяти.
- **`<`** (меньше), **`>`** (больше), **`=`** (равно), **`<=`** (меньше или равно), **`>=`** (больше или равно): извлекают два операнда, производят сравнение, возвращают в стек `1` (истина) или `0` (ложь).

### Унарные операции:
- **`NEG`** (унарный минус): меняет знак операнда на вершине стека.
- **`r`** (чтение): извлекает из стека адрес переменной и запрашивает ввод с консоли в эту ячейку памяти.
- **`w`** (вывод): извлекает из стека значение и выводит его на экран.

### Операции переходов (управление потоком):
- **`j`** (безусловный переход): переносит указатель выполнения интерпретатора на указанную метку.
- **`jf`** (условный переход по лжи): извлекает из стека логическое значение. Если оно равно `0` (ложь), осуществляет переход по указанной метке. В противном случае выполнение продолжается линейно.
- **`m`** (метка): адрес в массиве ОПС, использующийся для команд `j` и `jf`.

---

# 6. Формат ОПС и реализация интерпретатора

### 6.1. Формат ОПС

ОПС хранится в виде массива объектов (атомов), каждый из которых имеет тип и значение.

```math
\text{Atom} = 
\begin{cases} 
\langle \text{CONST, value} \rangle & \text{Числовая константа (целая или вещественная)} \\
\langle \text{VAR, name\_index} \rangle & \text{Адрес переменной / индекс в таблице имен} \\
\langle \text{CMD, op\_code} \rangle & \text{Код операции из раздела 5} \\
\langle \text{LABEL, pc\_index} \rangle & \text{Адрес перехода в массиве ОПС (метка)} \\
\end{cases}
```
### 6.2. Примеры ОПС

**Для выражения** `y := a[x] * 2 + -5 ;`
ОПС: `vy va vx i k2 * k5 NEG + :=`
(где `va` - база массива, `vx` - индекс, `i` - индексация, `NEG` - смена знака).

**Для цикла** `while x < 5 do begin x := x + 1 ; end ;`
ОПС: `m1 vx k5 < jf m2 vx vx k1 + := j m1 m2`
(где `m1` и `m2` - адреса меток для входа и выхода из цикла).

### 6.3. Алгоритм интерпретации (Стек-машина)

Процесс интерпретации использует счетчик команд `PC`, стек операндов и таблицу памяти переменных.

```math
\begin{aligned}
&\text{while } PC < \text{length}(OPS): \\
&\quad \text{item} = OPS[PC] \\
&\quad \text{if item.type == CONST or item.type == VAR:} \\
&\quad \quad \text{Stack.push(item)} \\
&\quad \quad PC = PC + 1 \\
&\quad \text{else if item.type == CMD:} \\
&\quad \quad \text{if item.cmd == j:} \\
&\quad \quad \quad PC = OPS[PC+1].label\_value \\
&\quad \quad \text{else if item.cmd == jf:} \\
&\quad \quad \quad \text{cond} = \text{Stack.pop()} \\
&\quad \quad \quad \text{if cond == 0: } PC = OPS[PC+1].label\_value \\
&\quad \quad \quad \text{else: } PC = PC + 2 \\
&\quad \quad \text{else:} \\
&\quad \quad \quad \text{args} = \text{Stack.pop(needed\_count)} \\
&\quad \quad \quad \text{result} = \text{ExecuteOperation(item.cmd, args)} \\
&\quad \quad \quad \text{if result exists: Stack.push(result)} \\
&\quad \quad \quad PC = PC + 1
\end{aligned}
```
