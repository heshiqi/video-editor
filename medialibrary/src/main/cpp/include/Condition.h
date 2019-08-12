//
// Created by 何士奇 on 2019-08-12.
//

#ifndef VIDEO_EDITOR_CONDITION_H
#define VIDEO_EDITOR_CONDITION_H

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include "Errors.h"

typedef  int64_t nsecs_t;

class Condition{
public:
    enum {
        PRIVATE = 0,
        SHARED = 1
    };
    enum WakeUpType{
        WAKE_UP_ONE = 0,
        WAKE_UP_ALL = 1
    };

    Condition();
    Condition(int type);
    ~Condition();

    status_t
};

#endif //VIDEO_EDITOR_CONDITION_H
