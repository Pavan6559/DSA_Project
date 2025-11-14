#include "commitNodeList.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

using namespace std;

//  GLOBAL HELPER FUNCTIONS (used for commit ID + timestamp)

class SHA1 {
public:
    SHA1() { reset(); }
    void reset() {
        length_low = length_high = 0;
        message_block_index = 0;
        computed = false;
        corrupted = false;
        H[0] = 0x67452301;
        H[1] = 0xEFCDAB89;
        H[2] = 0x98BADCFE;
        H[3] = 0x10325476;
        H[4] = 0xC3D2E1F0;
    }
    void input(const unsigned char* message_array, unsigned length) {
        if (!length) return;
        if (computed || corrupted) { corrupted = true; return; }
        while (length-- && !corrupted) {
            message_block[message_block_index++] = (*message_array & 0xFF);
            length_low += 8;
            if (length_low == 0) {
                length_high++;
                if (length_high == 0)
                    corrupted = true;
            }
            if (message_block_index == 64)
                processMessageBlock();
            ++message_array;
        }
    }
    void input(const std::string &s) { input((const unsigned char*)s.c_str(), (unsigned)s.size()); }
    std::string final() {
        if (corrupted) return "";
        if (!computed) {
            padMessage();
            // message may be padded with multiple blocks
            for (int i = 0; i < 5; ++i) {
                // produce hex string
                std::ostringstream os;
                os << std::hex << std::setfill('0') << std::setw(8) << (H[i] & 0xFFFFFFFF);
                result += os.str();
            }
            computed = true;
        }
        return result;
    }
private:
    void processMessageBlock() {
        const unsigned K[] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
        unsigned W[80];
        for (int t = 0; t < 16; ++t)
            W[t] = ((unsigned)message_block[t * 4]) << 24 |
                   ((unsigned)message_block[t * 4 + 1]) << 16 |
                   ((unsigned)message_block[t * 4 + 2]) << 8 |
                   ((unsigned)message_block[t * 4 + 3]);
        for (int t = 16; t < 80; ++t)
            W[t] = leftrotate(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);

        unsigned A = H[0], B = H[1], C = H[2], D = H[3], E = H[4];
        for (int t = 0; t < 80; ++t) {
            unsigned temp = leftrotate(A, 5) + f(t, B, C, D) + E + K[t / 20] + W[t];
            E = D; D = C; C = leftrotate(B, 30); B = A; A = temp;
        }
        H[0] += A; H[1] += B; H[2] += C; H[3] += D; H[4] += E;
        message_block_index = 0;
    }
    unsigned f(int t, unsigned B, unsigned C, unsigned D) {
        if (t < 20) return (B & C) | ((~B) & D);
        if (t < 40) return B ^ C ^ D;
        if (t < 60) return (B & C) | (B & D) | (C & D);
        return B ^ C ^ D;
    }
    unsigned leftrotate(unsigned x, unsigned n) { return (x << n) | (x >> (32 - n)); }
    void padMessage() {
        // backup current block_index
        message_block[message_block_index++] = 0x80;
        if (message_block_index > 56) {
            while (message_block_index < 64)
                message_block[message_block_index++] = 0;
            processMessageBlock();
        }
        while (message_block_index < 56)
            message_block[message_block_index++] = 0;
        // append length
        message_block[56] = (unsigned char)((length_high >> 24) & 0xFF);
        message_block[57] = (unsigned char)((length_high >> 16) & 0xFF);
        message_block[58] = (unsigned char)((length_high >> 8) & 0xFF);
        message_block[59] = (unsigned char)(length_high & 0xFF);
        message_block[60] = (unsigned char)((length_low >> 24) & 0xFF);
        message_block[61] = (unsigned char)((length_low >> 16) & 0xFF);
        message_block[62] = (unsigned char)((length_low >> 8) & 0xFF);
        message_block[63] = (unsigned char)(length_low & 0xFF);
        processMessageBlock();
    }

    unsigned H[5];
    unsigned long long length_low, length_high;
    unsigned char message_block[64];
    int message_block_index = 0;
    bool computed = false;
    bool corrupted = false;
    std::string result;
};


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

