
# VersionVault  
### A Lightweight Git-Like Version Control System in C++

VersionVault is a custom-built, minimalist version control system inspired by **Git**, implemented entirely in **C++**.  
It recreates Git’s essential concepts — staging, commits, branching, checkout, merging, deterministic hashing, and snapshot-based versioning — all built from scratch.

This project provides a clean, educational deep-dive into how real VCS tools like Git function internally.

---
🔹 Why did we choose this project?

We chose this project because:

->Git is used by every developer, yet understanding how Git works internally is difficult.
->This project allowed us to combine Data Structures, Algorithms, and File System handling in one place.
->It is a systems-level project — the kind that companies actually value (OS, compiler, VCS, DB internals).

---

## 🔹Data Structures & Algorithms Used

| Feature        | DS/Algo Used                      | Why?                                        |
| -------------- | --------------------------------- | ------------------------------------------- |
| Commit history | **Reverse Linked List**           | Easy backward traversal from HEAD to parent |
| Staging area   | **Directory tree + recursion**    | To gather all files deterministically       |
| Commit hashing | **Custom SHA-1 Algorithm**        | Deterministic, content-based identity       |
| Branching      | **Pointer references**            | HEAD → refs/heads/<branch>                  |
| Merging        | **Fast-forward pointer movement** | Efficient, simple merges                    |
| File snapshot  | **Recursive file copy**           | Mimics Git’s snapshot model                 |

---
##  🔹Project Description

VersionVault uses a snapshot-based storage model where every commit stores a full copy of the project files inside:

.git/commits/<hash>/Data/


Commit hashes are generated using a **deterministic custom SHA-1 implementation**, ensuring the same content always produces the same hash — just like Git.

Key real-world mechanisms such as:

- Staging Area  
- Commit Linked-List using Parent Pointers  
- Branch References  
- Fast-Forward Merging  
- Snapshot Storage  
- Hash-based Deduplication  

are implemented to mirror the internal architecture of a real version control system.

This makes **VersionVault** a strong systems + DSA project involving file handling, hashing, linked lists, recursion, and multi-branch versioning.

---

## 📂 Project Structure

<img width="620" height="568" alt="Screenshot 2025-11-14 at 7 05 43 PM" src="https://github.com/user-attachments/assets/c1e6ef4c-d175-4c58-bfe6-5b87611075a7" />

---

## 🔹Branching Diagram


<img width="230" height="110" alt="image" src="https://github.com/user-attachments/assets/5d14a98d-f148-42aa-bb1b-8d5e9fbbd370" />


---

## 🔀 Branch Merge Diagram

### Before Merge:


<img width="276" height="83" alt="Screenshot 2025-11-14 at 7 15 34 PM" src="https://github.com/user-attachments/assets/cc76a4e2-a6de-4bf7-aee3-185c79c144e5" />


### After Merge (fast-forward):


<img width="392" height="79" alt="Screenshot 2025-11-14 at 7 29 27 PM" src="https://github.com/user-attachments/assets/f05316cc-90f5-42ee-b21e-95c0c4532bcc" />


---

## 🔗 Commit Chain (Linked-List Representation)


<img width="139" height="232" alt="Screenshot 2025-11-14 at 7 20 08 PM" src="https://github.com/user-attachments/assets/d80ccc53-ae21-402b-897b-a1f4f9e5c60b" />


Each commit stores its FULL HASH, MESSAGE, TIMESTAMP, and PARENT inside:

commitInfo.txt


---

## ⭐ Features & Commands

### **Core Commands**

| Feature                     | Command                            | Description |
|----------------------------|-------------------------------------|-------------|
| **Initialize Repository**  | `./nodeVC init`                     | Creates `.git/`, staging area & commit store |
| **Add File(s)**            | `./nodeVC add <file>`               | Stages specific files |
| **Add All Files**          | `./nodeVC add .`                    | Stages entire directory |
| **Commit Changes**         | `./nodeVC commit -m "msg"`          | Creates new commit with SHA-1 hash & snapshot |
| **View Commit Log**        | `./nodeVC log`                      | Shows commit history of *current branch* |
| **Revert to Commit**       | `./nodeVC revert <hash>`            | Creates a new commit restoring an older version |
| **Create Branch**          | `./nodeVC branch <branch>`          | Creates new branch pointer |
| **List Branches**          | `./nodeVC branch`                   | Lists branches (marks current one) |
| **Checkout Branch**        | `./nodeVC checkout <branch>`        | Switches HEAD to selected branch |
| **Merge Branch**           | `./nodeVC merge <branch>`           | Fast-forward merges another branch into current head |

---

##  🔹Internal Mechanisms

| Mechanism                    | Explanation |
|-----------------------------|-------------|
| **Deterministic SHA-1 Hashing** | Commit hash depends on message, timestamp, parent & file contents |
| **Snapshot-based Commits**     | Each commit stores full file snapshot in `Data/` |
| **Reverse Linked-List History** | Each commit stores parent pointer, enabling backward traversal |
| **Branch Isolation**            | Each branch has its own HEAD pointer, history is not mixed |
| **Fast-forward Merge**          | Branch pointer moves forward if another branch is ahead |
| **Staging Area System**         | Only staged files are committed (like Git) |
| **Commit Metadata Storage**     | `commitInfo.txt` stores hash, message, timestamp, parent |

---

##  🔹Quick Start

./nodeVC init

./nodeVC add .

./nodeVC commit -m "initial commit"

./nodeVC branch dev

./nodeVC checkout dev

./nodeVC add .

./nodeVC commit -m "dev work"

./nodeVC checkout main

./nodeVC merge dev

./nodeVC log



