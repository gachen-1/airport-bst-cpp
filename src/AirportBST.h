#ifndef AIRPORT_BST_H
#define AIRPORT_BST_H

#include <string>
#include <vector>

#include "Airport.h"

// Binary search tree of airports, keyed by IATA code.
// Insert, search, delete and every traversal are written with loops (no recursion on the
// tree height), so even a chain of 6,072 nodes cannot overflow the call stack.
class AirportBST
{
public:
    AirportBST();
    ~AirportBST();

    void clear();
    int size() const;
    bool empty() const;
    std::string rootCode() const;

    // Insert (rules I1 to I4). Returns false for a duplicate code (I4), which is not stored.
    bool insert(const Airport& a);

    // Search (rules S1 to S4). 'path' receives every code compared, in order.
    // Returns the airport, or nullptr if the search reaches NULL (S1).
    const Airport* find(const std::string& code, std::vector<std::string>& path) const;

    // Same search without recording the path, for timing. 'comparisons' = nodes compared.
    bool contains(const std::string& code, int& comparisons) const;

    const Airport* findMin() const;   // follow left links
    const Airport* findMax() const;   // follow right links

    // Delete (rules D1 to D3, root included). 'report' describes the case that was used.
    // Returns false if the code is not in the tree.
    bool remove(const std::string& code, std::string& report);

    // Shape
    int height() const;               // edges on the longest root-to-leaf path; -1 if empty
    double averageDepth() const;      // mean depth of all nodes (root has depth 0)
    std::vector<std::vector<std::string>> levels(int maxLevels) const;   // level order
    std::string deepestCode() const;

    // One example code for each node shape: 0 leaf, 1 only a left child,
    // 2 only a right child, 3 two children. The root is never chosen. "" if none.
    std::string exampleCode(int kind) const;

    // Traversals
    void inorder(std::vector<const Airport*>& out) const;
    void preorderCodes(std::vector<std::string>& out) const;

    // BST property with the range (lo, hi) method.
    bool validate() const;

    // Every airport with FROM <= code <= TO, in order, without entering subtrees that lie
    // outside the range. 'visited' = number of nodes looked at.
    void range(const std::string& from, const std::string& to,
               std::vector<const Airport*>& out, int& visited) const;

    // Replace the tree by a balanced one built from airports sorted by code
    // (middle element as the root of every subtree).
    void buildBalanced(const std::vector<Airport>& sorted);

private:
    struct Node
    {
        Airport data;
        Node* left;
        Node* right;
        explicit Node(const Airport& a) : data(a), left(nullptr), right(nullptr) {}
    };

    Node* root;
    int count;

    Node* buildRange(const std::vector<Airport>& sorted, int lo, int hi);

    AirportBST(const AirportBST&) = delete;
    AirportBST& operator=(const AirportBST&) = delete;
};

#endif
