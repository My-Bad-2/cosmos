#include <drivers/ioapic.h>
#include <log.h>

#include <utils/math.h>
#include <utils/mmio.h>
#include <utils/sync.h>
#include <utils/vector.h>

#define IOAPIC_IND(base) ((uint8_t*)(base) + IOAPIC_IOREGSEL)
#define IOAPIC_DAT(base) ((uint8_t*)(base) + IOAPIC_IOWIN)
#define IOAPIC_EOIR(base) ((uint8_t*)(base) + 0x40)

// The minimum address space required past the base address
#define IOAPIC_WINDOW_SIZE 0x44
// The minimum version that supported the EOIR
#define IOAPIC_EOIR_MIN_VERSION 0x20

// IO APIC register offsets
#define IOAPIC_REG_RTE(idx) (0x10 + 2 * (idx))

// Macros for extracting data from REG_ID
#define IOAPIC_ID_ID(v) (((v) >> 24) & 0xf)

// Macros for extracting data from REG_VER
#define IOAPIC_VER_MAX_REDIR_ENTRY(v) (((v) >> 16) & 0xff)
#define IOAPIC_VER_VERSION(v) ((v) & 0xff)

// Macros for writing REG_RTE entries
#define IOAPIC_RTE_DST(v) (((uint64_t)(v)) << 56)
#define IOAPIC_RTE_EXTENDED_DST_ID(v) (((uint64_t)((v) & 0xf)) << 48)
#define IOAPIC_RTE_MASKED (1ULL << 16)
#define IOAPIC_RTE_TRIGGER_MODE(tm) (((uint64_t)(tm)) << 15)
#define IOAPIC_RTE_POLARITY(p) (((uint64_t)(p)) << 13)
#define IOAPIC_RTE_DST_MODE(dm) (((uint64_t)(dm)) << 11)
#define IOAPIC_RTE_DELIVERY_MODE(dm) ((((uint64_t)(dm)) & 0x7) << 8)
#define IOAPIC_RTE_VECTOR(x) (((uint64_t)(x)) & 0xff)
#define IOAPIC_RTE_MASK IOAPIC_RTE_VECTOR(0xff)

// Macros for reading REG_RTE entries
#define IOAPIC_RTE_REMOTE_IRR (1ULL << 14)
#define IOAPIC_RTE_DELIVERY_STATUS (1ULL << 12)
#define IOAPIC_RTE_GET_POLARITY(r)                                             \
	((enum interrupt_polarity)(((r) >> 13) & 0x1))
#define IOAPIC_RTE_GET_TRIGGER_MODE(r)                                         \
	((enum interrupt_trigger_mode)(((r) >> 15) & 0x1))
#define IOAPIC_RTE_GET_VECTOR(r) ((uint8_t)((r) & 0xFF))

// Technically this can be larger, but the spec as of the 100-Series doesn't
// guarantee where the additional redirections will be.
#define IOAPIC_NUM_REDIRECTIONS 120

struct ioapic {
	struct ioapic_descriptor desc;
	virt_addr_t virt_addr;
	uint8_t version;
	uint8_t max_redirection_entry;
	uint64_t saved_rtes[IOAPIC_NUM_REDIRECTIONS];
};

struct lock ioapic_lock = LOCK_INITIALIZER;
size_t num_ioapics = 0;
bool ioapic_enabled = false;
cvector_vector_type(struct ioapic) ioapics = NULL;
struct ioapic_isa_override isa_overrides[NUM_ISA_IRQS];

bool ioapic_is_enabled(void) {
	return ioapic_enabled;
}

struct ioapic* ioapic_resolve_global_irq(uint32_t irq) {
	for (size_t i = 0; i < num_ioapics; ++i) {
		uint32_t start = cvector_at(ioapics, i)->desc.global_irq_base;
		uint32_t end = start + cvector_at(ioapics, i)->max_redirection_entry;

		if (start <= irq && irq <= end) {
			return cvector_at(ioapics, i);
		}
	}

	return NULL;
}

