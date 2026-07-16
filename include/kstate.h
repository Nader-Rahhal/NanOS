#pragma once

struct kernel_state {
    const char* current_working_dir;
    const char* boot_time;
    const char* kernel_name;
    const char* kernel_version_major;
    const char* kernel_version_minor;
    const char* timezone;
    const char* font_type;
};