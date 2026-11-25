#include "AVLTree.h"
#include "WarehouseInput.h"
#include <cmath>
#include <iostream>

using namespace std;

AVLTree::AVLTree(){
    root = nullptr;
}

AVLTree::~AVLTree(){
    destroyTree(root);
    root = nullptr;
}

int AVLTree::getHeight(Node* node) const{
    if (node == nullptr) {
        return 0;
    }
    return node->height;
}

int AVLTree::getMax(int a, int b) const{
    if (a > b) {
        return a;
    }
    return b;
}

// Left aur right subtree ki height dekh kar current node ki height update karo
void AVLTree::updateHeight(Node* node){
    if (node == nullptr) {
        return;
    }
    int leftH = getHeight(node->left);
    int rightH = getHeight(node->right);
    node->height = 1 + getMax(leftH, rightH);
}

// Balance factor = left height - right height
int AVLTree::getBalance(Node* node) const{
    if (node == nullptr) {
        return 0;
    }
    return getHeight(node->left) - getHeight(node->right);
}

// Left rotation (RR aur LR case ke liye)
AVLTree::Node* AVLTree::rotateLeft(Node* node){
    Node* newRoot = node->right;
    Node* movedSubtree = newRoot->left;

    newRoot->left = node;
    node->right = movedSubtree;

    // Pehle neeche wale node ki height update karo, phir naye root ki
    updateHeight(node);
    updateHeight(newRoot);

    return newRoot;
}

// Right rotation (LL aur RL case ke liye)
AVLTree::Node* AVLTree::rotateRight(Node* node){
    Node* newRoot = node->left;
    Node* movedSubtree = newRoot->right;

    newRoot->right = node;
    node->left = movedSubtree;

    updateHeight(node);
    updateHeight(newRoot);

    return newRoot;
}

// Tree ko balanced rakhne ke liye check karo (LL, LR, RR, RL cases)
AVLTree::Node* AVLTree::balanceNode(Node* node){
    updateHeight(node);
    int bf = getBalance(node);

    // Left heavy case
    if (bf > 1) {
        // Agar left child ka right subtree bada hai toh LR case hai
        if (getHeight(node->left->right) > getHeight(node->left->left)) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }

    // Right heavy case
    if (bf < -1) {
        // Agar right child ka left subtree bada hai toh RL case hai
        if (getHeight(node->right->left) > getHeight(node->right->right)) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }

    return node;
}

AVLTree::Node* AVLTree::insertNode(Node* branch, Node* newNode){
    if (branch == nullptr) {
        return newNode;
    }
    if (newNode->product.productID < branch->product.productID) {
        branch->left = insertNode(branch->left, newNode);
    } else {
        branch->right = insertNode(branch->right, newNode);
    }
    return balanceNode(branch);
}

// Naya product validate karke AVL tree mein insert karo
bool AVLTree::insert(const string& id, const string& name, double weight, int threshold){
    if (!validWarehouseID(id, "P")) {
        return false;
    }
    if (name.empty()) {
        return false;
    }
    if (!isfinite(weight) || weight <= 0) {
        return false;
    }
    if (threshold < 0) {
        return false;
    }
    // Duplicate product ID not allowed
    if (search(id) != nullptr) {
        return false;
    }

    Node* newNode = new Node(id, name, weight, threshold);
    root = insertNode(root, newNode);
    return true;
}
// Product search throughIDS
Product* AVLTree::search(const string& productID) const{
    Node* curr = root;
    while (curr != nullptr) {
        if (productID == curr->product.productID) {
            return &curr->product;
        }
        if (productID < curr->product.productID) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return nullptr;
}

// Right subtree se sabse chhota node (inorder successor) alag nikal lo
// Isse product copy nahi karna padta aur uske rack pointers safe rehte hain
AVLTree::Node* AVLTree::MinNode(Node* branch, Node*& minNode){
    if (branch->left == nullptr) {
        minNode = branch;
        return branch->right;
    }
    branch->left = MinNode(branch->left, minNode);
    return balanceNode(branch);
}

// Recursive delete function
AVLTree::Node* AVLTree::eraseNode(Node* branch, const string& productID){
    if (branch == nullptr) {
        return nullptr;
    }

    if (productID < branch->product.productID) {
        branch->left = eraseNode(branch->left, productID);
    } else if (productID > branch->product.productID) {
        branch->right = eraseNode(branch->right, productID);
    } else {
        // Node mil gaya jisko delete karna hai
        Node* leftChild = branch->left;
        Node* rightChild = branch->right;
        delete branch;

        // Agar ek hi child hai ya koi child nahi hai
        if (leftChild == nullptr) {
            return rightChild;
        }
        if (rightChild == nullptr) {
            return leftChild;
        }

        // Agar dono child hain toh right subtree ka minimum node upar le aao
        Node* successor = nullptr;
        rightChild = MinNode(rightChild, successor);
        successor->left = leftChild;
        successor->right = rightChild;
        return balanceNode(successor);
    }

    return balanceNode(branch);
}

// Delete Product only if it has no reservations
bool AVLTree::erase(const string& productID){
    Product* product = search(productID);
    if (product == nullptr) {
        return false;
    }
    if (product->hasReservations()) {
        return false;
    }

    root = eraseNode(root, productID);
    return true;
}

// Delete nodes using Post-order traversal
void AVLTree::destroyTree(Node* branch){
    if (branch == nullptr) {
        return;
    }
    destroyTree(branch->left);
    destroyTree(branch->right);
    delete branch;
}

// print low stock products in sorted order using In-order traversal
void AVLTree::displayLowStockHelper(Node* branch, bool& found) const{
    if (branch == nullptr) {
        return;
    }

    displayLowStockHelper(branch->left, found);

    long long available = branch->product.availableStock();
    if (available < branch->product.reorderLevel) {
        found = true;
        cout << branch->product.productID << " " << branch->product.name
             << " | available " << available
             << " | reorder " << branch->product.reorderLevel << "\n";
    }

    displayLowStockHelper(branch->right, found);
}

void AVLTree::displayLowStock() const{
    bool found = false;
    displayLowStockHelper(root, found);
    if (!found) {
        cout << "No products below reorder level.\n";
    }
}

// Order cancel ya requeue hone par uske saare reservations free karo
void AVLTree::releaseOrderHelper(Node* branch, const string& orderID){
    if (branch == nullptr) {
        return;
    }

    releaseOrderHelper(branch->left, orderID);

    RackEntry* rack = branch->product.racks;
    while (rack != nullptr) {
        rack->releaseAll(orderID);
        rack = rack->next;
    }

    releaseOrderHelper(branch->right, orderID);
}

void AVLTree::releaseOrderReservations(const string& orderID){
    releaseOrderHelper(root, orderID);
}
