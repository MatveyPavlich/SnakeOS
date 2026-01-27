#include "stdint.h"

struct spinlock {
        int locked;
        uint64_t irq_flags;
};

void spinlock_init(struct spinlock *lk);
void spin_lock(struct spinlock *lk);
void spin_unlock(struct spinlock *lk);