struct ioapic* ioapic_resolve_global_irq_safe(uint32_t irq) {
	struct ioapic* res = ioapic_resolve_global_irq(irq);

	if (res) {
		return res;
	}

	log_fatal("Could not resolve Global IRQ %u", irq);
	return NULL;
}

uint32_t ioapic_read_reg(struct ioapic* ioapic, uint8_t reg) {
	assert(ioapic != NULL);

	mmio_out32(IOAPIC_IND(ioapic->virt_addr), reg);
	uint32_t val = mmio_in32(IOAPIC_DAT(ioapic->virt_addr));

	return val;
}

void ioapic_write_reg(struct ioapic* ioapic, uint8_t reg, uint32_t val) {
	assert(ioapic != NULL);

	mmio_out32(IOAPIC_IND(ioapic->virt_addr), reg);
	mmio_out32(IOAPIC_DAT(ioapic->virt_addr), val);
}

size_t ioapic_read_redirection_entry(struct ioapic* ioapic,
									 uint32_t global_irq) {
	assert(global_irq >= ioapic->desc.global_irq_base);

	uint32_t offset = global_irq - ioapic->desc.global_irq_base;
	assert(offset <= ioapic->max_redirection_entry);

	uint8_t reg_id = (uint8_t)IOAPIC_REG_RTE(offset);
	size_t result = ioapic_read_reg(ioapic, reg_id);
	result |= ((size_t)ioapic_read_reg(ioapic, (uint8_t)(reg_id + 1))) << 32;

	return result;
}

void ioapic_write_redirection_entry(struct ioapic* ioapic, uint32_t global_irq,
									size_t value) {
	assert(global_irq >= ioapic->desc.global_irq_base);

	uint32_t offset = global_irq - ioapic->desc.global_irq_base;
	assert(offset <= ioapic->max_redirection_entry);

	uint8_t reg_id = (uint8_t)IOAPIC_REG_RTE(offset);
	ioapic_write_reg(ioapic, reg_id, (uint32_t)value);
	ioapic_write_reg(ioapic, (uint8_t)(reg_id + 1), (uint32_t)(value >> 32));
}

bool ioapic_is_valid_irq(uint32_t global_irq) {
	return ioapic_resolve_global_irq(global_irq) != NULL;
}

void ioapic_issue_eoi(uint32_t global_irq, uint8_t vec) {
	struct ioapic* ioapic = ioapic_resolve_global_irq_safe(global_irq);

	try_lock(&ioapic_lock);

	assert(ioapic->version >= IOAPIC_EOIR_MIN_VERSION);
	mmio_out32(IOAPIC_EOIR(ioapic->virt_addr), vec);

	lock_release(&ioapic_lock);
}

void ioapic_set_irq_mask(uint32_t global_irq) {
	struct ioapic* ioapic = ioapic_resolve_global_irq_safe(global_irq);

	try_lock(&ioapic_lock);

	size_t reg = ioapic_read_redirection_entry(ioapic, global_irq);
	reg |= IOAPIC_RTE_MASKED;
	ioapic_write_redirection_entry(ioapic, global_irq, reg);

	lock_release(&ioapic_lock);
}

void ioapic_clear_irq_mask(uint32_t global_irq) {
	struct ioapic* ioapic = ioapic_resolve_global_irq_safe(global_irq);

	try_lock(&ioapic_lock);

	size_t reg = ioapic_read_redirection_entry(ioapic, global_irq);
	reg &= ~IOAPIC_RTE_MASKED;
	ioapic_write_redirection_entry(ioapic, global_irq, reg);

	lock_release(&ioapic_lock);
}

