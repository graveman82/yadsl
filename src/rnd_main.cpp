#include <iostream>
#include <stdio.h> // printf()
#include <wx/wx.h>


#include "StlListAlloc.h"
#include <list>

#include <stdlib.h> // rand()


int Random(int high) {
    double k = (double)rand() / RAND_MAX;
    return high * k;
}

struct Person {
    std::string name_;
    int age_;

    Person(const char* name, int age) : name_(name), age_(age) {}
};

struct Person2 {
    std::string name_;
    int age_;
    //int dummy_;

    Person2(const char* name, int age) : name_(name), age_(age) {}
};

void Test() {

    std::list<Person, yadsl::StlListAllocator<Person> > personList;
    personList.push_back(Person("Marja Ivanovna", 25));
    personList.push_back(Person("Ivan Durak", 35));
    personList.pop_back();
    personList.push_back(Person("Koschei", 350));
    personList.push_back(Person("Baba Yaga", 250));


    std::list<Person2, yadsl::StlListAllocator<Person2> > person2List;
    person2List.push_back(Person2("Marja Ivanovna2", 25));
    person2List.push_back(Person2("Ivan Durak2", 35));
    person2List.pop_back();
    person2List.push_back(Person2("Koschei2", 350));
    person2List.push_back(Person2("Baba Yaga2", 250));
    printf("test finished\n");
}

void Test2() {

    std::list<Person, yadsl::StlListAllocator<Person> > personList;
    personList.push_back(Person("Marja Ivanovna", 25));
    personList.push_back(Person("Ivan Durak", 35));
    personList.pop_back();
    personList.push_back(Person("Koschei", 350));
    personList.push_back(Person("Baba Yaga", 250));


    std::list<Person2, yadsl::StlListAllocator<Person2> > person2List;
    person2List.push_back(Person2("Marja Ivanovna2", 25));
    person2List.push_back(Person2("Ivan Durak2", 35));
    person2List.pop_back();
    person2List.push_back(Person2("Koschei2", 350));
    person2List.push_back(Person2("Baba Yaga2", 250));
    printf("test finished\n");
}

int main(){

    Test();
    Test2();

    std::cout << "Press any key to quit..." << std::endl;
    std::cin.get();
    std::cin.get();
}
