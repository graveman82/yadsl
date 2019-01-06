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