void ioapic_configure_irq(uint32_t global_irq,
						  enum interrupt_trigger_mode trig_mode,
						  enum interrupt_polarity polarity,
						  enum apic_interrupt_delivery_mode del_mode, bool mask,
						  enum apic_interrupt_dst_mode dst_mode, uint8_t dst,
						  uint8_t vector) {
	struct ioapic* ioapic = ioapic_resolve_global_irq_safe(global_irq);

	try_lock(&ioapic_lock);

	if ((del_mode == DELIVERY_MODE_FIXED ||
		 del_mode == DELIVERY_MODE_LOWEST_PRI) &&
		((vector < PLATFORM_INTERRUPT_BASE) ||
		 (vector > PLATFORM_INTERRUPT_MAX))) {
		mask = true;
	}

	size_t reg = IOAPIC_RTE_TRIGGER_MODE(trig_mode);
	reg |= IOAPIC_RTE_POLARITY(polarity);
	reg |= IOAPIC_RTE_DELIVERY_MODE(del_mode);
	reg |= IOAPIC_RTE_DST_MODE(dst_mode);
	reg |= IOAPIC_RTE_DST(dst);
	reg |= IOAPIC_RTE_VECTOR(vector);

	if (mask) {
		reg |= IOAPIC_RTE_MASKED;
	}

	ioapic_write_redirection_entry(ioapic, global_irq, reg);

	lock_release(&ioapic_lock);
}

void ioapic_fetch_irq_config(uint32_t global_irq,
							 enum interrupt_trigger_mode* trig_mode,
							 enum interrupt_polarity* polarity) {
	struct ioapic* ioapic = ioapic_resolve_global_irq_safe(global_irq);

	try_lock(&ioapic_lock);

	size_t reg = ioapic_read_redirection_entry(ioapic, global_irq);

	if (trig_mode) {
		*trig_mode = IOAPIC_RTE_GET_TRIGGER_MODE(reg);
	}

	if (polarity) {
		*polarity = IOAPIC_RTE_GET_POLARITY(reg);
	}

	lock_release(&ioapic_lock);
}

void ioapic_configure_irq_vector(uint32_t global_irq, uint8_t vector) {
	struct ioapic* ioapic = ioapic_resolve_global_irq_safe(global_irq);

	try_lock(&ioapic_lock);

	size_t reg = ioapic_read_redirection_entry(ioapic, global_irq);

	if ((IOAPIC_RTE_GET_VECTOR(reg) < PLATFORM_INTERRUPT_BASE) ||
		(IOAPIC_RTE_GET_VECTOR(reg) > PLATFORM_INTERRUPT_MAX)) {
		reg |= IOAPIC_RTE_MASKED;
	}

	reg &= ~IOAPIC_RTE_MASK;
	reg |= IOAPIC_RTE_VECTOR(vector);

	ioapic_write_redirection_entry(ioapic, global_irq, reg);

	lock_release(&ioapic_lock);
}

uint8_t ioapic_fetch_irq_vector(uint32_t global_irq) {
	struct ioapic* ioapic = ioapic_resolve_global_irq_safe(global_irq);

	try_lock(&ioapic_lock);

	size_t reg = ioapic_read_redirection_entry(ioapic, global_irq);
	uint8_t vector = IOAPIC_RTE_GET_VECTOR(reg);

	lock_release(&ioapic_lock);

	return vector;
}

void ioapic_set_mask_isa_irq(uint8_t isa_irq) {
	assert(isa_irq < NUM_ISA_IRQS);
	uint32_t global_irq = isa_irq;

	if (isa_overrides[isa_irq].remapped) {
		global_irq = isa_overrides[isa_irq].global_irq;
	}

	ioapic_set_irq_mask(global_irq);
}

void ioapic_clear_mask_isa_irq(uint8_t isa_irq) {
	assert(isa_irq < NUM_ISA_IRQS);
	uint32_t global_irq = isa_irq;

	if (isa_overrides[isa_irq].remapped) {
		global_irq = isa_overrides[isa_irq].global_irq;
	}

	ioapic_clear_irq_mask(global_irq);
}

