struct spinlock {
        int locked;
        unsigned long irq_flags;
};

void spinlock_init(struct spinlock *lk);
void spin_lock(struct spinlock *lk);
void spin_unlock(struct spinlock *lk);
