//
// Created by azakharov on 4/27/2021.
//

#include <gtest/gtest.h>

#include "../src/sprint_6/single_linked_list.h"

using namespace sprint_6::data_structures;
using namespace std::literals;

constexpr int list_size{5};

TEST(SingleLinkedList, EmptyListStateIsValid) {
    SingleLinkedList<int> list;

    EXPECT_TRUE(list.IsEmpty()) << "IsEmpty method should detect when list is empty"s;
    EXPECT_EQ(list.GetSize(), 0u) << "Size of empty array should be equal to zero"s;
}

TEST(SingleLinkedList, PushFrontMethod) {
    SingleLinkedList<int> list;

    for (int id = 1; id < list_size; ++id) {
        list.PushFront(id);
        EXPECT_EQ(*list.begin(), id) << "Push front should insert elements at list start"s;
        EXPECT_EQ(list.GetSize(), static_cast<size_t>(id))
            << "Size of list should increase when element has been pushed front"s;
    }
}

TEST(SingleLinkedList, ClearEmptyList) {
    SingleLinkedList<int> list;
    EXPECT_TRUE(list.IsEmpty());

    list.Clear();
    EXPECT_TRUE(list.IsEmpty()) << "List should be valid and empty if we call Clear methods for the empty list"s;
}

TEST(SingleLinkedList, ClearNonEmptyList) {
    SingleLinkedList<int> list;

    auto add_elements = [&list](int elements_count) {
        for (int element = 0; element < elements_count; ++element)
            list.PushFront(element);
    };

    for (int attempt_id = 1; attempt_id < list_size; ++attempt_id) {
        add_elements(attempt_id);
        EXPECT_EQ(list.GetSize(), attempt_id) << "Expected non-empty list for this test"s;

        list.Clear();
        EXPECT_TRUE(list.IsEmpty()) << "The list should be empty after Clear method call for "s +
                                           std::to_string(attempt_id) + "elements";
    }
}

TEST(SingleLinkedList, EmptyListBehaviour) {
    SingleLinkedList<int> list;
    const auto& constant_list = list;

    EXPECT_TRUE(list.before_begin() == list.cbefore_begin())
        << "Method before_begin() should return the same results for const and NON-const objects"s;

    EXPECT_TRUE(list.before_begin() == constant_list.before_begin())
        << "Method before_begin() should return the same results for const and NON-const objects"s;

    EXPECT_TRUE(list.begin() == list.end()) << "NON-const iterators on begin()/end() of empty list should be equal"s;

    EXPECT_TRUE(constant_list.begin() == constant_list.end())
        << "Const iterators on begin()/end() of empty list should be equal"s;

    EXPECT_TRUE(constant_list.cbegin() == constant_list.cend())
        << "Const iterators on cbegin()/cend() of empty list should be equal"s;

    EXPECT_TRUE(list.cbegin() == constant_list.end())
        << "Const iterators on cbegin()/end() of empty list should be equal"s;

    EXPECT_TRUE(list.cend() == constant_list.begin())
        << "Const iterators on begin()/cend() of empty list should be equal"s;
}

TEST(SingleLinkedList, NonEmptyListBehaviour) {
    SingleLinkedList<int> list;
    const auto& constant_list = list;

    for (int id = 1; id < list_size; ++id) {
        list.PushFront(id);

        EXPECT_TRUE(!list.IsEmpty()) << "List with elements should not marked be empty"s;
        EXPECT_EQ(list.GetSize(), id) << "List with " + std::to_string(id) +
                                             " inserted element(s) should have the same size"s;

        EXPECT_TRUE(list.begin() != list.end()) << "Iterators ob begin()/end() for non-empty list should be different"s;
        EXPECT_TRUE(list.cbegin() != constant_list.end());
        EXPECT_TRUE(list.cend() != constant_list.begin());
    }
}

