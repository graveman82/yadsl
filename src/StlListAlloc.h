#ifndef YADSL_STL_LIST_ALLOC_H
#define YADSL_STL_LIST_ALLOC_H

#include <vector>
#include <list>
#include <limits>

#define YADSL_LISTALLOCATOR_DEBUG 1 // вкл(1)/выкл(0) отладки аллокатора списка stl
#define YADSL_LISTALLOCATOR_DEBUG_USE_MSGBOX 1 // вкл/выкл использование диалога сообщений


#ifdef YADSL_USE_WXDEBUG
#include <wx/wx.h>
#endif
#include <wx/sharedptr.h>

#if YADSL_LISTALLOCATOR_DEBUG
#include <stdio.h>
#if YADSL_LISTALLOCATOR_DEBUG_USE_MSGBOX
#include <windows.h>
#endif
#endif

#include "BaseTypes.h"

/** @file StlListAlloc.h.
Аллокатор для std::list.
*/
namespace yadsl_private{
extern const char* stlListAllocReporter;
} // end of yadsl_private

namespace yadsl {

using std::size_t;
using std::ptrdiff_t;


//-----------------------------------------------------------------------------

template<int N>
class StlListAllocCache {
private:
    enum { kNodeMemBlockMarker = 0xA965 };
    // Блок памяти узла списка

    struct NodeMemBlock{
        uint8_t buf_[N + 2];
        uint16_t* MarkZone() { return reinterpret_cast<uint16_t*>(&buf_[N]); }
        const uint16_t* MarkZone() const { return reinterpret_cast<const uint16_t*>(const_cast<const uint8_t*>(&buf_[N])); }
        void Mark() { *MarkZone() = kNodeMemBlockMarker; }
        NodeMemBlock() { Mark(); }
        ~NodeMemBlock() { Mark(); }
        bool Marked() const { return (*MarkZone()) == kNodeMemBlockMarker; }
    };

    // кэш
    std::vector<NodeMemBlock*> v_;

    StlListAllocCache() {}
    StlListAllocCache(const StlListAllocCache& oth);
    StlListAllocCache& operator=(const StlListAllocCache& oth);
    static wxSharedPtr<StlListAllocCache<N> > singleton_;
public:
    static StlListAllocCache<N>& Instance() {
        if (singleton_.get() == 0) {
            singleton_ = wxSharedPtr<StlListAllocCache<N> >(new StlListAllocCache<N>());
        }
        return *singleton_;
    }

    ~StlListAllocCache() {
        ClearCache();
#ifdef YADSL_LISTALLOCATOR_DEBUG
#ifdef YADSL_LISTALLOCATOR_DEBUG_USE_MSGBOX
        wchar_t buf[128];
        wsprintf(buf, L"~StlListAllocCache(%d)\n", N);
        MessageBoxW(0, buf, L"[yadsl, stl list allocator]:", MB_TOPMOST | MB_OK);
#endif
#endif
    }
    // Выделить блок памяти для узла списка
    void* AllocateNode(){

        if (v_.empty()) {
            v_.push_back(new NodeMemBlock());
        }
        NodeMemBlock* mem = v_.back();
#ifdef YADSL_LISTALLOCATOR_DEBUG
        printf("%s allocated mem block markered byte contains:%X\n", yadsl_private::stlListAllocReporter, (int)(*mem->MarkZone()));
#endif
        v_.pop_back();
        return static_cast<void*>(&mem->buf_[0]);
    }

    // Освободить выделенную ранее для узла списка память
    void DeallocateNode(void* p){
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
#endif
        NodeMemBlock* mem = static_cast<NodeMemBlock*>(p);
#ifdef YADSL_LISTALLOCATOR_DEBUG
        printf("%s deallocated mem block markered byte contains:%X\n", yadsl_private::stlListAllocReporter, (int)(*mem->MarkZone()));
#endif
#ifdef YADSL_USE_WXDEBUG
        wxASSERT_MSG(mem->Marked(), wxT("Its seems you trying deallocate mem which is not allocated by this allocator\n or it was writing out of allocated range"));
#endif
        v_.push_back(mem);
    }

