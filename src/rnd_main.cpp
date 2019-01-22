#include <iostream>
#include <stdio.h> // printf()
#include <wx/wx.h>

#include "ClassInstMemBlockPool.h"

#include <stdlib.h> // rand()

using namespace yadsl;

class Person {
public:
    std::string name_;
    int age_;

    Person( const char* name, int age) : name_(name), age_(age) {}
};

void Test() {
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
}

int main(){
    Test();

    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}
