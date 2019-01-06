#ifndef YADSL_IDVALUEVECTOR_H_
#define YADSL_IDVALUEVECTOR_H_

/** @file IdValueVector.h.

Назначение: вектор с возможностью хранение пар "идентификатор-значение" с логарифмичеси зависимой скоростью поиска элементов.
Статус: нет тестировано.
*/

#include <vector>
#include <algorithm>

#include "BaseTypes.h"

namespace yadsl
{

// Элемент вектора IdValueVector
template <typename T>
class IdValueVectorElement {
public:
    enum { /** @brief Недопустмое значение идентификатора. */ kNoId = 0xffffffff  };

private:
    uint id_;   // Идентификатор (имя) элемента
    T value_;   // Значение элемента

public:
    IdValueVectorElement(uint id = kNoId) : id_(id) {}
    uint GetId() const { return id_; }
    const T& GetValue() const { return value_; }
    T& GetValue() { return value_; }
    void SetValue(const T& value) { value_ = value; }

    bool operator < (const IdValueVectorElement<T>& oth) const { return id_ < oth.id_; }
    bool operator == (const IdValueVectorElement<T>& oth) const { return id_ == oth.id_; }
    bool operator != (const IdValueVectorElement<T>& oth) const { return !(*this == oth); }
};

/** @brief Вектор, в котором поддерживается упорядоченность элементов. */
template <typename T>
class IdValueVector {
public:
    typedef IdValueVectorElement<T> Element;
    typedef std::vector<Element> ElementVec;

private:
    ElementVec v_;

public:
    /** @brief Поиск элемента в векторе по заданному идентификатору.
    @return указатель на элемент или 0.
    @note указатель действителен до первой операции вставки или стирания элемента в векторе.
    */
    Element* Find(uint id) {
        Element elemToFound(id);
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFound);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFound);
        /* Случай 1 - v: e1, e2, ..., eN, где eN < elemToFound. Тогда:
        start:  v_.end();
        end:    v_.end().

        Случай 2 - v: e1, e2, ..., eN, где eN = elemToFound. Тогда:
        start:  eN;
        end:    v_.end().

        Случай 3 - v: e1, e2, ..., eN-1, eN, где eN-1 = elemToFound, eN > elemToFound. Тогда:
        start:  eN-1;
        end:    eN.

        Случай 3 - v: e1, e2, ..., eN, где eN > elemToFound. Тогда:
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
    @return true, если элемент был вставлен, false - в обратном случае.
    */
    bool Insert(uint id, const T& value) {
        Element elemToFound(id);
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFound);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFound);
        if (start != end) {
            return false;
        }
        elemToFound.SetValue(value);
        if (start == v_.end()) {
            v_.push_back(elemToFound);
        }
        else {
            v_.insert(start, elemToFound);
        }
        return true;
    }

    /** @brief Стереть элемент с заданным идентификатором.

    Факт стирания можно определить по идентификатору возвращенного элемента. Если элемент был стерт, его идентификатор
    должен иметь допустимое значение.
    @return Если элемент найден, возвращается его копия, если нет - элемент, инициализированный значением по умолчанию.
    */
    Element Erase(uint id) {
        Element elemToFound(id) ;
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFound);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFound);
        if (start != v_.end() && start != end) {
            elemToFound.SetValue(start->GetValue());
            v_.erase(start);
            return elemToFound;
        }
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

