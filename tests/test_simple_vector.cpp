//
// Created by azakharov on 5/26/2021.
//

#include <gtest/gtest.h>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>

#include "../src/sprint_7/simple_vector.h"

using namespace std::literals;

class X {
public:  // Constructors & assigment operators
    X() : X(5) {}
    explicit X(size_t num) : x_(num) {}
    X(const X& other) = delete;
    X& operator=(const X& other) = delete;
    X(X&& other) noexcept {
        x_ = std::exchange(other.x_, 0);
    }
    X& operator=(X&& other) noexcept {
        x_ = std::exchange(other.x_, 0);
        return *this;
    }

public:  // Methods
    [[nodiscard]] size_t GetX() const {
        return x_;
    }

private:  // Fields
    size_t x_;
};

SimpleVector<int> GenerateVector(size_t size) {
    SimpleVector<int> v(size);
    std::iota(v.begin(), v.end(), 1);
    return v;
}

TEST(SimpleVector, ConstructorsBehaviour) {
    const size_t default_size = 5u;
    {
        SimpleVector<int> v;

        EXPECT_EQ(v.GetSize(), 0u) << "Size of vector, created with default constructor should be 0"s;
        EXPECT_TRUE(v.IsEmpty()) << "Vector, created with default constructor should be empty"s;
        EXPECT_EQ(v.GetCapacity(), 0u) << "Capacity of the vector, created with default constructor should be 0"s;
    }
    {
        SimpleVector<int> v(default_size);

        EXPECT_EQ(v.GetSize(), default_size) << "Vector size should be equal to the size, passed to the constructor"s;
        EXPECT_EQ(v.GetCapacity(), default_size)
            << "Vector capacity should be equal to the size, passed to the constructor"s;
        EXPECT_TRUE(!v.IsEmpty()) << "Vector with size, passed to constructor should not be empty"s;
        EXPECT_TRUE(std::all_of(v.begin(), v.end(), [](int value) { return value == 0; }))
            << "All elements of vector with passed size to the constructor should be equal to 0"s;
    }
    {
        const int default_value = 42;
        SimpleVector<int> v(default_size, default_value);
        EXPECT_EQ(v.GetSize(), default_size);
        EXPECT_EQ(v.GetCapacity(), default_size);
        EXPECT_TRUE(std::all_of(v.begin(), v.end(), [default_value](int value) { return value == default_value; }))
            << "All elements of vector with passed default value should be equal to this default value"s;
    }
    {
        SimpleVector<int> v{0, 1, 2};
        EXPECT_EQ(v.GetSize(), 3)
            << "Size of the vector, created with initializer list should be equal to the number of element in the list"s;
        EXPECT_EQ(v.GetCapacity(), 3)
            << "Capacity of the vector, created with initializer list should be equal to the number of element in the list"s;
        for (int id = 0; id < 3; ++id)
            EXPECT_EQ(v[id], id) << "Vector created with initializer list should have the same values as list does"s;
    }
    {
        SimpleVector<int> numbers{1, 2};
        auto numbers_copy(numbers);
        EXPECT_EQ(numbers_copy.GetSize(), numbers.GetSize())
            << "Copy constructor should have the same size as original"s;

        for (size_t id = 0; id < numbers.GetSize(); ++id) {
            EXPECT_EQ(numbers_copy[id], numbers[id]) << "Copy constructor should copy the values from original"s;
            EXPECT_TRUE(&numbers_copy[id] != &numbers[id])
                << "Copy constructor should copy data to another memory node"s;
        }
    }
    {
        SimpleVector<int> v(Reserve(default_size));
        EXPECT_EQ(v.GetCapacity(), default_size)
            << "Reserve constructor should reserve place for specified number of elements"s;
        EXPECT_TRUE(v.IsEmpty()) << "Reserve constructor should be empty"s;
    }
}

TEST(SimpleVector, AccessToTheVectorElements) {
    SimpleVector<int> v{0, 1, 2};
    EXPECT_EQ(&v.At(v.GetSize() - 1), &v[v.GetSize() - 1])
        << "Operator [] and method at() should reference the same element address for same index"s;
    EXPECT_THROW(v.At(v.GetSize()), std::out_of_range)
        << "Method at() should throw std::out_of_range exception if index is greater or equal than vector size"s;
}

