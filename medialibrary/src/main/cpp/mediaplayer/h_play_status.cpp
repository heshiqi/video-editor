//
// Created by 何士奇 on 2019-08-08.
//

#include "h_play_status.h"
#include "../android_log.h"

HPlayStatus::HPlayStatus() {
    exit = false;
    pause = false;
    load = true;
    seek = false;
}

HPlayStatus::~HPlayStatus() {

}