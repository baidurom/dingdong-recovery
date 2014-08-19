#include "../dd_inter.h"
#include "../dd.h"
#include "../../../dd_op.h"

#define MOUNT_CACHE     1
#define MOUNT_DATA      2
#define MOUNT_SYSTEM    3
#define MOUNT_SDCARD    4
#define MOUNT_TOGGLE    5
#define MOUNT_SDINT    6
#define MOUNT_DESC_MOUNT       "1"
#define MOUNT_DESC_UNMOUNT     "0"
static struct _menuItem *mount_node;
static struct _menuItem *mount_sd_node;
static struct _menuItem *mount_sd_int_node = NULL;
static struct _menuItem *mount_cache_node = NULL;
static struct _menuItem *mount_data_node = NULL;
static struct _menuItem *mount_system_node = NULL;
static STATUS mount_menu_show(menuItem *p)
{
    //traverse all mount files
    //ensure cache

    //ensure sdcard
    ddOp_send(OP_ISMOUNT, 1, "/sdcard");
    if (ddOp_result_get_int() == 1)
    {
        menuItem_set_icon(mount_sd_node, ICON_ENABLE);
        menuItem_set_desc(mount_sd_node, MOUNT_DESC_MOUNT);
    }
    else
    {
        menuItem_set_icon(mount_sd_node, ICON_DISABLE);
        menuItem_set_desc(mount_sd_node, MOUNT_DESC_UNMOUNT);
    }
    if (acfg()->sd_int == 1)
    {
        //ensure sd-int
        ddOp_send(OP_ISMOUNT, 1, "/sdcard2");
        if (ddOp_result_get_int() == 1)
        {
            menuItem_set_icon(mount_sd_int_node, ICON_ENABLE);
            menuItem_set_desc(mount_sd_int_node, MOUNT_DESC_MOUNT);
        }
        else
        {
            menuItem_set_icon(mount_sd_int_node, ICON_DISABLE);
            menuItem_set_desc(mount_sd_int_node, MOUNT_DESC_UNMOUNT);
        }
    }

    //ensure data
    ddOp_send(OP_ISMOUNT, 1, "/data");
    if (ddOp_result_get_int() == 1)
    {
        menuItem_set_icon(mount_data_node, ICON_ENABLE);
        menuItem_set_desc(mount_data_node, MOUNT_DESC_MOUNT);
    }
    else
    {
        menuItem_set_icon(mount_data_node, ICON_DISABLE);
        menuItem_set_desc(mount_data_node, MOUNT_DESC_UNMOUNT);
    }
    ddOp_send(OP_ISMOUNT, 1, "/cache");
    if (ddOp_result_get_int() == 1)
    {
        menuItem_set_icon(mount_cache_node, ICON_ENABLE);
        menuItem_set_desc(mount_cache_node, MOUNT_DESC_MOUNT);
    }
    else
    {
        menuItem_set_icon(mount_cache_node, ICON_DISABLE);
        menuItem_set_desc(mount_cache_node, MOUNT_DESC_UNMOUNT);
    }
    //ensure system
    ddOp_send(OP_ISMOUNT, 1, "/system");
    if (ddOp_result_get_int() == 1)
    {
        menuItem_set_icon(mount_system_node, ICON_ENABLE);
        menuItem_set_desc(mount_system_node, MOUNT_DESC_MOUNT);
    }
    else
    {
        menuItem_set_icon(mount_system_node, ICON_DISABLE);
        menuItem_set_desc(mount_system_node, MOUNT_DESC_UNMOUNT);
    }

    //show menu
    return_val_if_fail(p != NULL, RET_FAIL);
    int n = p->get_child_count(p);
    return_val_if_fail(n > 0, RET_FAIL);
    int selindex = 0;
    return_val_if_fail(n >= 1, RET_FAIL);
    return_val_if_fail(n < ITEM_COUNT, RET_FAIL);
    struct _menuItem *temp = p->child;
    return_val_if_fail(temp != NULL, RET_FAIL);
    char **menu_item = malloc(n * sizeof(char *));
    assert_if_fail(menu_item != NULL);
    char **icon_item=malloc(n * sizeof(char *));
    assert_if_fail(icon_item != NULL);
    char **title_item= malloc(n * sizeof(char *));
    assert_if_fail(title_item != NULL);
    int i = 0;
    for (i = 0; i < n; i++)
    {
        menu_item[i] = temp->name;
        title_item[i] = temp->title_name;
        icon_item[i] = temp->icon;
        temp = temp->nextSilbing;
    }
    selindex = dd_mainmenu(p->name, menu_item, NULL, icon_item, n, 1);
    p->result = selindex;
    if (menu_item != NULL) free(menu_item);
    if (title_item != NULL) free(title_item);
    if (icon_item != NULL) free(icon_item);
    return p->result;
}
static STATUS mount_child_show(menuItem *p)
{
    return_val_if_fail(p != NULL, MENU_BACK);
    opType op_type = (p->desc[0] == '0')?OP_MOUNT : OP_UNMOUNT;
    switch(p->result)
    {
        case MOUNT_CACHE:
            ddOp_send(op_type, 1, "/cache");
            break;
        case MOUNT_DATA:
            ddOp_send(op_type, 1,  "/data");
            break;
        case MOUNT_SYSTEM:
            ddOp_send(op_type, 1, "/system");
            break;
        case MOUNT_SDCARD:
            ddOp_send(op_type, 1, "/sdcard");
            break;
        case MOUNT_SDINT:
            ddOp_send(op_type, 1, "/sdcard2");
            break;
        case MOUNT_TOGGLE:
        {
            if (op_type == OP_MOUNT)
                //mount 
                ddOp_send(OP_TOGGLE, 1, "1");
            else 
                //untoggle
                ddOp_send(OP_TOGGLE, 1, "0");
            break;
        }
        default:
            break;
    }
    if(strstr(ddOp_result_get_string(), "mounted") != NULL)
    {
        menuItem_set_icon(p, ICON_ENABLE);
        menuItem_set_desc(p, MOUNT_DESC_MOUNT);
    }
    else if(strstr(ddOp_result_get_string(), "ok") != NULL)
    {
        menuItem_set_icon(p, ICON_DISABLE);
        menuItem_set_desc(p, MOUNT_DESC_UNMOUNT);
    }
    else 
    {
        //assert_ui_if_fail(0);
        dd_alert(3, "<~mount.alert.title>", "<~mount.alert.desc>", NULL);
    }
    return MENU_BACK;
}

