#include <stdlib.h>
#include "../dd_inter.h"
#include "../dd.h"
#include "../../../dd_op.h"

#define POWER_REBOOT                 0
#define POWER_RECOVERY               1
#define POWER_BOOTLOADER             2
#define POWER_POWEROFF               3

static STATUS power_child_show(menuItem *p)
{
    //confirm
    if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
        switch(p->result) {
            case POWER_REBOOT:
            	if(acfg()->reboot_cmd != NULL)
            		ddOp_send(OP_REBOOT, 1, acfg()->reboot_cmd);
            	else
            		ddOp_send(OP_REBOOT, 1, "reboot");
                break;
            case POWER_BOOTLOADER:
            	if (acfg()->bootloader_cmd != NULL)
            		ddOp_send(OP_REBOOT, 1, acfg()->bootloader_cmd);
            	else
            		ddOp_send(OP_REBOOT, 1, "bootloader");
                break;
            case POWER_RECOVERY:
                ddOp_send(OP_REBOOT, 1, "recovery");
                break;
            case POWER_POWEROFF:
                ddOp_send(OP_REBOOT, 1, "poweroff");
                break;
            default:
                assert_if_fail(0);
            break;
        }
    }
    return MENU_BACK;
}

struct _menuItem* reboot_ui_init()
{
    struct _menuItem *temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    strncpy(temp->name, "<~reboot.null>", MENU_LEN);
    menuItem_set_icon(temp, "@reboot");
    temp->result = POWER_REBOOT;
    temp->show = &power_child_show;
    return temp;
}

struct _menuItem * power_ui_init()
{
    struct _menuItem *p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~power.name>");
    menuItem_set_title(p, "<~power.title>");
    menuItem_set_desc(p, "<~power.title.desc>");
    menuItem_set_icon(p,"@main.power");
    menuItem_set_show(p, &common_menu_show);
    p->result = 0;
    menuNode_init(p);
    //reboot
    struct _menuItem *temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    menuItem_set_title(temp, "<~reboot.null.title>");
    menuItem_set_icon(temp, "@reboot");
    menuItem_set_name(temp, "<~reboot.null>");
    menuItem_set_result(temp, POWER_REBOOT);
    menuItem_set_show(temp, &power_child_show);
    menuNode_add(p, temp);

    //reboot bootloader
    if (acfg()->enable_bootloader != 0) {
        temp = common_ui_init();
        return_null_if_fail(temp != NULL);
        menuItem_set_name(temp, "<~reboot.bootloader>");
        menuItem_set_icon(temp, "@reboot.bootloader");
        menuItem_set_result(temp, POWER_BOOTLOADER);
        menuItem_set_show(temp, &power_child_show);
        menuNode_add(p, temp);
    }

    //reboot recovery
    temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    menuItem_set_name(temp, "<~reboot.recovery>");
    menuItem_set_icon(temp, "@reboot.recovery");
    menuItem_set_result(temp, POWER_RECOVERY);
    menuItem_set_show(temp, &power_child_show);
    menuNode_add(p, temp);
    //poweroff
    temp = common_ui_init();
    return_null_if_fail(temp != NULL);
    menuItem_set_name(temp, "<~reboot.poweroff>");
    menuItem_set_icon(temp, "@reboot.power");
    menuItem_set_result(temp, POWER_POWEROFF);
    menuItem_set_show(temp, &power_child_show);
    menuNode_add(p, temp);
    return p;
}

