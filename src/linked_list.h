#include "kernel.h"

template <class T> class LinkedList {
    struct Node {
        T data;
        Node *next;
    };

    Node head;
    Node *tail;

  public:
    LinkedList() {
        head.next = nullptr;
        tail = &head;
    }

    // insert to last
    void insert(T elem) {
        Node *cur = (Node *)malloc(sizeof(Node));
        cur->data = elem;
        cur->next = nullptr;
        tail->next = cur;
        tail = cur;
    }

    void remove(T elem) {
        Node *cur = head.next;
        Node *prev = &head;
        while (cur && cur->next) {
            if (cur->data == elem) {
                prev->next = cur->next;
                free(cur);
                if (tail == cur) {
                    tail = prev;
                }
                break;
            }

            prev = cur;
            cur = cur->next;
        }
        fatal("elem not found");
    }

    bool is_empty() {
        return head.next == nullptr;
    }
    // push an item to the last of Queue
    void push_back(T elem) {
        return insert(elem);
    }
    // pop the first item
    T pop_front() {
        if (is_empty()) {
            fatal("Empty queue to pop");
        }
        Node *p = head.next;
        T tmp = p->data;
        head.next = p->next;
        if (tail == p) {
            tail = &head;
        }
        free(p);
        return tmp;
    }
};