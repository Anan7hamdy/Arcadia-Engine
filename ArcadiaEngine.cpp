// ArcadiaEngine.cpp - STUDENT TEMPLATE
// TODO: Implement all the functions below according to the assignment requirements

#include "ArcadiaEngine.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <iomanip>

using namespace std;

// =========================================================
// PART A: DATA STRUCTURES (Concrete Implementations)
// =========================================================

// --- 1. PlayerTable (Double Hashing) ---

class ConcretePlayerTable : public PlayerTable {
private:
    // TODO: Define your data structures here
    // Hint: You'll need a hash table with double hashing collision resolution
    static const int tableSize = 101; // new info (static const is required to inilize arr)
    int keys[tableSize];
    string names[tableSize];
    bool used[tableSize];

    // imp h1 with 'Multiplication hashing' or key % table size
    int h1(int key) {
        return key % tableSize;
        // const double A = 0.618033;
        // double fraction = key * A - floor(key * A) ;
        //return (int)(fraction * tableSize);
    }

    // imp h2 by this equation h2 = prime - (key % prime)
    int h2(int key) {
        int prime = 97; //why? based on prime definition (prime is a prime smaller than the table size).
        return prime - (key % prime);
    }

    // display hash table content
    void displayHash() {
        for (int i = 0; i < tableSize; i++) {
            if (used[i] != false)
                cout << "Index: " << i << " " << keys[i] << " " << names[i] << endl;
            else cout << "Index: " << i << " Empty " << endl;
        }cout << endl;
    }

public:
    ConcretePlayerTable() {
        // TODO: Initialize your hash table
        for (int i = 0; i < tableSize; i++) {
            used[i] = false;
        }
    }

    void insert(int playerID, string name) override {
        // TODO: Implement double hashing insert
        // Remember to handle collisions using h1(key) + i * h2(key)
        int index = h1(playerID);
        if (!used[index]) {
            used[index] = true;
            keys[index] = playerID;
            names[index] = name;
            displayHash();
            return;
        }
        int i = 1;
        int newI = index;
        while (i < tableSize) {
            newI = (index + i * h2(playerID)) % tableSize;

            if (!used[newI]) {
                used[newI] = true;
                keys[newI] = playerID;
                names[newI] = name;
                displayHash();
                cout << "There is a collision " << endl;
                return;
            }

            i++;
        }
        displayHash();
        cout << "Table is Full" << endl;
    }

    string search(int playerID) override {

        int index = h1(playerID);

        if (used[index] && keys[index] == playerID) {
            return names[index];
        }

        int i = 1;
        int newI = index;

        while (i < tableSize) {
            newI = (index + i * h2(playerID)) % tableSize;

            if (used[newI] && keys[newI] == playerID) {
                return names[newI];
            }

            if (!used[newI]) { // if empty not necessary to complete
                break;
            }

            i++;
        }
        return "";
    }
};
class ConcreteLeaderboard : public Leaderboard {
private:
    // The maximum number of levels
    static const int MAXLEVEL = 16;
    //The highest current level contains nodes no higher than MAX_LEVEL
    int currentMaxLevel;

    struct SkipListNode {
        int playerScore;
        int playerID;
        SkipListNode* forwardPointers[MAXLEVEL];

        SkipListNode(int score, int id) {
            playerScore = score;
            playerID = id;
            for (int i = 0; i < MAXLEVEL; i++)
                forwardPointers[i] = nullptr;
        }
    };

    SkipListNode* header;
    //Determines the random level of the new node
    int randomLevel() {
        int level = 1;
        // Use probabilistic method (50% chance) to increase the level
        while (level < MAXLEVEL && (rand() % 2 == 0)) {
            level++;
        }
        return level;
    }

