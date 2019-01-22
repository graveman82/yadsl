#include <algorithm>
#include "UniqIntGen.h"
#ifdef YADSL_USE_WXDEBUG
#include <wx/wx.h>
#endif

namespace yadsl
{

UniqIntGenerator::~UniqIntGenerator() {
#if YADSL_TEST_UNIQINTGENERATOR
    wxASSERT(IsValidWhenAllPutBack());
#endif
}

void UniqIntGenerator::Generate() {
    wxASSERT(generated_.empty());
    generated_.push_back(num_++);
}

uint UniqIntGenerator::Get() {
    if (generated_.empty()) {
        Generate();
    }
    wxASSERT(!generated_.empty());
    uint n = generated_.back();
    generated_.pop_back();
    return n;
}

bool UniqIntGenerator::Put(uint n) {
    if (n >= num_) { /* num_ увеличивается только тогда, когда генерируется новое число. Все числа i < num_ ранее сгенерированы.
    */
#ifdef YADSL_USE_WXDEBUG
        wxFAIL_MSG(wxT("out of range"));
#endif
        return false;
    }

    std::vector<uint>::iterator pos = std::lower_bound(generated_.begin(), generated_.end(), n);
    if (pos != generated_.end()) {
        if (*pos > n) {
            generated_.insert(pos, n);
            return true;
        }
        else /* pos == n */ {
#ifdef YADSL_USE_WXDEBUG
            wxASSERT(*pos == n); // case *pos < n  is unreachable, check it
            wxFAIL_MSG(wxT("duplicate inserting detected"));
#endif
            return false;
        }
    }
    // все числа в контейнере меньше, чем вставляемое
    generated_.push_back(n);
    return true;
}

#if YADSL_TRACE_UNIQINTGENERATOR
void UniqIntGenerator::TraceToConsole(bool fShowMemAddressOfInstance) {
    printf("UniqIntGenerator %s%x%s -- %d: ",
           fShowMemAddressOfInstance ? "(" : "",
           (int)this,
           fShowMemAddressOfInstance ? ")" : "",
           num_);
    for (uint i = 0; i < Unused(); i++) {
        printf("%d%s ", Data()[i], (i == Unused() -1) ? ".\n" : ",");
    }
    if (Unused() == 0) printf("\n");
}
#endif

#if YADSL_TEST_UNIQINTGENERATOR
bool UniqIntGenerator::IsValidWhenAllPutBack() const {
    for (size_t i = 0; i < generated_.size(); i++) {
        if (generated_[i] != i) return false;
    }
    return true;
}
#endif

} // end of yadsl


//-----------------------------------------------------------------------------

#if 0 // test code
{


#include <iostream>
#include <stdio.h> // printf()
#ifdef YADSL_USE_WXDEBUG
#include <wx/wx.h>
#endif

#include "UniqIntGen.h"

#include <stdlib.h> // rand()
#include  <algorithm>

using yadsl::uint;

int Random(int high) {
    double k = (double)rand() / RAND_MAX;
    return high * k;
}

bool RandomBool() {
    return (rand()%2) == 0;
}

// Распечатать элементы буфера с заданными индексами
void PrintBufByGivenIndices(const uint* a, const std::vector<int>& indexSeq) {
    printf("  buf elements with given index: \n  ");
    size_t cInRow = 0;
    for (size_t i = 0; i < indexSeq.size(); i++)  {
        printf("[%2d]: %-3d ", indexSeq[i], a[indexSeq[i]]);
        if (++cInRow == 6) {
            printf("\n  ");
            cInRow = 0;
        }
    }
    printf("\n");

    if (indexSeq.size() == 0) printf("\n");
}

// Распечатать элементы буфера, где индексы не равны значениям
void PrintBuf(const uint* a, size_t bufSize) {
    printf("  buf(with value != index): \n  ");
    size_t cInRow = 0;

    for (size_t i = 0; i < bufSize; i++)  {
        if (a[i] != i) {
            printf("[%2d]: %-3d ", i, a[i]);
            if (++cInRow == 6) {
                printf("\n  ");
                cInRow = 0;
            }
        }

    }

    printf("\n");

    if (bufSize == 0) printf("\n");
}

void Pass(int iPass, uint* buf, size_t bufSize,
          std::vector<int>& indexSeq,
          yadsl::UniqIntGenerator& uig) {

    printf("-- %d pass --\n", iPass);
    size_t cToPut = 0;
    while (cToPut == 0)
        cToPut = Random(bufSize / 3); // берем треть от размера буфера в качестве размера массива индексов
    printf ("Indices to put and get back: %d\n", cToPut);
    indexSeq.clear();
    /*
    Заполняем массив индексов
    */
    while(indexSeq.size() < cToPut) {
        int index = Random(bufSize - 1); // случайно выбираем индекс
        if (std::find(indexSeq.begin(), indexSeq.end(), index) == indexSeq.end()) {
            indexSeq.push_back(index);
        }
    }

    PrintBufByGivenIndices(buf, indexSeq);
    /* Возвращаем в генератор числа из буфера по заданным индексам
    */
    printf("Put in uig...\n");
    for (size_t i = 0; i < indexSeq.size(); i++) {
        uig.Put(buf[indexSeq[i]]);
    }
    printf("Put in uig done\n");

    uig.TraceToConsole();
    printf("\n");
    /* Помещаем по тем же индексам ранее возвращенные в генератор но теперь уже вновь выделяемые числа
    */
    printf("Put back in buf from uig...\n");
    for (size_t i = 0; i < indexSeq.size(); i++) {
        buf[indexSeq[i]] = uig.Get();
    }
    printf("Put back in buf from uig done\n");
    PrintBuf(buf, bufSize);
    printf("\n");
}

void Test() {
    yadsl::UniqIntGenerator uig;
    const size_t kBUF_SIZE = 30;
    uint a[kBUF_SIZE];

    printf("Fill buf by %d generated numbers from uig\n", kBUF_SIZE);
    for (size_t i = 0 ; i < kBUF_SIZE; i++) {
        a[i] = uig.Get();
    }
    std::vector<int> indexSeq;
    for (int iPass = 0 ; iPass < 5; iPass++) {
        Pass(iPass, a, kBUF_SIZE, indexSeq, uig);
    }

    printf("Put all numbers back to uig\n");
    for (size_t i = 0 ; i < kBUF_SIZE; i++) {
        uig.Put(a[i]);
    }
    PrintBuf(a, kBUF_SIZE);
    uig.TraceToConsole();

}


int main(){
    Test();

    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}

}
#endif // end of test code

