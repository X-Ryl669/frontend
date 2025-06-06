#include "muxshare.h"
#include "muxsnapshot.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../common/init.h"
#include "../common/common.h"
#include "../common/ui_common.h"
#include "../common/input/list_nav.h"

#define UI_PANEL 5
static lv_obj_t *ui_mux_panels[UI_PANEL];

static void show_help() {
    show_help_msgbox(ui_pnlHelp, ui_lblHelpHeader, ui_lblHelpContent,
                     lang.MUXSNAPSHOT.TITLE, lang.MUXSNAPSHOT.HELP);
}

static void create_snapshot_items() {
    char snapshot_paths[3][MAX_BUFFER_SIZE];
    const char *mount_points[] = {
            device.STORAGE.ROM.MOUNT,
            device.STORAGE.SDCARD.MOUNT,
            device.STORAGE.USB.MOUNT
    };

    for (int i = 0; i < 3; ++i) {
        snprintf(snapshot_paths[i], sizeof(snapshot_paths[i]), "%s/save/snapshot", mount_points[i]);
    }

    const char *snapshot_directories[] = {snapshot_paths[0], snapshot_paths[1], snapshot_paths[2]};
    char snapshot_dir[MAX_BUFFER_SIZE];
    char **file_names = NULL;
    size_t file_count = 0;

    for (size_t dir_index = 0;
         dir_index < sizeof(snapshot_directories) / sizeof(snapshot_directories[0]); ++dir_index) {
        snprintf(snapshot_dir, sizeof(snapshot_dir), "%s/", snapshot_directories[dir_index]);

        DIR *ad = opendir(snapshot_dir);
        if (!ad) continue;

        struct dirent *af;
        while ((af = readdir(ad))) {
            if (af->d_type == DT_REG) {
                char *last_dot = strrchr(af->d_name, '.');
                if (last_dot && strcasecmp(last_dot, ".zip") == 0) {
                    char **temp = realloc(file_names, (file_count + 1) * sizeof(char *));
                    if (!temp) {
                        perror(lang.SYSTEM.FAIL_ALLOCATE_MEM);
                        free(file_names);
                        closedir(ad);
                        return;
                    }
                    file_names = temp;

                    char full_app_name[MAX_BUFFER_SIZE];
                    snprintf(full_app_name, sizeof(full_app_name), "%s%s", snapshot_dir, af->d_name);
                    file_names[file_count] = strdup(full_app_name);
                    if (!file_names[file_count]) {
                        perror(lang.SYSTEM.FAIL_DUP_STRING);
                        free(file_names);
                        closedir(ad);
                        return;
                    }
                    file_count++;
                }
            }
        }
        closedir(ad);
    }

    if (!file_names) return;
    qsort(file_names, file_count, sizeof(char *), str_compare);

    ui_group = lv_group_create();
    ui_group_glyph = lv_group_create();
    ui_group_panel = lv_group_create();

    for (size_t i = 0; i < file_count; i++) {
        if (!file_names[i]) continue;
        char *base_filename = file_names[i];

        static char snapshot_name[MAX_BUFFER_SIZE];
        snprintf(snapshot_name, sizeof(snapshot_name), "%s",
                 str_remchar(str_replace(base_filename, strip_dir(base_filename), ""), '/'));

        static char snapshot_store[MAX_BUFFER_SIZE];
        snprintf(snapshot_store, sizeof(snapshot_store), "%s", strip_ext(snapshot_name));

        ui_count++;

        add_item(&items, &item_count, base_filename, snapshot_store, "snapshot", ITEM);

        lv_obj_t *ui_pnlSnapshot = lv_obj_create(ui_pnlContent);
        apply_theme_list_panel(ui_pnlSnapshot);

        lv_obj_t *ui_lblSnapshotItem = lv_label_create(ui_pnlSnapshot);
        apply_theme_list_item(&theme, ui_lblSnapshotItem, snapshot_store);

        lv_obj_t *ui_lblSnapshotItemGlyph = lv_img_create(ui_pnlSnapshot);
        apply_theme_list_glyph(&theme, ui_lblSnapshotItemGlyph, mux_module, items[i].extra_data);

        lv_group_add_obj(ui_group, ui_lblSnapshotItem);
        lv_group_add_obj(ui_group_glyph, ui_lblSnapshotItemGlyph);
        lv_group_add_obj(ui_group_panel, ui_pnlSnapshot);

        free(base_filename);
    }

    if (ui_count > 0) lv_obj_update_layout(ui_pnlContent);

    free(file_names);
}

