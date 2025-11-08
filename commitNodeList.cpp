#include "commitNodeList.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <string>

using namespace std;

#ifdef _WIN32
    #include <process.h>
    #define getpid _getpid
#else
    #include <unistd.h>
#endif

//  GLOBAL HELPER FUNCTIONS (used for commit ID + timestamp)

string gen_random(const int len)
{
    srand((unsigned)time(NULL) * getpid());
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    string tmp;
    tmp.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp += alphanum[rand() % (sizeof(alphanum) - 1)];

    return tmp;
}

string get_time()
{
    time_t t = std::time(0);
    tm* now = std::localtime(&t);

    return to_string(now->tm_year + 1900) + "/" +
           to_string(now->tm_mon + 1) + "/" +
           to_string(now->tm_mday) + " " +
           to_string(now->tm_hour) + ":" +
           to_string(now->tm_min) + "\n";
}


//  INTERNAL CLASS (PRIVATE TO THIS .CPP FILE)
class commitNode
{
private:
    string commitID;
    string commitMsg;
    string nextCommitID;

    void createCommitNode()
    {

    }

public:
    commitNode() {}

    commitNode(string id, string msg)
        : commitID(id), commitMsg(msg)
    {
        createCommitNode();
    }

    string getCommitID() { return commitID; }
    string getCommitMsg() { return commitMsg; }

    void writeNextCommitID(string id)
    {

    }

    string checkNextCommitId()
    {

    }

    void revertCommitNode(string fromHash)
    {

    }
};

//  commitNodeList CLASS IMPLEMENTATION (matches .h file)
