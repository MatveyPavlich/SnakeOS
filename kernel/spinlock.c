#include "kprint.h"
#include "stdint.h"
#include "spinlock.h"

uint64_t irq_save(void) {
        unsigned long flags;
        __asm__ volatile("pushf; pop %0; cli" : "=r"(flags) :: "memory");
        return flags;
}

void irq_restore(uint64_t flags) {
        __asm__ volatile("push %0; popf" :: "r"(flags) : "memory");
}

void spinlock_init(struct spinlock *lk)
{
        lk->locked = 0;
        lk->irq_flags = 0;
}

void spin_lock(struct spinlock *lk) {
        uint64_t flags = irq_save();
        if (lk->locked)
                kprint("Double lock No lock placed.\n");
        lk->locked = 1;
        lk->irq_flags = flags;
}

void spin_unlock(struct spinlock *lk) {
        if (!lk->locked)
                kprint("Unlock without a lock.\n");
        lk->locked = 0;
        irq_restore(lk->irq_flags);
}
