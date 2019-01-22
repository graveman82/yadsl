#ifndef YADSL_IDVALUEVECTOR_H_
#define YADSL_IDVALUEVECTOR_H_

/** @file IdValueVector.h.

Назначение: вектор с возможностью хранение пар "идентификатор-значение" с логарифмичеси зависимой скоростью поиска элементов.
Статус: нет тестировано.
*/

#include <vector>
#include <algorithm>

#include "BaseTypes.h"

/// флаг включения теста для класса вектора с возможностью хранение пар "идентификатор-значение". Включать только при тестировании вектора (негативно сказывается на производительности)
#define YADSL_TEST_IDVALUEVECTOR 0
#if YADSL_TEST_IDVALUEVECTOR
#include <algorithm>
#include <wx/wx.h>
#endif
namespace yadsl
{


/** @brief Элемент вектора IdValueVector и IdValueMultiVector.
Данные хранятся в векторе не в прямом виде, а внутри этого класса - экземпляр типа данных вложен в этот класс.
Обратиться непосредственно к самим данным можно через метод GetValue(), который возвращает ссылку на экземпляр типа данных.
@code
elem->GetValue().<member>
@endcode
*/
template <typename T>
class IdValueVectorElement {
private:
    uint id_;   // Идентификатор (имя) элемента
    T value_;   // Значение элемента

public:
    IdValueVectorElement(uint id = kNoId) : id_(id) {}
    /// Доступ к идентификатору
    uint GetId() const { return id_; }
    /// Доступ к значению (чтение)
    const T& GetValue() const { return value_; }
    /// Доступ к значению (чтение и запись)
    T& GetValue() { return value_; }
    /// Изменение значения
    void SetValue(const T& value) { value_ = value; }

    bool operator < (const IdValueVectorElement<T>& oth) const { return id_ < oth.id_; }
    bool operator == (const IdValueVectorElement<T>& oth) const { return id_ == oth.id_; }
    bool operator != (const IdValueVectorElement<T>& oth) const { return !(*this == oth); }
};

/** @brief Вектор, в котором поддерживается упорядоченность элементов.
Пример использования:
@code
struct Person {
    std::string name_;
    int age_;
};
IdValueVector<Person> v;
uint JamesCameronId = 2029;
Person director;
director.name_ = "James Cameron";
director.age_ = 64;
v.Insert(JamesCameronId, director);
IdValueVectorElement<Person>* elem = v.Find(JamesCameronId);
printf("Person(%d): %s %d years old\n",
       JamesCameronId,
       elem->GetValue().name_.c_str(),
       elem->GetValue().age_);
v.Erase(JamesCameronId);
printf("Is vector empty? %s\n", v.Size() == 0 ? "yes" : "no");
@endcode
*/
template <typename T>
class IdValueVector {
public:
    typedef IdValueVectorElement<T> Element;
    typedef std::vector<Element> ElementVec;

private:
    ElementVec v_;

#if YADSL_TEST_IDVALUEVECTOR
    bool IsSorted() const {
        ElementVec vcopy = v_;
        std::stable_sort(vcopy.begin(), vcopy.end());
        for (size_t i = 0; i < v_.size(); i++) {
            if (v_[i] != vcopy[i]) return false;
        }
        return true;
    }

#endif
public:
    /** @brief Поиск элемента в векторе по заданному идентификатору.
    @return указатель на элемент или 0.
    @note указатель действителен до первой операции вставки или стирания элемента в векторе.
    */
    Element* Find(uint id) {
        Element elemToFind(id);
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFind);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFind);
        /* Случай 1 - v: e1, e2, ..., eN, где eN < elemToFind. Тогда:
        start:  v_.end();
        end:    v_.end().

        Случай 2 - v: e1, e2, ..., eN, где eN = elemToFind. Тогда:
        start:  eN;
        end:    v_.end().

        Случай 3 - v: e1, e2, ..., eN-1, eN, где eN-1 = elemToFind, eN > elemToFind. Тогда:
        start:  eN-1;
        end:    eN.

        Случай 3 - v: e1, e2, ..., eN, где eN > elemToFind. Тогда:
        start:  eN;
        end:    eN.

        В случаях, когда элемент, равный заданному, существует, start != end
        */

        if (start != end) {
            return &(*start);
        }
        return 0;
    }

    /** @brief Вставка элемента с заданным идентификатором и значением.

    Вставляется копия элемента.
    @param id идентификатор нового элемента.
    @param value значение нового элемента.
    @return true, если элемент был вставлен, false - в обратном случае (когда элемент с таким идентификатором уже присутствует в контейнере).
    */
    bool Insert(uint id, const T& value) {
        Element elemToFind(id);
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFind);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFind);
        if (start != end) {
            return false;
        }
        elemToFind.SetValue(value);
        if (start == v_.end()) {
            v_.push_back(elemToFind);
        }
        else {
            v_.insert(start, elemToFind);
        }
#if YADSL_TEST_IDVALUEVECTOR
        wxASSERT(IsSorted());
#endif
        return true;
    }

    /** @brief Стереть элемент с заданным идентификатором.

    Факт стирания можно определить по идентификатору возвращенного элемента. Если элемент был стерт, его идентификатор
    должен иметь допустимое значение.
    @return Если элемент найден, возвращается его копия, если нет - элемент, инициализированный значением по умолчанию.
    */
    Element Erase(uint id) {
        Element elemToFind(id) ;
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFind);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFind);
        if (start != v_.end() && start != end) {
            elemToFind.SetValue(start->GetValue());
            v_.erase(start);
#if YADSL_TEST_IDVALUEVECTOR
            wxASSERT(IsSorted());
#endif
            return elemToFind;
        }
