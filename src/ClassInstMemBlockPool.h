#ifndef YADSL_CLASSINSTMEMBLOCKPOOL_H_
#define YADSL_CLASSINSTMEMBLOCKPOOL_H_


#ifdef YADSL_USE_IDVALUEVECTOR_IN_CLASSINSTANCEMEMBLOCKPOOL
#include "IdValueVector.h"
#else
#include <map>
#include "BaseTypes.h"
#endif

#include <wx/wx.h>
#include <wx/SharedPtr.h>

#include "UniqIntGen.h"

namespace yadsl
{

#define YADSL_TEST_CLASSINSTANCEMEMBLOCKPOOL 0
/** @brief Пул блоков памяти для размещения экземпляров заданного в шаблоне типа. Это не обязательно классы, тип хранимых данных может быть
и структурой и даже встроенным типом. Однако не рекомендуется размещать в пуле встроенные типы и маленькие структуры из-за больших накладных
расходов на память.
Вся память, выделенная из кучи экземпляром этого класса, будет автоматически возвращена в кучу при уничтожении экземпляра этого класса.

Пример использования:
@code
class Person {
public:
    std::string name_;
    int age_;

    Person( const char* name, int age) : name_(name), age_(age) {}
};
ClassInstanceMemBlockPool<Person> personPool;
void* mem = personPool.Alloc();
new(mem) Person("James Cameron", 64);
Person* director = (Person*)mem;
printf("Person: %s %d years old\n",
       director->name_.c_str(),
       director->age_);
personPool.Free(director);

mem = personPool.Alloc();
new(mem) Person("Arnold Schwarzenegger", 71);
Person* actor = (Person*)mem;
printf("Person: %s %d years old\n",
       actor->name_.c_str(),
       actor->age_);

printf("Person pool size: %d\n", personPool.AllBlockNum());
@endcode

###Сборка###
Макрос YADSL_USE_IDVALUEVECTOR_IN_CLASSINSTANCEMEMBLOCKPOOL отвечает за реализацию на основе IdValueVector. Без установки
этого макроса пул будет собран на основе std::map.
*/
template <typename T>
class ClassInstanceMemBlockPool {
private:
    // Блок памяти
    struct MemBlock {
        uint8_t data_[sizeof(T)];   // непосредственно сам участок памяти
        uint id_;                    // идентификатор блока памяти
        int fConstructed_; // флагом конструированности взята замысловатая последовательность бит (надежнее чем просто true), 0xA965

        MemBlock(uint id) : id_(id), fConstructed_(0) {}
#if YADSL_TEST_CLASSINSTANCEMEMBLOCKPOOL // if test on
        ~MemBlock() { printf("MemBlock(%d) dtor\n", id_); }
#endif
    };

    typedef MemBlock* PMemBlock;
#ifdef YADSL_USE_IDVALUEVECTOR_IN_CLASSINSTANCEMEMBLOCKPOOL
    typedef IdValueVector<wxSharedPtr<MemBlock> > MemBlockMap;
#else
    typedef std::map<uint, wxSharedPtr<MemBlock> > MemBlockMap;
#endif

    UniqIntGenerator uig_; // генератор уникальных целочисленных идентификаторов
    MemBlockMap pool_;      // пул свободных блоков памяти

    // Получить указатель на память блока по его идентификатору
    uint8_t* Mem(uint blockId) {
#ifdef YADSL_USE_IDVALUEVECTOR_IN_CLASSINSTANCEMEMBLOCKPOOL
        IdValueVector::Element* freeBlockPos= pool_.Find(blockId);
    #ifdef YADSL_USE_WXDEBUG
        wxASSERT(freeBlockPos != 0);
        wxASSERT(blockId == freeBlockPos->GetValue()->id_);
    #endif
        return &freeBlockPos->GetValue()->data_[0];

#else // std::map
        typename MemBlockMap::iterator freeBlockPos = pool_.find(blockId);
    #ifdef YADSL_USE_WXDEBUG
        wxASSERT(freeBlockPos != pool_.end());
        wxASSERT(blockId == freeBlockPos->second->id_);
    #endif
        return &freeBlockPos->second->data_[0];
#endif
    }

public:
    /** @brief Выделить свободный блок памяти.

    @note Если в пуле не останется свободных блоков, они будут добавлены в пул из кучи.
    */
    void* Alloc() {
        bool fUigEmpty = (uig_.Unused() == 0) ? true: false;
        uint blockId = kNoId;
        if (fUigEmpty) {
            PMemBlock memBlock = new(std::nothrow) MemBlock(blockId);
            if (memBlock == 0) {
                return 0;
            }
            blockId = uig_.Get();
            memBlock->id_ = blockId;
#ifdef YADSL_USE_IDVALUEVECTOR_IN_CLASSINSTANCEMEMBLOCKPOOL
            pool_.Insert(blockId, wxSharedPtr<MemBlock>(memBlock));
#else
            pool_[blockId] = wxSharedPtr<MemBlock>(memBlock);
#endif
        }
        else {
            blockId = uig_.Get();
        }
        return static_cast<void*>(Mem(blockId));
    }

