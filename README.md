# Оптическая передача данных для управления БЛА, BRICS-2024
![](logo_mephi.jpg)
## Цели проекта
+ Осуществление преобразования данных ввода в оптический сигнал
+ Изменение компоновки дрона для установки нового принимающего устройства
+ Реализация полноценного управления БЛА с помощью оптической передачи данных
## Основные функции и преимущества системы
Система представляет собой передатчик и приемник оптического типа, главным преимуществом которой является невосприимчевость к побочным
радиосигналам, что в свою очередь исключает нарушение управления посторонним субъектом.
## Целевая аудитория
Данная система является актуальной для любой категории пользователей, так как делает управление БЛА более стабильным и уменьшает вероятность потери устройства.
## Участники команды
+ [Стецкий Сергей](@Stinger_insane)
+ [Зимин Вадим](@VKzhuk)
+ [Бузанов Игорь](@accelerationaltrolge)


Роли участников проекта
==========================

| ФИО Участника | Роль | Обязанности |
| --- | --- | --- |
| Стецкий Сергей | Инженер-программист 1, капитан, пилот | Работа с визуализацией и автоматизированной отправкой результатов мониторинга, написание кода для автономного полета квадрокоптера, разработка алгоритма безопасного полета квадрокоптера. Общее руководство работой команды, распределение обязанностей и контроль соблюдения дедлайнов. Организация предполетной подготовки, обслуживание БАС, осуществление визуального пилотирования при возникновении внештатных ситуаций. |
| Бузанов Игорь | Инженер-программист 2 | Алгоритмы компьютерного зрения для реализации автономных миссий квадрокоптера, работа с датчиками. Организация работы команды в GitHub. |
| Зимин Вадим | Инженер-техник, пилот | Моделирование и изготовление полезной нагрузки квадрокоптера, работа с датчиками, тестирование, техобслуживание и пилотирование квадрокоптера. Организация предполетной подготовки, обслуживание БАС, осуществление визуального пилотирования при возникновении внештатных ситуаций. |

Распределение задач
=====================

| Описание задачи | Ответственный | Срок выполнения | Статус | Технологии/инструменты/ПО |
| --- | --- | --- | --- | --- |
| Разработка крепления для приемника к нижней панели дрона "Клевер" | Бузанов Игорь | До 24.09.2024 | В процессе работы | САПР Solidworks, измерительные приборы |
| Проверка оборудования | Зимин Вадим | До 23.09.2024 | В процессе работы | Оборудование |
| Организация рабочих библиотек, разработка и планирование первичного кода | Стецкий Сергей | До 24.09.2024 | В процессе работы | Visual Studio Code, ЯП: С, C++, Python |

## Структура проекта
Проект состоит из аппаратной и программной части. Аппаратная часть представляет модулятор, задачей которого является конвертация управляющего сигнала в видимое излучение, а также демодулятор, который устанавливается на дрон, принимает оптический сигнал, декодирует его и передает на полетный контроллер. Модулятор и демодулятор реализованы при помощи микроконтроллеров STM32.
## Failsafe
В случае, если видимость между передатчиком и приемником пропадет, дрон может прекратить принимать команды и станет неуправляемым. Для того чтобы избежать этой ситуации,был придуман механихм failsafe: в процессе полета дрон запоминает свой маршрут. В случае потери сигнала дрон переходит в автономный режим и выполняет полет по маршруту в обратном направлении в надежде снова принять сигнал. Если сигнал восстановлен, управление передается обратно оператору. Если в процессе полета сигнал так и не будет восстановлен, дрон совершит аварийную посадку. Данный механих реализован при помощи raspberry pi и модуля [simple_offboard](https://clover.coex.tech/ru/simple_offboard.html). На текущий момент для навигации используется карта маркеров ArUco, однако система навигации может быть изменена на другую по желанию пользователя (например на GPS).
## Детали реализации проекта
### Модулятор
Для передачи сигнала используется [FSK-модуляция](https://en.wikipedia.org/wiki/Frequency-shift_keying). Её суть заключается в том, что для передачи каждого бита информации используется различная частота модуляции для передачи 0 или 1. Преимуществами данного метода модуляции является простота реализации и помехоустойчивость. Недостатком явзяется более низкая пропускная способность по сравнению с другими видами модуляции (например квадратурной модуляцией). Модулированный сигнал подается на [цифро-аналоговый преобразователь](https://en.wikipedia.org/wiki/Digital-to-analog_converter) микроконтроллера, подключенный к источнику света.
### Демодулятор
Сигнал принимается при помощи установленного на дрон фотодиода. После этого сигнал попадает на [фильтр верхних частот](https://en.wikipedia.org/wiki/High-pass_filter), который отсекает постоянную составляющую сигнала. Это позволяет системе работать в условиях сильного внешнего освещения. Отфильтрованный сигнал попадает на аналогово-цифровой преобразователь микроконтроллера, после чего выполняется процесс его демодуляции.
### Прием и компрессия сигнала SBUS
Аппаратура радиоуправления подключается к модулятору по протоколу SBUS (последовательный протокол, разработанный специально для управления БПЛА). Ввиду ограниченной пропускной способности необходимо выполнить сжатие управляющих пакетов чтобы обеспечить приемлимую частоту передачи пакетов. Сжатие производится путем уменьшения разряднисти вспомогательных каналов с 11 до 1 бита. Таким образом удалось добиться частоты передачи пакетов около 50 Гц.
### Контроль целостности
При передаче данных по оптическому каналу могут возникать помехи, которые могут повлият на целостьность сигнала. Для избежания ошибок при передаче в пакет добавляется [контрольная сумма](https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks) (crc32). Контрольня сумма изменится, если в процессе передачи в пакете возникла ошибка. Принимающая сторона заново вычисляет контрольную сумму пакета. Если она не совпадает, то пакет отбрасывается.
### Реализация механизма failsafe
Скрипт failsafe.py запускается на raspberry pi в процессе полета периодически запрашивает текущие координаты дрона и сохраняет их в [стек](https://en.wikipedia.org/wiki/Stack_(abstract_data_type)). Данные распологаются в оперативной памяти и сохраняются на протяжении полета. В управляющие каналы был добавлен дополнительный канал, состояни которого позволяет определить, имеется ли в данный момент на входе демодулятора стабильный сигнал. Скрипт, запущенный на raspberry pi, следит за состоянием этого канала путем подписки на топик '/mavros/rc/in'. Если значение этого канала принимает значение 2000, что свидетельствует о потере сигнала, в этом случае скрипт переводит полетный контроллер в режим OFFBOARD и выполняет последовательный полет по точкам маршрута в обратном направлении используя сервис '/navigate'.
