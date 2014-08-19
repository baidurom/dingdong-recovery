#ifndef RECOVERY_EXTENDEDCOMMANDS_H
#define RECOVERY_EXTENDEDCOMMANDS_H

void create_fstab();
void process_volumes();
int usb_connected();
int get_battery_capacity();
int remove_system_app(const char *dest_app_path, const char *app_list);
void set_usb_storage_enable();
void set_usb_storage_disable();
int huawei_charge_detect();

#endif