    //Finds the predecessor nodes for insertion
    void searchForInsertion(int score, int id, SkipListNode* update[MAXLEVEL]) {
        // create node for traverse skip linked list
        SkipListNode* current = header;

        // Start from the highest level and go down (vertical movement)
        for (int i = currentMaxLevel - 1; i >= 0; i--) {
            //Keep moving right as long as it's allowed (horizontal movement)
            while (current->forwardPointers[i] != nullptr) {
                SkipListNode* next = current->forwardPointers[i];

                // 1. Primary Sort: Next score is strictly greater (Descending Score)
                // 2. Secondary Sort: Scores are equal AND Next ID is smaller (Ascending ID)
                bool MoveRight = (next->playerScore > score) || (next->playerScore == score && next->playerID < id);

                if (MoveRight) {
                    current = next; //move right one step
                }
                else {
                    break; // move down
                }
            }
            // Save Predecessor: update[i] is the last node visited before dropping down
            update[i] = current;
        }
    }
    int searchForDeletion(int id) {
        SkipListNode* currentNode = header;
        SkipListNode* nextNode = currentNode->forwardPointers[0];

        while (nextNode != nullptr) {
            if (nextNode->playerID == id) {
                return nextNode->playerScore;
            }
            else {
                currentNode = nextNode;
                nextNode = nextNode->forwardPointers[0];
            }
        }
        return -1;
    }

public:
    ConcreteLeaderboard() {
        // Initialize the Header Node
        header = new SkipListNode(INT_MAX, -1);
        // Seed the random number generator
        srand(time(nullptr));
        currentMaxLevel = 1;
    }

    ~ConcreteLeaderboard() {
        SkipListNode* curr = header;
        while (curr) {
            SkipListNode* next = curr->forwardPointers[0];
            delete curr;
            curr = next;
        }
    }

    void addScore(int playerID, int score) override {
        // Array to store the nodes that need to be updated (predecessors)
        SkipListNode* update[MAXLEVEL];

        // Find the insertion path and fill the update array
        searchForInsertion(score, playerID, update);

        //Determine the random height for the new node
        int level = randomLevel();

        // update the current max level
        if (level > currentMaxLevel) {
            for (int i = currentMaxLevel; i < level; i++)
                update[i] = header;
            currentMaxLevel = level;
        }

        //Create the new node and set its data
        SkipListNode* newNode = new SkipListNode(score, playerID);

        // Insert the new node into the Skip List
        for (int i = 0; i < level; i++) { // Only loop up to the randomly chosen height
            //the pointer of the new node point to the node that the previous node was pointing to update[i].
            newNode->forwardPointers[i] = update[i]->forwardPointers[i];

            //the pointer of the previous node (update[i]) at this level now point to the new node.
            update[i]->forwardPointers[i] = newNode;
        }
    }

    void removePlayer(int playerID) override {
        // Search for the score by ID
        int score = searchForDeletion(playerID);
        if (score == -1) { //Not Found
            return;
        }
        else {
            //Find The Predecessors using score and id
            SkipListNode* update[MAXLEVEL];
            searchForInsertion(score, playerID, update);

            // create deleted node
            SkipListNode* nodeToDelete = update[0]->forwardPointers[0];
            if (nodeToDelete == nullptr || nodeToDelete->playerID != playerID) {
                // To ensure we delete the correct node
                return;
            }
            // Unlinking all levels
            for (int i = 0; i < MAXLEVEL; i++) {
                // Check if the node does not exist in level i
                if (update[i]->forwardPointers[i] != nodeToDelete) {
                    // if not , go down
                    break;
                }
                // Redirect pointers
                update[i]->forwardPointers[i] = nodeToDelete->forwardPointers[i];
            }
            // Clear memory
            delete nodeToDelete;
            // Update Current Max Level
            if (currentMaxLevel > 1 && header->forwardPointers[currentMaxLevel - 1] == nullptr) {
                currentMaxLevel--;
            }
        }
    }

    vector<int> getTopN(int n) override {
        vector<int> top_n_players;
        //First node in the level 0
        SkipListNode* currentNode = header->forwardPointers[0];
        //Continue until we reach the end of the list and have collected the required number of players.
        while (currentNode != nullptr && top_n_players.size() < n) {
            top_n_players.push_back(currentNode->playerID);
            currentNode = currentNode->forwardPointers[0];
        }

        //Return top N player IDs in descending score order
        return top_n_players;
    }
};

// --- 3. AuctionTree (Red-Black Tree) ---