public:
    /** @brief Поиск элемента в векторе по заданному идентификатору.
    @param start - итератор вектора, содежащий позицию первого элемента, равного или большего, чем заданный.
    @param start - итератор вектора, содежащий позицию первого элемента, большего, чем заданный.
    @return true, если элемент будет найден.
    */
    bool Find(uint id,
              typename ElementVec::iterator& start,
              typename ElementVec::iterator& end ) {
        Element elemToFound(id);
        start = std::lower_bound(v_.begin(), v_.end(), elemToFound);
        end = std::upper_bound(v_.begin(), v_.end(), elemToFound);

        if (start != end) {
            return true;
        }
        return false;
    }

    /** @brief Вставка элемента с заданным идентификатором и значением.
    */
    void Insert(uint id, const T& value) {
        Element elemToFound(id);
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFound);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFound);

        elemToFound.SetValue(value);
        if (start == v_.end()) {
            v_.push_back(elemToFound);
        }
        else {
            v_.insert(start, elemToFound);
        }
    }

    /** @brief Стереть элементы с заданным идентификатором.
    */
    void Erase(uint id) {
        Element elemToFound(id) ;
        typename ElementVec::iterator start = std::lower_bound(v_.begin(), v_.end(), elemToFound);
        typename ElementVec::iterator end = std::upper_bound(v_.begin(), v_.end(), elemToFound);
        if (start != v_.end() && start != end) {
            v_.erase(start, end);
        }
    }

    /** @brief Стереть элемент, используя заданный итератор.
    */
    void Erase(typename ElementVec::iterator& pos) {
        v_.erase(pos);
    }

    /** @brief Возвращает размер вектора. */
    size_t Size() const { return v_.size(); }
    /** @brief Доступ к элементу по индексу. */
    const Element& operator[] (uint index) const { return v_[index]; }
    /** @brief Стереть все элементы в векторе. */
    void Clear() { v_.clear(); }
};


//-----------------------------------------------------------------------------

#if 0 // test code
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

void PrintVector(const yadsl::IdValueVector<float>& v, const char* comment = 0) {
    if (comment == 0) printf("v: ");
    else printf("%s. v: ", comment);
    for (size_t i = 0; i < v.Size(); i++) {
        printf("%d, ", v[i].GetId());
    }
    if (v.Size() == 0) printf("empty");
    printf("\n");
}

// Распечатать значения элементов буфера, которые будут вставляться в вектор
void PrintBufToPut(const unsigned int* a, const std::vector<unsigned int>& indexSeq) {
    printf("  selected values from buffer: ");
    for (size_t i = 0; i < indexSeq.size(); i++)  {
        printf("%d, ", a[indexSeq[i]]);
    }
    printf("\n");
}

void PrintBuf(const unsigned int* a, unsigned int bufSize) {
    printf("buf: ");
    for (size_t i = 0; i < bufSize; i++)  {
        printf("%d, ", a[i]);
    }
    printf("\n");
}

void Test1() {
    yadsl::IdValueVector<float> v;

    const unsigned int kBUF_SIZE = 50;
    const unsigned int kMAX_INDEX = kBUF_SIZE - 1;
    unsigned int ids[kBUF_SIZE];

    // Наполняем вектор идентификаторами [0, 50)
    for (unsigned int i = kBUF_SIZE; i > 0 ; i--) {
        v.Insert(i - 1, 0.0f);
    }
    PrintVector(v, "Vector initialized by [0, 50)"); //

    // Наполняем буфер, стирая вектор
    for (unsigned int i = 0 ; i < kBUF_SIZE; i++) {
        ids[i] = v.Erase(i).GetId();
    }
    PrintVector(v, "Vector erased while buffer filling"); // Не должно остаться ни одного элемента
    PrintBuf(ids, kBUF_SIZE); // 0, 1 ,... , 49


    std::vector<unsigned int> indexSeq;
    const int cPass = 10;
    for (int iPass = 0 ; iPass < cPass; iPass++) {
        // Сколько элементов добавить в вектор
        unsigned int cToPut = Random(kBUF_SIZE / 3); // аксимум 1/3 от общего числа элементов в буфере ids
        indexSeq.clear();
        // Наполняем контейнер индексов
        while(indexSeq.size() < cToPut) {
            unsigned int index = Random(kMAX_INDEX);
            if (std::find(indexSeq.begin(), indexSeq.end(), index) == indexSeq.end()) {
                indexSeq.push_back(index);
            }
        }
        printf("%d pass\n", iPass);
        PrintBufToPut(ids, indexSeq);
        // Вставляем выбранные идентификаторы в вектор
        for (size_t i = 0; i < indexSeq.size(); i++) {
            unsigned int index = indexSeq[i];
            v.Insert(ids[index], 0);
        }
        PrintVector(v, "Vector was filled from buffer using index sequence");
        printf("\n");

        // Наполняем буфер, стирая вектор
        for (unsigned int i = 0 ; i < indexSeq.size(); i++) {
            unsigned int index = indexSeq[i];
            ids[index] = v[i].GetId();
        }
        v.Clear();
        PrintVector(v, "Vector erased while buffer filling"); // Не должно остаться ни одного элемента
        PrintBuf(ids, kBUF_SIZE); //

    }

    for (unsigned int i = 0 ; i < kBUF_SIZE; i++) {
        unsigned int index = i;
        v.Insert(ids[index], 0);
    }
    PrintBuf(ids, kBUF_SIZE);
    PrintVector(v);

}

