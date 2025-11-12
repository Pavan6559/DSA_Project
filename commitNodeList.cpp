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
        filesystem::create_directories(filesystem::current_path() / ".git" / "commits" / commitID);
        auto infoPath = filesystem::current_path() / ".git" / "commits" / commitID / "commitInfo.txt";

        ofstream write(infoPath.string());
        write << "1." + commitID << "\n"
              << "2." + commitMsg << "\n"
              << "3." + get_time();

        auto staging = filesystem::current_path() / ".git" / "staging_area";
        filesystem::copy(staging,
                         filesystem::current_path() / ".git" / "commits" / commitID / "Data",
                         filesystem::copy_options::update_existing | filesystem::copy_options::recursive);
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
        nextCommitID = id;
        auto path = filesystem::current_path() / ".git" / "commits" / commitID / "nextCommitInfo.txt";
        ofstream write(path.string());
        write << id;
    }

    string checkNextCommitId()
    {
        auto path = filesystem::current_path() / ".git" / "commits" / commitID / "nextCommitInfo.txt";
        if (!filesystem::exists(path)) return "";

        ifstream file(path.string());
        getline(file, nextCommitID);
        return nextCommitID;
    }

    void revertCommitNode(string fromHash)
    {
        filesystem::create_directories(
            filesystem::current_path() / ".git" / "commits" / commitID / "Data"
        );

        filesystem::copy(
            filesystem::current_path() / ".git" / "commits" / fromHash / "Data",
            filesystem::current_path() / ".git" / "commits" / commitID / "Data",
            filesystem::copy_options::recursive |
            filesystem::copy_options::overwrite_existing   // <-- THIS FIXES IT
        );
    }
};

//  commitNodeList CLASS IMPLEMENTATION (matches .h file)

commitNodeList::commitNodeList()
{
    // nothing to do here but required by linker
}

void commitNodeList::addOnTail(string msg)
{
    // parent = current branch head
    string parent = get_current_branch_head_hash();
    string timestamp = get_time_str();
    string fullHash = compute_commit_hash(parent, msg, timestamp); // 40-char hex
    string shortHash = fullHash.substr(0, 8);

    // create commit dir named with shortHash (for readability), but store full hash in commitInfo
    fs::path commitPath = fs::current_path() / ".git" / "commits" / shortHash;
    fs::create_directories(commitPath);

    // write commitInfo.txt (store full hash in line 1 to be unambiguous)
    ofstream info(commitPath / "commitInfo.txt");
    info << "1." << fullHash << "\n";
    info << "2." << msg << "\n";
    info << "3." << timestamp << "\n";
    info.close();

    // copy staging_area -> commit/Data (overwrite if exists)
    fs::path staging = fs::current_path() / ".git" / "staging_area";
    fs::create_directories(commitPath / "Data");
    fs::copy(staging, commitPath / "Data", fs::copy_options::recursive | fs::copy_options::overwrite_existing);

    // update branch ref: HEAD -> refs/heads/<branch>
    string headRef = get_HEAD_ref();
    if (!headRef.empty()) {
        set_ref_value(headRef, fullHash);
    } else {
        // no HEAD? set to refs/heads/main
        set_ref_value("refs/heads/main", fullHash);
        ofstream head(fs::current_path() / ".git" / "HEAD");
        head << "ref: refs/heads/main";
    }

    cout << "Committed: " << shortHash << endl;
}

void commitNodeList::revertCommit(string commitHash)
{
    // get original commit info
    auto infoFile = filesystem::current_path() / ".git" / "commits" / commitHash / "commitInfo.txt";
    if (!filesystem::exists(infoFile)) {
        cout << "Invalid commit hash!\n";
        return;
    }

    string msg;
    string line;
    ifstream file(infoFile.string());
    while (getline(file, line))
        if (line[0] == '2')   // commit message line
            msg = line.substr(2);

    commitNode node(gen_random(8), msg);  // <-- use original commit message
    node.revertCommitNode(commitHash);
}


void commitNodeList::printCommitList()
{
    cout << "\n===== COMMIT LOG =====\n";

    auto commitDir = filesystem::current_path() / ".git" / "commits";

    if (!filesystem::exists(commitDir))
    {
        cout << "No commits yet.\n";
        return;
    }

    for (const auto& dir : filesystem::directory_iterator(commitDir))
    {
        auto infoFile = dir.path() / "commitInfo.txt";
        if (!filesystem::exists(infoFile)) continue;

        ifstream file(infoFile.string());
        string info;

        while (getline(file, info))
        {
            if (info[0] == '1')
                cout << "Commit ID:   " << info.substr(2) << endl;
            if (info[0] == '2')
                cout << "Message:     " << info.substr(2) << endl;
            if (info[0] == '3')
                cout << "Date & Time: " << info.substr(2) << endl;
        }

        cout << "--------------------------\n";
    }
}