static string read_file_text(const fs::path &p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return string();
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// Walk staging directory, collect file paths sorted to ensure deterministic hash
static void collect_staging_files(const fs::path &staging, vector<fs::path> &out) {
    if (!fs::exists(staging)) return;
    for (auto &entry : fs::recursive_directory_iterator(staging)) {
        if (fs::is_regular_file(entry.path())) {
            out.push_back(entry.path());
        }
    }
    sort(out.begin(), out.end());
}

static string compute_sha1_of_string(const string &s) {
    SHA1 sha;
    sha.input((const unsigned char*)s.data(), (unsigned)s.size());
    return sha.final();
}

// Compute commit hash based on parentRef + message + timestamp + file contents (deterministic)
static string compute_commit_hash(const string &parentHash, const string &message, const string &timestamp) {
    fs::path staging = fs::current_path() / ".git" / "staging_area";
    vector<fs::path> files;
    collect_staging_files(staging, files);
    SHA1 sha;
    // include parent, message, timestamp
    sha.input((const unsigned char*)parentHash.data(), (unsigned)parentHash.size());
    sha.input((const unsigned char*)message.data(), (unsigned)message.size());
    sha.input((const unsigned char*)timestamp.data(), (unsigned)timestamp.size());
    // include file paths + content in sorted order
    for (auto &p : files) {
        string relative = fs::relative(p, staging).generic_string();
        sha.input((const unsigned char*)relative.data(), (unsigned)relative.size());
        string content = read_file_text(p);
        sha.input((const unsigned char*)content.data(), (unsigned)content.size());
    }
    return sha.final(); // full 40-char hex
}

// get current branch ref from HEAD (returns branch name and ref path)
static string get_HEAD_ref() {
    fs::path headPath = fs::current_path() / ".git" / "HEAD";
    if (!fs::exists(headPath)) return string();
    string line;
    ifstream f(headPath.string());
    getline(f, line);
    // expected format: "ref: refs/heads/main"
    if (line.rfind("ref:", 0) == 0) {
        auto ref = line.substr(5);
        return ref;
    }
    return string();
}

static string get_current_branch_name() {
    string ref = get_HEAD_ref();
    if (ref.empty()) return string();
    // ref is like "refs/heads/main"
    size_t pos = ref.find_last_of('/');
    if (pos == string::npos) return ref;
    return ref.substr(pos + 1);
}

static string get_ref_value(const string &ref) {
    fs::path refPath = fs::current_path() / ".git" / ref;
    if (!fs::exists(refPath)) return string();
    string v; ifstream f(refPath.string()); getline(f, v); return v;
}

static void set_ref_value(const string &ref, const string &value) {
    fs::path refPath = fs::current_path() / ".git" / ref;
    fs::create_directories(refPath.parent_path());
    ofstream f(refPath.string());
    f << value;
}

// get parent commit hash from current branch ref
static string get_current_branch_head_hash() {
    string ref = get_HEAD_ref();
    if (ref.empty()) return string();
    return get_ref_value(ref);
}

static string get_time_str() {
    time_t t = time(nullptr);
    tm *now = localtime(&t);
    ostringstream os;
    os << (now->tm_year + 1900) << "/" << (now->tm_mon + 1) << "/" << now->tm_mday << " " << now->tm_hour << ":" << now->tm_min;
    return os.str();
}

//  commitNodeList CLASS IMPLEMENTATION (matches .h file)

commitNodeList::commitNodeList()
{
    // nothing to do here, but required by the linker
}

void commitNodeList::addOnTail(string msg)
{
    namespace fs = std::filesystem;

    // 1. Parent commit (current branch HEAD)
    string parent = get_current_branch_head_hash();
    parent.erase(remove(parent.begin(), parent.end(), '\n'), parent.end());
    string timestamp = get_time_str();

    // 2. Compute full 40-char commit hash
    string fullHash = compute_commit_hash(parent, msg, timestamp);
    string shortHash = fullHash.substr(0, 8);

    // 3. Create commit directory (named by shortHash)
    fs::path commitPath = fs::current_path() / ".git" / "commits" / shortHash;
    fs::create_directories(commitPath);

    // write commitInfo.txt (store full hash in line 1 to be unambiguous)
    {
        ofstream info(commitPath / "commitInfo.txt");
        info << "1." << fullHash << "\n";
        info << "2." << msg << "\n";
        info << "3." << timestamp << "\n";
        info << "4." << parent << "\n";  
    }

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
    string targetFullHash;
    if (commitHash == "HEAD") {
        string cur = get_current_branch_head_hash();
        if (cur.empty()) {
            cout << "No HEAD to revert to\n";
            return;
        }
        targetFullHash = cur;
    } else {
        // search commits folder for matching full or short hash
        fs::path commitDir = fs::current_path() / ".git" / "commits";
        bool found = false;
        for (auto &d : fs::directory_iterator(commitDir)) {
            fs::path infoFile = d.path() / "commitInfo.txt";
            if (!fs::exists(infoFile)) continue;
            string line; ifstream f(infoFile.string());
            getline(f, line);
            if (line.size() > 2) {
                string full = line.substr(2);
                string shortH = full.substr(0, 8);
                if (commitHash == full || commitHash == shortH || commitHash == d.path().filename().string()) {
                    targetFullHash = full;
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            cout << "Commit not found: " << commitHash << "\n";
            return;
        }
    }
    fs::path commitDir = fs::current_path() / ".git" / "commits";
    string foundDirName;
    for (auto &d : fs::directory_iterator(commitDir)) {
        fs::path infoFile = d.path() / "commitInfo.txt";
        if (!fs::exists(infoFile)) continue;
        string line; ifstream f(infoFile.string());
        getline(f, line);
        if (line.size() > 2) {
            string full = line.substr(2);
            if (full == targetFullHash) {
                foundDirName = d.path().filename().string();
                break;
            }
        }
    }
    if (foundDirName.empty()) {
        cout << "Could not resolve commit hash to folder\n";
        return;
    }
    // read message from original commit
    string msg;
    {
        ifstream fin(fs::current_path() / ".git" / "commits" / foundDirName / "commitInfo.txt");
        string ln;
        while (getline(fin, ln)) {
            if (ln.size() > 0 && ln[0] == '2') { msg = ln.substr(2); break; }
        }
    }

    // create a new commit that restores the target commit
    string parent = get_current_branch_head_hash();
    string timestamp = get_time_str();
    // compute new commit hash using targetFullHash content to ensure determinism
    // We'll include special marker "revert:"+targetFullHash in message so it's recorded
    string combinedMsg = string("revert:") + targetFullHash + "|" + msg;
    // For revert, we need to compute hash based on the files we're copying (the target Data)
    // To keep deterministic, we temporarily copy target Data into staging area and compute
    fs::path staging = fs::current_path() / ".git" / "staging_area";
    // clear staging
    if (fs::exists(staging)) fs::remove_all(staging);
    fs::create_directories(staging);
    // copy target Data into staging
    fs::copy(fs::current_path() / ".git" / "commits" / foundDirName / "Data", staging, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

    string fullHash = compute_commit_hash(parent, combinedMsg, timestamp);
    string shortHash = fullHash.substr(0, 8);
    fs::path newCommitPath = fs::current_path() / ".git" / "commits" / shortHash;
    fs::create_directories(newCommitPath);
    ofstream info(newCommitPath / "commitInfo.txt");
    info << "1." << fullHash << "\n";
    info << "2." << msg << "\n";
    info << "3." << timestamp << "\n";
    info << "4." << parent      << "\n"; 
    info.close();
    // copy staging -> new commit Data (already in staging)
    fs::copy(staging, newCommitPath / "Data", fs::copy_options::recursive | fs::copy_options::overwrite_existing);

    // update current branch ref to this new commit
    string headRef = get_HEAD_ref();
    if (!headRef.empty()) set_ref_value(headRef, fullHash);
    cout << "Reverted: " << shortHash << endl;
}


void commitNodeList::printCommitList()
{
    cout << "\n===== COMMIT LOG =====\n";

    // 1. Get current branch HEAD (full hash)
    string current = get_current_branch_head_hash();
    if (current.empty()) {
        cout << "Branch has no commits.\n";
        return;
    }

    while (!current.empty()) {
        // short hash = folder name
        string shortHash = current.substr(0, 8);
        fs::path commitInfo = fs::current_path() / ".git" / "commits" / shortHash / "commitInfo.txt";

        if (!fs::exists(commitInfo))
            break;

        string fullHash, msg, timestamp, parent;
        ifstream f(commitInfo.string());
        string line;
        while (getline(f, line)) {
            if (line.rfind("1.",0) == 0) fullHash = line.substr(2);
            if (line.rfind("2.",0) == 0) msg       = line.substr(2);
            if (line.rfind("3.",0) == 0) timestamp = line.substr(2);
            if (line.rfind("4.",0) == 0) parent    = line.substr(2);
        }

        cout << "Commit:    " << shortHash << endl;
        cout << "Message:   " << msg << endl;
        cout << "Timestamp: " << timestamp << endl;
        cout << "-----------------------------\n";

        current = parent;  // move to previous commit
    }
}
