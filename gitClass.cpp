#include "gitClass.h"
#include "commitNodeList.h" 
#include <iostream>
#include <fstream>
#include <filesystem>
#include <list>
#include <string>

using namespace std;

//---------------------
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define END "\033[0m"
//---------------------


void gitClass::gitInit()
{
    filesystem::create_directory(".git");
    filesystem::create_directory(".git/staging_area");
    filesystem::create_directory(".git/commits");
}

void gitClass::gitAdd()
{
    const auto copyOptions = filesystem::copy_options::update_existing | filesystem::copy_options::recursive;

    for (const auto &dirEntry : filesystem::directory_iterator(filesystem::current_path()))
    {
        auto fileName = filesystem::path(dirEntry).filename();
        if (fileName != "git" && fileName != ".git")
        {
            if (filesystem::is_directory(fileName))
            {
                filesystem::copy(dirEntry, filesystem::current_path() / ".git" / "staging_area" / fileName, copyOptions);
            }
            else
            {
                filesystem::copy(dirEntry, filesystem::current_path() / ".git" / "staging_area" / "", copyOptions);
            }
        }
    }
}

void gitClass::gitAdd(string files[], int arrSize)
{
    const auto copyOptions = filesystem::copy_options::update_existing | filesystem::copy_options::recursive;

    for (int i = 0; i < arrSize; i++)
    {
        if (filesystem::exists(files[i]))
        {
            int last_index = files[i].find_last_of('/');
            if (last_index != string::npos)
            {
                auto newDir = filesystem::current_path() / ".git" / "staging_area" / files[i].substr(0, last_index);
                filesystem::create_directories(newDir);
                filesystem::copy(filesystem::current_path() / files[i], filesystem::current_path() / ".git" / "staging_area" / files[i], copyOptions);
            }
            else
            {
                if (filesystem::is_directory(files[i]))
                {
                    filesystem::copy(filesystem::current_path() / files[i], filesystem::current_path() / ".git" / "staging_area" / files[i], copyOptions);
                }
                else
                {
                    filesystem::copy(filesystem::current_path() / files[i], filesystem::current_path() / ".git" / "staging_area", copyOptions);
                }
            }
        }
        else
        {
            cout << files[i] << RED "does not exist!" END << endl;
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
