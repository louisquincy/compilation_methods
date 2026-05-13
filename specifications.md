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

# 2. КС-грамматика языка (Исходная)

Грамматика определяется четверкой $G = (N, \Sigma, P, S)$, где:

- $\Sigma$ — множество терминалов (лексем), определенных в п. 1.1.
- $N$ — множество нетерминалов (грамматических понятий).
- $P$ — множество правил порождения (продукций).
- $S$ — начальный нетерминал грамматики ($S = \text{Program}$).

### Список нетерминалов ($N$)
`{Program, StatementList, Statement, Assignment, IfStmt, WhileStmt, ReadStmt, WriteStmt, CompoundStmt, Expression, Comparison, Addition, Multiplication, Unary, Primary, ArrayIndex, CompOp}`

### Правила порождения ($P$)

```math
\begin{aligned}
\text{Program}      &\to \text{StatementList} \\
\text{StatementList} &\to \text{Statement StatementList} \mid \epsilon \\
\\
\text{Statement}    &\to \text{Assignment} \\
                    &\mid \text{IfStmt} \\
                    &\mid \text{WhileStmt} \\
                    &\mid \text{ReadStmt} \\
                    &\mid \text{WriteStmt} \\
                    &\mid \text{CompoundStmt} \\
\\
\text{Assignment}   &\to \text{L\_ID ArrayIndex L\_ASSIGN Expression L\_SEMICOLON} \\
                    &\mid \text{L\_ID L\_ASSIGN Expression L\_SEMICOLON} \\
\\
\text{IfStmt}       &\to \text{L\_IF Expression L\_THEN Statement L\_ELSE Statement} \\
                    &\mid \text{L\_IF Expression L\_THEN Statement} \\
\\
\text{WhileStmt}    &\to \text{L\_WHILE Expression L\_DO Statement} \\
\\
\text{ReadStmt}     &\to \text{L\_READ L\_ID ArrayIndex L\_SEMICOLON} \\
                    &\mid \text{L\_READ L\_ID L\_SEMICOLON} \\
\\
\text{WriteStmt}    &\to \text{L\_WRITE Expression L\_SEMICOLON} \\
\\
\text{CompoundStmt} &\to \text{L\_BEGIN StatementList L\_END} \\
\\
\text{Expression}   &\to \text{Comparison} \\
\\
\text{Comparison}   &\to \text{Addition CompOp Addition} \mid \text{Addition} \\
\text{CompOp}       &\to \text{L\_CMP} \\
\\
\text{Addition}     &\to \text{Addition L\_PLUS Multiplication} \\
                    &\mid \text{Addition L\_MINUS Multiplication} \\
                    &\mid \text{Multiplication} \\
\\
\text{Multiplication} &\to \text{Multiplication L\_MUL Unary} \\
                    &\mid \text{Multiplication L\_DIV Unary} \\
                    &\mid \text{Unary} \\
\\
\text{Unary}        &\to \text{L\_MINUS Unary} \mid \text{Primary} \\
\\
\text{Primary}      &\to \text{L\_NUM\_INT} \\
                    &\mid \text{L\_NUM\_REAL} \\
                    &\mid \text{L\_ID ArrayIndex} \\
                    &\mid \text{L\_ID} \\
                    &\mid \text{L\_LPAR Expression L\_RPAR} \\
\\
\text{ArrayIndex}   &\to \text{L\_LBRACK Expression L\_RBRACK} \\
\end{aligned}
```
---

# 3. КС-грамматика в нестрогой нормальной форме Грейбах (НФГ)

Для реализации нисходящего синтаксического анализатора (магазинного автомата) грамматика преобразована таким образом, чтобы каждое правило начиналось с терминала (лексемы) либо было пустым. Левая рекурсия устранена путем введения дополнительных нетерминалов — «хвостов» (Tail).

### 1. Список операторов (Главный нетерминал S)

