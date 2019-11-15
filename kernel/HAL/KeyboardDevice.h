#pragma once

#include "device.h"

class KeyboardDevice : public Device {
public:
    KeyboardDevice(String path): Device(path) {}

    int read(size_t count, void* buf) override;
    int write(char* data, size_t count) override;
    int ioctl(u32 request, void* buf) override;
};