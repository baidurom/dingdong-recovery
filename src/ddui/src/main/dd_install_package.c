/*
 * Copyright (C) 2011 Ahmad Amarullah ( http://amarullz.com/ )
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
 * Installer Proccess
 *
 */

#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "../dd_inter.h"
#include "../dd.h"
#include "../../../dd_op.h"

static struct _ddProgress_unit ddProgress_args = {
	.success_text = "",
	.fail_text = "",
	.path = NULL,
	.install_file = NULL,
	.wipe_cache = 0,
	.backup_restore_type = 0
};
static struct _ddProgress_unit *install_arg = &ddProgress_args;
static int is_sideload_package = 0;

static void install_package_progress(struct _ddProgress_unit *install_args){
	ddProgress_set_progress(0);

	char *wdata = dd_gettmpprop(INSTALL_CONF_FILE, "item.1.1");
	if(wdata){
		int wipe_data = atoi(wdata);
		if(wipe_data){
			ddProgress_set_text("\n--<~wipe.partition.alert>--\n");

	        ddProgress_set_text("<~wipe.partition.info>: /data");
	        ddOp_send(OP_WIPE, 1, "/data");
	        int result_wipe_data = ddOp_result_get_int();

			ddProgress_set_text("<~wipe.partition.info>: /cache");
	        ddOp_send(OP_WIPE, 1, "/cache");
	        int result_wipe_cache = ddOp_result_get_int();

	        if (result_wipe_data || result_wipe_cache)
	        	ddProgress_set_text("<~wipe.partition.fail>\n\n");
	        else
	        	ddProgress_set_text("<~wipe.partition.success>\n\n");
		}
	}

	ddOp_send(OP_INSTALL, 3, install_args->path, &install_args->wipe_cache, install_args->install_file);
	int result_install = ddOp_result_get_int();

	wdata = dd_gettmpprop(INSTALL_CONF_FILE, "item.1.2");
	if(wdata){
		int wipe_data = atoi(wdata);
		if(wipe_data && !result_install){
			ddProgress_set_text("\n--<~wipe.partition.alert>--\n");

	        ddProgress_set_text("<~wipe.partition.info>: /data");
	        ddOp_send(OP_WIPE, 1, "/data");
	        int result_wipe_data = ddOp_result_get_int();

			ddProgress_set_text("<~wipe.partition.info>: /cache");
	        ddOp_send(OP_WIPE, 1, "/cache");
	        int result_wipe_cache = ddOp_result_get_int();

	        if (result_wipe_data || result_wipe_cache)
	        	ddProgress_set_text("<~wipe.partition.fail>\n\n");
	        else
	        	ddProgress_set_text("<~wipe.partition.success>\n\n");
		}
	}

	wdata = dd_gettmpprop(INSTALL_CONF_FILE, "item.1.3");
	if(wdata){
		int remove_zip = atoi(wdata);
		if(remove_zip && !result_install){
			ddOp_send(OP_MOUNT, 1, install_args->path);
	        ddProgress_set_text("\n<~install.rm.package.tip>\n");
	        printf("Delete package: %s\n", install_args->path);
	        int rtn = unlink(install_args->path);
	        if(rtn)
	        	ddProgress_set_text("<~install.rm.package.fail>\n\n");
	        else
	        	ddProgress_set_text("<~install.rm.package.success>\n\n");
		}
	}

    if(result_install)
    	aw_post(aw_msg(16, 0, 0, 0));
    else
    	aw_post(aw_msg(15, 0, 0, 0));

    ddProgress_set_progress(1);
}

STATUS install_start(char* path, int wipe_cache, char* install_file, int echo, int autoinstall)
{
	int ret = 0;

    memset(install_arg, 0, sizeof(ddProgress_unit));
    strncpy(install_arg->success_text, "<~install.progress.success.info>", PATH_MAX);
    strncpy(install_arg->fail_text, "<~install.progress.fail.info>", PATH_MAX);
    install_arg->path = path;
    install_arg->wipe_cache = wipe_cache;
    install_arg->install_file = install_file;

    ddProgress_init(&install_package_progress, install_arg);
    if (autoinstall)
    	ret = dd_progress_animation("@list.zip", "<~sd.install.title>", "<~sd.install.title>", echo);
    else
    	ret = dd_progress("@list.zip", "<~sd.install.title>", "<~sd.install.title>", echo);
    return ret;
}
