#pragma once
#include "bootloader32.h"
#include "kernel.h"

template <class T> class LinkedList {
    struct Node {
        T data;
        Node *next;
    };

  public:
    Node head;
    Node *tail;

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
        while (cur) {
            if (cur->data == elem) {
                prev->next = cur->next;
                free(cur);
                if (tail == cur) {
                    tail = prev;
                }
                return;
            }

            prev = cur;
            cur = cur->next;
        }
        fatal("elem not found");
    }

    // return the first element matching function
    T locate(bool (*filter)(T)) {
        Node *cur = head.next;
        while (cur) {
            if (filter(cur->data)) {
                return cur->data;
            }
            cur = cur->next;
        }
        fatal("Element not found in linked list");
    }

    // execute func on each element.
    void for_each(void (*func)(T&)) {
        Node *cur = head.next;
        while (cur) {
            func(cur->data);
            cur = cur->next;
        }
    }

    // only keep elements if filter return true.
    void filter(bool (*filter)(T)) {
        Node *prev = &head;
        Node *cur = prev->next;
        while (cur) {
            if (!filter(cur->data)) {
                prev->next = cur->next;
                free(cur);
                if (tail == cur) {
                    tail = prev;
                }
                cur = prev->next;
                continue;
            }
            prev = cur;
            cur = cur->next;
        }
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