TEST(SimpleVector, ClearTheVector) {
    const size_t initial_size = 10u;
    SimpleVector<int> v(initial_size);
    v.Clear();

    EXPECT_EQ(v.GetSize(), 0u) << "Method clear() should set size of vector to 0"s;
    EXPECT_EQ(v.GetCapacity(), initial_size) << "Method clear() should not change capacity of the vector"s;
}

TEST(SimpleVector, ResizeVector) {
    const size_t initial_size = 3u;
    {
        const size_t new_size = initial_size + 4u;
        SimpleVector<int> v(initial_size);
        v[2] = 17;
        v.Resize(new_size);

        EXPECT_EQ(v.GetSize(), new_size) << "Method resize() should make size of the vector equal to method argument"s;
        EXPECT_TRUE(v.GetCapacity() >= v.GetSize())
            << "Method resize() should make capacity equal to the new vector size if it is greater that initial one"s;
        EXPECT_EQ(v[2], 17) << "Method resize() should not change already initialized part of the vector"s;

        EXPECT_TRUE(std::all_of(v.begin() + initial_size, v.end(), [](int value) { return value == 0; }))
            << "Method resize() should be filled with default value part of vector after last existed element"s;
    }
    {
        SimpleVector<int> v{42, 55, 15};
        const size_t old_capacity = v.GetCapacity();
        v.Resize(initial_size - 1);

        EXPECT_EQ(v.GetSize(), initial_size - 1)
            << "Method resize() should make size of the vector equal to method argument"s;
        EXPECT_EQ(v.GetCapacity(), old_capacity)
            << "Method resize() should not change capacity if it's size is less that initial one"s;
        EXPECT_EQ(v[0], 42);
        EXPECT_EQ(v[1], 55);
    }
    {
        SimpleVector<int> v(initial_size);
        v.Resize(initial_size + 5);
        v[3] = 42;
        v.Resize(initial_size);
        v.Resize(initial_size + 2);
        EXPECT_EQ(v[3], 0)
            << "Method resize() should be filled with default value part of vector after last existed element"s;
    }
}
TEST(SimpleVector, IterateOveerVector) {
    {
        SimpleVector<int> v;
        EXPECT_EQ(v.begin(), nullptr) << "Empty vector begin() method should return nullptr"s;
        EXPECT_EQ(v.end(), nullptr) << "Empty vector end() method should return nullptr"s;
    }
    {
        SimpleVector<int> v(10, 42);
        EXPECT_TRUE(v.begin()) << "Method begin() for non-empty vector should not return nullptr"s;
        EXPECT_EQ(*v.begin(), 42) << "Method begin() should reference the first element in the array"s;
        EXPECT_EQ(v.end(), v.begin() + v.GetSize())
            << "Method end() should reference on the element after last in vector"s;
    }
}

TEST(SimpleVector, PushBackToVector) {
    {
        SimpleVector<int> v(1);
        v.PushBack(42);

        EXPECT_EQ(v.GetSize(), 2) << "Method PushBack() should increase size of the vector on 1"s;
        EXPECT_TRUE(v.GetCapacity() >= v.GetSize())
            << "Method PushBack() should reallocate memory if previous size is equal to capacity before the insertion"s;
        EXPECT_EQ(v[0], 0) << "Method PushBack() should not change previous content of the vector"s;
        EXPECT_EQ(v[1], 42) << "Method PushBack() should add element at the end of vector"s;
    }
}

TEST(SimpleVector, PopBackFromVector) {
    SimpleVector<int> v{0, 1, 2, 3};
    const size_t old_capacity = v.GetCapacity();
    const auto old_begin = v.begin();
    v.PopBack();

    EXPECT_EQ(v.GetCapacity(), old_capacity) << "Method PopBack() should not change capacity of the vector"s;
    EXPECT_EQ(v.begin(), old_begin) << "Method PopBack() should not reallocate the memory"s;
    EXPECT_TRUE((v == SimpleVector<int>{0, 1, 2})) << "Method PopBack() should remove only the last element"s;
}

