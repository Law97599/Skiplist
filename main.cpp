/* ************************************************************************
> File Name:     main.cpp
> Author:        JasonLaw
> Created Time:  2022-01-02
> Description:    
 ************************************************************************/

#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main() {

    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skipList.load_file函数
    SkipList<int, std::string> skipList(6);
	skipList.InsertElement(1, "a"); 
	skipList.InsertElement(3, "b"); 
	skipList.InsertElement(7, "c"); 
	skipList.InsertElement(8, "d"); 
	skipList.InsertElement(9, "e"); 
	skipList.InsertElement(19, "f"); 
	skipList.InsertElement(19, "g"); 

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.DumpFile();

    // skipList.load_file();

    skipList.SearchElement(9);
    skipList.SearchElement(18);


    skipList.DisplayList();

    skipList.DeleteElement(3);
    skipList.DeleteElement(7);

    std::cout << "skipList size:" << skipList.size() << std::endl;

    skipList.DisplayList();
}