#include <iostream>
#include "gitClass.h"
#include <vector>
using namespace std;

//---------------------
#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YEL  "\x1B[33m"
#define BLU  "\x1B[34m"
#define MAG  "\x1B[35m"
#define CYN  "\x1B[36m"
#define WHT  "\x1B[37m"
#define END  "\033[0m"
//---------------------

int main(int argc, char *argv[])
{
    gitClass gitClassObj;
    if(argc >= 2)
    {
        string argument = string(argv[1]);
        //git init
        if (argument == "init")
        {
            gitClassObj.gitInit();
        }
        //git add
        else if (argument == "add")
        {
            if(argc == 2){
                cout << RED "missing arguments!" <<endl;
                cout << "Provide a third argument e.g." << endl;
                cout << "git add <'.' | 'file_name'>" END << endl;
            }
            if(argc >= 3){
                string argumentC = string(argv[2]);
                if (argumentC == ".") {
                    gitClassObj.gitAdd();
                }
                else if (argc == 3)
                {
                    string files[1] = {string(argv[2])};
                    gitClassObj.gitAdd(files, 1);
                } else {
                    vector<string> files;
                    for (int i = 2; i < argc; i++)
                    {
                        files.push_back(string(argv[i]));
                    }
                    gitClassObj.gitAdd(files.data(), files.size());
                }
            }

        }
        //git commit
        else if (argument == "commit")
        {
            if(argc == 4)    //[ git, commit, -m, "msg" ]
            {
                string argumentC = string(argv[2]);
                string argumentD = string(argv[3]);
                if(argumentC == "-m")
                {
                    gitClassObj.gitCommit(argumentD);
                    cout << "files committed successfully" << endl;
                }
            }
            else
            {
                cout << RED "missing arguments!" <<endl;
                cout << "Provide with a message field e.g." << endl;
                cout << "git commit -m 'my commit message'" END << endl;
            }
        }
        // git revert
        else if(argument == "revert")
        {
            if(argc == 3)
            {
                string argumentC = string(argv[2]);
                gitClassObj.gitRevert(argumentC);
            }
            else
            {
                cout << RED "invalid arguments, should be like: " << endl;
                cout << "git revert <'HEAD'|'commit_hash'>" END<< endl;
            }
        }
        // git merge
        else if (argument == "merge")
        {
            if(argc == 3)
                gitClassObj.gitMerge(string(argv[2]));
            else
                cout << RED "invalid arguments: git merge <branch>" END << endl;
        }
        // git log
        else if(argument == "log")
        {
            gitClassObj.gitLog();
        }
        // git branch
        else if(argument == "branch")
        {
            if(argc == 3) {
                gitClassObj.gitBranch(string(argv[2]));
            } else {
                cout << RED "invalid arguments: git branch <name>" END << endl;
            }
        }
        // git checkout
        else if(argument == "checkout")
        {
            if(argc == 3) {
                gitClassObj.gitCheckout(string(argv[2]));
            } else {
                cout << RED "invalid arguments: git checkout <name>" END << endl;
            }
        }
        //cherry-pick
        else if (argument == "cherry-pick") {
            if (argc == 3)
                gitClassObj.gitCherryPick(string(argv[2]));
            else
                cout << RED "invalid arguments: git cherry-pick <commit_hash>" END << endl;
        }
        // git current
        else if(argument == "current")
        {
            gitClassObj.gitCurrentBranch();
        }
        //wrong arguments
        else
        {
            cout << RED "Invalid arguments" END << endl;
        }

    }
    else
    {
        cout << YEL "nodeVC - simplified local version control\n\n";
        cout << "Usage: " << endl;
        cout << "nodeVC init                           ->   initialize repository" << endl;
        cout << "nodeVC add <'.'|'file_name'>          ->   add files to staging area" << endl;
        cout << "nodeVC commit -m 'message'            ->   commit your staging files" << endl;
        cout << "nodeVC revert <'HEAD'|'commit_hash'>  ->   revert to a specific commit" << endl;
        cout << "nodeVC log                            ->   show commit history" << endl;
        cout << "nodeVC branch <name>                  ->   create branch" << endl;
        cout << "nodeVC checkout <name>                ->   switch to branch" << endl;
        cout << "nodeVC current                        ->   show current branch" END << endl;
    }
}
