#include "ui_muxoption.h"

lv_obj_t *ui_pnlSearch_option;
lv_obj_t *ui_pnlCore_option;
lv_obj_t *ui_pnlGovernor_option;
lv_obj_t *ui_pnlTag_option;

lv_obj_t *ui_lblSearch_option;
lv_obj_t *ui_lblCore_option;
lv_obj_t *ui_lblGovernor_option;
lv_obj_t *ui_lblTag_option;

lv_obj_t *ui_icoSearch_option;
lv_obj_t *ui_icoCore_option;
lv_obj_t *ui_icoGovernor_option;
lv_obj_t *ui_icoTag_option;

lv_obj_t *ui_lblCore_option_value;
lv_obj_t *ui_lblGovernor_option_value;
lv_obj_t *ui_lblTag_option_value;

void init_muxoption(lv_obj_t *ui_pnlContent) {
    ui_pnlSearch_option = lv_obj_create(ui_pnlContent);
    ui_pnlCore_option = lv_obj_create(ui_pnlContent);
    ui_pnlGovernor_option = lv_obj_create(ui_pnlContent);
    ui_pnlTag_option = lv_obj_create(ui_pnlContent);

    ui_lblSearch_option = lv_label_create(ui_pnlSearch_option);
    lv_label_set_text(ui_lblSearch_option, "");
    ui_lblCore_option = lv_label_create(ui_pnlCore_option);
    lv_label_set_text(ui_lblCore_option, "");
    ui_lblGovernor_option = lv_label_create(ui_pnlGovernor_option);
    lv_label_set_text(ui_lblGovernor_option, "");
    ui_lblTag_option = lv_label_create(ui_pnlTag_option);
    lv_label_set_text(ui_lblTag_option, "");

    ui_icoSearch_option = lv_img_create(ui_pnlSearch_option);
    ui_icoCore_option = lv_img_create(ui_pnlCore_option);
    ui_icoGovernor_option = lv_img_create(ui_pnlGovernor_option);
    ui_icoTag_option = lv_img_create(ui_pnlTag_option);

    ui_lblCore_option_value = lv_label_create(ui_pnlCore_option);
    lv_label_set_text(ui_lblCore_option_value, "");
    ui_lblGovernor_option_value = lv_label_create(ui_pnlGovernor_option);
    lv_label_set_text(ui_lblGovernor_option_value, "");
    ui_lblTag_option_value = lv_label_create(ui_pnlTag_option);
    lv_label_set_text(ui_lblTag_option_value, "");
}