class ConcreteAuctionTree : public AuctionTree {
private:
    class RBNode {
    public:
        int id;
        int price;
        bool color; // t red f black
        RBNode* left, * right, * parent;
        RBNode(int id, int price) {
            this->id = id;
            this->price = price;
            this->color = true;
            this->left = this->right = this->parent = nullptr;
        }
    };
    // TODO: Define your Red-Black Tree node structure
    // Hint: Each node needs: id, price, color, left, right, parent pointers
    RBNode* root;
public:
    ConcreteAuctionTree() {
        root = nullptr;
        // TODO: Initialize your Red-Black Tree
    }
    // BST insert
    void insert(RBNode* y) {
        RBNode* x = root;
        RBNode* parent = nullptr;
        while (x != nullptr) {
            parent = x;
            if (x->price > y->price) {
                x = x->left;
            }
            else {
                x = x->right;
            }
        }
        // y point to perant by perant bointer
        y->parent = parent;

        // the tree was empty
        if (parent == nullptr) {
            root = y;
        }//perant point to y by left or right
        else if (y->price > parent->price) {
            parent->right = y;
        }
        else if (y->price < parent->price) {
            parent->left = y;
        }
        else {
            if (y->id > parent->id) {
                parent->right = y;
            }
            else {
                parent->left = y;
            }

        }
    }
    void rightRotate(RBNode* y) {
        RBNode* x = y->left;
        y->left = x->right;

        if (x->right != nullptr)
            x->right->parent = y;

        x->parent = y->parent;

        if (y->parent == nullptr)
            root = x;
        else if (y == y->parent->right)
            y->parent->right = x;
        else
            y->parent->left = x;

        x->right = y;
        y->parent = x;

    }

    void leftRotate(RBNode* x) {
        RBNode* y = x->right;
        x->right = y->left;

        if (y->left != nullptr)
            y->left->parent = x;

        y->parent = x->parent;

        if (x->parent == nullptr)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;

        y->left = x;
        x->parent = y;
    }


    RBNode* getUncle(RBNode* y) {
        if (y == nullptr || y->parent == nullptr || y->parent->parent == nullptr)
            return nullptr;
        RBNode* grandparent = y->parent->parent;
        if (y->parent == grandparent->left) return grandparent->right;    //isLeftChild(y->parent)
        return grandparent->left;
    }
    //redblack tree fix balancing

    void fixBalance(RBNode* y) {
        RBNode* parent = nullptr;
        RBNode* grandParent = nullptr;

        while ((y != root) && (y->color == true) && (y->parent->color == true)) {
            parent = y->parent;
            grandParent = parent->parent;

            // Case 1: parent is red and uncle is red
            RBNode* uncle = getUncle(y);
            if (uncle != nullptr && (uncle->color == true)) {
                grandParent->color = !grandParent->color;
                parent->color = !parent->color;
                uncle->color = !uncle->color;
                y = grandParent;
            }
            // Case 2: parent is red and uncle is black
            else {
                // Triangle case
                if (parent == grandParent->left && y == parent->right) {
                    leftRotate(parent);
                }
                else if (parent == grandParent->right && y == parent->left) {
                    rightRotate(parent);
                }

                // Line case
                if (grandParent->right == parent) {
                    leftRotate(grandParent);
                }
                else {
                    rightRotate(grandParent);
                }

                swap(grandParent->color, parent->color);
                y = parent;
                parent = y->parent;
            }
        }

        root->color = false; // Root must be black
    }
    void insertItem(int itemID, int price) override {

        RBNode* newNode = new RBNode(itemID, price);
        insert(newNode);
        fixBalance(newNode);

        // TODO: Implement Red-Black Tree insertion
       // Remember to maintain RB-Tree properties with rotations and recoloring
    }

    //search helper with o(n) time complexity
    RBNode* search(RBNode* node, int itemID) {
        if (node == nullptr)
            return nullptr;
        if (node->id == itemID)
            return node;

        RBNode* foundNode = search(node->left, itemID);
        if (foundNode != nullptr)
            return foundNode;

        return (search(node->right, itemID));

    }

    RBNode* getSibling(RBNode* y) {
        if (y == nullptr || y->parent == nullptr)
            return nullptr;
        if (y->parent->left == y) return y->parent->right;
        return y->parent->left;

    }

