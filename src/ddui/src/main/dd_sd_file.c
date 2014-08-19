/*
 * Copyright (C)
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
 *Author:dennise 
 *Date:2012-10-12
 *Descriptions;
 *    scan file system and show in screen, invoke callback function when touch file
 *  
 *
 *
 */
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "../dd_inter.h"
#include "../dd.h"

//invoke fileFun after touch the file.
//if return value from callback funciton is not equal 0, continue, others , back to up layer
//just support single thread
static char *g_title_name = NULL;
//callback function 
static fileFun g_fun = NULL;
static fileFilterFun g_file_filter_fun= NULL;
static void *g_data = NULL;
static int tomain = 0;
pthread_mutex_t g_file_scan_mutex = PTHREAD_MUTEX_INITIALIZER;

static void sd_file_dump_array(char **zips, char **zips_desc, int z_size)
{
    dd_debug("*************%s start******************\n", __FUNCTION__);
    int i = 0;
    for (i = 0; i < z_size; i++)
    {
        dd_debug("[%d]%s--%s\n", i, zips[i], zips_desc[i]);
    }
    dd_debug("*************%s end********************\n", __FUNCTION__);
}

static STATUS _file_scan(char *path, int path_len)
{
    return_val_if_fail(path != NULL, RET_FAIL);
    return_val_if_fail(strlen(path) <= path_len, RET_INVALID_ARG);
    DIR* d = NULL;
    struct dirent* de = NULL;
    int i = 0;
    int result = 0;
    d = opendir(path);
    return_val_if_fail(d != NULL, RET_FAIL);

    int d_size = 0;
    int d_alloc = 10;
    char** dirs = (char **)malloc(d_alloc * sizeof(char*));
    char** dirs_desc = (char **)malloc(d_alloc * sizeof(char*));
    return_val_if_fail(dirs != NULL, RET_FAIL);
    return_val_if_fail(dirs_desc != NULL, RET_FAIL);
    int z_size = 0;
    int z_alloc = 10;
    char** zips = (char **)malloc(z_alloc * sizeof(char*));
    char** zips_desc=(char **)malloc(z_alloc * sizeof(char*));
    return_val_if_fail(zips != NULL, RET_FAIL);
    return_val_if_fail(zips_desc != NULL, RET_FAIL);
    //zips[0] = strdup("../");
    //zips_desc[0]=strdup("../");

    while ((de = readdir(d)) != NULL) {
        int name_len = strlen(de->d_name);
        if (name_len <= 0) continue;
        char de_path[PATH_MAX];
        snprintf(de_path, PATH_MAX, "%s/%s", path, de->d_name);
        struct stat st ;
        assert_if_fail(stat(de_path, &st) == 0);
        if (de->d_type == DT_DIR) {
            //skip "." and ".." entries
            if (name_len == 1 && de->d_name[0] == '.') continue;
            if (name_len == 2 && de->d_name[0] == '.' && 
                    de->d_name[1] == '.') continue;
            if (d_size >= d_alloc) {
                d_alloc *= 2;
                dirs = (char **)realloc(dirs, d_alloc * sizeof(char*));
                assert_if_fail(dirs != NULL);
                dirs_desc = (char **)realloc(dirs_desc, d_alloc * sizeof(char*));
                assert_if_fail(dirs_desc != NULL);
            }
            dirs[d_size] = (char *)malloc(name_len + 2);
            assert_if_fail(dirs[d_size] != NULL);
            dirs_desc[d_size] = (char *)malloc(64);
            assert_if_fail(dirs_desc[d_size] != NULL);
            strncpy(dirs[d_size], de->d_name, name_len);
            dirs[d_size][name_len] = '/';
            dirs[d_size][name_len + 1] = '\0';
            time_t GMT_8 = st.st_mtime + 8*3600;
            snprintf(dirs_desc[d_size], 63, "%s" ,ctime(&GMT_8));
            dirs_desc[d_size][63] = '\0';
            ++d_size;
        } else if (de->d_type == DT_REG) {
            if (g_file_filter_fun == NULL || g_file_filter_fun(de->d_name, name_len) == 0)
            {
                if (z_size >= z_alloc) {
                    z_alloc *= 2;
                    zips = (char **) realloc(zips, z_alloc * sizeof(char*));
                    assert_if_fail(zips != NULL);
                    zips_desc = (char **) realloc(zips_desc, z_alloc * sizeof(char*));
                    assert_if_fail(zips_desc != NULL);
                }
                zips[z_size] = strdup(de->d_name);
                assert_if_fail(zips[z_size] != NULL);
                zips_desc[z_size] = (char*)malloc(64);
                assert_if_fail(zips_desc[z_size] != NULL);
                time_t GMT_8 = st.st_mtime + 8*3600;
                snprintf(zips_desc[z_size], 63, "%s%lldMB" ,ctime(&GMT_8), st.st_size/(uint64_t)(1024*1024));
                zips_desc[z_size][63] = '\0';
                z_size++;
            }
        }
    }
    closedir(d);

    // sort the result
    if (z_size > 1) {
        for (i = 0; i < z_size; i++) {
            int curMax = -1;
            int j;
            for (j = 0; j < z_size - i; j++) {
                if (curMax == -1 || strcasecmp(zips[curMax], zips[j]) < 0)
                    curMax = j;
            }
            char* temp = zips[curMax];
            char* temp_desc = zips_desc[curMax];
            zips[curMax] = zips[z_size - i - 1];
            zips[z_size - i - 1] = temp;
            zips_desc[curMax] = zips_desc[z_size - i - 1];
            zips_desc[z_size - i - 1] = temp_desc;
        }
    }

    if (d_size) {
        for (i = 0; i < d_size; i++) {
            int curMax = -1;
            int j;
            for (j = 0; j < d_size - i; j++) {
                if (curMax == -1 || strcasecmp(dirs[curMax], dirs[j]) < 0)
                    curMax = j;
            }
            char* temp = dirs[curMax];
            char* temp_desc = dirs_desc[curMax];
            dirs[curMax] = dirs[d_size - i - 1];
            dirs[d_size - i - 1] = temp;
            dirs_desc[curMax] = dirs_desc[d_size - i - 1];
            dirs_desc[d_size - i - 1] = temp_desc;
        }
    }

    // append dirs to the zips list
    if (d_size + z_size + 1 > z_alloc) {
        z_alloc = d_size + z_size + 1;
        zips = (char **)realloc(zips, z_alloc * sizeof(char*));
        assert_if_fail(zips != NULL);
        zips_desc = (char **)realloc(zips_desc, z_alloc * sizeof(char*));
        assert_if_fail(zips_desc != NULL);
    }
    for (i = 0; i < d_size; i++)
    {
        zips[z_size + i] = dirs[i];
        zips_desc[z_size + i] = dirs_desc[i];
    }
    free(dirs);
    z_size += d_size;
    zips[z_size] = NULL;
    zips_desc[z_size] = NULL;

    int chosen_item = -1;
    do {
 	   //if want to up to main menu, just return
 	   if(tomain) {
 		   result = MENU_MAIN;
 		   break;
 	   }

        if (NULL == g_title_name)
        {
            dd_error("g_title_name is NULL \n");
            result = -1;
            goto finish_done;
        }
 #ifdef DEBUG
        sd_file_dump_array(zips, zips_desc, z_size);
 #endif
        //printf("z_size: %d\n", z_size);
        chosen_item = dd_sdmenu(path, zips, zips_desc, z_size);
        //printf("chosen_item: %d\n", chosen_item);
        if (chosen_item == MENU_MAIN) {
     	   //go to main menu
     	   tomain = 1;
     	   result = MENU_MAIN;
     	   break;
        }
        if (chosen_item == -1) {
            //go up but continue browsing
     	   result = -1;
           break;
        }

        char * item = zips[chosen_item];
        if (!item){
            //go up but continue browsing
     	   result = -1;
           break;
        }
        int item_len = strlen(item);

        if (item[item_len - 1] == '/') {
            char new_path[PATH_MAX];
            strlcpy(new_path, path, PATH_MAX);
            strlcat(new_path, "/", PATH_MAX);
            strlcat(new_path, item, PATH_MAX);
            new_path[strlen(new_path) - 1] = '\0';
            result = _file_scan(new_path, PATH_MAX);
            if (result > 0 && result < MENU_MAIN-1) break;
        } else {
            // select a zipfile
            // the status to the caller
            char new_path[PATH_MAX];
            strlcpy(new_path, path, PATH_MAX);
            strlcat(new_path, "/", PATH_MAX);
            strlcat(new_path, item, PATH_MAX);
            int wipe_cache = 0;
            //if third parameter is 1, echo sucess dialog
            if (NULL == g_fun)
            {
                dd_error("g_fun is NULL in fun\n");
                result = -1;
                goto finish_done;
            }
            if (0 == g_fun(new_path, PATH_MAX, (void *)g_data))//execute callback fun success
            {
                //back to up layer
                result = -1;
            }
            else {
                //back to display list
                result = -2;
            }
            break;
        }
    } while(1);

finish_done:

    for (i = 0; i < z_size; ++i)
    {
        free(zips[i]);
        free(zips_desc[i]);
    }
    free(zips);
    return result;
}

//add data obejcet, optional argument in fileFun
STATUS file_scan(char *path, int path_len, char * title, int title_len, fileFun fun, void* data, fileFilterFun filter_fun)
{
    return_val_if_fail(path != NULL, RET_FAIL);
    return_val_if_fail(strlen(path) <= path_len, RET_INVALID_ARG);
    return_val_if_fail(title != NULL, RET_FAIL);
    return_val_if_fail(strlen(title) <= title_len, RET_INVALID_ARG);
    pthread_mutex_lock(&g_file_scan_mutex);
    g_title_name = title;
    g_fun = fun;
    g_data = data;
    g_file_filter_fun = filter_fun;
    pthread_mutex_unlock(&g_file_scan_mutex);
    tomain = 0;
    return _file_scan(path, path_len);
}
