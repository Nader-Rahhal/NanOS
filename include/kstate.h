#pragma once

struct kernel_state {
    char* current_working_dir;
    char* boot_time;
    char* kernel_name;
    char* kernel_version_major;
    char* kernel_version_minor;
    char* timezone;
    char* font_type;
};