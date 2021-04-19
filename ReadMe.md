# Yandex Practicum C++

:boom: **Sprint #5 description added [here](#Sprint-5)** 

- [ ] Add `conan` package manager

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

Code with updated `SearchServer` version and corresponding to the changes tests could be found on branch:
[dev/sprint-3](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-3).

Implemented items:
1. Updates in `SearchServer` class:
    * [server.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-3/sprint_3/server_search.h)
    * [server.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-3/sprint_1/server_search.cpp)
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


## Sprint 4

Code with search server could be found here: [dev/sprint-4](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-4)

Implemented items:
1. Code of `SearchServer` engine was split up into several files, which are located in the folder: 
  [sprint_4](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-4/sprint_4)
2. Paginator functionality could be found here:
    * [paginator.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-4/sprint_4/paginator.h)
3. Query of requests to `SearchServer' could be found here:
    * [request_queue.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-4/sprint_4/request_queue.h)
    * [request_queue.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-4/sprint_4/request_queue.cpp)


## Sprint 5

:exclamation: :exclamation: :exclamation:

Code with search server could be found here: [dev/sprint-5](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-5)

Implemented items:
1. In `SearchServer` engine three function have been implemented: `GetWordFrequencies`, `RemoveDocument` and `RemoveDuplicates`
   
2. Duration logger functionality could be found here:
   * [log_duration.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-5/sprint_5/log_duration.h)
3. Boost & Gtest functionality have been integrate to the project:
   * [Gtest](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-5/tests)



# Notes