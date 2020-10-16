// Copyright 2020 Valentin Vanelslande
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "common_types.h"
#include "json/single_include/nlohmann/json.hpp"

#ifdef _WIN32
#define VVCTRE_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define VVCTRE_PLUGIN_EXPORT extern "C"
#endif

static const char* required_function_names[] = {
    "vvctre_settings_set_custom_layout_top_left",
    "vvctre_settings_set_custom_layout_top_top",
    "vvctre_settings_set_custom_layout_top_right",
    "vvctre_settings_set_custom_layout_top_bottom",
    "vvctre_settings_set_custom_layout_bottom_left",
    "vvctre_settings_set_custom_layout_bottom_top",
    "vvctre_settings_set_custom_layout_bottom_right",
    "vvctre_settings_set_custom_layout_bottom_bottom",
    "vvctre_button_device_new",
    "vvctre_button_device_get_state",
    "vvctre_settings_apply",
    "vvctre_settings_set_use_custom_layout",
    "vvctre_settings_get_layout_width",
    "vvctre_settings_get_layout_height",
    "vvctre_set_os_window_size",
};

typedef void (*vvctre_settings_set_custom_layout_top_left_t)(u16 value);
typedef void (*vvctre_settings_set_custom_layout_top_top_t)(u16 value);
typedef void (*vvctre_settings_set_custom_layout_top_right_t)(u16 value);
typedef void (*vvctre_settings_set_custom_layout_top_bottom_t)(u16 value);
typedef void (*vvctre_settings_set_custom_layout_bottom_left_t)(u16 value);
typedef void (*vvctre_settings_set_custom_layout_bottom_top_t)(u16 value);
typedef void (*vvctre_settings_set_custom_layout_bottom_right_t)(u16 value);
typedef void (*vvctre_settings_set_custom_layout_bottom_bottom_t)(u16 value);
typedef void* (*vvctre_button_device_new_t)(void* plugin_manager, const char* params);
typedef bool (*vvctre_button_device_get_state_t)(void* device);
typedef void (*vvctre_settings_apply_t)();
typedef void (*vvctre_settings_set_use_custom_layout_t)(bool value);
typedef void (*vvctre_set_os_window_size_t)(void* plugin_manager, int width, int height);
typedef u32 (*vvctre_settings_get_layout_width_t)();
typedef u32 (*vvctre_settings_get_layout_height_t)();

static vvctre_settings_set_custom_layout_top_left_t vvctre_settings_set_custom_layout_top_left;
static vvctre_settings_set_custom_layout_top_top_t vvctre_settings_set_custom_layout_top_top;
static vvctre_settings_set_custom_layout_top_right_t vvctre_settings_set_custom_layout_top_right;
static vvctre_settings_set_custom_layout_top_bottom_t vvctre_settings_set_custom_layout_top_bottom;
static vvctre_settings_set_custom_layout_bottom_left_t
    vvctre_settings_set_custom_layout_bottom_left;
static vvctre_settings_set_custom_layout_bottom_top_t vvctre_settings_set_custom_layout_bottom_top;
static vvctre_settings_set_custom_layout_bottom_right_t
    vvctre_settings_set_custom_layout_bottom_right;
static vvctre_settings_set_custom_layout_bottom_bottom_t
    vvctre_settings_set_custom_layout_bottom_bottom;
static vvctre_button_device_new_t vvctre_button_device_new;
static vvctre_button_device_get_state_t vvctre_button_device_get_state;
static vvctre_settings_apply_t vvctre_settings_apply;
static vvctre_settings_set_use_custom_layout_t vvctre_settings_set_use_custom_layout;
static vvctre_set_os_window_size_t vvctre_set_os_window_size;
static vvctre_settings_get_layout_width_t vvctre_settings_get_layout_width;
static vvctre_settings_get_layout_height_t vvctre_settings_get_layout_height;

static void* plugin_manager;
static void* button = nullptr;
static bool button_pressed = false;
static u64 current_custom_layout = 0;
static bool resize_window_to_layout_size = false;

struct CustomLayout {
    struct {
        u16 left;
        u16 top;
        u16 right;
        u16 bottom;
    } top_screen, bottom_screen;
};
std::vector<CustomLayout> custom_layouts;

VVCTRE_PLUGIN_EXPORT int GetRequiredFunctionCount() {
    return 15;
}

VVCTRE_PLUGIN_EXPORT const char** GetRequiredFunctionNames() {
    return required_function_names;
}