TEST(SimpleVector, GeneralComparisonOperators) {
    EXPECT_TRUE((SimpleVector{1, 2, 3} == SimpleVector{1, 2, 3}));
    EXPECT_TRUE((SimpleVector{1, 2, 3} != SimpleVector{1, 2, 2}));

    EXPECT_TRUE((SimpleVector{1, 2, 3} < SimpleVector{1, 2, 3, 1}));
    EXPECT_TRUE((SimpleVector{1, 2, 3} > SimpleVector{1, 2, 2, 1}));

    EXPECT_TRUE((SimpleVector{1, 2, 3} >= SimpleVector{1, 2, 3}));
    EXPECT_TRUE((SimpleVector{1, 2, 4} >= SimpleVector{1, 2, 3}));
    EXPECT_TRUE((SimpleVector{1, 2, 3} <= SimpleVector{1, 2, 3}));
    EXPECT_TRUE((SimpleVector{1, 2, 3} <= SimpleVector{1, 2, 4}));
}

TEST(SimpleVector, SwapBehaviour) {
    SimpleVector<int> v1{42, 666};
    SimpleVector<int> v2;
    for (int id = 0; id < 3; ++id)
        v2.PushBack(id);

    const int* const begin1 = &v1[0];
    const int* const begin2 = &v2[0];

    const size_t capacity1 = v1.GetCapacity();
    const size_t capacity2 = v2.GetCapacity();

    const size_t size1 = v1.GetSize();
    const size_t size2 = v2.GetSize();

    static_assert(noexcept(v1.swap(v2)));
    v1.swap(v2);

    EXPECT_EQ(&v2[0], begin1) << "Method Swap() should swap memory nodes"s;
    EXPECT_EQ(&v1[0], begin2) << "Method Swap() should swap memory nodes"s;

    EXPECT_EQ(v1.GetSize(), size2) << "Method swap() should swap size"s;
    EXPECT_EQ(v2.GetSize(), size1) << "Method swap() should swap size"s;

    EXPECT_EQ(v1.GetCapacity(), capacity2) << "Method swap() should swap capacity"s;
    EXPECT_EQ(v2.GetCapacity(), capacity1) << "Method swap() should swap capacity"s;
}

TEST(SimpleVector, AssigmentOperator) {
    SimpleVector<int> src_vector{1, 2, 3, 4};
    SimpleVector<int> dst_vector{1, 2, 3, 4, 5, 6};
    dst_vector = src_vector;
    EXPECT_EQ(dst_vector, src_vector);
}

TEST(SimpleVector, InsertElementMethod) {
    SimpleVector<int> v{1, 2, 3, 4};
    v.Insert(v.begin() + 2, 42);
    EXPECT_TRUE((v == SimpleVector<int>{1, 2, 42, 3, 4}));
}

TEST(SimpleVector, EraseElementMethod) {
    SimpleVector<int> v{1, 2, 3, 4};
    auto position = v.begin() + 2;
    v.Erase(position);
    EXPECT_TRUE((v == SimpleVector<int>{1, 2, 4}));
}

TEST(SimpleVector, ReserveMethod) {
    SimpleVector<int> v;
    v.Reserve(5);

    EXPECT_EQ(v.GetCapacity(), 5) << "Method reserve() should make capacity equal to the argument value"s;
    EXPECT_TRUE(v.IsEmpty());

    v.Reserve(1);
    EXPECT_EQ(v.GetCapacity(), 5)
        << "Method reserve() should make reduce capacity if new size is less than the old one"s;

    for (int i = 0; i < 10; ++i)
        v.PushBack(i);
    EXPECT_EQ(v.GetSize(), 10);

    v.Reserve(100);

    EXPECT_EQ(v.GetSize(), 10)
        << "Method reserve() should not change size, if new capacity is greater than the old one"s;
    EXPECT_EQ(v.GetCapacity(), 100) << "Method reserve() should make capacity equal to the argument value"s;

    for (int i = 0; i < 10; ++i)
        EXPECT_EQ(v[i], i) << "Method reserve() should change existed elements"s;
}

