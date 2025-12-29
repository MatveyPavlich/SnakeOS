#pragma once

typedef enum {
        K_OK        = 0,
        K_EINVAL    = -1,
        K_ENOSPC    = -2,
        K_EEXIST    = -3,
        K_ENODEV    = -4,
} kerrno_t;