```math
\begin{aligned}
S            &\to \text{L\_ID ID\_Tail S} \\
             &\mid \text{L\_IF E L\_THEN S ElsePart S} \\
             &\mid \text{L\_WHILE E L\_DO S S} \\
             &\mid \text{L\_READ L\_ID ArrayAccess L\_SEMICOLON S} \\
             &\mid \text{L\_WRITE E L\_SEMICOLON S} \\
             &\mid \text{L\_BEGIN SL L\_END S} \\
             &\mid \epsilon \\
\\
\text{ID\_Tail}    &\to \text{L\_LBRACK E L\_RBRACK L\_ASSIGN E L\_SEMICOLON} \\
             &\mid \text{L\_ASSIGN E L\_SEMICOLON} \\
\\
\text{ElsePart}     &\to \text{L\_ELSE S} \mid \epsilon \\
\\
\text{SL}           &\to \text{S StatementList} \\
\end{aligned}
```
### 2. Выражения (E)

```math
\begin{aligned}
E            &\to \text{T E\_Tail} \\
\\
\text{E\_Tail}       &\to \text{L\_PLUS T E\_Tail} \\
             &\mid \text{L\_MINUS T E\_Tail} \\
             &\mid \text{CompTail} \\
             &\mid \epsilon \\
\\
\text{CompTail}     &\to \text{L\_CMP T CompTail} \\
             &\mid \epsilon \\
\end{aligned}
```
### 3. Мультипликативные операции (T)

```math
\begin{aligned}
T            &\to \text{F T\_Tail} \\
\\
\text{T\_Tail}       &\to \text{L\_MUL F T\_Tail} \\
             &\mid \text{L\_DIV F T\_Tail} \\
             &\mid \epsilon \\
\end{aligned}
```
### 4. Первичные элементы и унарные операции (F)

```math
\begin{aligned}
F            &\to \text{L\_MINUS F} \\
             &\mid \text{L\_NUM\_INT} \\
             &\mid \text{L\_NUM\_REAL} \\
             &\mid \text{L\_ID F\_ID\_Tail} \\
             &\mid \text{L\_LPAR E L\_RPAR} \\
\\
\text{F\_ID\_Tail}    &\to \text{L\_LBRACK E L\_RBRACK} \\
             &\mid \epsilon \\
\\
\text{ArrayAccess}   &\to \text{L\_LBRACK E L\_RBRACK} \mid \epsilon \\
\end{aligned}
```
---

# 4. Семантические действия для генерации ОПС

Процесс генерации Обратной Польской Строки (ОПС) происходит одновременно с синтаксическим анализом. В ячейки таблицы LL(1)-анализатора встраиваются вызовы семантических программ (SP).

### 4.1. Список семантических программ

| Программа | Описание действия |
| :--- | :--- |
| **SP_PUSH** | Поместить текущий операнд (индекс переменной в таблице имен или значение константы) в выходную строку ОПС. |
| **SP_OP** | Записать код текущей операции (`+`, `-`, `*`, `/`, `CMP`) в выходную строку ОПС. |
| **SP_NEG** | Записать в ОПС код унарного отрицания `NEG`. |
| **SP_INDEX** | Записать в ОПС код операции `INDEX` (вычисление адреса элемента массива). |
| **SP_IF_JF** | 1. Создать новую метку `L1`. 2. Записать в ОПС `L1` и операцию `JF`. 3. Положить `L1` в стек меток. |
| **SP_IF_J** | 1. Создать новую метку `L2`. 2. Записать в ОПС `L2` и операцию `J`. 3. Извлечь `L1` из стека меток, записать текущий адрес ОПС в поле адреса метки `L1`. 4. Положить `L2` в стек меток. |
| **SP_IF_END** | Извлечь метку из стека меток, записать в неё текущий адрес конца блока. |
| **SP_W_START**| Запомнить текущую позицию в ОПС как адрес начала цикла `LoopStart`. |
| **SP_W_JF** | Создать метку `LoopExit`. Записать в ОПС `LoopExit` и `JF`. Положить `LoopExit` в стек меток. |
| **SP_W_END** | 1. Записать в ОПС `LoopStart` и `J`. 2. Извлечь `LoopExit` и заполнить её текущим адресом ОПС. |

### 4.2. Фрагмент грамматики со встроенными действиями

```math
\begin{aligned}
E &\to T \text{ E\_Tail} \\
\text{E\_Tail} &\to \text{L\_PLUS } T \{ \text{SP\_OP(+)} \} \text{ E\_Tail} \\
               &\mid \text{L\_MINUS } T \{ \text{SP\_OP(-)} \} \text{ E\_Tail} \\
               &\mid \epsilon \\
\\
\text{IfStmt}  &\to \text{L\_IF } E \{ \text{SP\_IF\_JF} \} \text{ L\_THEN } S \{ \text{SP\_IF\_J} \} \text{ L\_ELSE } S \{ \text{SP\_IF\_END} \} \\
\end{aligned}
```
---

