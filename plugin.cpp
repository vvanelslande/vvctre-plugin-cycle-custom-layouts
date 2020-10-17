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
    "vvctre_set_os_window_size",
    "vvctre_set_os_window_position",
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
typedef void (*vvctre_set_os_window_position_t)(void* plugin_manager, int x, int y);

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
static vvctre_set_os_window_position_t vvctre_set_os_window_position;

static void* plugin_manager;
static void* button = nullptr;
static bool button_pressed = false;
static u64 current_custom_layout = -1;
static bool load_first_layout_when_vvctre_is_starting_and_emulation_is_starting_for_the_first_time = true;

struct CustomLayout {
    struct Screen {
        u16 left = 0;
        u16 top = 0;
        u16 right = 0;
        u16 bottom = 0;
    } top_screen, bottom_screen;
    struct ResizeWindow {
        bool enabled = false;
        int width = 0;
        int height = 0;
    } resize_window;
    struct MoveWindow {
        bool enabled = false;
        int x = 0;
        int y = 0;
    } move_window;
};
std::vector<CustomLayout> custom_layouts;

VVCTRE_PLUGIN_EXPORT int GetRequiredFunctionCount() {
    return 14;
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
    vvctre_set_os_window_size = (vvctre_set_os_window_size_t)required_functions[12];
    vvctre_set_os_window_position = (vvctre_set_os_window_position_t)required_functions[13];
}

VVCTRE_PLUGIN_EXPORT void InitialSettingsOpening() {
    std::ifstream file("cycle-custom-layouts-plugin-settings.json");
    if (!file.fail()) {
        std::ostringstream oss;
        oss << file.rdbuf();

        const nlohmann::json json = nlohmann::json::parse(oss.str());

        button = vvctre_button_device_new(plugin_manager, json["button"].get<std::string>().c_str());

        if (json.count("load_first_layout_when_vvctre_is_starting_and_emulation_is_starting_for_the_first_time")) {
            load_first_layout_when_vvctre_is_starting_and_emulation_is_starting_for_the_first_time = json["load_first_layout_when_vvctre_is_starting_and_emulation_is_starting_for_the_first_time"].get<bool>();
        }

        for (const nlohmann::json& json_layout : json["layouts"]) {
            CustomLayout custom_layout{};
            custom_layout.top_screen = CustomLayout::Screen{
                json_layout["top_screen"]["left"].get<u16>(),
                json_layout["top_screen"]["top"].get<u16>(),
                json_layout["top_screen"]["right"].get<u16>(),
                json_layout["top_screen"]["bottom"].get<u16>(),
            };
            custom_layout.bottom_screen = CustomLayout::Screen{
                json_layout["bottom_screen"]["left"].get<u16>(),
                json_layout["bottom_screen"]["top"].get<u16>(),
                json_layout["bottom_screen"]["right"].get<u16>(),
                json_layout["bottom_screen"]["bottom"].get<u16>(),
            };
            if (json_layout.count("resize_window")) {
                custom_layout.resize_window = CustomLayout::ResizeWindow{
                    json_layout["resize_window"]["enabled"].get<bool>(),
                    json_layout["resize_window"]["width"].get<int>(),
                    json_layout["resize_window"]["height"].get<int>(),
                };
            }
            if (json_layout.count("move_window")) {
                custom_layout.move_window = CustomLayout::MoveWindow{
                    json_layout["move_window"]["enabled"].get<bool>(),
                    json_layout["move_window"]["x"].get<int>(),
                    json_layout["move_window"]["y"].get<int>(),
                };
            }
            custom_layouts.push_back(custom_layout);
        }
    }

    if (load_first_layout_when_vvctre_is_starting_and_emulation_is_starting_for_the_first_time && !custom_layouts.empty()) {
        vvctre_settings_set_use_custom_layout(true);
        vvctre_settings_set_custom_layout_top_left(custom_layouts[0].top_screen.left);
        vvctre_settings_set_custom_layout_top_top(custom_layouts[0].top_screen.top);
        vvctre_settings_set_custom_layout_top_right(custom_layouts[0].top_screen.right);
        vvctre_settings_set_custom_layout_top_bottom(custom_layouts[0].top_screen.bottom);
        vvctre_settings_set_custom_layout_bottom_left(custom_layouts[0].bottom_screen.left);
        vvctre_settings_set_custom_layout_bottom_top(custom_layouts[0].bottom_screen.top);
        vvctre_settings_set_custom_layout_bottom_right(custom_layouts[0].bottom_screen.right);
        vvctre_settings_set_custom_layout_bottom_bottom(custom_layouts[0].bottom_screen.bottom);
        if (custom_layouts[0].resize_window.enabled) {
            vvctre_set_os_window_size(plugin_manager, custom_layouts[0].resize_window.width,
                                      custom_layouts[0].resize_window.height);
        }
        if (custom_layouts[0].move_window.enabled) {
            vvctre_set_os_window_position(plugin_manager,
                                          custom_layouts[0].move_window.x,
                                          custom_layouts[0].move_window.y);
        }
        current_custom_layout = 0;
    }
}

VVCTRE_PLUGIN_EXPORT void EmulationStarting() {
    if (load_first_layout_when_vvctre_is_starting_and_emulation_is_starting_for_the_first_time && !custom_layouts.empty()) {
        if (custom_layouts[0].resize_window.enabled) {
            vvctre_set_os_window_size(plugin_manager, custom_layouts[0].resize_window.width,
                                      custom_layouts[0].resize_window.height);
        }
        if (custom_layouts[0].move_window.enabled) {
            vvctre_set_os_window_position(plugin_manager,
                                          custom_layouts[0].move_window.x,
                                          custom_layouts[0].move_window.y);
        }
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
        if (custom_layouts[current_custom_layout].resize_window.enabled) {
            vvctre_set_os_window_size(plugin_manager,
                                      custom_layouts[current_custom_layout].resize_window.width,
                                      custom_layouts[current_custom_layout].resize_window.height);
        }
        if (custom_layouts[current_custom_layout].move_window.enabled) {
            vvctre_set_os_window_position(plugin_manager,
                                          custom_layouts[current_custom_layout].move_window.x,
                                          custom_layouts[current_custom_layout].move_window.y);
        }
        button_pressed = false;
    }
}
