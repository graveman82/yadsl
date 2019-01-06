#ifndef YADSL_EC_MANAGER_H_
#define YADSL_EC_MANAGER_H_

#include <vector>

#ifdef YADSL_USE_WXDEBUG
#include <wx/wx.h>
#endif

#include "BaseTypes.h"
#include "ClassInstMemBlockPool.h"
#include "Entity.h"


#ifdef YADSL_USE_OWNLIST_IN_ENTITY
#include "List.h"
#else
#include <list>
#endif


namespace yadsl
{

/// Вид компоненты
enum Ec_Kind {
    kEc_Kind_Class, ///< компонента-класс (требуется  память для размещения компоненты и внешний вызов конструктора)
    kEc_Kind_Pod,   ///< компонента-POD (требуется  память для размещения компоненты)
    kEc_Kind_Com,  ///< компонента-указатель на COM-объект
    kEc_Kind_Num
};

/** @brief Менеджер компоненты сущности.
@param N максимальное число компонент такого же типа в одной сущности.
*/
template <typename T, int Kind = kEc_Kind_Pod, int N = 1>
class Ec_Manager {
    typedef Entity* PEntity;
public:
#ifdef YADSL_USE_OWNLIST_IN_ENTITY
    typedef List<PEntity, kPOD_LIST> EntityAccessPoints;
#else
    typedef std::list<PEntity, ListAllocator<PEntity >> EntityAccessPoints;
#endif


private: // vars
    int id_; // идентификатор компоненты

    // пул блоков памяти для размещения компоненты
    ClassInstanceMemBlockPool<T>* memPool_;

    // контейнер точек доступа к экземплярам сущностей, которые содержат данную компоненту
    EntityAccessPoints owners_[N];

private: // methods

    bool NeedMem() const { return Kind == kEc_Kind_Class || Kind == kEc_Kind_Pod; }
    bool NeedOutCtorCall() const { return Kind == kEc_Kind_Class; }

    /** @brief Проверка наличия компоненты среди данных сущности.
    @param entity - указатель на сущность.
    @param componentIndex - индекс компоненты среди компонент такого же типа.
    @param [out] ppMem - адрес переменной, куда сохраняется указатель на память экземпляра типа компоненты с данным индексом. Может быть равен нулю.
    @return Возвращает true, если компонента найдена и false в обратном случае.
    */
    bool FindComponent(Entity* entity, uint componentIndex, void** ppMem = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(entity != 0);
#endif
        return entity->GetComponent(GetComponentId(), componentIndex, ppMem);
    }

    /** @brief Сконвертировать указатель на память компоненты в типизированный указатель.
    Допускается только для сконструированных компонент.
    */
    T* ConvertToComponentType(void* p) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
#endif

        if (NeedOutCtorCall()) {
#ifdef YADSL_USE_WXDEBUG
            wxASSERT(memPool_->MarkedAsConstructed(p));
#endif
        }
        return reinterpret_cast<T*> (p);
    }

    // Добавление точки доступа на сущность
    void AddEntityAccessPoint(Entity* entity, uint componentIndex) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(componentIndex < uint(N));
#endif
#ifdef YADSL_USE_OWNLIST_IN_ENTITY
        EntityAccessPoints::Node* ownerPos = owners_[componentIndex].push_back(entity);
        entity->SetComponentOwnerPos(GetComponentId(), componentIndex, ownerPos);
#else
        owners_[componentIndex].push_back(entity);
        entity->SetComponentOwnerPos(GetComponentId(), componentIndex, entity.get());
#endif
    }

    // Добавление точки доступа на сущность
    void RemoveEntityAccessPoint(Entity* entity, uint componentIndex) {
        // Удалить точку доступа на сущность
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(componentIndex < uint(N));
#endif
#ifdef YADSL_USE_OWNLIST_IN_ENTITY
        void* p = 0;
        entity->GetComponent(GetComponentId(), componentIndex, 0, &p);
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
#endif
        EntityAccessPoints::Node* ownerPos = reinterpret_cast<EntityAccessPoints::Node*>(p);
        owners_[componentIndex].erase(ownerPos);
        entity->SetComponentOwnerPos(GetComponentId(), componentIndex, ownerPos);
#else
        owners_[componentIndex].remove(entity);
#endif
    }

