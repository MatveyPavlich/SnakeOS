/* API interfrace for char drivers */

/* Implementation of char devices. The following assumptions were made:
 * 1. Single core for now
 * 2. Drivers register during boot
 * 3. No unregister yet
 * 4. Names are unique
 * 5. Small, fixed number of devices
 */

struct cdev_ops {
        ssize_t (*read)(void *buf, size_t n);
        ssize_t (*write)(const void *buf, size_t n);
        int (*ioctl)(int cmd, void *arg);
};

struct cdev {
        const char *name;
        struct cdev_ops *ops;
        void *priv;
};

int cdev_register(struct cdev *dev);

/* Retrieve a cdev identified by the provided 'name' */ 
struct cdev *cdev_get(const char *name);