// Сгенерировать массив имен
void GenerateNames(std::vector<std::string>& names, size_t n) {
    std::vector<char> chars;
    for (char c = 'a'; c < 'z'; c++ ) {
        chars.push_back(c);
    }

    size_t kMaxLength = 10;
    names.clear();
    while (names.size() < n) {
        std::string s;
        size_t l = Random(kMaxLength);
        if (l == 0) l++;
        while (s.length() < l) {
            s.push_back(chars[Random(chars.size() - 1)]);
        }
        names.push_back(s);
    }

}

void Test2() {

    std::vector<std::string> names;
    yadsl::IdValueVector<std::string> v;

    for (int pass = 0; pass < 10; pass++) {
        GenerateNames(names, 40); // сгенерировать 20 имен
        size_t cToPut = Random(names.size() / 4);
        size_t vOldSize = v.Size();
        while (v.Size() - vOldSize < cToPut) {
            size_t iName = Random(names.size() - 1);
            v.Insert(iName + pass * names.size(), names[iName]);
            //printf("putting: %d -- \"%s\"\n", iName, names[iName].c_str());
        }

        size_t cToErase = Random(cToPut / 2);
        for (size_t i = 0; i < cToErase; i++) {
            size_t iToErase = Random(v.Size() - 1);
            v.Erase(v[iToErase].GetId());
        }
    }


    // printing
    printf("v: \n");
    for (size_t i = 0; i < v.Size(); i++) {
        printf("%d -- \"%s\"\n", v[i].GetId(), v[i].GetValue().c_str());
    }

}

void Test3() {

    std::vector<std::string> names;
    yadsl::IdValueMultiVector<std::string> v;
    std::multimap<yadsl::uint, std::string> m;

    for (int pass = 0; pass < 10; pass++) {
        GenerateNames(names, 40); // сгенерировать 20 имен
        size_t cToPut = Random(names.size() / 4);
        size_t vOldSize = v.Size();
        while (v.Size() - vOldSize < cToPut) {
            size_t iName = Random(names.size() - 1);
            v.Insert(iName + pass * names.size(), names[iName]);
            m.insert(std::make_pair(iName + pass * names.size(), names[iName]));
            //printf("putting: %d -- \"%s\"\n", iName, names[iName].c_str());
        }

        size_t cToErase = Random(cToPut / 2);
        for (size_t i = 0; i < cToErase; i++) {
            size_t iToErase = Random(v.Size() - 1);
            yadsl::uint id = v[iToErase].GetId();
            v.Erase(id);

            //
            std::multimap<yadsl::uint, std::string>::iterator start = m.lower_bound(id);
            std::multimap<yadsl::uint, std::string>::iterator end = m.upper_bound(id);
            if (start != m.end() && start != end) {
                m.erase(start, end);
            }
        }
    }


    // printing
    printf("v: \n");
    for (size_t i = 0; i < v.Size(); i++) {
        printf("%d -- \"%s\"\n", v[i].GetId(), v[i].GetValue().c_str());
    }

    printf("m: \n");
    for (std::multimap<yadsl::uint, std::string>::iterator it = m.begin(); it != m.end(); ++it) {
        printf("%d -- \"%s\"\n", it->first, it->second.c_str());
    }

}

int main(){
    Test1();

    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}

#endif

} // end of yadsl

#endif // YADSL_IDVALUEVECTOR_H_