static void list_nav_move(int steps, int direction) {
    if (ui_count <= 0) return;
    first_open ? (first_open = 0) : play_sound(SND_NAVIGATE, 0);

    for (int step = 0; step < steps; ++step) {
        apply_text_long_dot(&theme, ui_pnlContent, lv_group_get_focused(ui_group),
                            lv_obj_get_user_data(lv_group_get_focused(ui_group_panel)));

        if (direction < 0) {
            current_item_index = (current_item_index == 0) ? ui_count - 1 : current_item_index - 1;
        } else {
            current_item_index = (current_item_index == ui_count - 1) ? 0 : current_item_index + 1;
        }

        nav_move(ui_group, direction);
        nav_move(ui_group_glyph, direction);
        nav_move(ui_group_panel, direction);
    }

    update_scroll_position(theme.MUX.ITEM.COUNT, theme.MUX.ITEM.PANEL, ui_count, current_item_index, ui_pnlContent);
    set_label_long_mode(&theme, lv_group_get_focused(ui_group),
                        lv_obj_get_user_data(lv_group_get_focused(ui_group_panel)));
    nav_moved = 1;
}

static void list_nav_prev(int steps) {
    list_nav_move(steps, -1);
}

static void list_nav_next(int steps) {
    list_nav_move(steps, +1);
}

static void handle_a() {
    if (msgbox_active) return;

    if (ui_count > 0) {
        play_sound(SND_CONFIRM, 0);

        size_t exec_count;
        const char *args[] = {(INTERNAL_PATH "script/mux/extract.sh"), items[current_item_index].name, NULL};
        const char **exec = build_term_exec(args, &exec_count);

        if (exec) {
            config.VISUAL.BLACKFADE ? fade_to_black(ui_screen) : unload_image_animation();
            run_exec(exec, exec_count, 0);
        }
        free(exec);

        write_text_to_file(MUOS_IDX_LOAD, "w", INT, current_item_index);

        load_mux("snapshot");

        close_input();
        mux_input_stop();
    }
}

