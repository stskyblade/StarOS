#include "bootloader32.h"
#include "kernel.h"
#include "linked_list.h"

void schedular() {
    debug("Running Schedular...");
    while (true) {
        if (!ready_queue.is_empty()) {
            Process *p = ready_queue.pop_front();
            p->status = Running;
            running_queue.push_back(p);
            CURRENT_PROCESS = p;
            debug("Schedular switching to process...");
            switch_to_process(p);
        }
        ksleep(1);
        printf(".");
    }
}