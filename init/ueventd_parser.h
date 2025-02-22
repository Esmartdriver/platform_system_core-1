/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _INIT_UEVENTD_PARSER_H
#define _INIT_UEVENTD_PARSER_H

#include <string>
#include <vector>

#include "devices.h"

namespace android {
namespace init {

struct UeventdConfiguration {
    std::vector<Subsystem> subsystems;
    std::vector<SysfsPermissions> sysfs_permissions;
    std::vector<Permissions> dev_permissions;
    std::vector<std::string> firmware_directories;
    std::vector<Aliases> aliases;
    bool enable_modalias_handling = false;
    size_t uevent_socket_rcvbuf_size = 0;
};

UeventdConfiguration ParseConfig(const std::vector<std::string>& configs);

}  // namespace init
}  // namespace android

#endif
