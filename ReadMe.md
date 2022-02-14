# Yandex Practicum C++

:boom: **Sprint #15 description added [here](#Sprint-16)**

- [ ] Add `conan` package manager (could not find packages)
- [X] Make good `CMAKE` basing on chosen compiler
- [X] Add `ANTLR` to the project

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
3. Unit tests execution from the
   the [main.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-2/main.cpp)

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

Code with search server could be found
here: [dev/sprint-4](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-4)

Implemented items:

1. Code of `SearchServer` engine was split up into several files, which are located in the folder:
   [sprint_4](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-4/sprint_4)
2. Paginator functionality could be found here:
    * [paginator.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-4/sprint_4/paginator.h)
3. Query of requests to `SearchServer' could be found here:
    * [request_queue.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-4/sprint_4/request_queue.h)
    * [request_queue.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-4/sprint_4/request_queue.cpp)

## Sprint 5

Code with search server could be found
here: [dev/sprint-5](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-5)

Implemented items:

1. In `SearchServer` engine three function have been implemented: `GetWordFrequencies`, `RemoveDocument`
   and `RemoveDuplicates`

2. Duration logger functionality could be found here:
    * [log_duration.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-5/src/sprint_5/log_duration.h)
3. Boost & Gtest functionality have been integrate to the project:
    * [Gtest](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-5/tests)

## Sprint 6

Code with search server could be found
here: [dev/sprint-6](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-6)

Implemented items:

1. `SingleLinkedList` template class could be found here:
    * [single_linked_list.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint_6/src/sprint_6/single_linked_list.h)
    * [single_linked_list.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint_6/src/sprint_6/single_linked_list.cpp)
2. Unit tests of `SingleLinkedList` class:
    * [test_single_linked_list.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint_6/tests/test_single_linked_list.cpp)

## Sprint 7

Code with search server could be found
here: [dev/sprint-7](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint_7)

Implemented items:

1. `SimpleVector` template class could be found here:
    * [simple_vector.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint_7/src/sprint_7/simple_vector.h)
    * [array_ptr.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint_7/src/sprint_7/array_ptr.h)
2. Unit tests of `SimpleVector` class:
    * [test_simple_vector.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint_7/tests/test_simple_vector.cpp)

## Sprint 8

Code with search server could be found
here: [dev/sprint-8](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-8)

Implemented items:

1. Multi-treading for methods of `SearchServer` class:
    * Methods list: `RemoveDocument()`, `FindAllDocuments()`, `ParseQuery()`, `FindTopDocuments()`, `MatchDocument()`
    * Could be
      found [search_server.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-8/sprint_8/search_server.h)
2. Change `std::string` on `std::string_view` for performance improvement:
    * Could be
      found [search_server.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-8/sprint_8/search_server.h)
3. Add multi-threading functions to process queries:
    * [process_queries.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-8/sprint_8/process_queries.h)
4. Class `ConcurrentMap` and `ForEach()` method to thread safe process `std::map<k,v>`:
    * [concurent_map.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-8/sprint_8/concurent_map.h)

## Sprint 9

Code with search server could be found
here: [dev/sprint-9](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-9)

Implemented items:

1. `TransportCatalogue` class:
    * Could be
      found [transport_catalogue.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-9/src/sprint_9/transport_catalogue.h)
2. Input data readers:
    * Could be
      found [stat_reader.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-9/src/sprint_9/stat_reader.h)
3. Output statistics writers:
    * Could be
      found [input_reader.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-9/src/sprint_9/input_reader.h)

## Sprint 10

Code with transport catalogue visualization
map: [dev/sprint-10](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-10)

Implemented items:

1. `TransportCatalogue` class update:
    * Could be
      found [transport_catalogue.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-10/src/sprint_10/transport_catalogue.h)
2. `JsonReader` library:
    * Could be
      found [json_reader.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-10/src/sprint_10/json_reader.h)
3. `SvgReader` library:
    * [svg.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-10/src/sprint_10/svg.h)

![alt text](/data/img/simple_map.jpg)

## Sprint 11

Code with JSON builder could be found
here: [dev/sprint-11](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-11)

Implemented items:

1. Class `Builder`, which provides an interface for JSON creation:
    * [json_builder.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-11/src/sprint_11/json_builder.h)
    * [json_builder.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-11/src/sprint_11/json_builder.cpp)
2. Request handler adjustment to creation of JSON response via `Builder` class.
    * [request_handler.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-11/src/sprint_11/request_handler.cpp)

## Sprint 12

Code with JSON builder could be found
here: [dev/sprint-12](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-12)

Implemented items:

1. Class `TransportRouter`, which builds the fastest route from point `A` to point `B`:
    * [transport_router.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-12/src/sprint_12/transport_router.h)
    * [transport_router.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-12/src/sprint_12/transport_router.cpp)

In the graph, used to build the fastest routes, each stop represents as 2 vertexes:

* `start` - passenger arrives to this vertex on the stop (necessary to take into account bus waiting)
* `end` - passenger start wide from this vertex

P.S. Implementation notes:

* Each route starts from the `start` vertex of the first stop, and ends with `start` vertex of the last stop.
* Each bus ride starts with the `end` vertex stop and ends on the `start` vertex.
* Bus could go through the same stop several times - only the lowes time is chosen to build the fastest route.

## Sprint 13

Code changes for `Optional<Type>` and `Vector<Type>` could be found
here: [dev/sprint-13](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-13)

Implemented items:

1. Class `Vector<Type>` is analog of `std::vector<Type>`:
    * [vector.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-13/src/sprint_13/vector.h)
2. Class `Optional<Type>` is analog of `std::optional<Type>:
    * [optional.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-13/src/sprint_13/optional.h)

