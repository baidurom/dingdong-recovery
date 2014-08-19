/*
 * Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Descriptions:
 * -------------
 * Main executable for DD Installer Binary
 *
 */
#include "../dd_inter.h"
#include "../dd.h"


struct _menuItem *g_main_menu;//main menu
struct _menuItem *g_root_menu;//language ui

static STATUS main_ui_clear(struct _menuItem *p)
{
    //release tree, post order release
    if (p == NULL)
        return RET_OK;
    main_ui_clear(p->child);
    main_ui_clear(p->nextSilbing);
    free(p);
    return RET_OK;
}

static STATUS main_ui_clear_root()
{
    return main_ui_clear(g_root_menu);
}
static struct _menuItem *tree_init()
{
    //main menu
    g_main_menu = common_ui_init();
    return_null_if_fail(g_main_menu != NULL);
    menuItem_set_name(g_main_menu, "<~mainmenu.name>");
    menuItem_set_show(g_main_menu, &common_ui_show);
    //add back operation
    g_main_menu = menuNode_init(g_main_menu);

    //inital mainmenu
    //add power
    menuNode_add(g_main_menu, power_ui_init());
    //add sd operation 
    menuNode_add(g_main_menu, sd_ui_init());
    //add wipe
    menuNode_add(g_main_menu, wipe_ui_init());
    //add mount and enable usb storage
    menuNode_add(g_main_menu, mount_ui_init());
    //add backup
    menuNode_add(g_main_menu, backup_ui_init());
    //add tools operation
    menuNode_add(g_main_menu, advanced_ui_init());

    return g_main_menu;
}
STATUS main_ui_init()
{
    dd_printf("Initializing Surface...\n");
    unlink(DD_TMP"/"INSTALL_CONF_FILE);
    remove_directory(DD_TMP);
    unlink(DD_TMP_S);
    create_directory(DD_TMP);
    symlink(DD_TMP, DD_TMP_S);

    //dd  config init
    dd_ui_init();
    //read config file and execute it
    dd_ui_config("/res/init.conf");
    //input thread start
    ui_init();
    //graphic thread start, print background
    ag_init();
    //dd_ui start
    dd_ui_start();
	//device config after dd_ui start
    dd_ui_config("/res/device.conf");
    dd_gen_verfile();
    dd_lang_init();
    dd_loadwelcome();
    tree_init();

    return RET_OK;
}
STATUS main_ui_show()
{
    struct _menuItem *node_show = g_main_menu;
    int index = 0;
    int curmain = 1;
    //show mainmenu

    while (index != MENU_QUIT)
    {
        return_val_if_fail(node_show != NULL, RET_FAIL);
        return_val_if_fail(node_show->show != NULL, RET_FAIL);
       	dd_set_isbgredraw(1);
        index = node_show->show(node_show);
        if (index > 0 && index < MENU_BACK) {
            node_show = node_show->get_child_by_index(node_show, index);
        }
        else if (index == MENU_BACK || index == 0 )
        {
            if (node_show->parent != NULL)
                node_show = node_show->parent;
        }
        else if (index == MENU_MAIN)
        {
        	node_show = g_main_menu;
        }
        else
        {
            //TODO add MENU QUIT or some operation?
            printf("invalid index %d in %s\n", index, __FUNCTION__);
        }
    }
    return RET_FAIL;
}

STATUS main_ui_release()
{

#ifndef _DD_NODEBUG
  dd_dump_malloc();
#endif
  dd_ui_end();
  ag_close_thread();
  //clear ui tree
  main_ui_clear_root(); 
  //-- Release All
  ag_closefonts();  //-- Release Fonts
  dd_debug("Font Released\n");
  ev_exit();        //-- Release Input Engine
  dd_debug("Input Released\n");
  ag_close();       //-- Release Graph Engine
  dd_debug("Graph Released\n");

  dd_debug("Cleanup Temporary\n");
  usleep(500000);
  unlink(DD_TMP_S);
  unlink(DD_TMP"/"INSTALL_CONF_FILE);
  remove_directory(DD_TMP);
  return RET_OK;
}