    //redblack tree fix balancing after deletion
    void fixBalanceDel(RBNode* DB) {
        while (DB != nullptr && DB != root) {
            if (DB == nullptr || DB->parent == nullptr)
                break;
            // find brother
            RBNode* brother = getSibling(DB);
            if (brother == nullptr) {
                DB = DB->parent;
                continue;
            }
            //case1 : brother is red
            if (brother->color == true) {
                if (brother->parent->left == brother) {
                    rightRotate(DB->parent);
                    DB->parent->color = !DB->parent->color;
                    brother->color = !brother->color;
                    continue;
                }
                else {
                    leftRotate(DB->parent);
                    DB->parent->color = !DB->parent->color;
                    brother->color = !brother->color;
                    continue;
                }
            }
            //brother is black      
                //case2 : brother is black and both of brother's children are black
            if ((brother->left == nullptr || brother->left->color == false) &&
                (brother->right == nullptr || brother->right->color == false)) {
                if (DB->parent->color == true) {
                    DB->parent->color = false;
                    brother->color = true;
                    DB = nullptr;
                    continue;
                }
                else {
                    brother->color = true;
                    DB = DB->parent;
                }
                continue;
            }
            // case 3 : brother is black & the near child is Red & the other child black
            RBNode* near;
            RBNode* far;
            if (DB->parent->left == DB) {
                near = brother->left;
                far = brother->right;
            }
            else {
                near = brother->right;
                far = brother->left;
            }

            if ((near != nullptr && near->color == true) && (far == nullptr || far->color == false)) {
                if (DB->parent->left == DB) {
                    rightRotate(brother);
                }
                else {
                    leftRotate(brother);
                }
                swap(brother->color, near->color);
                continue;
            }//case 4 : brother is black & the far child is Red
            if (far != nullptr && far->color == true) {
                if (DB->parent->left == DB) {
                    leftRotate(DB->parent);
                }
                else {
                    rightRotate(DB->parent);
                }
                swap(DB->parent->color, brother->color);
                far->color = false;
                DB = nullptr;
            }
        }
    }

    void deleteItem(int itemID) override {
        RBNode* selected = search(root, itemID);
        if (selected == nullptr) {
            return;
        }
        else if (selected == root && selected->left == nullptr && selected->right == nullptr) {
            delete root;
            root = nullptr;
            return;
        }

        else {

            //if the node has one right child
            if (selected->right != nullptr && selected->left == nullptr) {
                swap(selected->id, selected->right->id);
                swap(selected->price, selected->right->price);
                selected = selected->right;
            } //if the node has one left child
            else if (selected->left != nullptr && selected->right == nullptr) {
                swap(selected->id, selected->left->id);
                swap(selected->price, selected->left->price);
                selected = selected->left;
            }// if the node has two children 
            else if (selected->left != nullptr && selected->right != nullptr) {
                RBNode* predecessor = selected->left;
                while (predecessor->right != nullptr) {
                    predecessor = predecessor->right;
                }swap(selected->id, predecessor->id);
                swap(selected->price, predecessor->price);
                selected = predecessor;
            }
            // if the node is red leaf just delete it
            if (selected->color == true) {
                if (selected->parent->left == selected) {
                    selected->parent->left = nullptr;
                }
                else {
                    selected->parent->right = nullptr;
                }
                delete selected;
                return;
            }

            else {
                RBNode* DB = nullptr;
                RBNode* DBparent = selected->parent;

                if (selected->parent->left == selected) {
                    selected->parent->left = nullptr;
                }
                else {
                    selected->parent->right = nullptr;
                }delete selected;
                fixBalanceDel(DBparent);

            }
            return;

            // TODO: Implement Red-Black Tree deletion
            // This is complex - handle all cases carefully
        }
    }

};


// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================

// find closest sum to total/2
int solve(int current , int remaining, int dp[][1000],vector<int>& coins,int n) {
    // TODO: Implement partition problem using DP
    // Goal: Minimize |sum(subset1) - sum(subset2)|
    // Hint: Use subset sum DP to find closest sum to total/2
    int sum1 = 0 ;
    int sum2 = 0 ;
    if (current == n || remaining == 0)
        return 0;
        
    if(dp[current][remaining] != -1)
        return dp[current][remaining];    
   
    sum1 = solve(current + 1, remaining, dp, coins, n);
    
    if (coins[current] <= remaining){
        sum2 = coins[current] + solve(current + 1, remaining - coins[current], dp, coins, n);
    }
    dp[current][remaining] = max(sum1, sum2);
    return dp[current][remaining];
    
}

//helper function and return difference 
int optimizeLootSplit (int n,vector<int>& coins) {
    int sum = 0 ;
    for(int i=0 ; i<n ;i++)sum+=coins[i];
    int half=sum/2;
    int dp[100][1000];

    for (int i = 0; i < n; i++){
        for (int j = 0; j <= half; j++){
            dp[i][j] = -1 ;
            }
        }    
    int best = solve( 0 , half,dp , coins,n );
    return sum -(2*best);
  
}


