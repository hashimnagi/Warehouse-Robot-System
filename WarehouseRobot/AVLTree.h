#pragma once
#include "Product.h"

class AVLTree {
private:
    struct Node {
        Product product;
        int height;
        Node* left;
        Node* right;

        Node(const std::string& id, const std::string& name, double weight, int threshold)
            : product(id, name, weight, threshold)
        {
            height = 1;
            left = nullptr;
            right = nullptr;
        }
    };

    Node* root;

    // Helper functions for AVL tree balancing and operations
    int getHeight(Node* node) const;
    int getMax(int a, int b) const;
    void updateHeight(Node* node);
    int getBalance(Node* node) const;
    Node* rotateLeft(Node* node);
    Node* rotateRight(Node* node);
    Node* balanceNode(Node* node);
    Node* insertNode(Node* branch, Node* newNode);
    Node* MinNode(Node* branch, Node*& minNode);
    Node* eraseNode(Node* branch, const std::string& productID);
    void destroyTree(Node* branch);

    // In-order traversal helpers 
    void displayLowStockHelper(Node* branch, bool& found) const;
    void releaseOrderHelper(Node* branch, const std::string& orderID);

public:
    AVLTree();
    ~AVLTree();

    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    bool insert(const std::string& id, const std::string& name, double weight, int threshold);
    bool erase(const std::string& productID);
    Product* search(const std::string& productID) const;

    void displayLowStock() const;
    void releaseOrderReservations(const std::string& orderID);
};
