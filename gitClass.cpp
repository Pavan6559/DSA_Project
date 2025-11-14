#include "gitClass.h"
#include "commitNodeList.h" 
#include <iostream>
#include <fstream>
#include <filesystem>
#include <list>
#include <string>

using namespace std;

//colours
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define END  "\033[0m"

namespace fs = std::filesystem;

static void write_HEAD_ref(const string &branchRef) {
    fs::path head = fs::current_path() / ".git" / "HEAD";
    fs::create_directories(head.parent_path());
    ofstream f(head.string());
    f << "ref: " << branchRef;
}

static string get_HEAD_ref_local() {
    fs::path head = fs::current_path() / ".git" / "HEAD";
    if (!fs::exists(head)) return string();
    string line; ifstream f(head.string()); getline(f, line);
    if (line.rfind("ref:", 0) == 0) return line.substr(5);
    return string();
}

static string get_ref_value_local(const string &ref) {
    fs::path refPath = fs::current_path() / ".git" / ref;
    if (!fs::exists(refPath)) return string();
    string v; ifstream f(refPath.string()); getline(f, v); return v;
}

static void set_ref_value_local(const string &ref, const string &value) {
    fs::path refPath = fs::current_path() / ".git" / ref;
    fs::create_directories(refPath.parent_path());
    ofstream f(refPath.string());
    f << value;
}

void gitClass::gitInit()
{
    filesystem::create_directory(".git");
    filesystem::create_directory(".git/staging_area");
    filesystem::create_directory(".git/commits");
    // create refs/heads/main and HEAD pointing to it (empty initially)
    fs::create_directories(".git/refs/heads");
    fs::path mainRef = fs::current_path() / ".git" / "refs" / "heads" / "main";
    // leave it empty until first commit
    write_HEAD_ref("refs/heads/main");
    cout << "Initialized empty nodeVC repository with branch 'main'.\n";
}

void gitClass::gitAdd()
{
    const auto copyOptions = filesystem::copy_options::update_existing | filesystem::copy_options::recursive;

    for (const auto &dirEntry : fs::directory_iterator(fs::current_path())){
            auto fileName = fs::path(dirEntry).filename();
            // skip .git folder
            if (fileName == ".git") continue;
    
            if (fs::is_directory(dirEntry))
            {
                fs::copy(dirEntry, fs::current_path() / ".git" / "staging_area" / fileName, copyOptions);
            }
            else
            {
                fs::copy(dirEntry, fs::current_path() / ".git" / "staging_area" / fileName, copyOptions);
            }
        }
}

void gitClass::gitAdd(string files[], int arrSize)
{
    const auto copyOptions = fs::copy_options::update_existing | fs::copy_options::recursive;

    for (int i = 0; i < arrSize; i++)
    {
        if (fs::exists(files[i]))
        {
            fs::path src = fs::current_path() / files[i];
            fs::path dest = fs::current_path() / ".git" / "staging_area" / files[i];
            fs::create_directories(dest.parent_path());
            fs::copy(src, dest, copyOptions);
        }
        else
        {
            cout << files[i] << " does not exist!" << endl;
        }
    }
}

void gitClass::gitCommit(string msg)
{
    list.addOnTail(msg);
}

void gitClass::gitRevert(string commitHash)
{
    list.revertCommit(commitHash);
}

void gitClass::gitLog()
{
    list.printCommitList();
}


// new functions: branch, checkout, current

void gitClass::gitBranch(string branchName) {
    // create refs/heads/<branchName> pointing to current head (or empty)
    string headRef = get_HEAD_ref_local();
    string curHash;
    if (!headRef.empty()) curHash = get_ref_value_local(headRef);
    string newRef = string("refs/heads/") + branchName;
    set_ref_value_local(newRef, curHash);
    cout << "Branch created: " << branchName << "\n";
}