//knapsack problem
int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int>>& items) {
    // TODO: Implement 0/1 Knapsack using DP
    // items = {weight, value} pairs
    // Return maximum value achievable within capacity
    int n = items.size();
    int w = capacity;
    int arr[n + 1][w + 1];
    for (int i = 0; i <= n; i++) { // O(n)
        for (int j = 0; j <= w; j++) { // O(w)
            if (i == 0 || j == 0) {
                arr[i][j] = 0;
            }
            else {
                int weight = items[i - 1].first;
                int value = items[i - 1].second;

                if (weight > j) {
                    arr[i][j] = arr[i - 1][j]; // put the last or above val
                }
                else {
                    arr[i][j] = max(arr[i - 1][j], arr[i - 1][j - weight] + value);
                }
            }
        }
    }

    for (int i = 0; i <= items.size(); i++) {
        for (int j = 0; j <= capacity; j++) {
            cout << arr[i][j] << " ";
        }cout << endl;
    }
    return arr[n][w];
};

//long long InventorySystem::countStringPossibilities(string s) {
//    //So that the number of possibilities does not exceed the largest value in long long
//    const long long M = 1000000007;
//    // String length
//    int len = static_cast<int>(s.size());
//    
//    // Array to save possibilities for each char
//    long long prob[len + 1];
//
//    // Base case
//    prob[0] = 1;
//
//    for (int i = 1; i <= len; i++) {
//        // If I have any single letter, then initially its probability equals the probability of the letter that precedes it.
//        prob[i] = prob[i - 1];
//        // If the letter and the one following it are nn or uu, I will combine the possibilities of the (i-2) letter with its possibilities.
//        if ((s[i - 1] == 'n' && s[i - 2] == 'n') || (s[i - 1] == 'u' && s[i - 2] == 'u')) {
//            prob[i] = (prob[i] + prob[i - 2]) % M;
//        }
//    }
//    return prob[len];
//}


long long InventorySystem::countStringPossibilities(string s) {
    const long long M = 1000000007;
    int len = static_cast<int>(s.size());

    long long* prob = new long long[len + 1]; // allocate dynamically
    prob[0] = 1;

    for (int i = 1; i <= len; i++) {
        prob[i] = prob[i - 1];
        if (i >= 2 && ((s[i - 1] == 'n' && s[i - 2] == 'n') || (s[i - 1] == 'u' && s[i - 2] == 'u'))) {
            prob[i] = (prob[i] + prob[i - 2]) % M;
        }
    }

    long long result = prob[len];
    delete[] prob; // important to free memory
    return result;
}


// =========================================================
// PART C: WORLD NAVIGATOR (Graphs)
// =========================================================

bool WorldNavigator::pathExists(int n, vector<vector<int>>& edges, int source, int dest) {
    // TODO: Implement using BFS -> Breadth First search O(V + E)
    // edges are bidirectional
    if (source == dest) {  // best case
        return true;
    }

    vector<vector<int>>graph(n);

    //make all possible edges ex. 0 -> 1, 1 -> 0
    for (auto edge : edges) { //O(E)
        graph[edge[0]].push_back(edge[1]);
        graph[edge[1]].push_back(edge[0]);
    }
    vector<bool> visited(n, false);
    queue<int> q;

    q.push(source);
    visited[source] = true;

    while (!q.empty()) { // O(V + E)
        int curr = q.front();
        q.pop();
        if (curr == dest) {
            return true;
        }
        for (auto neighbor : graph[curr]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }

    return false;
}

long long WorldNavigator::minBribeCost(int n, int m, long long goldRate, long long silverRate,
    vector<vector<int>>& roadData) {
    // TODO: Implement Minimum Spanning Tree (Kruskal's or Prim's)
    // roadData[i] = {u, v, goldCost, silverCost}
    // Total cost = goldCost * goldRate + silverCost * silverRate
    // Return -1 if graph cannot be fully connected
    return -1;
}

string WorldNavigator::sumMinDistancesBinary(int n, vector<vector<int>>& roads) {
    // initialize the matrix A
    const int inf = 1e9;
    vector<vector<int>> A(n, vector<int>(n, inf));
    // handle case of 0 or 1 vertix, return 0 distance path
    if (n <= 1) return 0;


    for (int i = 0; i < n; i++)
    {
        A[i][i] = 0;
    }
    for (auto& road : roads)
    {
        int v1 = road[0];
        int v2 = road[1];
        int d = road[2];
        // handle vertices values out of n bounds
        if (v1 < 0 || v2 < 0 || v1 >= n || v2 >= n)
        {
            continue;
        }

        // handle loops
        if (v1 == v2)
            continue;

        // handle 0 and non power of 2 distances
        if (!(d > 0) || !((d & (d - 1)) == 0))
        {
            continue;
        }

        // handle negative distances
        if (d < 0)
            continue;

        A[v1][v2] = d;
    }

    // update the matrix to get shortest paths
    for (int k = 0; k < n; k++)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                A[i][j] = min(A[i][j], A[i][k] + A[k][j]);
            }
        }
    }

    // add shortest paths in the upper triangle
    long long sum = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i < j && A[i][j] != inf)
                sum += A[i][j];
        }
    }
    // bin output generation
    string bin = "";
    if (sum == 0)
        bin += "0";
    while (sum > 0)
    {
        bin = char((sum % 2) + '0') + bin;
        sum /= 2;
    }

    return bin;
}