VVCTRE_PLUGIN_EXPORT void PluginLoaded(void* core, void* plugin_manager_,
                                       void* required_functions[]) {
    plugin_manager = plugin_manager_;
    vvctre_settings_set_custom_layout_top_left =
        (vvctre_settings_set_custom_layout_top_left_t)required_functions[0];
    vvctre_settings_set_custom_layout_top_top =
        (vvctre_settings_set_custom_layout_top_top_t)required_functions[1];
    vvctre_settings_set_custom_layout_top_right =
        (vvctre_settings_set_custom_layout_top_right_t)required_functions[2];
    vvctre_settings_set_custom_layout_top_bottom =
        (vvctre_settings_set_custom_layout_top_bottom_t)required_functions[3];
    vvctre_settings_set_custom_layout_bottom_left =
        (vvctre_settings_set_custom_layout_bottom_left_t)required_functions[4];
    vvctre_settings_set_custom_layout_bottom_top =
        (vvctre_settings_set_custom_layout_bottom_top_t)required_functions[5];
    vvctre_settings_set_custom_layout_bottom_right =
        (vvctre_settings_set_custom_layout_bottom_right_t)required_functions[6];
    vvctre_settings_set_custom_layout_bottom_bottom =
        (vvctre_settings_set_custom_layout_bottom_bottom_t)required_functions[7];
    vvctre_button_device_new = (vvctre_button_device_new_t)required_functions[8];
    vvctre_button_device_get_state = (vvctre_button_device_get_state_t)required_functions[9];
    vvctre_settings_apply = (vvctre_settings_apply_t)required_functions[10];
    vvctre_settings_set_use_custom_layout =
        (vvctre_settings_set_use_custom_layout_t)required_functions[11];
    vvctre_settings_get_layout_width = (vvctre_settings_get_layout_width_t)required_functions[12];
    vvctre_settings_get_layout_height = (vvctre_settings_get_layout_height_t)required_functions[13];
    vvctre_set_os_window_size = (vvctre_set_os_window_size_t)required_functions[14];
}

VVCTRE_PLUGIN_EXPORT void InitialSettingsOpening() {
    std::ifstream file("cycle-custom-layouts-plugin-settings.json");
    if (!file.fail()) {
        try {
            std::ostringstream oss;
            oss << file.rdbuf();

            const nlohmann::json json = nlohmann::json::parse(oss.str());

            button =
                vvctre_button_device_new(plugin_manager, json["button"].get<std::string>().c_str());

            resize_window_to_layout_size = json["resize_window_to_layout_size"].get<bool>();

            for (const nlohmann::json& layout : json["layouts"]) {
                custom_layouts.push_back(CustomLayout{
                    {
                        layout["top_screen"]["left"].get<u16>(),
                        layout["top_screen"]["top"].get<u16>(),
                        layout["top_screen"]["right"].get<u16>(),
                        layout["top_screen"]["bottom"].get<u16>(),
                    },
                    {
                        layout["bottom_screen"]["left"].get<u16>(),
                        layout["bottom_screen"]["top"].get<u16>(),
                        layout["bottom_screen"]["right"].get<u16>(),
                        layout["bottom_screen"]["bottom"].get<u16>(),
                    },
                });
            }
        } catch (nlohmann::json::exception&) {
        }
    }

    if (!custom_layouts.empty()) {
        vvctre_settings_set_use_custom_layout(true);
        vvctre_settings_set_custom_layout_top_left(custom_layouts[0].top_screen.left);
        vvctre_settings_set_custom_layout_top_top(custom_layouts[0].top_screen.top);
        vvctre_settings_set_custom_layout_top_right(custom_layouts[0].top_screen.right);
        vvctre_settings_set_custom_layout_top_bottom(custom_layouts[0].top_screen.bottom);
        vvctre_settings_set_custom_layout_bottom_left(custom_layouts[0].bottom_screen.left);
        vvctre_settings_set_custom_layout_bottom_top(custom_layouts[0].bottom_screen.top);
        vvctre_settings_set_custom_layout_bottom_right(custom_layouts[0].bottom_screen.right);
        vvctre_settings_set_custom_layout_bottom_bottom(custom_layouts[0].bottom_screen.bottom);
    }
}

VVCTRE_PLUGIN_EXPORT void BeforeDrawingFPS() {
    if (button == nullptr) {
        return;
    }

    if (!button_pressed && vvctre_button_device_get_state(button)) {
        button_pressed = true;
    } else if (button_pressed && !vvctre_button_device_get_state(button)) {
        current_custom_layout = (current_custom_layout == (custom_layouts.size() - 1))
                                    ? 0
                                    : (current_custom_layout + 1);
        vvctre_settings_set_use_custom_layout(true);
        vvctre_settings_set_custom_layout_top_left(
            custom_layouts[current_custom_layout].top_screen.left);
        vvctre_settings_set_custom_layout_top_top(
            custom_layouts[current_custom_layout].top_screen.top);
        vvctre_settings_set_custom_layout_top_right(
            custom_layouts[current_custom_layout].top_screen.right);
        vvctre_settings_set_custom_layout_top_bottom(
            custom_layouts[current_custom_layout].top_screen.bottom);
        vvctre_settings_set_custom_layout_bottom_left(
            custom_layouts[current_custom_layout].bottom_screen.left);
        vvctre_settings_set_custom_layout_bottom_top(
            custom_layouts[current_custom_layout].bottom_screen.top);
        vvctre_settings_set_custom_layout_bottom_right(
            custom_layouts[current_custom_layout].bottom_screen.right);
        vvctre_settings_set_custom_layout_bottom_bottom(
            custom_layouts[current_custom_layout].bottom_screen.bottom);
        vvctre_settings_apply();
        if (resize_window_to_layout_size) {
            vvctre_set_os_window_size(
                plugin_manager,
                static_cast<int>((custom_layouts[current_custom_layout].top_screen.right -
                                  custom_layouts[current_custom_layout].top_screen.left) +
                                 (custom_layouts[current_custom_layout].bottom_screen.right -
                                  custom_layouts[current_custom_layout].top_screen.left)),
                static_cast<int>((custom_layouts[current_custom_layout].top_screen.bottom -
                                  custom_layouts[current_custom_layout].top_screen.top) +
                                 (custom_layouts[current_custom_layout].bottom_screen.bottom -
                                  custom_layouts[current_custom_layout].top_screen.top)));
        }
        button_pressed = false;
    }
}
