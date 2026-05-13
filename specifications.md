# Техническая спецификация транслятора-интерпретатора

Данный документ содержит полное описание структуры и логики транслятора-интерпретатора, разработанного согласно учебному заданию.

---

## 1. Лексический анализатор

Лексический анализатор реализован как табличный детерминированный конечный автомат (ДКА).

### 1.1. Список лексем (терминалов)

| Код | Имя лексемы | Пример | Описание |
| :--- | :--- | :--- | :--- |
| 1 | `L_ID` | `x`, `arr` | Идентификатор |
| 2 | `L_NUM_INT` | `1024` | Целое число |
| 3 | `L_NUM_REAL` | `3.14` | Вещественное число |
| 4 | `L_READ` | `read` | Ввод данных |
| 5 | `L_WRITE` | `write` | Вывод данных |
| 6 | `L_IF` | `if` | Условный оператор |
| 7 | `L_THEN` | `then` | Ключевое слово "тогда" |
| 8 | `L_ELSE` | `else` | Ключевое слово "иначе" |
| 9 | `L_WHILE` | `while` | Цикл с предусловием |
| 10 | `L_DO` | `do` | Тело цикла |
| 11 | `L_BEGIN` | `begin` | Начало блока операторов |
| 12 | `L_END` | `end` | Конец блока операторов |
| 13 | `L_ASSIGN` | `:=` | Оператор присваивания |
| 14 | `L_PLUS` | `+` | Сложение |
| 15 | `L_MINUS` | `-` | Вычитание / Унарный минус |
| 16 | `L_MUL` | `*` | Умножение |
| 17 | `L_DIV` | `/` | Деление |
| 18 | `L_LPAR` | `(` | Левая круглая скобка |
| 19 | `L_RPAR` | `)` | Правая круглая скобка |
| 20 | `L_LBRACK` | `[` | Левая квадратная скобка |
| 21 | `L_RBRACK` | `]` | Правая квадратная скобка |
| 22 | `L_SEMICOLON`| `;` | Разделитель операторов |
| 23 | `L_CMP` | `<`, `>`, `=` | Операции сравнения |
| 24 | `L_EOF` | `$` | Конец файла |

### 1.2. Таблица переходов лексического автомата

Работа автомата описывается таблицей переходов $\delta(State, Class) \to NewState$.

```math
\begin{array}{|c|c|c|c|c|c|c|c|c|}
\hline
\text{State \ Class} & \text{Let} & \text{Dig} & \text{Dot} & \text{Col} & \text{Eq} & \text{Cmp} & \text{Space} & \text{Sep} \\ \hline
\text{0 (Start)} & 1 & 2 & \text{err} & 5 & 23 & 6 & 0 & 23 \\ \hline
\text{1 (ID)} & 1 & 1 & 23 & 23 & 23 & 23 & 23 & 23 \\ \hline
\text{2 (Int)} & 23 & 2 & 3 & 23 & 23 & 23 & 23 & 23 \\ \hline
\text{3 (Dot)} & \text{err} & 4 & \text{err} & \text{err} & \text{err} & \text{err} & \text{err} & \text{err} \\ \hline
\text{4 (Real)} & 23 & 4 & 23 & 23 & 23 & 23 & 23 & 23 \\ \hline
\text{5 (Col)} & \text{err} & \text{err} & \text{err} & \text{err} & 13 & \text{err} & \text{err} & \text{err} \\ \hline
\text{6 (Cmp)} & 23 & 23 & 23 & 23 & 23 & 23 & 23 & 23 \\ \hline
\end{array}
```
---

## 2. Исходная КС-грамматика языка

Грамматика $G = \langle \Sigma, N, S, P \rangle$.

```math
\begin{aligned}
\text{Program} & \to \text{StatementList} \\
\text{StatementList} & \to \text{Statement StatementList} \mid \epsilon \\
\text{Statement} & \to \text{Assignment} \mid \text{IfStmt} \mid \text{WhileStmt} \mid \text{ReadStmt} \mid \text{WriteStmt} \mid \text{CompoundStmt} \\
\text{Assignment} & \to \text{L\_ID [ Expression ] := Expression ;} \mid \text{L\_ID := Expression ;} \\
\text{IfStmt} & \to \text{if Expression then Statement else Statement} \mid \text{if Expression then Statement} \\
\text{WhileStmt} & \to \text{while Expression do Statement} \\
\text{ReadStmt} & \to \text{read L\_ID [ Expression ] ;} \mid \text{read L\_ID ;} \\
\text{WriteStmt} & \to \text{write Expression ;} \\
\text{CompoundStmt} & \to \text{begin StatementList end} \\
\text{Expression} & \to \text{Term } \{ (+ \mid - \mid \text{L\_CMP}) \text{ Term } \} \\
\text{Term} & \to \text{Factor } \{ (* \mid /) \text{ Factor } \} \\
\text{Factor} & \to \text{( Expression )} \mid \text{L\_ID [ Expression ]} \mid \text{L\_ID} \mid \text{L\_NUM\_INT} \mid \text{L\_NUM\_REAL} \mid \text{- Factor}
\end{aligned}
```
---

## 3. КС-грамматика в нестрогой форме Грейбах

Преобразована для табличного LL(1)-анализатора (устранена левая рекурсия):