    /** @brief Вернуть блок в пул свободных блоков.

    Указатель на этот блок теперь опасен, так как данный блок может быть выделен заново под
    другой экземпляр. Может возникнуть логическая ошибка, когда данные в блоке будут изменяться через старый указатель.
    Рекомендуется приравнять заданный указатель к нулю после вызова этой функции.

    @note Блок не возвращается в кучу.
    */
    void Free(void* p, bool fEraseMemContent = false) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
#endif
        PMemBlock block = reinterpret_cast<PMemBlock>(p);
        uig_.Put(block->id_);
        if (fEraseMemContent) {
            memset(&block->data_[0], 0, sizeof(block->data_));
        }
#if YADSL_TEST_CLASSINSTANCEMEMBLOCKPOOL // if test on
        strcpy((char*)block->data_, "unnamed");
#endif
    }

    /// Возвращает true, если заданный блок был помечен как блок, в котором сконструирован экземпляр класса
    bool MarkedAsConstructed(void* p) const {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
#endif
        MemBlock* block = reinterpret_cast<PMemBlock>(p);
        return block->fConstructed_ == 0xA965;
    }

    /// Пометить заданный блок как блок, в котором сконструирован экземпляр класса
    void MarkAsConstructed(void* p) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
#endif
        MemBlock* block = reinterpret_cast<PMemBlock>(p);
        block->fConstructed_ = 0xA965;
    }

    /// Снять метку с заданного блока как блока, в котором сконструирован экземпляр класса
    void MarkAsDestructed(void* p) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
#endif
        MemBlock* block = reinterpret_cast<PMemBlock>(p);
        block->fConstructed_ = 0;
    }

    /// Возвращает число всех блоков, свободных и выделенных. Для оценки потребления памяти и отладки.
    int AllBlockNum() const { return uig_.Num(); }
};

} // end of yadsl


//-----------------------------------------------------------------------------

#if 0 // code for test

#include <iostream>
#include <stdio.h> // printf()
#include <wx/wx.h>

#include "ClassInstMemBlockPool.h"
#include <new>

class Dummy {
public:
    char name_[16];
    int age_;

    Dummy(const char* name, int age) : age_(age) {
        strcpy(name_, name);
    }

    void Print() {
        int* blockId = (int*)(this + 1);
        printf("I am %s and %d years old (block id: %d)\n", name_, age_, *blockId);
    }
};

void Test() {

    yadsl::ClassInstanceMemBlockPool <Dummy> memPool;
    void* p = memPool.Alloc();
    Dummy* d1 = new(p) Dummy("Petrovich", 65);
    p = memPool.Alloc();
    Dummy* d2 = new(p) Dummy("Jho", 18);
    p = memPool.Alloc();
    Dummy* d3 = new(p) Dummy("Bolo Yene", 70);
    d1->Print();
    d2->Print();
    d3->Print();
    printf("\n");

    memPool.Free(d3);
    memPool.Free(d1);
    d1->Print();
    d2->Print();
    d3->Print();
    printf("\n");

    p = memPool.Alloc();
    Dummy* d4 = new(p) Dummy("Suzy", 25);
    d1->Print();
    d2->Print();
    d3->Print();
    d4->Print();
    printf("\n");

    p = memPool.Alloc();
    Dummy* d5 = new(p) Dummy("MacGregor", 47);
    d1->Print();
    d2->Print();
    d3->Print();
    d4->Print();
    d5->Print();
    printf("\n");

    p = memPool.Alloc();
    Dummy* d6 = new(p) Dummy("Hafiz", 95);
    d1->Print();
    d2->Print();
    d3->Print();
    d4->Print();
    d5->Print();
    d6->Print();
    printf("\n");
}

int main(){

    Test();

    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}

#endif

#endif // YADSL_CLASSINSTMEMBLOCKPOOL_H_

