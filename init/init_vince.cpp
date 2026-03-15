/*
   Copyright (c) 2016, The CyanogenMod Project
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <android-base/properties.h>
#include <sys/sysinfo.h>

#include <string>
#include <vector>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "property_service.h"
#include "vendor_init.h"

using android::base::GetProperty;
using android::init::property_set;
using std::string;

char const *heaptargetutilization;
char const *heapminfree;
char const *heapmaxfree;

std::vector<string> ro_props_default_source_order = {
    "", "odm.", "product.", "system.", "vendor.",
};

void property_override(char const prop[], char const value[], bool add = true) {
  auto pi = (prop_info *)__system_property_find(prop);

  if (pi != nullptr) {
    __system_property_update(pi, value, strlen(value));
  } else if (add) {
    __system_property_add(prop, strlen(prop), value, strlen(value));
  }
}

void check_device() {
  struct sysinfo sys;
  sysinfo(&sys);

  if (sys.totalram > 2048ull * 1024 * 1024) {
    // from phone-xhdpi-4096-dalvik-heap.mk
    heaptargetutilization = "0.6";
    heapminfree = "8m";
    heapmaxfree = "16m";
  } else {
    // from phone-xhdpi-2048-dalvik-heap.mk
    heaptargetutilization = "0.75";
    heapminfree = "512k";
    heapmaxfree = "8m";
  }
}

void set_ro_build_prop(const std::string &source, const std::string &prop,
                       const std::string &value) {
  auto prop_name = "ro." + source + "build." + prop;
  property_override(prop_name.c_str(), value.c_str(), false);
}

void set_ro_product_prop(const std::string &source, const std::string &prop,
                         const std::string &value) {
  auto prop_name = "ro.product." + source + prop;
  property_override(prop_name.c_str(), value.c_str(), false);
}

void load_device_properties() {
  string region = GetProperty("ro.boot.hwc", ""), fingerprint, description,
         model;

  // Set fingerprint and description based on region
  if (region == "CN" || region == "China") {
    fingerprint = "xiaomi/vince/vince:8.1.0/OPM1.171019.019/"
                  "V11.0.3.0.OEGCNXM:user/release-keys";
    description =
        "vince-user 8.1.0 OPM1.171019.019 V11.0.3.0.OEGCNXM release-keys";
    model = "Redmi 5 Plus";
  } else if (region == "Russia") {
    fingerprint =
        "xiaomi/vince_ru/vince:7.1.2/N2G47H/V9.6.3.0.NEGRUFD:user/release-keys";
    description = "vince-user 7.1.2 N2G47H V9.6.3.0.NEGRUFD release-keys";
    model = "Redmi 5 Plus";
  } else if (region == "India") {
    fingerprint = "xiaomi/vince/vince:8.1.0/OPM1.171019.019/"
                  "V11.0.2.0.OEGMIXM:user/release-keys";
    description =
        "vince-user 8.1.0 OPM1.171019.019 V11.0.2.0.OEGMIXM release-keys";
    model = "Redmi Note 5";
  } else {
    // Global (default)
    fingerprint = "xiaomi/vince/vince:8.1.0/OPM1.171019.019/"
                  "V11.0.2.0.OEGMIXM:user/release-keys";
    description =
        "vince-user 8.1.0 OPM1.171019.019 V11.0.2.0.OEGMIXM release-keys";
    model = "Redmi 5 Plus";
  }

  // Set all properties
  for (const auto &source : ro_props_default_source_order) {
    set_ro_build_prop(source, "fingerprint", fingerprint);
    set_ro_product_prop(source, "model", model);
  }

  property_override("ro.build.description", description.c_str());
}

void set_avoid_gfxaccel_config() {
  struct sysinfo sys;
  sysinfo(&sys);

  if (sys.totalram <= 3072ull * 1024 * 1024) {
    // Reduce memory footprint
    property_set("ro.config.avoid_gfx_accel", "true");
  }
}

void vendor_load_properties() {
  check_device();
  load_device_properties();
  set_avoid_gfxaccel_config();

  property_set("dalvik.vm.heapstartsize", "8m");
  property_set("dalvik.vm.heapgrowthlimit", "192m");
  property_set("dalvik.vm.heapsize", "512m");
  property_set("dalvik.vm.heaptargetutilization", heaptargetutilization);
  property_set("dalvik.vm.heapminfree", heapminfree);
  property_set("dalvik.vm.heapmaxfree", heapmaxfree);
}