TEST(SingleLinkedList, ListIteratorPostfixPrefixIncrement) {
    std::vector<int> elements = {1, 2, 3};

    SingleLinkedList<int> list;
    for (auto iter = elements.rbegin(); iter != elements.rend(); ++iter)
        list.PushFront(*iter);

    SingleLinkedList<int>::Iterator iterator_to_current = list.begin();
    for (const auto element : elements) {
        EXPECT_EQ(*iterator_to_current, element) << "PREFIX increment should work as expected"s;
        ++iterator_to_current;
    }

    iterator_to_current = list.begin();
    for (const auto element : elements) {
        EXPECT_EQ(*iterator_to_current, element) << "POSTFIX increment should increase the current iterator"s;
        iterator_to_current++;
    }

    EXPECT_TRUE(iterator_to_current == list.end())
        << "Incremented last element iterator should point to the list end()"s;
}

TEST(SingleLinkedList, IteratorsConverting) {
    SingleLinkedList<int> list;
    list.PushFront(1);

    SingleLinkedList<int>::ConstantIterator constant_iterator(list.begin());
    ASSERT_TRUE(constant_iterator == list.cbegin());
    ASSERT_TRUE(*constant_iterator == *list.cbegin());

    SingleLinkedList<int>::ConstantIterator constant_iterator_2;
    // Присваивание ConstIterator-у значения Iterator
    constant_iterator_2 = list.begin();
    ASSERT_TRUE(constant_iterator_2 == constant_iterator);
}

TEST(SingleLinkedList, RandeBasedForSupport) {
    std::vector<int> elements = {1, 2, 3};

    SingleLinkedList<int> list;
    for (const auto element : elements)
        list.PushFront(element);

    auto iterator_on_expected = elements.rbegin();
    for (auto actual_element : list)
        EXPECT_EQ(actual_element, *iterator_on_expected++) << "Forward range-based for should work as expected"s;
}

TEST(SingleLinkedList, DereferenceOperatorBehaviour) {
    SingleLinkedList<std::string> string_list;
    std::string insert_string{"first"s};
    char symbol = '!';

    string_list.PushFront(insert_string);
    EXPECT_TRUE(string_list.cbegin()->length() == insert_string.length());
    string_list.begin()->push_back(symbol);
    EXPECT_TRUE(*string_list.begin() == insert_string + symbol);
}

TEST(SingleLinkedList, ComparisonOperatorsForListsWithTheSameSize) {
    SingleLinkedList<int> first{1, 2, 3};
    SingleLinkedList<int> second{1, 2, 3};
    SingleLinkedList<int> third{1, 2, 4};

    EXPECT_TRUE(first == second) << "Lists should be equal, if they have the same elements and size"s;
    EXPECT_TRUE(first != third)
        << "List with the same size should be different if they have at least on different element"s;

    EXPECT_TRUE(first < third)
        << "operator<(); List with the first-to-meet less element should be less, if lists have the same sizes"s;
    EXPECT_TRUE(first <= third)
        << "operator<=(); List with the first-to-meet less element should be less or equal, if lists have the same sizes"s;
    EXPECT_TRUE(first <= second) << "operator<=(); Two equal lists should be less or equal each other"s;

    EXPECT_TRUE(third > first)
        << "operator>(); List with the first-to-meet less element should be greater, if lists have the same sizes"s;
    EXPECT_TRUE(third >= first)
        << "operator>=(); List with the first-to-meet less element should be greater or equal, if lists have the same sizes"s;
    EXPECT_TRUE(second >= first) << "operator>=(); Two equal lists should be greater or equal each other"s;
}

TEST(SingleLinkedList, ComparisonOperatorsForListsWithDifferentSizes) {
    SingleLinkedList<int> first{1, 2};
    SingleLinkedList<int> second{1, 2};
    SingleLinkedList<int> third{1, 2, 3};

    EXPECT_TRUE(first != third) << "Lists should NOT be equal if they have different sizes"s;

    EXPECT_TRUE(first < third) << "operator<(); List with smaller size should be less"s;
    EXPECT_TRUE(third > first) << "operator>(); List with bigger size should be greater"s;

    EXPECT_TRUE(first <= third) << "operator<=(); List with bigger size should be greater"s;
    EXPECT_TRUE(third >= first) << "operator>=(); List with smaller size should be less"s;
}

