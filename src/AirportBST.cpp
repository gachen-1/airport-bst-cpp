#include "AirportBST.h"

AirportBST::AirportBST() : root(nullptr), count(0) {}

AirportBST::~AirportBST()
{
    clear();
}

void AirportBST::clear()
{
    // Free every node with an explicit stack (works for a chain of any length).
    std::vector<Node*> stack;
    if (root != nullptr) stack.push_back(root);
    while (!stack.empty())
    {
        Node* n = stack.back();
        stack.pop_back();
        if (n->left != nullptr) stack.push_back(n->left);
        if (n->right != nullptr) stack.push_back(n->right);
        delete n;
    }
    root = nullptr;
    count = 0;
}

int AirportBST::size() const { return count; }

bool AirportBST::empty() const { return root == nullptr; }

std::string AirportBST::rootCode() const
{
    return root == nullptr ? "" : root->data.iata;
}

// ------------------------------------------------------------------ insert
bool AirportBST::insert(const Airport& a)
{
    if (root == nullptr)                         // I1: empty tree
    {
        root = new Node(a);
        count++;
        return true;
    }
    Node* cur = root;
    while (true)
    {
        if (a.iata < cur->data.iata)             // I2: go left
        {
            if (cur->left == nullptr)            // I1: empty spot
            {
                cur->left = new Node(a);
                count++;
                return true;
            }
            cur = cur->left;
        }
        else if (a.iata > cur->data.iata)        // I3: go right
        {
            if (cur->right == nullptr)           // I1: empty spot
            {
                cur->right = new Node(a);
                count++;
                return true;
            }
            cur = cur->right;
        }
        else                                     // I4: duplicate, do nothing
        {
            return false;
        }
    }
}

// ------------------------------------------------------------------ search
const Airport* AirportBST::find(const std::string& code, std::vector<std::string>& path) const
{
    path.clear();
    Node* cur = root;
    while (cur != nullptr)                       // S1 when cur becomes NULL
    {
        path.push_back(cur->data.iata);
        if (code == cur->data.iata) return &cur->data;   // S2: found
        if (code < cur->data.iata) cur = cur->left;      // S3: go left
        else cur = cur->right;                           // S4: go right
    }
    return nullptr;
}

bool AirportBST::contains(const std::string& code, int& comparisons) const
{
    comparisons = 0;
    Node* cur = root;
    while (cur != nullptr)
    {
        comparisons++;
        if (code == cur->data.iata) return true;
        if (code < cur->data.iata) cur = cur->left;
        else cur = cur->right;
    }
    return false;
}

const Airport* AirportBST::findMin() const
{
    if (root == nullptr) return nullptr;
    Node* cur = root;
    while (cur->left != nullptr) cur = cur->left;
    return &cur->data;
}

const Airport* AirportBST::findMax() const
{
    if (root == nullptr) return nullptr;
    Node* cur = root;
    while (cur->right != nullptr) cur = cur->right;
    return &cur->data;
}

// ------------------------------------------------------------------ delete
bool AirportBST::remove(const std::string& code, std::string& report)
{
    // Search for the node and remember its parent (B1).
    Node* parent = nullptr;
    Node* cur = root;
    while (cur != nullptr && cur->data.iata != code)
    {
        parent = cur;
        if (code < cur->data.iata) cur = cur->left;
        else cur = cur->right;
    }
    if (cur == nullptr)                          // S1 / B2: not found, nothing changes
    {
        report = code + " is not in the tree (S1): nothing deleted";
        return false;
    }

    bool isRoot = (parent == nullptr);
    std::string rootNote = isRoot ? ", the root" : "";

    if (cur->left != nullptr && cur->right != nullptr)
    {
        // D3: two children. Find the inorder successor: right once, then left as far as possible.
        Node* succParent = cur;
        Node* succ = cur->right;
        while (succ->left != nullptr)
        {
            succParent = succ;
            succ = succ->left;
        }
        std::string succCase = (succ->right == nullptr) ? "D1" : "D2";
        report = "Deleted " + code + " (case D3: two children" + rootNote + "; successor "
                 + succ->data.iata + " copied in, successor removed with " + succCase + ")";

        cur->data = succ->data;                  // copy the successor into the node
        if (succParent == cur) succParent->right = succ->right;   // successor was cur's right child
        else succParent->left = succ->right;     // successor never has a left child
        delete succ;
    }
    else
    {
        // D1 (leaf) or D2 (one child): the only child (or NULL) takes the node's place.
        Node* child = (cur->left != nullptr) ? cur->left : cur->right;
        if (cur->left == nullptr && cur->right == nullptr)
            report = "Deleted " + code + " (case D1: leaf" + rootNote + ")";
        else if (cur->left != nullptr)
            report = "Deleted " + code + " (case D2: only a left child, " + child->data.iata + rootNote + ")";
        else
            report = "Deleted " + code + " (case D2: only a right child, " + child->data.iata + rootNote + ")";

        if (parent == nullptr) root = child;     // B6: the root pointer itself changes
        else if (parent->left == cur) parent->left = child;
        else parent->right = child;
        delete cur;
    }
    count--;
    return true;
}