// =========================================================
// PART D: SERVER KERNEL (Greedy)
// =========================================================

int ServerKernel::minIntervals(vector<char>& tasks, int n) {
    priority_queue<pair<int, char>> freq_task_queue;    //(freq, task)
    vector<int> freq(26, 0);
    queue<pair<int, pair<int, char>>> cooldown; // (ready interval, (remaining freq, task id))

    // handle empty tasks
    if (tasks.empty()) return 0;

    // handle huge input sizes
    if (tasks.size() > 104)
        return -1;

    // handle n = 0 case
    if (n == 0) return tasks.size();

    // initializing freq vector
    for (char task : tasks)
    {
        if (task < 'A' || task > 'Z')
            return -1;
        int index = task - 'A';
        freq[index]++;
    }

    // initializing freq_task_queue
    for (int i = 0; i < 26; i++)
    {
        if (freq[i] > 0) //to make sure adding onle existing tasks
            freq_task_queue.push(make_pair(freq[i], char('A' + i)));
    }

    // interval counter
    int interval = 0;

    while (!(freq_task_queue.empty() && cooldown.empty()))
    {
        // check if cooling down ends
        while (!cooldown.empty() && cooldown.front().first == interval)
        {
            freq_task_queue.push(cooldown.front().second);
            cooldown.pop();
        }

        // implement the task in oreder
        if (!freq_task_queue.empty())
        {
            auto task = freq_task_queue.top();
            freq_task_queue.pop();
            task.first -= 1;

            if (task.first > 0)
                cooldown.push(make_pair(interval + n + 1, task));
        }
        // for both idle case or the next different task
        interval++;
    }
    return interval;
}
// =========================================================
// FACTORY FUNCTIONS (Required for Testing)
// =========================================================

extern "C" {
    PlayerTable* createPlayerTable() {
        return new ConcretePlayerTable();
    }

    Leaderboard* createLeaderboard() {
        return new ConcreteLeaderboard();
    }

    AuctionTree* createAuctionTree() {
        return new ConcreteAuctionTree();
    }
}

// ==========================================
// TEST UTILITIES
// ==========================================
class StudentTestRunner {
    int count = 0;
    int passed = 0;
    int failed = 0;

public:
    void runTest(string testName, bool condition) {
        count++;
        cout << "TEST: " << left << setw(50) << testName;
        if (condition) {
            cout << "[ PASS ]";
            passed++;
        }
        else {
            cout << "[ FAIL ]";
            failed++;
        }
        cout << endl;
    }

    void printSummary() {
        cout << "\n==========================================" << endl;
        cout << "SUMMARY: Passed: " << passed << " | Failed: " << failed << endl;
        cout << "==========================================" << endl;
        cout << "TOTAL TESTS: " << count << endl;
        if (failed == 0) {
            cout << "Great job! All basic scenarios passed." << endl;
            cout << "Now make sure to handle edge cases (empty inputs, collisions, etc.)!" << endl;
        }
        else {
            cout << "Some basic tests failed. Check your logic against the PDF examples." << endl;
        }
    }
};

