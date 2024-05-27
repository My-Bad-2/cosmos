#include <cpu/registers.h>
#include <log.h>

void exception_handler(iframe_t* frame) {
    (void)frame;

    log_fatal("Exception occurred!");
}

void nmi_handler(iframe_t* frame) {
    (void)frame;

    log_fatal("NMI occurred!");
}