void gitClass::gitCheckout(string branchName) {
    fs::path branchRef = fs::current_path() / ".git" / "refs" / "heads" / branchName;
    if (!fs::exists(branchRef)) {
        cout << "Branch does not exist.\n";
        return;
    }
    ofstream head(fs::current_path() / ".git" / "HEAD");
    head << "ref: refs/heads/" << branchName;
    cout << "Switched to branch '" << branchName << "'\n";
}

void gitClass::gitCurrentBranch() {
    string headRef = get_HEAD_ref_local();
    if (headRef.empty()) {
        cout << "No branch (HEAD not set)\n";
        return;
    }
    size_t pos = headRef.find_last_of('/');
    string name = (pos==string::npos) ? headRef : headRef.substr(pos+1);
    cout << name << "\n";
}

void gitClass::gitMerge(std::string sourceBranch) {
    namespace fs = std::filesystem;
    std::string sourceRef = "refs/heads/" + sourceBranch;
    fs::path sourcePath = fs::current_path() / ".git" / sourceRef;
    if (!fs::exists(sourcePath)) {
        std::cout << "Branch not found: " << sourceBranch << "\n";
        return;
    }

    // Get source branch commit hash
    std::string srcHash;
    {
        std::ifstream in(sourcePath.string());
        getline(in, srcHash);
    }

    if (srcHash.empty()) {
        std::cout << "Nothing to merge, " << sourceBranch << " is empty.\n";
        return;
    }

    // Copy data from developer branch's head commit to staging
    fs::path commitDir = fs::current_path() / ".git" / "commits";
    bool found = false;
    std::string shortHash = srcHash.substr(0,8);
    for (auto &d : fs::directory_iterator(commitDir)) {
        fs::path info = d.path() / "commitInfo.txt";
        if (!fs::exists(info)) continue;
        std::ifstream file(info.string());
        std::string line; getline(file, line);
        if (line.size() > 2) {
            std::string full = line.substr(2);
            if (full == srcHash) {
                fs::path staging = fs::current_path() / ".git" / "staging_area";
                if (fs::exists(staging)) fs::remove_all(staging);
                fs::create_directories(staging);
                fs::copy(d.path() / "Data", staging, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                found = true;
                break;
            }
        }
    }

    if (!found) {
        std::cout << "Could not find commit data for " << sourceBranch << "\n";
        return;
    }

    // Create merge commit on current branch
    std::string msg = "Merge branch '" + sourceBranch + "'";
    list.addOnTail(msg);
    std::cout << "Merged branch '" << sourceBranch << "' into current branch.\n";
}

void gitClass::gitCherryPick(std::string commitHash) {
    namespace fs = std::filesystem;

    fs::path commitPath = fs::current_path() / ".git" / "commits" / commitHash;
    if (!fs::exists(commitPath)) {
        std::cout << RED << "Commit not found: " << commitHash << END << std::endl;
        return;
    }

    fs::path dataPath = commitPath / "Data";
    if (!fs::exists(dataPath)) {
        std::cout << RED << "No data folder for commit: " << commitHash << END << std::endl;
        return;
    }

    // Step 1: Copy commit's data to staging area
    fs::path staging = fs::current_path() / ".git" / "staging_area";
    if (fs::exists(staging)) fs::remove_all(staging);
    fs::create_directories(staging);
    fs::copy(dataPath, staging, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

    // Step 2: Read commit message
    std::string msg = "Cherry-picked commit " + commitHash.substr(0, 8);
    fs::path infoPath = commitPath / "commitInfo.txt";
    if (fs::exists(infoPath)) {
        std::ifstream file(infoPath.string());
        std::string line;
        while (getline(file, line)) {
            if (line[0] == '2') {
                msg = line.substr(2) + " (cherry-picked)";
                break;
            }
        }
    }

    // Step 3: Make a new commit on the current branch
    list.addOnTail(msg);

    std::cout << GRN << "Cherry-picked commit " << commitHash.substr(0, 8)
              << " into current branch." << END << std::endl;
}
// {
//     list.printCommitStatus();

// }