static void handle_b() {
    if (msgbox_active) {
        play_sound(SND_CONFIRM, 0);
        msgbox_active = 0;
        progress_onscreen = 0;
        lv_obj_add_flag(msgbox_element, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    play_sound(SND_BACK, 0);

    close_input();
    mux_input_stop();
}

static void handle_menu() {
    if (msgbox_active) return;

    if (progress_onscreen == -1) {
        play_sound(SND_CONFIRM, 0);
        show_help();
    }
}

static void init_elements() {
    ui_mux_panels[0] = ui_pnlFooter;
    ui_mux_panels[1] = ui_pnlHeader;
    ui_mux_panels[2] = ui_pnlHelp;
    ui_mux_panels[3] = ui_pnlProgressBrightness;
    ui_mux_panels[4] = ui_pnlProgressVolume;

    adjust_panel_priority(ui_mux_panels, sizeof(ui_mux_panels) / sizeof(ui_mux_panels[0]));

    if (bar_footer) lv_obj_set_style_bg_opa(ui_pnlFooter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    if (bar_header) lv_obj_set_style_bg_opa(ui_pnlHeader, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_label_set_text(ui_lblPreviewHeader, "");
    lv_label_set_text(ui_lblPreviewHeaderGlyph, "");

    process_visual_element(CLOCK, ui_lblDatetime);
    process_visual_element(BLUETOOTH, ui_staBluetooth);
    process_visual_element(NETWORK, ui_staNetwork);
    process_visual_element(BATTERY, ui_staCapacity);

    lv_label_set_text(ui_lblMessage, "");

    lv_label_set_text(ui_lblNavA, lang.GENERIC.RESTORE);
    lv_label_set_text(ui_lblNavX, lang.GENERIC.REMOVE);
    lv_label_set_text(ui_lblNavB, lang.GENERIC.BACK);

    lv_obj_t *nav_hide[] = {
            ui_lblNavAGlyph,
            ui_lblNavA,
            ui_lblNavBGlyph,
            ui_lblNavB,
            ui_lblNavXGlyph,
            ui_lblNavX
    };

    for (int i = 0; i < sizeof(nav_hide) / sizeof(nav_hide[0]); i++) {
        lv_obj_clear_flag(nav_hide[i], LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
    }

    if (!ui_count) {
        lv_obj_add_flag(ui_lblNavA, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
        lv_obj_add_flag(ui_lblNavAGlyph, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
        lv_obj_add_flag(ui_lblNavX, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
        lv_obj_add_flag(ui_lblNavXGlyph, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
    } else {
        lv_obj_clear_flag(ui_lblNavA, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
        lv_obj_clear_flag(ui_lblNavAGlyph, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
        lv_obj_clear_flag(ui_lblNavX, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
        lv_obj_clear_flag(ui_lblNavXGlyph, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_FLOATING);
    }

#if TEST_IMAGE
    display_testing_message(ui_screen);
#endif

    kiosk_image = lv_img_create(ui_screen);
    load_kiosk_image(ui_screen, kiosk_image);

    overlay_image = lv_img_create(ui_screen);
    load_overlay_image(ui_screen, overlay_image);
}

static void ui_refresh_task() {
    if (nav_moved) {
        if (lv_group_get_obj_count(ui_group) > 0) {
            struct _lv_obj_t *element_focused = lv_group_get_focused(ui_group);
            lv_obj_set_user_data(element_focused, get_last_subdir(strip_ext(items[current_item_index].name), '/', 4));

            adjust_wallpaper_element(ui_group, 0, GENERAL);
        }
        adjust_panel_priority(ui_mux_panels, sizeof(ui_mux_panels) / sizeof(ui_mux_panels[0]));

        lv_obj_move_foreground(overlay_image);

        lv_obj_invalidate(ui_pnlContent);
        nav_moved = 0;
    }
}

int muxsnapshot_main() {
    init_module("muxsnapshot");

    init_theme(1, 1);

    init_ui_common_screen(&theme, &device, &lang, lang.MUXSNAPSHOT.TITLE);
    init_elements();

    lv_obj_set_user_data(ui_screen, mux_module);
    lv_label_set_text(ui_lblDatetime, get_datetime());

    load_wallpaper(ui_screen, NULL, ui_pnlWall, ui_imgWall, GENERAL);

    init_fonts();
    create_snapshot_items();

    int sys_index = 0;
    if (file_exist(MUOS_IDX_LOAD)) {
        sys_index = read_line_int_from(MUOS_IDX_LOAD, 1);
        remove(MUOS_IDX_LOAD);
    }

    if (ui_count > 0) {
        if (sys_index > -1 && sys_index <= ui_count && current_item_index < ui_count) list_nav_move(sys_index, +1);
    } else {
        lv_label_set_text(ui_lblScreenMessage, lang.MUXSNAPSHOT.NONE);
    }

    load_kiosk(&kiosk);

    init_timer(ui_refresh_task, NULL);

    mux_input_options input_opts = {
            .swap_axis = (theme.MISC.NAVIGATION_TYPE == 1),
            .press_handler = {
                    [MUX_INPUT_A] = handle_a,
                    [MUX_INPUT_B] = handle_b,
                    [MUX_INPUT_MENU_SHORT] = handle_menu,
                    [MUX_INPUT_DPAD_UP] = handle_list_nav_up,
                    [MUX_INPUT_DPAD_DOWN] = handle_list_nav_down,
                    [MUX_INPUT_L1] = handle_list_nav_page_up,
                    [MUX_INPUT_R1] = handle_list_nav_page_down,
            },
            .hold_handler = {
                    [MUX_INPUT_DPAD_UP] = handle_list_nav_up_hold,
                    [MUX_INPUT_DPAD_DOWN] = handle_list_nav_down_hold,
                    [MUX_INPUT_L1] = handle_list_nav_page_up,
                    [MUX_INPUT_R1] = handle_list_nav_page_down,
            }
    };
    list_nav_set_callbacks(list_nav_prev, list_nav_next);
    init_input(&input_opts, true);
    mux_input_task(&input_opts);

    return 0;
}