TEST(SingleLinkedList, SwapFunction) {
    SingleLinkedList<int> actual_left{1, 2, 3};
    SingleLinkedList<int> actual_right{4, 5};

    const auto expected_left = actual_right;
    const auto expected_right = actual_left;

    swap(actual_left, actual_right);

    EXPECT_EQ(actual_left, expected_left) << "Swap function should change elements of the lists."s;
    EXPECT_EQ(actual_right, expected_right) << "Swap function should change elements of the lists."s;
}

TEST(SingleLinkedList, SwapMethod) {
    SingleLinkedList<int> actual_left{1, 2, 3};
    SingleLinkedList<int> actual_right{4, 5};

    const auto expected_left = actual_right;
    const auto expected_right = actual_left;

    actual_left.swap(actual_right);

    EXPECT_EQ(actual_left, expected_left) << "Swap function should change elements of the lists."s;
    EXPECT_EQ(actual_right, expected_right) << "Swap function should change elements of the lists."s;
}

TEST(SingleLinkedList, InitializerListConstructor) {
    const std::vector<int> expected_elements = {1, 2, 3};
    auto expected_value = expected_elements.begin();

    SingleLinkedList<int> list = {1, 2, 3};

    EXPECT_EQ(list.GetSize(), expected_elements.size())
        << "Created from the Initializer list, linked list should have the same size as Initialized list has"s;

    for (const auto& element : list)
        EXPECT_EQ(element, *expected_value++)
            << "Initializer list should create single linked list with elements in correct order"s;
}

TEST(SingleLinkedList, CopyConstructor) {
    SingleLinkedList<int> empty_original;
    const auto empty_copy(empty_original);
    EXPECT_TRUE(empty_original == empty_copy) << "Copy constructor should create correct copy of EMPTY list"s;

    SingleLinkedList<int> original = {1, 2, 3};
    const auto copy(original);
    EXPECT_EQ(copy, original) << "Copy constructor should create correct copy of NON-empty list"s;
}

TEST(SingleLinkedList, AssigmentOperator) {
    SingleLinkedList<int> original = {1, 2, 3};
    SingleLinkedList<int> copy;
    copy = original;

    EXPECT_TRUE(copy == original) << "Copy constructor should create correct copy of list"s;
}

TEST(SingleLinkedList, ThowExceptionOnIncorrectCopy) {
    struct ThrowOnCopy {
        ThrowOnCopy() = default;
        explicit ThrowOnCopy(int& copy_counter) noexcept : countdown_ptr(&copy_counter) {}
        ThrowOnCopy(const ThrowOnCopy& other)
            : countdown_ptr(other.countdown_ptr)  //
        {
            if (countdown_ptr) {
                if (*countdown_ptr == 0) {
                    throw std::bad_alloc();
                } else {
                    --(*countdown_ptr);
                }
            }
        }
        // Присваивание элементов этого типа не требуется
        ThrowOnCopy& operator=(const ThrowOnCopy& rhs) = delete;
        // Адрес счётчика обратного отсчёта. Если не равен nullptr, то уменьшается при каждом копировании.
        // Как только обнулится, конструктор копирования выбросит исключение
        int* countdown_ptr = nullptr;

        void swap(ThrowOnCopy& other) noexcept {
            std::swap(countdown_ptr, other.countdown_ptr);
        }
    };

    SingleLinkedList<ThrowOnCopy> source;
    source.PushFront(ThrowOnCopy{});
    source.PushFront(ThrowOnCopy{});
    auto thrower = source.begin();
    source.PushFront(ThrowOnCopy{});

    int copy_counter = 0;
    thrower->countdown_ptr = &copy_counter;

    SingleLinkedList<ThrowOnCopy> destination;
    destination.PushFront(ThrowOnCopy{});
    int destination_counter = 10;
    destination.begin()->countdown_ptr = &destination_counter;
    destination.PushFront(ThrowOnCopy{});

    try {
        destination = source;
        EXPECT_TRUE(false) << "Exception on assigment should be thrown"s;
    } catch (const std::bad_alloc&) {
        EXPECT_EQ(destination.GetSize(), 2) << "List should not change it's size when exception has been thrown"s;
        auto current_iterator = destination.begin();
        EXPECT_TRUE(current_iterator != destination.end());
        EXPECT_TRUE(current_iterator->countdown_ptr == nullptr);

        ++current_iterator;
        EXPECT_TRUE(current_iterator != destination.end());
        EXPECT_TRUE(current_iterator->countdown_ptr == &destination_counter);
        EXPECT_EQ(destination_counter, 10);
    } catch (...) {
        EXPECT_TRUE(false) << "No other exceptions expected when exception has been thrown on assigment"s;
    }
}

