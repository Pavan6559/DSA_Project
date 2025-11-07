#pragma once
#include <string>
class commitNodeList
{
public:
    commitNodeList();
    void addOnTail(std::string msg);
    void revertCommit(std::string commitHash);
    void printCommitList();
};
