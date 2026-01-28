#include "kprint.h"
#include "stdint.h"
#include "spinlock.h"
#include "panic.h"

uint64_t irq_save(void)
{
        uint64_t flags;
        __asm__ volatile (
                        "pushfq\n\t"
                        "popq %0\n\t"
                        "cli"
                        : "=r"(flags)
                        :
                        : "memory"
                        );
        return flags;
}

void irq_restore(uint64_t flags)
{
        __asm__ volatile (
                        "pushq %0\n\t"
                        "popfq"
                        :
                        : "r"(flags)
                        : "memory"
                        );
}

void spinlock_init(struct spinlock *lk)
{
        lk->locked = 0;
        lk->irq_flags = 0;
}

void spin_lock(struct spinlock *lk)
{
        uint64_t flags = irq_save();

        if (lk->locked)
                panic("spin_lock: double lock");

        lk->locked = 1;
        lk->irq_flags = flags;
}

void spin_unlock(struct spinlock *lk)
{
        if (!lk->locked)
                panic("spin_unlock: unlock without lock");

        lk->locked = 0;
        irq_restore(lk->irq_flags);
}