StudentTestRunner runner;

// ==========================================
// PART A: DATA STRUCTURES
// ==========================================

void test_PartA_DataStructures() {
    cout << "\n--- Part A: Data Structures ---" << endl;

    // 1. PlayerTable (Double Hashing)
    // Requirement: Basic Insert and Search
    PlayerTable* table = createPlayerTable();
    runner.runTest("PlayerTable: Insert 'Alice' and Search", [&]() {
        table->insert(101, "Alice");
        return table->search(101) == "Alice";
        }());
    runner.runTest("PlayerTable: Insert 'Alice' and Search", [&]() {
        table->insert(101, "Alice");
        return table->search(101) == "Alice";
        }());
    // test for col
    runner.runTest("PlayerTable: Insert 'Collision' and Search", [&]() {
        table->insert(202, "Collision");
        return table->search(202) == "Collision";
        }());
    delete table;

    // 2. Leaderboard (Skip List)
    Leaderboard* board = createLeaderboard();
    // Test A: Basic High Score
    runner.runTest("Leaderboard: Add Scores & Get Top 1", [&]() {
        board->addScore(1, 100);
        board->addScore(2, 200); // 2 is Leader
        vector<int> top = board->getTopN(1);
        return (!top.empty() && top[0] == 2);
        }());

    //Test B: Tie-Breaking Visual Example (Crucial!)
    //PDF Visual Example: Player A (ID 10) 500pts, Player B (ID 20) 500pts.
    //Correct Order: ID 10 then ID 20.
    runner.runTest("Leaderboard: Tie-Break (ID 10 before ID 20)", [&]() {
        board->addScore(10, 500);
        board->addScore(20, 500);
        vector<int> top = board->getTopN(2);
        // We expect {10, 20} NOT {20, 10}
        if (top.size() < 2) return false;
        return (top[0] == 10 && top[1] == 20);
        }());

    delete board;
    //
    //     // 3. AuctionTree (Red-Black Tree)
    //     // Requirement: Insert items without crashing
        AuctionTree* tree = createAuctionTree();
        runner.runTest("AuctionTree: Insert Items", [&]() {
          tree->insertItem(1, 100);
          tree->insertItem(2, 50);
           return true; // Pass if no crash
        }());
         delete tree;
}

// ==========================================
// PART B: INVENTORY SYSTEM
// ==========================================

void test_PartB_Inventory() {
    cout << "\n--- Part B: Inventory System ---" << endl;

    // // 1. Loot Splitting (Partition)
    // // PDF Example: coins = {1, 2, 4} -> Best split {4} vs {1,2} -> Diff 1
    // runner.runTest("LootSplit: {1, 2, 4} -> Diff 1", [&]() {
    //     vector<int> coins = {1, 2, 4};
    //     return InventorySystem::optimizeLootSplit(3, coins) == 1;
    // }());

    // 2. Inventory Packer (Knapsack)
    // PDF Example: Cap=10, Items={{1,10}, {2,20}, {3,30}}. All fit. Value=60.
    runner.runTest("Knapsack: Cap 3, All Fit -> Value 40", [&]() {
        vector<pair<int, int>> items = { {1, 10}, {2, 15}, {3, 40} };
        return InventorySystem::maximizeCarryValue(3, items) == 40;
        }());
    runner.runTest("Knapsack: Cap 5, All Fit -> Value 55", [&]() {
        vector<pair<int, int>> items = { {1, 10}, {2, 15}, {3, 40} };
        return InventorySystem::maximizeCarryValue(5, items) == 55;
        }());
    runner.runTest("Knapsack: Cap 10, All Fit -> Value 60", [&]() {
        vector<pair<int, int>> items = { {1, 10}, {2, 20}, {3, 30} };
        return InventorySystem::maximizeCarryValue(10, items) == 60;
        }());

    // 3. Chat Autocorrect (String DP)
    // PDF Example: "uu" -> "uu" or "w" -> 2 possibilities
    runner.runTest("ChatDecorder: 'uu' -> 2 Possibilities", [&]() {
        return InventorySystem::countStringPossibilities("uu") == 2;
        }());
    runner.runTest("ChatDecorder: 'uunn' -> 4 Possibilities", [&]() {
        return InventorySystem::countStringPossibilities("uunn") == 4;
        }());
}