#if YADSL_TEST_IDVALUEVECTOR
        wxASSERT(IsSorted());
#endif
        return Element();
    }

    /** @brief Возвращает размер вектора. */
    size_t Size() const { return v_.size(); }
    /** @brief Доступ к элементу по индексу. */
    const Element& operator[] (uint index) const { return v_[index]; }
    /** @brief Стереть все элементы в векторе. */
    void Clear() { v_.clear(); }
};

/** @brief Вектор, в котором поддерживается упорядоченность элементов.
Возможны элементы с одинаковыми идентификаторами.
*/
template <typename T>
class IdValueMultiVector {
public:
    typedef IdValueVectorElement<T> Element;
    typedef std::vector<Element> ElementVec;

private:
    ElementVec v_;
#if YADSL_TEST_IDVALUEVECTOR
    bool IsSorted() const {
        ElementVec vcopy = v_;
        std::stable_sort(vcopy.begin(), vcopy.end());
        for (size_t i = 0; i < v_.size(); i++) {
            if (v_[i] != vcopy[i]) return false;
        }
        return true;
    }

#endif
public:

    typename ElementVec::iterator EndIterator() { return v_.end(); }

    /** @brief Поиск элемента в векторе по заданному идентификатору.
    @param start[out] - итератор вектора, содежащий позицию первого элемента, равного или большего, чем заданный.
    @param end[out] - итератор вектора, содежащий позицию первого элемента, большего, чем заданный.
    @return true, если элемент будет найден.
    */
    bool Find(uint id,
              typename ElementVec::iterator& start,
              typename ElementVec::iterator& end ) {
        Element elemToFind(id);
        start = std::lower_bound(v_.begin(), v_.end(), elemToFind);
        end = std::upper_bound(v_.begin(), v_.end(), elemToFind);

        if (start != end) {
            return true;
        }
        return false;
    }

    /** @brief Вставка элемента с заданным идентификатором и значением.
    */
    void Insert(uint id, const T& value) {
        Element elemToFind(id);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFind);

        elemToFind.SetValue(value);
        if (end == v_.end()) {
            v_.push_back(elemToFind);
        }
        else {
            v_.insert(end, elemToFind);
        }
#if YADSL_TEST_IDVALUEVECTOR
        wxASSERT(IsSorted());
#endif
    }

    /** @brief Стереть все элементы с заданным идентификатором.
    */
    void Erase(uint id) {
        Element elemToFind(id) ;
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFind);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFind);
        if (start != v_.end() && start != end) {
            v_.erase(start, end);
        }
#if YADSL_TEST_IDVALUEVECTOR
        wxASSERT(IsSorted());
#endif
    }

    /** @brief Стереть элемент, используя заданный итератор.
    */
    void Erase(typename ElementVec::iterator& pos) {
        v_.erase(pos);
#if YADSL_TEST_IDVALUEVECTOR
        wxASSERT(IsSorted());
#endif
    }

    /** @brief Возвращает размер вектора. */
    size_t Size() const { return v_.size(); }
    /** @brief Доступ к элементу по индексу. */
    const Element& operator[] (uint index) const { return v_[index]; }
    /** @brief Стереть все элементы в векторе. */
    void Clear() { v_.clear(); }
};


//-----------------------------------------------------------------------------

} // end of yadsl

#if 0 // test code
{
    #include <iostream>
#include <stdio.h> // printf()
#include <wx/wx.h>

#include "IdValueVector.h"

#include <stdlib.h> // rand()
#include <algorithm>
#include <map>

int Random(int high) {
    double k = (double)rand() / RAND_MAX;
    return high * k;
}

template<typename V>
void PrintVector(const V& v, const char* comment = 0) {
    if (comment == 0) printf("v: ");
    else printf("%s. v: ", comment);
    for (size_t i = 0; i < v.Size(); i++) {
        printf("%d, ", v[i].GetId());
    }
    if (v.Size() == 0) printf("empty");
    printf("\n");
}

void Test() {
    yadsl::IdValueVector<float> v;
    yadsl::uint a[] = {10, 12, 5, 3, 8, 1, 17};
    for (size_t i = 0; i < sizeof(a) / sizeof(yadsl::uint); i++) {
        v.Insert(a[i], 0.0);
    }
    PrintVector(v, "Vector filled");

    v.Erase(10);
    v.Erase(3);

    PrintVector(v, "3, 10 was erased from vector");

    v.Insert(4, 0.0);
    v.Insert(20, 0.0);
    v.Insert(11, 0.0);

    PrintVector(v, "4, 20, 11 was inserted to vector");
}

void Test2() {
    yadsl::IdValueMultiVector<float> v;
    yadsl::uint a[] = {
        10, 12, 5, 3, 8, 1, 17, 20,
        12, 8, 1, 17
    };
    for (size_t i = 0; i < sizeof(a) / sizeof(yadsl::uint); i++) {
        v.Insert(a[i], 0.0);
    }
    PrintVector(v, "Vector filled");

    v.Erase(10);
    v.Erase(12);
    v.Erase(3);

    PrintVector(v, "10, 12, 3 was erased from vector");

    v.Insert(11, 0.0);
    v.Insert(4, 0.0);
    v.Insert(8, 0.0);
    v.Insert(11, 0.0);
    v.Insert(17, 0.0);

    PrintVector(v, "11, 4, 8, 11, 17 was inserted to vector");
}

int main(){
    Test2();

    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}

}
#endif // end of test code

#endif // YADSL_IDVALUEVECTOR_H_