// ------------------------------------------------------------------ shape
std::vector<std::vector<std::string>> AirportBST::levels(int maxLevels) const
{
    // Level order with a queue, one level at a time. maxLevels < 0 means all levels.
    std::vector<std::vector<std::string>> result;
    std::vector<Node*> level;
    if (root != nullptr) level.push_back(root);
    while (!level.empty() && (maxLevels < 0 || (int)result.size() < maxLevels))
    {
        std::vector<std::string> codes;
        std::vector<Node*> next;
        int n = (int)level.size();
        for (int i = 0; i < n; i++)
        {
            codes.push_back(level[i]->data.iata);
            if (level[i]->left != nullptr) next.push_back(level[i]->left);
            if (level[i]->right != nullptr) next.push_back(level[i]->right);
        }
        result.push_back(codes);
        level = next;
    }
    return result;
}

int AirportBST::height() const
{
    if (root == nullptr) return -1;
    return (int)levels(-1).size() - 1;
}

double AirportBST::averageDepth() const
{
    if (root == nullptr) return 0.0;
    std::vector<std::vector<std::string>> all = levels(-1);
    long long total = 0;
    int n = (int)all.size();
    for (int d = 0; d < n; d++) total += (long long)d * (long long)all[d].size();
    return (double)total / (double)count;
}

std::string AirportBST::deepestCode() const
{
    std::vector<std::vector<std::string>> all = levels(-1);
    if (all.empty()) return "";
    return all.back().front();
}

std::string AirportBST::exampleCode(int kind) const
{
    // Look through the tree in level order and return the first node (not the root)
    // that has the requested shape.
    std::vector<Node*> queue;
    if (root != nullptr) queue.push_back(root);
    for (int i = 0; i < (int)queue.size(); i++)
    {
        Node* n = queue[i];
        bool hasLeft = (n->left != nullptr);
        bool hasRight = (n->right != nullptr);
        int shape = (!hasLeft && !hasRight) ? 0 : (hasLeft && !hasRight) ? 1 : (!hasLeft && hasRight) ? 2 : 3;
        if (n != root && shape == kind) return n->data.iata;
        if (hasLeft) queue.push_back(n->left);
        if (hasRight) queue.push_back(n->right);
    }
    return "";
}

// ------------------------------------------------------------------ traversals
void AirportBST::inorder(std::vector<const Airport*>& out) const
{
    out.clear();
    std::vector<Node*> stack;
    Node* cur = root;
    while (cur != nullptr || !stack.empty())
    {
        while (cur != nullptr)                   // go left as far as possible
        {
            stack.push_back(cur);
            cur = cur->left;
        }
        Node* n = stack.back();
        stack.pop_back();
        out.push_back(&n->data);                 // visit
        cur = n->right;                          // then the right subtree
    }
}

void AirportBST::preorderCodes(std::vector<std::string>& out) const
{
    out.clear();
    std::vector<Node*> stack;
    if (root != nullptr) stack.push_back(root);
    while (!stack.empty())
    {
        Node* n = stack.back();
        stack.pop_back();
        out.push_back(n->data.iata);             // node first
        if (n->right != nullptr) stack.push_back(n->right);   // pushed first, visited last
        if (n->left != nullptr) stack.push_back(n->left);
    }
}

// ------------------------------------------------------------------ validate
bool AirportBST::validate() const
{
    // Every node must lie strictly inside the range (lo, hi) inherited from its ancestors.
    // An empty string stands for "no bound".
    struct Item
    {
        Node* node;
        std::string lo;
        std::string hi;
    };
    std::vector<Item> stack;
    if (root != nullptr) stack.push_back({root, "", ""});
    while (!stack.empty())
    {
        Item it = stack.back();
        stack.pop_back();
        const std::string& key = it.node->data.iata;
        if (!it.lo.empty() && !(it.lo < key)) return false;
        if (!it.hi.empty() && !(key < it.hi)) return false;
        if (it.node->left != nullptr) stack.push_back({it.node->left, it.lo, key});    // left: hi = key
        if (it.node->right != nullptr) stack.push_back({it.node->right, key, it.hi});  // right: lo = key
    }
    return true;
}

// ------------------------------------------------------------------ range query
void AirportBST::range(const std::string& from, const std::string& to,
                       std::vector<const Airport*>& out, int& visited) const
{
    // Inorder walk that only goes left when the key is larger than FROM, and only goes
    // right when the key is smaller than TO, so subtrees outside the range are never entered.
    out.clear();
    visited = 0;
    std::vector<Node*> stack;
    Node* cur = root;
    while (cur != nullptr || !stack.empty())
    {
        while (cur != nullptr)
        {
            visited++;
            stack.push_back(cur);
            cur = (from < cur->data.iata) ? cur->left : nullptr;
        }
        Node* n = stack.back();
        stack.pop_back();
        if (!(n->data.iata < from) && !(to < n->data.iata)) out.push_back(&n->data);
        cur = (n->data.iata < to) ? n->right : nullptr;
    }
}

// ------------------------------------------------------------------ balanced build
AirportBST::Node* AirportBST::buildRange(const std::vector<Airport>& sorted, int lo, int hi)
{
    // Recursion depth is only about log2(n), so recursion is safe here.
    if (lo > hi) return nullptr;
    int mid = lo + (hi - lo) / 2;
    Node* n = new Node(sorted[mid]);
    count++;
    n->left = buildRange(sorted, lo, mid - 1);
    n->right = buildRange(sorted, mid + 1, hi);
    return n;
}

void AirportBST::buildBalanced(const std::vector<Airport>& sorted)
{
    clear();
    root = buildRange(sorted, 0, (int)sorted.size() - 1);
}