TEST(SingleLinkedLit, InsertAfterIteratorMethod) {
    SingleLinkedList<int> list;
    list.InsertAfter(list.before_begin(), 1);
    EXPECT_EQ(list.GetSize(), 1) << "Method InsertAfter() should increase size of list on 1"s;
    EXPECT_EQ(*list.begin(), 1) << "Method InsertAfter() should correctly insert element at before_begin() position"s;

    list.InsertAfter(list.begin(), 2);
    EXPECT_EQ(list.GetSize(), 2) << "Method InsertAfter() should increase size of list on 1"s;
    EXPECT_EQ(*(++list.begin()), 2)
        << "Method InsertAfter() should correctly insert element at specified by iterator position"s;

    list.InsertAfter(++list.begin(), 3);
    EXPECT_EQ(list.GetSize(), 3) << "Method InsertAfter() should increase size of list on 1"s;
    EXPECT_EQ(*(++(++list.begin())), 3)
        << "Method InsertAfter() should correctly insert element at specified by iterator position"s;

    list.PushFront(4);
    EXPECT_EQ(list.GetSize(), 4);

    const SingleLinkedList<int> expected_list = {4, 1, 2, 3};
    EXPECT_EQ(list, expected_list) << "Methods: InsertAfter() and PushFront() should work correctly together"s;
}

TEST(SingleLinkedLit, PopFrontMethodForEmptyList) {
    SingleLinkedList<int> empty_list;

    ASSERT_DEATH(empty_list.PopFront();, ".*Assertion.*")
        << "PopFront() method should correctly work on the empty list"s;
}

TEST(SingleLinkedLit, PopFrontMethodForNonEmptyList) {
    SingleLinkedList<int> list{1, 2, 3, 4};
    const int initial_list_size = list.GetSize();

    // Remove all but the last element of the list
    for (int id = 1; id < initial_list_size; ++id) {
        list.PopFront();
        EXPECT_EQ(list.GetSize(), initial_list_size - id) << "PopFront() method should decrease size of list on 1"s;
        EXPECT_EQ(*list.begin(), id + 1);
    }

    // Remove the last element of the list
    list.PopFront();
    EXPECT_TRUE(list.IsEmpty()) << "List should be empty when all elements have been removed with PopFront() method"s;
}

TEST(SingleLinkedLit, EraseAfterMethodForNonEnptyList) {
    using IntList = SingleLinkedList<int>;
    SingleLinkedList<int> list{1, 2, 3, 4};

    auto position = list.EraseAfter(list.begin());
    EXPECT_EQ(list.GetSize(), 3) << "EraseAfter() method should decrease size of list on 1"s;
    EXPECT_EQ(list, IntList({1, 3, 4})) << "EraseAfter should correctly remove elements by iterator"s;

    position = list.EraseAfter(position);
    EXPECT_EQ(list.GetSize(), 2) << "EraseAfter() method should decrease size of list on 1"s;
    EXPECT_EQ(list, IntList({1, 3})) << "EraseAfter should correctly remove elements by iterator"s;

    list.EraseAfter(list.begin());
    EXPECT_EQ(list.GetSize(), 1) << "EraseAfter() method should decrease size of list on 1"s;
    EXPECT_EQ(list, IntList({1})) << "EraseAfter should correctly remove elements by iterator"s;

    EXPECT_EQ(position, list.end());

    list.EraseAfter(list.before_begin());
    EXPECT_TRUE(list.IsEmpty()) << "List should be empty after EraseAfter() removed the last element in the list"s;

    ASSERT_DEATH(list.EraseAfter(list.before_begin());, ".*Assertion.*")
        << "EraseAfter() method should assert if the list is empty"s;
}
