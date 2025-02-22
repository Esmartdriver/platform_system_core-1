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

#ifndef _INIT_DEVICES_H
#define _INIT_DEVICES_H

#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <android-base/file.h>
#include <selinux/label.h>

#include "uevent.h"
#include "uevent_handler.h"

namespace android {
namespace init {

class Aliases {
    private:
    int minor_;
    int major_;
    int productId_;
    int vendorId_;
    std::string alias_to_;

    public:
    static const int ANY;

    Aliases(const std::string& to, int productId, int vendorId, int major, int minor);

    std::string AliasTo() const { return alias_to_; };

    bool Matches(int productId, int vendorId, int major, int minor) const;

    std::string ToString() const;

    friend void TestAliases(const Aliases& expected, const Aliases& test);
};

class Permissions {
  public:
    friend void TestPermissions(const Permissions& expected, const Permissions& test);

    Permissions(const std::string& name, mode_t perm, uid_t uid, gid_t gid);

    bool Match(const std::string& path) const;

    mode_t perm() const { return perm_; }
    uid_t uid() const { return uid_; }
    gid_t gid() const { return gid_; }

  protected:
    const std::string& name() const { return name_; }

  private:
    std::string name_;
    mode_t perm_;
    uid_t uid_;
    gid_t gid_;
    bool prefix_;
    bool wildcard_;
};

class SysfsPermissions : public Permissions {
  public:
    friend void TestSysfsPermissions(const SysfsPermissions& expected, const SysfsPermissions& test);

    SysfsPermissions(const std::string& name, const std::string& attribute, mode_t perm, uid_t uid,
                     gid_t gid)
        : Permissions(name, perm, uid, gid), attribute_(attribute) {}

    bool MatchWithSubsystem(const std::string& path, const std::string& subsystem) const;
    void SetPermissions(const std::string& path) const;

  private:
    const std::string attribute_;
};

class Subsystem {
  public:
    friend class SubsystemParser;
    friend void TestSubsystems(const Subsystem& expected, const Subsystem& test);

    enum DevnameSource {
        DEVNAME_UEVENT_DEVNAME,
        DEVNAME_UEVENT_DEVPATH,
    };

    Subsystem() {}
    Subsystem(const std::string& name) : name_(name) {}
    Subsystem(const std::string& name, DevnameSource source, const std::string& dir_name)
        : name_(name), devname_source_(source), dir_name_(dir_name) {}

    // Returns the full path for a uevent of a device that is a member of this subsystem,
    // according to the rules parsed from ueventd.rc
    std::string ParseDevPath(const Uevent& uevent) const {
        std::string devname = devname_source_ == DEVNAME_UEVENT_DEVNAME
                                      ? uevent.device_name
                                      : android::base::Basename(uevent.path);

        return dir_name_ + "/" + devname;
    }

    bool operator==(const std::string& string_name) const { return name_ == string_name; }

  private:
    std::string name_;
    DevnameSource devname_source_ = DEVNAME_UEVENT_DEVNAME;
    std::string dir_name_ = "/dev";
};

class DeviceHandler : public UeventHandler {
  public:
    friend class DeviceHandlerTester;

    DeviceHandler();
    DeviceHandler(std::vector<Permissions> dev_permissions,
                  std::vector<SysfsPermissions> sysfs_permissions,
                  std::vector<Subsystem> subsystems,
                  std::vector<Aliases> aliases,
                  std::set<std::string> boot_devices, bool skip_restorecon);
    virtual ~DeviceHandler() = default;

    void HandleUevent(const Uevent& uevent) override;
    void ColdbootDone() override;

    std::vector<std::string> GetBlockDeviceSymlinks(const Uevent& uevent) const;

  private:
    bool FindPlatformDevice(std::string path, std::string* platform_device_path) const;
    std::tuple<mode_t, uid_t, gid_t> GetDevicePermissions(
        const std::string& path, const std::vector<std::string>& links) const;
    void MakeDevice(const std::string& path, bool block, int major, int minor,
                    const std::vector<std::string>& links) const;
    void HandleDevice(const std::string& action, const std::string& devpath,
                      const std::string& upath, bool block, int major,
                      int minor, const std::vector<std::string>& links) const;
    void FixupSysPermissions(const std::string& upath, const std::string& subsystem) const;
    bool GetDeviceAlias(const std::string &upath, int major, int minor,
                        std::string &alias_link) const;

    std::vector<Permissions> dev_permissions_;
    std::vector<SysfsPermissions> sysfs_permissions_;
    std::vector<Subsystem> subsystems_;
    std::vector<Aliases> aliases_;
    std::set<std::string> boot_devices_;
    bool skip_restorecon_;
    std::string sysfs_mount_point_;
};

// Exposed for testing
void SanitizePartitionName(std::string* string);

}  // namespace init
}  // namespace android

#endif