public:
    Ec_Manager() {
        id_ = Entity::GenerateComponentId();
        if (NeedMem()) {
            memPool_ =  new ClassInstanceMemBlockPool <T>;
        }
    }

    ~Ec_Manager() {
        if (NeedMem()) {
            delete memPool_;
        }
        for (uint i = 0; i < (uint)N; i++) {
            GetOwners(i).clear();
        }
    }

    ///< Возвращает идентификатор компоненты.
    int GetComponentId() const { return id_; }

    /** @brief Доступ к памяти компоненты сущности.
    @param entity - указатель на сущность.
    @param componentIndex - индекс компоненты среди компонент такого же типа. По умолчанию равен 0.
    @param [out] ppMem - адрес переменной, куда сохраняется указатель на память экземпляра типа компоненты с данным индексом.
    @return Возвращает true, если компонента найдена и false в обратном случае.
    */
    bool GetComponentMem(Entity* entity, void** ppMem, uint componentIndex = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(entity != 0);
        wxASSERT(ppMem != 0);
#endif
        return entity->GetComponent(GetComponentId(), componentIndex, ppMem);
    }

    /** @brief Доступ к компоненте сущности.
    @param entity - указатель на сущность.
    @param componentIndex - индекс компоненты среди компонент такого же типа. По умолчанию равен 0.
    @return Возвращает указатель на компоненту, если компонента найдена и для нее выделена память и 0 в обратном случае.
    */
    T* GetComponentOf(Entity* entity, uint componentIndex = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(entity != 0);
#endif
        void* p = 0;
        if (GetComponentMem(entity, &p, componentIndex)) {
            if (p == 0) return 0;
            return ConvertToComponentType(p);
        }
        return 0;
    }

    /** @brief Добавление экземпляра компоненты в сущность.

    Используйте этот метод для добавления компоненты в сущность.
    Для компонент-классов привязывается только память под экземпляр компоненты. Конструирования экземпляра компоненты здесь не происходит,
    поскольку при конструировании компоненты могут понадобиться параметры. Компоненту необходимо сконструировать в выделенной для нее памяти
    вне пределов этого класса, используя методы GetComponentMem() и MarkComponentAsConstructed().
    Пример использования для компонент-классов:
    @code
    // Компонента (не POD, а класс,поскольку есть член типа класса fname_)
    struct WeaponData {
        int ammo_;
        std::string fname_;

        WeaponData(int ammo, const char* fname) : ammo_(ammo), fname_(fname) {}
    };
    yadsl::Ec_Manager<WeaponData, yadsl::kEc_Kind_Class> weaponDataEcMng;// we have a std::string member --> component is not POD but is a class
    Entity* shotgun = new Entity;
    weaponDataEcMng.AddComponentTo(shotgun);

    // Конструируем компоненту-класс
    void* pShotgunWeaponDataMem = 0;
    weaponDataEcMng.GetComponentMem(shotgun, &pShotgunWeaponDataMem);
    new (pShotgunWeaponDataMem) WeaponData(12, "shotgun.dat");
    weaponDataEcMng.MarkComponentAsConstructed(shotgun);

    printf ("shotgun ammo: %d, cfg: '%s'\n", weaponDataEcMng.GetComponentOf(shotgun)->ammo_,
        weaponDataEcMng.GetComponentOf(shotgun)->fname_.c_str());

    // Удаление ресурсов
    weaponDataEcMng.RemoveComponentFrom(shotgun);
    delete shotgun;
    @endcode

    @param entity - указатель на сущность.
    @param componentIndex - индекс компоненты среди компонент такого же типа. По умолчанию равен 0.
    @return true, если нет ошибок.
    */
    bool AddComponentTo(Entity* entity, uint componentIndex = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT_MSG(NeedMem(), wxT("Operation is not allowed for this component kind"));
        wxASSERT(entity != 0);
        wxASSERT_MSG(!FindComponent(entity, componentIndex), wxT("avoid double component adding"));
#endif

        void* p = 0;
        p = memPool_->Alloc();
        if (p == 0) return false;
        entity->SetOrInsertComponent(GetComponentId(), componentIndex, p);
        AddEntityAccessPoint(entity, componentIndex);
        return true;
    }

    /** @brief Удалить экземпляр компоненты с заданным индексом из данных сущности.
    @param entity - указатель на сущность.
    @param componentIndex - индекс компоненты среди компонент такого же типа. По умолчанию равен 0.
    */
    void RemoveComponentFrom(Entity* entity, uint componentIndex = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT_MSG(NeedMem(), wxT("Operation is not allowed for this component kind"));
        wxASSERT(entity != 0);
#endif

        void* p = 0;
        bool fHasComponent = FindComponent(entity, componentIndex, &p);
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(fHasComponent);
        wxASSERT(p != 0);
#endif
        if (NeedOutCtorCall()) {
            T* instance = ConvertToComponentType(p);
#ifdef YADSL_USE_WXDEBUG
            wxASSERT(instance != 0);
#endif
            instance->~T();
            memPool_->MarkAsDestructed(p);
        }

        memPool_->Free(p);
        RemoveEntityAccessPoint(entity, componentIndex);
        //entity->SetOrInsertComponent(GetComponentId(), componentIndex, 0);
        entity->EraseComponent(GetComponentId(), componentIndex);
    }

    /** @brief Добавление экземпляра COM-компоненты в сущность.

    Используйте этот метод для добавления COM-компоненты в сущность.

    Пример использования:
    @code

    @endcode

    @param entity - указатель на сущность.
    @param pComResource - ресурс COM-объекта.
    @param componentIndex - индекс компоненты среди компонент такого же типа. По умолчанию равен 0.
    @return true, если нет ошибок.
    */
    bool AddComComponentTo(Entity* entity, void* pComResource, uint componentIndex = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT_MSG(Kind == kEc_Kind_Com, wxT("Operation is not allowed for this component kind"));
        wxASSERT(entity != 0);
        wxASSERT(pComResource != 0);
        wxASSERT_MSG(!FindComponent(entity, componentIndex), wxT("avoid double component adding"));
#endif
        entity->SetOrInsertComponent(GetComponentId(), componentIndex, pComResource);
        AddEntityAccessPoint(entity, componentIndex);
        return true;
    }

    /** @brief Удалить экземпляр COM-компоненты с заданным индексом из данных сущности.
    @param entity - указатель на сущность.
    @param componentIndex - индекс компоненты среди компонент такого же типа. По умолчанию равен 0.
    */
    void RemoveComComponentFrom(Entity* entity, uint componentIndex = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT_MSG(Kind == kEc_Kind_Com, wxT("Operation is not allowed for this component kind"));
        wxASSERT(entity != 0);
#endif

        void* p = 0;
        bool fHasComponent = FindComponent(entity, componentIndex, &p);
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(fHasComponent);
        wxASSERT(p != 0);
#endif
        T* instance = ConvertToComponentType(p);
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(instance != 0);
#endif
        instance->Release();
        //entity->SetOrInsertComponent(GetComponentId(), componentIndex, 0);
        RemoveEntityAccessPoint(entity, componentIndex);
        entity->EraseComponent(GetComponentId(), componentIndex);

    }

    /** @brief Задать экземпляру компоненты с заданным индексом метку "Сконструирована". */
    void MarkComponentAsConstructed(Entity* entity, uint componentIndex = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT_MSG(NeedOutCtorCall(), wxT("Operation is not allowed for this component kind"));
        wxASSERT(entity != 0);
#endif
        void* p = 0;
        FindComponent(entity, componentIndex, &p);
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
        wxASSERT(!memPool_->MarkedAsConstructed(p));
#endif
        memPool_->MarkAsConstructed(p);
    }

    /** @brief Проверить метку "Сконструирована" у экземпляра компоненты с заданным индексом. */
    bool IsComponentConstructed(Entity* entity, uint componentIndex = 0) const {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT_MSG(NeedOutCtorCall(), wxT("Operation is not allowed for this component kind"));
        wxASSERT(entity != 0);
#endif

        void* p = 0;
        FindComponent(entity, componentIndex, &p);
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(p != 0);
#endif
        return memPool_->MarkedAsConstructed(p);
    }

    /** @brief Возвращает вектор указателей обладателей данной компоненты. */
    EntityAccessPoints& GetOwners(uint componentIndex = 0) {
#ifdef YADSL_USE_WXDEBUG
        wxASSERT(componentIndex < uint(N));
#endif
        return owners_[componentIndex];
    }
};



} // end of yadsl


