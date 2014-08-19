#ifndef NANDROID_H
#define NANDROID_H


int nandroid_backup(const char* backup_path);
int nandroid_restore(const char* backup_path, int restore_boot, int restore_system, int restore_data, int restore_cache, int restore_sdext, int restore_wimax);
int nandroid_advanced_backup(const char* backup_path, const char *root);
int nandroid_backup_manage(const char* path);

#define NANDROID_BACKUP_FORMAT_FILE "/sdcard/dingdong/recovery/backup/.default_backup_format"
#define NANDROID_BACKUP_FORMAT_TAR 0
#define NANDROID_BACKUP_FORMAT_DUP 1
#define NANDROID_BACKUP_FORMAT_TGZ 2

#endif
