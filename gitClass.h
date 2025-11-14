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

    //new features
    void gitBranch(std::string branchName);
    void gitCheckout(std::string branchName);
    void gitCurrentBranch();

    //new features
    void gitMerge(std::string sourceBranch);
    void gitCherryPick(std::string commitHash); 
};