```math
\begin{aligned}
S & \to \text{read L\_ID ArrayAccess ; } S \\
S & \to \text{write E ; } S \\
S & \to \text{if E then S ElsePart S} \\
S & \to \text{while E do S S} \\
S & \to \text{begin SL end S} \\
S & \to \text{L\_ID ID\_Op S} \\
S & \to \epsilon \\
\text{ID\_Op} & \to \text{ArrayAccess := E ;} \mid \text{ := E ;} \\
\text{ElsePart} & \to \text{else S} \mid \epsilon \\
SL & \to \text{Statement StatementList} \\
E & \to \text{T E\_Tail} \\
\text{E\_Tail} & \to \text{L\_PLUS T E\_Tail} \mid \text{L\_MINUS T E\_Tail} \mid \text{L\_CMP T E\_Tail} \mid \epsilon \\
T & \to \text{F T\_Tail} \\
\text{T\_Tail} & \to \text{L\_MUL F T\_Tail} \mid \text{L\_DIV F T\_Tail} \mid \epsilon \\
F & \to \text{L\_LPAR E L\_RPAR} \mid \text{L\_MINUS F} \mid \text{L\_NUM\_INT} \mid \text{L\_NUM\_REAL} \mid \text{L\_ID F\_ID\_Tail} \\
\text{F\_ID\_Tail} & \to \text{L\_LBRACK E L\_RBRACK} \mid \epsilon \\
\text{ArrayAccess} & \to \text{L\_LBRACK E L\_RBRACK} \mid \epsilon
\end{aligned}
```
---

## 4. Семантические действия для генерации ОПС

Генерация ОПЗ (Обратной Польской Записи) выполняется параллельно с синтаксическим разбором. В таблицу LL(1)-анализатора встроены вызовы семантических программ (SP).

### 4.1. Описание семантических программ

| Программа | Момент вызова | Действие |
| :--- | :--- | :--- |
| **SP1** | При обработке `L_ID`, `L_INT`, `L_REAL` | Записать операнд (индекс в таблице имен или константу) в выходную строку ОПС. |
| **SP2** | После завершения разбора `T` в `E_Tail` | Записать код бинарной операции (`+`, `-` или `CMP`) в ОПС. |
| **SP3** | После завершения разбора `F` в `T_Tail` | Записать код операции (`*`, `/`) в ОПС. |
| **SP4** | В правиле `F -> L_MINUS F` | Записать код унарного минуса `NEG` в ОПС. |
| **SP5** | При разборе `L_RBRACK` (массивы) | Записать операцию `INDEX` для вычисления адреса элемента. |
| **SP6 (IF)** | Сразу после разбора условия `E` | Сгенерировать в ОПС `M1 JF`. Запомнить позицию `M1` в стеке меток. |
| **SP7 (IF)** | После завершения ветки `THEN` | Сгенерировать `M2 J`. Записать текущий адрес ОПС в место, зарезервированное под `M1`. |
| **SP8 (WHILE)** | Перед началом разбора условия `E` | Запомнить в стеке меток текущую позицию ОПС как начало цикла `M_start`. |
| **SP9 (WHILE)** | После завершения тела цикла | Сгенерировать `M_start J`. Записать текущий адрес ОПС в метку выхода `M_exit`. |

---

## 5. Список операций ОПС

Ниже приведен полный перечень операций, поддерживаемых интерпретатором, и их влияние на стек операндов.

| Операция | Мнемоника | Аргументы | Эффект на стеке | Описание |
| :--- | :--- | :--- | :--- | :--- |
| Сложение | `ADD` | 2 | `[a, b] -> [a+b]` | Складывает два числа. |
| Вычитание | `SUB` | 2 | `[a, b] -> [a-b]` | Вычитает верхнее из предпоследнего. |
| Умножение | `MUL` | 2 | `[a, b] -> [a*b]` | Умножает два числа. |
| Деление | `DIV` | 2 | `[a, b] -> [a/b]` | Делит с проверкой на ноль. |
| Отрицание | `NEG` | 1 | `[a] -> [-a]` | Унарный минус. |
| Сравнение | `CMP_x` | 2 | `[a, b] -> [0/1]` | Сравнивает `a` и `b` ($<, >, =, \dots$). |
| Присваивание| `ASSIGN` | 2 | `[val, addr] -> []` | Записывает `val` по адресу `addr`. |
| Индексация | `INDEX` | 2 | `[idx, base] -> [addr]` | Вычисляет `base + idx`. |
| Ввод | `READ` | 1 | `[addr] -> []` | Считывает число в `addr`. |
| Вывод | `WRITE` | 1 | `[val] -> []` | Выводит `val` на экран. |
| Переход | `J` | 1 (адрес) | — | Безусловный переход по метке. |
| Усл. переход| `JF` | 2 | `[cond] -> []` | Переход по метке, если `cond == 0`. |

---

## 6. Формат ОПС и структура интерпретатора

### 6.1. Представление ОПС в памяти

ОПС хранится в виде массива структур (объектов) `Step`, где каждый элемент содержит:
1. **Тип (`Type`):** 
   - `CONST`: числовая константа.
   - `VAR_ADDR`: адрес переменной или начала массива.
   - `OPERATION`: код операции из п. 5.
   - `LABEL`: индекс элемента в массиве ОПС для команд перехода.
2. **Значение (`Value`):** Число или индекс.

### 6.2. Алгоритм работы интерпретатора

Интерпретатор работает в цикле по указателю текущей команды `PC` (Program Counter):

1. Получить элемент `OPS[PC]`.
2. Если это **Операнд** (`CONST` или `VAR_ADDR`):
   - Положить его значение в стек операндов.
   - `PC++`.
3. Если это **Операция**:
   - Извлечь из стека необходимое количество операндов.
   - Выполнить действие.
   - Если операция не является переходом — положить результат в стек (если есть) и `PC++`.
   - Если операция **переход** (`J` или `JF`) — обновить `PC` значением метки.
4. Повторять, пока `PC < OPS.length`.

### 6.3. Обработка ошибок выполнения
- Деление на ноль.
- Использование неинициализированной переменной.
- Выход за границы массива (проверка `Index < Size`).