// ==========================================
// PART C: WORLD NAVIGATOR
// ==========================================

void test_PartC_Navigator() {
    cout << "\n--- Part C: World Navigator ---" << endl;
    //
    // 1. Safe Passage (Path Exists)
    // PDF Example: 0-1, 1-2. Path 0->2 exists.
    runner.runTest("PathExists: 0->1->2 -> True", [&]() {
        vector<vector<int>> edges = { {0, 1}, {1, 2} };
        return WorldNavigator::pathExists(3, edges, 0, 2) == true;
        }());
    runner.runTest("PathExists: 0->0 -> True", [&]() {
        vector<vector<int>> edges = {};
        return WorldNavigator::pathExists(1, edges, 0, 0) == true;
        }());
    runner.runTest("PathExists: 0->1, 2->3 -> False", [&]() {
        vector<vector<int>> edges = { {0, 1}, {2, 3} };
        return WorldNavigator::pathExists(4, edges, 0, 3) == false;
        }());

    //     // 2. The Bribe (MST)
    //     // PDF Example: 3 Nodes. Roads: {0,1,10}, {1,2,5}, {0,2,20}. Rate=1.
    //     // MST should pick 10 and 5. Total 15.
    //     runner.runTest("MinBribeCost: Triangle Graph -> Cost 15", [&]() {
    //         vector<vector<int>> roads = {
    //             {0, 1, 10, 0},
    //             {1, 2, 5, 0},
    //             {0, 2, 20, 0}
    //         };
    //         // n=3, m=3, goldRate=1, silverRate=1
    //         return WorldNavigator::minBribeCost(3, 3, 1, 1, roads) == 15;
    //     }());
    //
    //     // 3. Teleporter (Binary Sum APSP)
    //     // PDF Example: 0-1 (1), 1-2 (2). Distances: 1, 2, 3. Sum=6 -> "110"
    runner.runTest("BinarySum: Line Graph -> '110'", [&]() {
        vector<vector<int>> roads = {
        {0, 1, 1},
        {1, 2, 2}
        };
        return WorldNavigator::sumMinDistancesBinary(3, roads) == "110";
        }());
    runner.runTest("BinarySum: Line Graph -> '100'", [&]() {
        vector<vector<int>> roads = {
        {0, 1, 4}
        };
        return WorldNavigator::sumMinDistancesBinary(2, roads) == "100";
        }());
    runner.runTest("BinarySum: Line Graph -> '1010'", [&]() {
        vector<vector<int>> roads = {
        {0, 1, 2},
         {0,2,8}
        };
        return WorldNavigator::sumMinDistancesBinary(3, roads) == "1010";
        }());
}

// ==========================================
// PART D: SERVER KERNEL
// ==========================================

void test_PartD_Kernel() {
    cout << "\n--- Part D: Server Kernel ---" << endl;

    // 1. Task Scheduler
    // PDF Example: Tasks={A, A, B}, n=2.
    // Order: A -> B -> idle -> A. Total intervals: 4.
    runner.runTest("Scheduler: {A, A, B}, n=2 -> 4 Intervals", [&]() {
        vector<char> tasks = { 'A', 'A', 'B' };
        return ServerKernel::minIntervals(tasks, 2) == 4;
        }());
    runner.runTest("Scheduler: {A, A, A}, n=2 -> 7 Intervals", [&]() {
        vector<char> tasks = { 'A', 'A', 'A' };
        return ServerKernel::minIntervals(tasks, 2) == 7;
        }());
    runner.runTest("Scheduler: {A, B, C}, n=2 -> 3 Intervals", [&]() {
        vector<char> tasks = { 'A', 'B', 'C' };
        return ServerKernel::minIntervals(tasks, 2) == 3;
        }());
    runner.runTest("Scheduler: {A, A, A, B, B, B}, n=2 -> 8 Intervals", [&]() {
        vector<char> tasks = { 'A', 'A', 'A', 'B', 'B', 'B' };
        return ServerKernel::minIntervals(tasks, 2) == 8;
        }());
}

int main() {
    cout << "Arcadia Engine - Student Happy Path Tests" << endl;
    cout << "-----------------------------------------" << endl;

    test_PartA_DataStructures();
    test_PartB_Inventory();
    test_PartC_Navigator();
    test_PartD_Kernel();

    runner.printSummary();

    return 0;
}
