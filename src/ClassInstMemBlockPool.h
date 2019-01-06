#ifndef YADSL_CLASSINSTMEMBLOCKPOOL_H_
#define YADSL_CLASSINSTMEMBLOCKPOOL_H_

#include <map>

#include <wx/wx.h>
#include <wx/SharedPtr.h>

#include "UniqIntGen.h"

namespace yadsl
{

#define CLASSINSTANCEMEMBLOCKPOOL_TEST 0
/** @brief Пул блоков памяти для размещения экземпляров заданного в шаблоне типа.
Вся память, выделенная из кучи экземпляром этого класса, будет автоматически возвращена в кучу при уничтожении экземпляра этого класса.
*/
template <typename T>
class ClassInstanceMemBlockPool {
private:
    // Блок памяти
    struct MemBlock {
        uint8_t data_[sizeof(T)];   // непосредственно сам участок памяти
        int id_;                    // идентификатор блока памяти
        int fConstructed_; // флагом конструированности взята замысловатая последовательность бит (надежнее чем просто true), 0xA965

        MemBlock(int id) : id_(id), fConstructed_(0) {}
#if CLASSINSTANCEMEMBLOCKPOOL_TEST // if test on
        ~MemBlock() { printf("MemBlock(%d) dtor\n", id_); }
#endif
    };

    typedef MemBlock* PMemBlock;
    typedef std::map<int, wxSharedPtr<MemBlock> > MemBlockMap;

    UniqIntGenerator uig_; // генератор уникальных целочисленных идентификаторов
    MemBlockMap pool_;      // пул свободных блоков памяти

    // Получить указатель на память блока по его идентификатору
    uint8_t* Mem(int blockId) {
        typename MemBlockMap::iterator freeBlockPos = pool_.find(blockId);
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(freeBlockPos != pool_.end());
        wxASSERT(blockId == freeBlockPos->second->id_);
#endif
        return &freeBlockPos->second->data_[0];
    }

public:
    /** @brief Выделить свободный блок памяти.

    @note Если в пуле не останется свободных блоков, они будут добавлены в пул из кучи.
    */
    void* Alloc() {
        bool fUigEmpty = (uig_.Size() == 0) ? true: false;
        int blockId = UniqIntGenerator::kNoId;
        if (fUigEmpty) {
            PMemBlock memBlock = new(std::nothrow) MemBlock(blockId);
            if (memBlock == 0) {
                return 0;
            }
            blockId = uig_.Get();
            memBlock->id_ = blockId;
            pool_[blockId] = wxSharedPtr<MemBlock>(memBlock);
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
#if CLASSINSTANCEMEMBLOCKPOOL_TEST // if test on
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

