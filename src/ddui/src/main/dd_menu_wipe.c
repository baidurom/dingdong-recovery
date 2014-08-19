#include "../dd_inter.h"
#include "../dd.h"
#include "../../../dd_op.h"

#define WIPE_FACTORY     1
#define WIPE_DATA        2
#define WIPE_CACHE       3
#define WIPE_SDCARD    	 4
#define WIPE_INTSDCARD   5
#define WIPE_BOOT        6
#define WIPE_SYSTEM      7

STATUS wipe_item_show(menuItem *p)
{
	if (p->result == WIPE_SDCARD) {
		ddOp_send(OP_MOUNT, 1, "/sdcard");
    	int result = ddOp_result_get_int();
    	if (result) {
        	dd_alert(4, p->name, "<~sd.mount.error>", NULL, acfg()->text_ok);
        	return MENU_BACK;
    	}
	}

    if (RET_YES == dd_confirm(3, p->name, p->desc, p->icon)) {
        dd_busy_process(0);
        switch(p->result) {
            case WIPE_FACTORY:
                ddOp_send(OP_WIPE, 1, "/data");
                ddOp_send(OP_WIPE, 1, "/cache");
                break;
            case WIPE_BOOT:
                ddOp_send(OP_WIPE, 1, "/boot");
                break;
            case WIPE_SYSTEM:
                ddOp_send(OP_WIPE, 1, "/system");
                break;
            case WIPE_DATA:
                ddOp_send(OP_WIPE, 1, "/data");
                break;
            case WIPE_CACHE:
                ddOp_send(OP_WIPE, 1, "/cache");
                break;
            case WIPE_SDCARD:
               	ddOp_send(OP_WIPE, 1, "/sdcard");
                break;
            case WIPE_INTSDCARD:
                ddOp_send(OP_WIPE, 1, "/sdcard2");
                break;
            default:
                assert_if_fail(0);
                break;
        }
    }
    return MENU_BACK;

}

STATUS wipe_menu_show(menuItem *p)
{
    return_val_if_fail(p != NULL, RET_FAIL);
    int n = p->get_child_count(p);
    int select = 0;
    return_val_if_fail(n >= 1, RET_FAIL);
    return_val_if_fail(n < ITEM_COUNT, RET_FAIL);
    struct _menuItem *temp = p->child;
    return_val_if_fail(temp != NULL, RET_FAIL);
    char **menu_item = malloc(n * sizeof(char *));
    assert_if_fail(menu_item != NULL);
    int i = 0;
    for (i = 0; i < n; i++)
    {
        menu_item[i] = temp->name;
        temp = temp->nextSilbing;
    }
    select = dd_mainmenu(p->name, menu_item, NULL, NULL, n, 1);
    p->result = select;
    if (menu_item != NULL) free(menu_item);
    return p->result;
}
struct _menuItem* wipe_ui_init()
{
    struct _menuItem* p = common_ui_init();
    return_null_if_fail(p != NULL);
    menuItem_set_name(p, "<~wipe.name>");
    menuItem_set_title(p, "<~wipe.title>");
    menuItem_set_icon(p, "@main.wipe");
    menuItem_set_desc(p, "<~wipe.title.desc>");
    menuItem_set_show(p, &wipe_menu_show);
    menuNode_init(p);
    //wipe_data/factory reset
    struct _menuItem* temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~wipe.factory.name>");
    menuItem_set_result(temp, WIPE_FACTORY);
    menuItem_set_show(temp, &wipe_item_show);
    //wipe_boot
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~wipe.boot.name>");
    menuItem_set_result(temp, WIPE_BOOT);
    menuItem_set_show(temp, &wipe_item_show);
    //wipe_system
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~wipe.system.name>");
    menuItem_set_result(temp, WIPE_SYSTEM);
    menuItem_set_show(temp, &wipe_item_show);
    //wipe_data
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~wipe.data.name>");
    menuItem_set_result(temp, WIPE_DATA);
    menuItem_set_show(temp, &wipe_item_show);
    //wipe_cache
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~wipe.cache.name>");
    menuItem_set_result(temp, WIPE_CACHE);
    menuItem_set_show(temp, &wipe_item_show);
    //wipe sdcard
    temp = common_ui_init();
    menuNode_add(p, temp);
    if(acfg()->sd_ext == 0 && acfg()->sd_int == 1)
    	menuItem_set_name(temp, "<~wipe.sdint.name>");
    else
    	menuItem_set_name(temp, "<~wipe.sdcard.name>");
    menuItem_set_result(temp, WIPE_SDCARD);
    menuItem_set_show(temp, &wipe_item_show);

    if (acfg()->sd_ext == 1 && acfg()->sd_int == 1){
        //wipe internal sdcard
        temp = common_ui_init();
        menuNode_add(p, temp);
        menuItem_set_name(temp, "<~wipe.sdint.name>");
        menuItem_set_result(temp, WIPE_INTSDCARD);
        menuItem_set_show(temp, &wipe_item_show);
    }

    return p;
}