    // Очистить кэш блоков памяти узлов списка
    void ClearCache(){
#ifdef YADSL_LISTALLOCATOR_DEBUG
        int cElem = 0;
#endif
        while (!v_.empty()){
            NodeMemBlock* mem = v_.back();
            v_.pop_back();
            delete mem;
#ifdef YADSL_LISTALLOCATOR_DEBUG
            cElem++;
#endif
        }
#ifdef YADSL_LISTALLOCATOR_DEBUG
        printf("%s yadsl Stl cache cleaned. Cache size was: %d elements\n", yadsl_private::stlListAllocReporter, cElem);
#endif
    }
};

template <int N>
wxSharedPtr<StlListAllocCache<N> > StlListAllocCache<N>::singleton_;

//-----------------------------------------------------------------------------

class StlListAllocNumException: public std::exception {
    virtual const char* what() const throw()
    {
        return "Too big number of elements to allocate passed";
    }
};

/** @brief Аллокатор для списка stl. */
template <typename LT>
class StlListAllocator {
private:


public:
	// Определения типов
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef LT* pointer;
	typedef const LT* const_pointer;
	typedef LT& reference;
	typedef const LT& const_reference;
	typedef LT value_type;

	/* Таким образом, rebind<T_OTH>::other - это StlListAllocator<T_OTH>
	*/
	template<typename T_OTH>
	struct rebind{
		typedef StlListAllocator<T_OTH> other;
	};

	/* Конструкторы и деструктор, которые ничего не делают,
	поскольку аллокатор не имеет состояния.
	*/
    StlListAllocator() throw() { }

    StlListAllocator(const StlListAllocator& oth) throw() { }

	template<typename T_OTH>
	StlListAllocator(const StlListAllocator<T_OTH>&) throw() { }

	~StlListAllocator() throw() { }

	pointer address(reference r) const noexcept { return std::addressof(r); }

	const_pointer address(const_reference r) const noexcept { return std::addressof(r); }

	pointer allocate(size_type num, const void* = 0) {
		if (num > 1 || num > this->max_size())
			throw StlListAllocNumException();
#if YADSL_LISTALLOCATOR_DEBUG
        printf("%s allocated %d elements of size %d bytes\n", yadsl_private::stlListAllocReporter, num, sizeof(LT));
#endif
        void* raw = StlListAllocCache<sizeof(LT)>::Instance().AllocateNode();
        return static_cast<LT*> (raw);
	}

	void deallocate(pointer p, size_type) {
	    StlListAllocCache<sizeof(LT)>::Instance().DeallocateNode(p);
#if YADSL_LISTALLOCATOR_DEBUG
        printf("%s deallocated element\n", yadsl_private::stlListAllocReporter);
#endif
    }

	/// Возвращает максимальное количество элементов, для которых может быть выделена память
	size_type max_size() const noexcept {
		return std::numeric_limits<size_t>::max() / sizeof(LT);
	}

	template<typename LT2, typename... Args>
	void construct(LT2* p, Args&&... args) {
		::new((void *)p) LT2(std::forward<Args>(args)...);
	}

	template<typename LT2>
	void destroy(LT2* p) {
		p->~LT2();
	}

};

template<typename T1, typename T2>
inline bool operator==(const StlListAllocator<T1>&, const StlListAllocator<T2>&) noexcept
{ return true; }

template<typename LT>
inline bool operator==(const StlListAllocator<LT>&, const StlListAllocator<LT>&) noexcept
{ return true; }

template<typename T1, typename T2>
inline bool operator!=(const StlListAllocator<T1>&, const StlListAllocator<T2>&) noexcept
{ return false; }

template<typename LT>
inline bool operator!=(const StlListAllocator<LT>&, const StlListAllocator<LT>&) noexcept
{ return false; }


} // end of yadsl

#endif // YADSL_STL_LIST_ALLOC_H
