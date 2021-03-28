# Yandex Practicum C++

:boom: **Sprint #3 description added [here](#Sprint-3)** 

## Sprint 1

1. Code as it was in web-form could be found in the **main.cpp** file on branch 
[dev/sprint-1](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-1)
2. Code, which I've tried to split by \*.h and \*.cpp files could be found on the 
[master](https://github.com/ZakharovAleksey/yandex_cpp/tree/main) branch

Implemented items:
1. Class `SearchServer`:
    * [server.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-2/sprint_1/server.h)
    * [server.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-2/sprint_1/server.cpp)

## Sprint 2

Code with **Unit Testing framework** (from lectures) and **Unit Tests** for the 
`SearchServer` could be found on branch: 
[dev/sprint-2](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-2).

Implemented items:
1. Unit testing framework: 
    * [testing_framework.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-2/sprint_2/testing_framework.h)
    * [testing_framework.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-2/sprint_2/testing_framework.cpp)
2. Unit tests for `SearchServer` class:
    * [server_test.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-2/sprint_2/server_test.h)
    * [server_test.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-2/sprint_2/server_test.cpp)
3. Unit tests execution from the the [main.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-2/main.cpp) 

## Sprint 3


:exclamation: 
**All changes in** `sprint_3` **are related to the changes in** `SearchServer` 
**class, which was implemented in** `sprint_1`, **that's why I made all 
modifications and refactoring in SPRINT_1 folder.**
:exclamation:

Code with updated `SearchServer` version and corresponding to the changes tests could be found on branch:
[dev/sprint-3](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-3).

Implemented items:
1. Updates in `SearchServer` class:
    * [server.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-3/sprint_1/server.h)
    * [server.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-3/sprint_1/server.cpp)
2. Updates in unit tests for `SearchServer` class:
   * [server_test.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-3/sprint_2/server_test.h)
   * [server_test.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-3/sprint_2/server_test.cpp)
   
P.S. Additionally I've updated the `unit-test` framework, to make it easy to use. 

Main changes are (
[testing_framework.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-3/sprint_2/testing_framework.h);
[testing_framework.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-3/sprint_2/testing_framework.cpp) ):
* `ASSERT_THROW` macros, which checks if the function should throw
* `RunTests` class which is used to run all tests implicitly
* Existing functionality refactoring to match first two items :)








# Notes