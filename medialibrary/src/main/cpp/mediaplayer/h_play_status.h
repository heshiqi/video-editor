//
// Created by 何士奇 on 2019-08-08.
//

#ifndef VIDEO_EDITOR_H_PLAY_STATUS_H
#define VIDEO_EDITOR_H_PLAY_STATUS_H

class HPlayStatus {
public:
    bool exit;
    bool pause;
    bool load;
    bool seek;
public:
    HPlayStatus();

    ~HPlayStatus();
};

#endif //VIDEO_EDITOR_H_PLAY_STATUS_H