P.S. Both classes implementation use **raw memory**, so the speed should be close to the analogs from `STL` library.

## Sprint 14

Code changes could be found here: [dev/sprint-14](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-14)

Implemented items:

1. Serialization of `TransportCatalogue`, `TransportRouter`, `Settings` (both: routing and rendering):
    * [serialization.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-14/src/sprint_14/serialization.h)
    * [serialization.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-14/src/sprint_14/serialization.cpp)
2. Requests handling is now split into 2 parts, where on the intermediate state we serialize all the date.

## Sprint 15

Code changes could be found here: [dev/sprint-15](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-15)

Implemented items:
Language interpreter, consists from the following steps:

1. Lexical analyzer:
    * [lexer.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-15/src/sprint_15/lexer.h)
    * [lexer.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-15/src/sprint_15/lexer.cpp)
2. Syntax analyzer (runtime + parser):
    * [runtime.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-15/src/sprint_15/runtime.h)
    * [runtime.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-15/src/sprint_15/runtime.cpp)
3. Semantics analyzer:
    * [statement.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-15/src/sprint_15/statement.h)
    * [statement.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-15/src/sprint_15/statement.cpp)
4. Table of symbols

## Sprint 16

:exclamation: :exclamation: :exclamation:

Code changes could be found here: [dev/sprint-16](https://github.com/ZakharovAleksey/yandex_cpp/tree/dev/sprint-16)

Simple Excel-like sheet to work with text and formulas only.

Main implemented items:
1. Sheet:
   * [sheet.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-16/src/sprint_16/sheet.h)
   * [sheet.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-16/src/sprint_16/sheet.cpp)
2. Cell:
   * [cell.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-16/src/sprint_16/cell.h)
   * [cell.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-16/src/sprint_16/cell.cpp)
3. Formulas:
   * [formula.h](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-16/src/sprint_16/formula.h)
   * [formula.cpp](https://github.com/ZakharovAleksey/yandex_cpp/blob/dev/sprint-16/src/sprint_16/formula.cpp)
4. Adjustments to use formula parser with [ANTLR](https://www.antlr.org/) 

# Notes

## ANTLR Installation

Steps to install & test:
1. Download & install **Java SE Runtime Environment 8**
2. Download & install **Java SE Development Kit 8**
3. Download *.jar file & set up all Path environment variable & *.bat files: [here](https://superuser.com/questions/1397513/please-help-me-resolve-error-antlr-is-not-recognized-command)
4. Test the installation: [here](https://github.com/antlr/antlr4/blob/master/doc/getting-started.md)

Steps to integrate with C++:
1. Documentation page: [here](https://github.com/antlr/antlr4/blob/master/doc/cpp-target.md)
2. Notes: [here](https://beyondtheloop.dev/Antlr-cpp-cmake/)

P.S. In general, if ANTLR has been installed successfully, the only thing u need to do to integrate it with C++ 
is just download the `runtime` module (see. above) together with `*.jar` file and place it in project folder. 
CMakeFile will handle everything else. 


