#ifndef _MT6577_BOOT_MODE_H_
#define _MT6577_BOOT_MODE_H_

/* MT6577 boot type definitions */
typedef enum 
{
    NORMAL_BOOT = 0,
    META_BOOT = 1,
    RECOVERY_BOOT = 2,    
    SW_REBOOT = 3,
    FACTORY_BOOT = 4,
    ADVMETA_BOOT = 5,
    ATE_FACTORY_BOOT = 6,
    ALARM_BOOT = 7,
    UNKNOWN_BOOT
} BOOTMODE;

typedef enum {
    BR_POWER_KEY = 0,
    BR_USB,
    BR_RTC,
    BR_WDT,
    BR_WDT_BY_PASS_PWK,
    BR_TOOL_BY_PASS_PWK,
    BR_UNKNOWN
} boot_reason_t;

/*META COM port type*/
 typedef enum
{
    META_UNKNOWN_COM = 0,
    META_UART_COM,
    META_USB_COM
} META_COM_TYPE;

typedef struct {
  unsigned int maggic_number;
  BOOTMODE boot_mode;
  unsigned int  e_flag;
  unsigned int  log_port;
  unsigned int  log_baudrate;
  unsigned char log_enable;
  unsigned char reserved[3];
  unsigned int dram_rank_num;
  unsigned int dram_rank_size[4];
  unsigned int boot_reason;
  META_COM_TYPE meta_com_type;
  unsigned int meta_com_id;
} BOOT_ARGUMENT;

/* chip SW version */
typedef enum {
    CHIP_SW_VER_01 = 0x0000,
    CHIP_SW_VER_02 = 0x0001
} CHIP_SW_VER;

#define BOOT_ARGUMENT_MAGIC 0x504c504c

extern unsigned int BOOT_ARGUMENT_LOCATION;
extern void boot_mode_select(void);
extern BOOTMODE g_boot_mode;
extern CHIP_SW_VER  mt_get_chip_sw_ver(void);

#endif
