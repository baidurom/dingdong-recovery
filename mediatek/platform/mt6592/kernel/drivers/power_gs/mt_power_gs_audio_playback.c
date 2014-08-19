#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_power_gs.h>

extern unsigned int *AP_CG_gs_mp3_play;
extern unsigned int AP_CG_gs_mp3_play_len;

extern unsigned int *AP_DCM_gs_mp3_play;
extern unsigned int AP_DCM_gs_mp3_play_len;

extern unsigned int *PMIC23_LDO_BUCK_gs_mp3_play;
extern unsigned int PMIC23_LDO_BUCK_gs_mp3_play_len;

extern unsigned int *CHG33_CHG_BUCK_gs_mp3_play;
extern unsigned int CHG33_CHG_BUCK_gs_mp3_play_len;

void mt_power_gs_dump_audio_playback(void)
{
    mt_power_gs_compare("Audio Playback",                           \
                        AP_CG_gs_mp3_play, AP_CG_gs_mp3_play_len,   \
                        AP_DCM_gs_mp3_play, AP_DCM_gs_mp3_play_len, \
                        PMIC23_LDO_BUCK_gs_mp3_play, PMIC23_LDO_BUCK_gs_mp3_play_len, \
                        CHG33_CHG_BUCK_gs_mp3_play, CHG33_CHG_BUCK_gs_mp3_play_len);
}

static int dump_audio_playback_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "mt_power_gs : audio_playback\n");

    mt_power_gs_dump_audio_playback();

    len = p - buf;
    return len;
}

static void __exit mt_power_gs_audio_playback_exit(void)
{
    //return 0;
}

static int __init mt_power_gs_audio_playback_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;

    if (!mt_power_gs_dir)
    {
        printk("[%s]: mkdir /proc/mt_power_gs failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("dump_audio_playback", S_IRUGO | S_IWUSR | S_IWGRP, mt_power_gs_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = dump_audio_playback_read;
        }
    }

    return 0;
}

module_init(mt_power_gs_audio_playback_init);
module_exit(mt_power_gs_audio_playback_exit);

MODULE_DESCRIPTION("MT Power Golden Setting - Audio Playback");