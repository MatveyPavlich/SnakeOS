#include "stddef.h"
#include "cdev.h"
#include "kerrno.h"
#define MAX_CDEV 16

static struct cdev *cdev_table[MAX_CDEVS];
static size_t cdev_count = 0;

int cdev_register(struct cdev *dev)
{
        if (!dev || !dev->name || !dev->ops)
                return K_EINVAL;

        if (cdev_count >= MAX_CDEVS)
                return K_ENOSPC;

        for (size_t i = 0; i < cdev_count; i++) {
                if (strcmp(cdev_table[i]->name, dev->name) == 0)
                        return K_EEXIST;
        }

        cdev_table[cdev_count++] = dev;
        return K_OK;
}

/* Iterator for looking up a char device. Add a new cdev_for_each() function
 * in the future to allow the caller to specify a callback.
 */
struct cdev *cdev_get(const char *name)
{
        if (!name)
                return NULL;

        for (size_t i = 0; i < cdev_count; i++) {
                if (strcmp(cdev_table[i]->name, name) == 0)
                        return cdev_table[i];
        }

        return NULL;
}