//-----------------------------------------------------------------------------

#if 0 // code for test
#include <iostream>
#include <stdio.h> // printf()
#include <wx/wx.h>

#include "EC_Manager.h"

#include <stdlib.h> // rand()
#include  <map>

int Random(int high) {
    double k = (double)rand() / RAND_MAX;
    return high * k;
}

// Компонента (не POD, а класс,поскольку есть член типа класса fname_)
struct WeaponData {
    int ammo_;
    std::string fname_;

    WeaponData(int ammo, const char* fname) : ammo_(ammo), fname_(fname) {}
};

yadsl::Ec_Manager<WeaponData, yadsl::kEc_Kind_Class> weaponDataEcMng;// we have a std::string member --> component is not POD but is a class


void Test() {
    using yadsl::Entity;

    Entity* shotgun = new Entity;
    weaponDataEcMng.AddComponentTo(shotgun);

    // Конструируем компоненту-класс
    void* pShotgunWeaponDataMem = 0;
    weaponDataEcMng.GetComponentMem(shotgun, &pShotgunWeaponDataMem);
    new (pShotgunWeaponDataMem) WeaponData(12, "shotgun.dat");
    weaponDataEcMng.MarkComponentAsConstructed(shotgun);

    printf ("shotgun ammo: %d, cfg: '%s'\n",
            weaponDataEcMng.GetComponentOf(shotgun)->ammo_,
            weaponDataEcMng.GetComponentOf(shotgun)->fname_.c_str());

    // Удаление ресурсов
    weaponDataEcMng.RemoveComponentFrom(shotgun);
    delete shotgun;
}


int main(){

    Test();
    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}

#endif

#endif // YADSL_EC_MANAGER_H_

