#include <arch.h>
#include <cpu/cpu.h>
#include <cpu/interrupts.h>
#include <cpu/registers.h>
#include <log.h>

static char* interrupt_exception_messages[32] = {
	"Division By Zero",
	"Debug",
	"Non-Maskable Interrupt",
	"Breakpoint",
	"Detected Overflow",
	"Out Of Bounds",
	"Invalid Opcode",
	"No Co-processor",
	"Double Fault",
	"Co-processor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception",
	"Control Protection Exception",
	"Reserved",
	"Hypervisor Injection Exception",
	"VMM Communication Exception",
	"Security Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

static void dump_interrupt_frame(iframe_t* iframe) {
	log_trace("CS : %#08lx RIP: %0#lx  EFL: %#08lx", iframe->cs, iframe->ip,
			  iframe->flags);
	log_trace("RAX: %#08lx RBX: %#08lx RCX: %#08lx", iframe->rax, iframe->rbx,
			  iframe->rcx);
	log_trace("RDX: %#08lx RSI: %#08lx RBP: %#08lx", iframe->rdx, iframe->rsi,
			  iframe->rbp);
	log_trace("RSP: %#08lx R8 : %#08lx R9 : %#08lx", iframe->rsi, iframe->r8,
			  iframe->r9);
	log_trace("R10: %#08lx R11: %#08lx R12: %#08lx", iframe->r10, iframe->r11,
			  iframe->r12);
	log_trace("R13: %#08lx R14: %#08lx R15: %#08lx", iframe->r13, iframe->r14,
			  iframe->r15);
	log_trace("EC : %#08lx USP: %#08lx USS: %#08lx", iframe->err_code,
			  iframe->user_sp, iframe->user_ss);
	log_trace("CR0: %#08lx CR2: %#08lx CR3: %#08lx", read_cr0(), read_cr2(),
			  read_cr3());
}

static void handle_cpu_exceptions(iframe_t* iframe) {
	log_error("EXCEPTION OCCURRED(%lu) -> %s", iframe->vector,
			  interrupt_exception_messages[iframe->vector]);

	dump_interrupt_frame(iframe);

	log_fatal("Halting system");
}

void exception_handler(iframe_t* iframe) {
	arch_disable_interrupts();

	if (iframe->vector < 32) {
		handle_cpu_exceptions(iframe);
	}

	call_interrupt_handler(iframe);

	arch_enable_interrupts();
}

void nmi_handler(iframe_t* frame) {
	(void)frame;

	log_fatal("NMI occurred!");
}