# 5. Список операций ОПС

Ниже приведена полная спецификация команд промежуточного языка (ОПС), которые обрабатываются интерпретатором.

### 5.1. Арифметические и логические операции

| Код | Мнемоника | Аргументы | Эффект на стеке (до $\to$ после) |
| :--- | :--- | :--- | :--- |
| 101 | `ADD` | 2 | $[a, b] \to [a + b]$ |
| 102 | `SUB` | 2 | $[a, b] \to [a - b]$ |
| 103 | `MUL` | 2 | $[a, b] \to [a \cdot b]$ |
| 104 | `DIV` | 2 | $[a, b] \to [a / b]$ (с проверкой на 0) |
| 105 | `NEG` | 1 | $[a] \to [-a]$ |
| 106 | `CMP_LT` | 2 | $[a, b] \to [a < b \ ? \ 1 : 0]$ |
| 107 | `CMP_GT` | 2 | $[a, b] \to [a > b \ ? \ 1 : 0]$ |
| 108 | `CMP_EQ` | 2 | $[a, b] \to [a = b \ ? \ 1 : 0]$ |

### 5.2. Операции управления памятью и потоком

| Код | Мнемоника | Аргументы | Эффект на стеке | Описание |
| :--- | :--- | :--- | :--- | :--- |
| 201 | `ASSIGN` | 2 | $[val, addr] \to []$ | Сохранить `val` по адресу `addr`. |
| 202 | `INDEX` | 2 | $[idx, base] \to [addr]$ | Вычислить адрес `base + idx`. |
| 203 | `READ` | 1 | $[addr] \to []$ | Считать число с консоли в `addr`. |
| 204 | `WRITE` | 1 | $[val] \to []$ | Вывести значение `val` на экран. |
| 301 | `J` | 1 (метка) | $[] \to []$ | Безусловный переход. |
| 302 | `JF` | 2 (мет, конд)| $[cond] \to []$ | Переход по метке, если `cond = 0`. |

---

# 6. Формат ОПС и реализация интерпретатора

### 6.1. Структура элемента ОПС

Каждый элемент ОПС — это структура (атом), описываемая следующим образом:

```math
\text{Item} = 
\begin{cases} 
\langle \text{Type: CONST, Value: } n \rangle & \text{Числовая константа} \\
\langle \text{Type: VAR, Value: } id \rangle & \text{Ссылка на таблицу имен (адрес)} \\
\langle \text{Type: CMD, Value: } op\_code \rangle & \text{Код операции из раздела 5} \\
\langle \text{Type: LABEL, Value: } target \rangle & \text{Адрес перехода в ОПС} \\
\end{cases}
```
### 6.2. Алгоритм интерпретации

Интерпретатор реализует классическую стек-машину. Процесс выполнения описывается следующим циклом:

1. **Инициализация:** `PC = 0` (указатель команд), `Stack = empty`.
2. **Цикл выполнения:** Пока `PC < OPS.length`:
    - `current = OPS[PC]`
    - Если `current.Type` $\in \{ \text{CONST, VAR} \}$:
        - `Stack.push(current)`
        - `PC = PC + 1`
    - Если `current.Type == CMD`:
        - Если `current.Value == J`: `PC = OPS[PC+1].Value`
        - Если `current.Value == JF`:
            - `addr = OPS[PC+1].Value`
            - `cond = Stack.pop()`
            - Если `cond == 0`: `PC = addr`
            - Иначе: `PC = PC + 2`
        - Иначе (арифметика, ввод/вывод):
            - Извлечь аргументы из `Stack`.
            - Выполнить операцию.
            - Положить результат в `Stack` (если операция возвращает значение).
            - `PC = PC + 1`
3. **Завершение:** Если стек пуст и `PC` достиг конца — программа выполнена успешно.

### 6.3. Таблицы имен и данных

В процессе работы интерпретатор использует две основные таблицы:
1. **Таблица имен:** хранит строковые имена переменных и их текущие типы/свойства.
2. **Память (Data Memory):** массив, где по индексам из таблицы имен хранятся реальные значения (целые или вещественные). Для массивов выделяется непрерывный блок ячеек.