struct _menuItem *mount_ui_init()
{
    struct _menuItem* p = common_ui_init();
    return_null_if_fail(p != NULL);
    strncpy(p->name, "<~mount.name>", MENU_LEN);
    menuItem_set_title(p, "<~mount.title>");
    menuItem_set_desc(p, "<~mount.title.desc>");
    menuItem_set_icon(p, "@main.mount");
    menuItem_set_show(p, &mount_menu_show);
    menuNode_init(p);
    //mount data
    struct _menuItem* temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~mount.data.name>");
    menuItem_set_result(temp, MOUNT_DATA);
    menuItem_set_icon(temp, ICON_DISABLE);
    menuItem_set_desc(temp, MOUNT_DESC_UNMOUNT);
    menuItem_set_show(temp, &mount_child_show);
    mount_data_node = temp;
    //mount cache
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~mount.cache.name>");
    menuItem_set_icon(temp, ICON_DISABLE);
    menuItem_set_result(temp, MOUNT_CACHE);
    menuItem_set_desc(temp, MOUNT_DESC_UNMOUNT);
    menuItem_set_show(temp, &mount_child_show);
    mount_cache_node = temp;
    //mount system
    temp = common_ui_init();
    menuNode_add(p, temp);
    menuItem_set_name(temp, "<~mount.system.name>");
    menuItem_set_result(temp, MOUNT_SYSTEM);
    menuItem_set_icon(temp, ICON_DISABLE);
    menuItem_set_desc(temp, MOUNT_DESC_UNMOUNT);
    menuItem_set_show(temp, &mount_child_show);
    mount_system_node = temp;
    //mount sdcard
    temp = common_ui_init();
    menuNode_add(p, temp);
    if(acfg()->sd_ext == 0 && acfg()->sd_int == 1)
    	menuItem_set_name(temp, "<~mount.sdint.name>");
    else
    	menuItem_set_name(temp, "<~mount.sdcard.name>");
    menuItem_set_result(temp, MOUNT_SDCARD);
    menuItem_set_icon(temp, ICON_DISABLE);
    menuItem_set_desc(temp, MOUNT_DESC_UNMOUNT);
    menuItem_set_show(temp, &mount_child_show);
    mount_sd_node = temp;
    if (acfg()->sd_ext == 1 && acfg()->sd_int == 1)
    {
        //mount external_sd
        temp = common_ui_init();
        menuNode_add(p, temp);
        menuItem_set_name(temp, "<~mount.sdint.name>");
        menuItem_set_result(temp, MOUNT_SDINT);
        menuItem_set_icon(temp, ICON_DISABLE);
        menuItem_set_desc(temp, MOUNT_DESC_UNMOUNT);
        menuItem_set_show(temp, &mount_child_show);
        mount_sd_int_node = temp;
    }
    //toggle usb stroage
    if (acfg()->enable_usb)
    {
        temp = common_ui_init();
        menuNode_add(p, temp);
        menuItem_set_name(temp, "<~mount.toggle.name>");
        menuItem_set_result(temp, MOUNT_TOGGLE);
        menuItem_set_icon(temp, ICON_DISABLE);
        menuItem_set_desc(temp, MOUNT_DESC_UNMOUNT);
        menuItem_set_show(temp, &mount_child_show);
    }
    mount_node = p;
    return p;

}