TEST(SimpleVector, TestTemporaryObjConstructor) {
    const size_t size = 1000000;
    SimpleVector<int> moved_vector(GenerateVector(size));
    EXPECT_EQ(moved_vector.GetSize(), size) << "Test with temporary object, copy elision"s;
}

TEST(SimpleVector, TemporaryObjOperator) {
    const size_t size = 1000000;
    SimpleVector<int> moved_vector;
    EXPECT_EQ(moved_vector.GetSize(), 0);
    moved_vector = GenerateVector(size);
    EXPECT_EQ(moved_vector.GetSize(), size)
        << "Test with temporary object, operator=(). Size of moved vector should be equal to 0"s;
}

TEST(SimpleVector, NamedMoveConstructor) {
    const size_t size = 1000000;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    EXPECT_EQ(vector_to_move.GetSize(), size);

    SimpleVector<int> moved_vector(std::move(vector_to_move));
    EXPECT_EQ(moved_vector.GetSize(), size) << "Test with named object, move constructor. Size should not change"s;
    EXPECT_EQ(vector_to_move.GetSize(), 0)
        << "Test with named object, move constructor. Size of moved vector should be 0"s;
}
//
TEST(SimpleVector, NamedMoveOperator) {
    const size_t size = 1000000;
    SimpleVector<int> vector_to_move(GenerateVector(size));
    EXPECT_EQ(vector_to_move.GetSize(), size);

    SimpleVector<int> moved_vector = std::move(vector_to_move);
    EXPECT_EQ(moved_vector.GetSize(), size) << "Test with named object, operator=(). Size should not change"s;
    EXPECT_EQ(vector_to_move.GetSize(), 0)
        << "Test with named object, move constructor. Size of moved vector should be 0"s;
}

TEST(SimpleVector, NoncopiableMoveConstructor) {
    const size_t size = 5;
    SimpleVector<X> vector_to_move;
    for (size_t i = 0; i < size; ++i)
        vector_to_move.PushBack(X(i));

    SimpleVector<X> moved_vector = std::move(vector_to_move);
    EXPECT_EQ(moved_vector.GetSize(), size) << "Test noncopiable object, move constructor. Size should not change"s;
    EXPECT_EQ(vector_to_move.GetSize(), 0)
        << "Test noncopiable object, move constructor. Size of moved vector should be 0"s;

    for (size_t i = 0; i < size; ++i)
        EXPECT_EQ(moved_vector[i].GetX(), i);
}

TEST(SimpleVector, NoncopiablePushBack) {
    const size_t size = 5;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i)
        v.PushBack(X(i));

    EXPECT_EQ(v.GetSize(), size)
        << "Test noncopiable push back. Size should be equal to the number of inserted elements"s;

    for (size_t i = 0; i < size; ++i)
        EXPECT_EQ(v[i].GetX(), i);
}

TEST(SimpleVector, NoncopiableInsert) {
    const size_t size = 5;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i)
        v.PushBack(X(i));

    v.Insert(v.begin(), X(size + 1));
    EXPECT_EQ(v.GetSize(), size + 1) << "Insert to the beginning should increase size"s;
    EXPECT_EQ(v.begin()->GetX(), size + 1);

    v.Insert(v.end(), X(size + 2));
    EXPECT_EQ(v.GetSize(), size + 2) << "Insert to the end should increase size"s;
    EXPECT_EQ((v.end() - 1)->GetX(), size + 2);

    v.Insert(v.begin() + 3, X(size + 3));
    EXPECT_EQ(v.GetSize(), size + 3) << "Insert to the middle should increase size"s;
    EXPECT_EQ((v.begin() + 3)->GetX(), size + 3);
}

TEST(SimpleVector, NoncopiableErase) {
    const size_t size = 3;
    SimpleVector<X> v;
    for (size_t i = 0; i < size; ++i)
        v.PushBack(X(i));

    auto it = v.Erase(v.begin());
    EXPECT_EQ(it->GetX(), 1);
}
