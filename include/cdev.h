/*
 * Char device (cdev) API
 *
 * This module provides a minimal character device abstraction used by
 * kernel drivers (e.g. keyboard, timer, console backends).
 *
 * NOTE: This interface is filesystem-agnostic. Devices are identified
 *       by name and accessed directly by kernel subsystems. A filesystem
 *       namespace (e.g. /dev) may be layered on top later without
 *       modifying drivers.
 *
 * Design assumptions:
 *  1. Single-core system (no locking required yet)
 *  2. Drivers register devices during boot
 *  3. Devices are not unregistered
 *  4. Device names are unique
 *  5. Small, fixed number of character devices
 */

#include "stddef.h"

/* struct cdev_ops - Character device operations. Drivers may leave unsupported
 *                   operations as NULL.
 * @read:            Read up to @n bytes into @buf. Returns n. of bytes read.
 * @write:           Write up to @n bytes from @buf. Returns n. of bytes written.
 * @ioctl:           Optional control interface for device-specific commands.
 */
struct cdev_ops {
        size_t (*read)(void *buf, size_t n);
        size_t (*write)(const void *buf, size_t n);
        int (*ioctl)(int cmd, void *arg);
};

/* struct cdev - Character device descriptor. Represents a registered character
 *               device. This structure is owned by the driver and must remain
 *               valid for the lifetime of the device.
 * @name:        Unique device name (identifier).
 * @ops:         Operations table implementing device behaviour.
 * @priv:        Driver-private data pointer (device state, buffers, etc.).
 */
struct cdev {
        const char *name;
        struct cdev_ops *ops;
        void *priv;
};

/* cdev_register - register a character device with the global cdev registry.
 *                 Must be called during early boot before devices are accessed.
 *                 Returns 0 on success or 1 on failure.
 */
int cdev_register(struct cdev *dev);

/* cdev_get - Lookup a registered character device by name. Returns a pointer
 *            to the registered cdev, or NULL if no such device exists.
 * @name:     Unique device name (identifier).
 */
struct cdev *cdev_get(const char *name);
