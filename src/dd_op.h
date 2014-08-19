/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  * See the License for the specific language governing permissions and * limitations under the License.
 */
#ifndef _DD_OP_H
#define _DD_OP_H


typedef enum _opType{
	OP_SIGNATURE,
    OP_MOUNT,
    OP_ISMOUNT,
    OP_UNMOUNT,
    OP_REBOOT,
    OP_POWEROFF,
    OP_INSTALL,
    OP_WIPE,
    OP_TOGGLE,
    OP_FORMAT,
    OP_RESTORE,
    OP_BACKUP,
    OP_ADVANCED_BACKUP,
    OP_BACKUP_MANAGE,
    OP_SYSTEM,
    OP_COPY,
    OP_SIDELOAD
}opType;

#define OP_RESULT_LEN 16
typedef struct _opResult{
    int ret;
    char result[OP_RESULT_LEN];
}opResult, popResult;

typedef opResult * (*opFunction)(int argc, char *argv[]);
typedef struct _opData{
    opType type;
    opFunction function;
}opData, *popData;
typedef struct _ddOp{
    struct _opData  *data;
    int alloc;
    int size;
}ddOp, *pddOp;
extern struct _opResult op_result;
int ddOp_init(int size);
int ddOp_register(opType type, opFunction function);
opResult * ddOp_send(opType type, int argc, char *args, ...);
opResult*  ddOp_result_set(int ret, char *str);
char* ddOp_result_get_string();
int ddOp_result_get_int();
opResult* op_toggle(int argc, char *argv[]);
#endif
