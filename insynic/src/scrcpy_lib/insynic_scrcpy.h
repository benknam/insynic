#ifndef INSYNIC_SCRCPY_H
#define INSYNIC_SCRCPY_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct insynic_scrcpy;

enum insynic_scrcpy_state {
    INSYNIC_SCRCPY_STATE_IDLE = 0,
    INSYNIC_SCRCPY_STATE_CONNECTING = 1,
    INSYNIC_SCRCPY_STATE_CONNECTED = 2,
    INSYNIC_SCRCPY_STATE_DISCONNECTED = 3,
    INSYNIC_SCRCPY_STATE_ERROR = 4,
};

typedef void (*insynic_scrcpy_state_cb)(enum insynic_scrcpy_state state,
                                        void *userdata);

struct insynic_scrcpy_config {
    const char *serial;
    const char *server_path;
    const char *adb_path;
    int max_size;
    int max_fps;
    uint32_t video_bit_rate;
    bool video_enabled;
    bool audio_enabled;
    bool control_enabled;
    bool turn_screen_off;
    bool otg_mode;
    int window_x;
    int window_y;
    int window_width;
    int window_height;
    void *nsview_ptr;
};

struct insynic_scrcpy *
insynic_scrcpy_create(const struct insynic_scrcpy_config *config);

struct insynic_scrcpy *
insynic_scrcpy_create_otg(const struct insynic_scrcpy_config *config);

bool
insynic_scrcpy_init_sdl(void);

void
insynic_scrcpy_destroy(struct insynic_scrcpy *s);

bool
insynic_scrcpy_start(struct insynic_scrcpy *s);

void
insynic_scrcpy_stop(struct insynic_scrcpy *s);

bool
insynic_scrcpy_process_events(struct insynic_scrcpy *s);

bool
insynic_scrcpy_run_main_thread(struct insynic_scrcpy *s);

enum insynic_scrcpy_state
insynic_scrcpy_get_state(struct insynic_scrcpy *s);

void
insynic_scrcpy_set_state_callback(struct insynic_scrcpy *s,
                                  insynic_scrcpy_state_cb cb,
                                  void *userdata);

bool
insynic_scrcpy_inject_keycode(struct insynic_scrcpy *s, uint32_t keycode,
                              bool down);

bool
insynic_scrcpy_inject_back(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_home(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_recent(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_menu(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_power(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_volume_up(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_volume_down(struct insynic_scrcpy *s);

bool
insynic_scrcpy_expand_notification_panel(struct insynic_scrcpy *s);

bool
insynic_scrcpy_expand_settings_panel(struct insynic_scrcpy *s);

bool
insynic_scrcpy_collapse_panels(struct insynic_scrcpy *s);

bool
insynic_scrcpy_rotate_device(struct insynic_scrcpy *s);

bool
insynic_scrcpy_set_display_power(struct insynic_scrcpy *s, bool on);

bool
insynic_scrcpy_toggle_display(struct insynic_scrcpy *s);

bool
insynic_scrcpy_inject_touch_action(struct insynic_scrcpy *s, int x, int y, int width, int height, bool is_down);

bool
insynic_scrcpy_inject_touch(struct insynic_scrcpy *s, int x, int y, int width, int height);

bool
insynic_scrcpy_get_device_size(struct insynic_scrcpy *s, int *width, int *height);

void *
insynic_scrcpy_get_nsview(struct insynic_scrcpy *s);

bool
insynic_scrcpy_get_window_position(struct insynic_scrcpy *s, int *x, int *y);

bool
insynic_scrcpy_get_window_size(struct insynic_scrcpy *s, int *width, int *height);

typedef void (*insynic_scrcpy_event_cb)(void *userdata);

void
insynic_scrcpy_set_event_callback(struct insynic_scrcpy *s,
                                  insynic_scrcpy_event_cb cb,
                                  void *userdata);

#ifdef __cplusplus
}
#endif

#endif
