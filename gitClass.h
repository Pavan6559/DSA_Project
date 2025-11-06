#pragma once
#include <string>
#include "commitNodeList.h"

class gitClass {
private:
    commitNodeList list;   
public:
    void gitInit();
    void gitAdd();
    void gitAdd(std::string files[], int size);
    void gitCommit(std::string msg);
    void gitRevert(std::string hash);
    void gitLog();
};