void ioapic_configure_isa_irq(uint8_t isa_irq,
							  enum apic_interrupt_delivery_mode del_mode,
							  bool mask, enum apic_interrupt_dst_mode dst_mode,
							  uint8_t dst, uint8_t vector) {
	assert(isa_irq < NUM_ISA_IRQS);
	uint32_t global_irq = isa_irq;

	enum interrupt_trigger_mode trig_mode = IRQ_TRIGGER_MODE_EDGE;
	enum interrupt_polarity polarity = IRQ_POLARITY_ACTIVE_HIGH;

	if (isa_overrides[isa_irq].remapped) {
		global_irq = isa_overrides[isa_irq].global_irq;
		trig_mode = isa_overrides[isa_irq].trigger_mode;
		polarity = isa_overrides[isa_irq].polarity;
	}

	ioapic_configure_irq(global_irq, trig_mode, polarity, del_mode, mask,
						 dst_mode, dst, vector);
}

uint32_t ioapic_isa_to_global(uint8_t isa_irq) {
	assert(isa_irq < NUM_ISA_IRQS);

	if (isa_overrides[isa_irq].remapped) {
		return isa_overrides[isa_irq].global_irq;
	}

	return isa_irq;
}

void ioapic_save(void) {
	for (size_t i = 0; i < num_ioapics; ++i) {
		struct ioapic* ioapic = cvector_at(ioapics, i);

		for (uint8_t j = 0; j <= ioapic->max_redirection_entry; ++j) {
			uint32_t global_irq = ioapic->desc.global_irq_base + j;
			size_t reg = ioapic_read_redirection_entry(ioapic, global_irq);
			ioapic->saved_rtes[j] = reg;
		}
	}
}

void ioapic_restore(void) {
	for (size_t i = 0; i < num_ioapics; ++i) {
		struct ioapic* ioapic = cvector_at(ioapics, i);

		for (uint8_t j = 0; j <= ioapic->max_redirection_entry; ++j) {
			uint32_t global_irq = ioapic->desc.global_irq_base + j;
			ioapic_write_redirection_entry(ioapic, global_irq,
										   ioapic->saved_rtes[j]);
		}
	}
}

struct gsi_range ioapic_get_gsi_range(void) {
	assert(num_ioapics);

	struct gsi_range range = {UINT32_MAX, 0};

	struct ioapic* ioapic;

	cvector_for_each_in(ioapic, ioapics) {
		range.start = MIN(range.start, ioapic->desc.global_irq_base);
		range.end = MAX(range.end, ioapic->desc.global_irq_base +
									   ioapic->max_redirection_entry + 1);
	}

	return range;
}

void ioapic_init(struct ioapic_descriptor* ioapic_descs, size_t ioapics_num,
				 struct ioapic_isa_override* overrides, size_t num_overrides) {
	assert(!ioapics);
	cvector_init(ioapics, 1, NULL);

	num_ioapics = ioapics_num;

	for (size_t i = 0; i < num_ioapics; ++i) {
		struct ioapic descriptor = {
			.desc = ioapic_descs[i],
		};

		cvector_push_back(ioapics, descriptor);
	}

	for (size_t i = 0; i < num_ioapics; ++i) {
		struct ioapic* ioapic = cvector_at(ioapics, i);
		phys_addr_t phys_addr = ioapic->desc.phys_addr;
		virt_addr_t virt_addr = to_higher_half(phys_addr);

		ioapic->virt_addr = virt_addr;

		uint32_t ver = ioapic_read_reg(ioapic, IOAPIC_REG_VER);
		ioapic->version = IOAPIC_VER_VERSION(ver);
		ioapic->max_redirection_entry = IOAPIC_VER_MAX_REDIR_ENTRY(ver);

		if (ioapic->max_redirection_entry > IOAPIC_NUM_REDIRECTIONS - 1) {
			ioapic->max_redirection_entry = IOAPIC_NUM_REDIRECTIONS - 1;
		}

		for (uint32_t j = 0; j <= ioapic->max_redirection_entry; ++j) {
			ioapic_write_redirection_entry(
				ioapic, j + ioapic->desc.global_irq_base, IOAPIC_RTE_MASKED);
		}
	}

	for (uint32_t i = 0; i < num_overrides; ++i) {
		uint8_t isa_irq = overrides[i].isa_irq;
		assert(isa_irq < NUM_ISA_IRQS);
		isa_overrides[isa_irq] = overrides[i];
	}

	ioapic_enabled = true;
}