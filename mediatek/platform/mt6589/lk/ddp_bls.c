#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>

#define POLLING_TIME_OUT 10000

static int gBLSMutexID = 3;

#if defined(ONE_WIRE_PULSE_COUNTING) 
#define MAX_PWM_WAVENUM 16
#define PWM_TIME_OUT 1000*100
static int g_previous_level = 0;
static int g_previous_wavenum = 0;
#endif

static unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;
    
#if defined(ONE_WIRE_PULSE_COUNTING) 
    mapped_level = (level + (MAX_PWM_WAVENUM / 2)) / MAX_PWM_WAVENUM;

    if (level != 0 && mapped_level == 0)
        mapped_level = 1;
    
    if (mapped_level > MAX_PWM_WAVENUM)
        mapped_level = MAX_PWM_WAVENUM;    
#else
    mapped_level = level;

    if (mapped_level > 0x100)
        mapped_level = 0x100;
#endif    
	return mapped_level;
}


int disp_poll_for_reg(unsigned int addr, unsigned int value, unsigned int mask, unsigned int timeout)
{
    unsigned int cnt = 0;
    
    while ((DISP_REG_GET(addr) & mask) != value)
    {
        cnt++;
        if (cnt > timeout)
        {
            return -1;
        }
    }

    return 0;
}

static int disp_bls_get_mutex()
{
#if !defined(MTK_AAL_SUPPORT)    
    if (gBLSMutexID < 0)
        return -1;

    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 1);
    if(disp_poll_for_reg(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0x2, 0x2, POLLING_TIME_OUT))
    {
        printf("[DDP] error! disp_bls_get_mutex(), get mutex timeout! \n");
        disp_dump_reg(DISP_MODULE_CONFIG);        
        return -1;
    }
#endif    
    return 0;
}

static int disp_bls_release_mutex()
{
#if !defined(MTK_AAL_SUPPORT)    
    if (gBLSMutexID < 0)
        return -1;
    
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0);
    if(disp_poll_for_reg(DISP_REG_CONFIG_MUTEX(gBLSMutexID), 0, 0x2, POLLING_TIME_OUT))
    {
        printf("[DDP] error! disp_bls_release_mutex(), release mutex timeout! \n");
        disp_dump_reg(DISP_MODULE_CONFIG);
        return -1;
    }
#endif    
    return 0;
}

void disp_bls_init(unsigned int srcWidth, unsigned int srcHeight)
{
    printf("[DDP] disp_bls_init : srcWidth = %d, srcHeight = %d\n", srcWidth, srcHeight);
    DISP_REG_SET(DISP_REG_BLS_SRC_SIZE, (srcHeight << 16) | srcWidth);
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
    DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x00050024);
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY_GAIN, 0x00000100);
    DISP_REG_SET(DISP_REG_BLS_EN, 0x80000000);                      // only enable PWM
}

int disp_bls_config(void)
{
#if !defined(MTK_AAL_SUPPORT) 
    printf("[DDP] disp_bls_config : gBLSMutexID = %d\n", gBLSMutexID);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 1);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gBLSMutexID), 0);
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gBLSMutexID), 0x200);    // BLS
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gBLSMutexID), 0);        // single mode

    if (disp_bls_get_mutex() == 0)
    {
#if defined(ONE_WIRE_PULSE_COUNTING) 
        g_previous_level = (DISP_REG_GET(DISP_REG_BLS_PWM_CON) & 0x80 > 7) * 0xFF;
        g_previous_wavenum = 0;
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, 0x00000080);
        DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x00050024 | (DISP_REG_GET(DISP_REG_BLS_PWM_CON) & 0x80));    
        DISP_REG_SET(DISP_REG_BLS_EN, 0x00000000);
#else
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, DISP_REG_GET(DISP_REG_BLS_PWM_DUTY));
        DISP_REG_SET(DISP_REG_BLS_PWM_CON, 0x00050024);
        DISP_REG_SET(DISP_REG_BLS_EN, 0x80000000);
#endif
        DISP_REG_SET(DISP_REG_BLS_PWM_DUTY_GAIN, 0x00000100);

        if (disp_bls_release_mutex() == 0)
            return 0;
    }
    return -1;
#endif
    return 0;
}


#if defined(ONE_WIRE_PULSE_COUNTING) 
int disp_bls_set_backlight(unsigned int level)
{
    int ret = 0;
    unsigned int wavenum = 0;
    unsigned int required_wavenum = 0;

    disp_bls_config();

    wavenum = 0;
    if (level > 0)
        wavenum = MAX_PWM_WAVENUM - brightness_mapping(level);

    printf("[DDP] disp_bls_set_backlight: level = %d (%d), previous level = %d (%d)\n",
        level, wavenum, g_previous_level, g_previous_wavenum);

    // [Case 1] y => 0
    //          disable PWM, idle value set to low
    // [Case 2] 0 => max
    //          disable PWM, idle value set to high
    // [Case 3] 0 => x or y => x
    //          idle value keep high
    //          disable PWM to reset wavenum
    //          re-enable PWM, set wavenum     

    if (g_previous_level != level)
    {
        DISP_REG_SET(DISP_REG_PWM_WAVE_NUM, 0x0);
        disp_bls_get_mutex();
        if (level == 0)
            DISP_REG_SET(DISP_REG_BLS_PWM_CON, DISP_REG_GET(DISP_REG_BLS_PWM_CON) & ~0x80);
        if (g_previous_level == 0)
            DISP_REG_SET(DISP_REG_BLS_PWM_CON, DISP_REG_GET(DISP_REG_BLS_PWM_CON) | 0x80);
        
        DISP_REG_SET(DISP_REG_BLS_EN, 0x0);
        disp_bls_release_mutex();

        // poll for PWM_SEND_WAVENUM to be clear
        if(disp_poll_for_reg(DISP_REG_PWM_SEND_WAVENUM, 0, 0xFFFFFFFF, POLLING_TIME_OUT))
        {
            printf("[DDP] fail to clear wavenum! PWM_SEND_WAVENUM = %d\n", DISP_REG_GET(DISP_REG_PWM_SEND_WAVENUM));
            ret = -1;
            goto Exit;
        }

        // y => x or 0 => x
        // y > x: change level from high to low,
        // x > y: change level from low to high, rounding to max
        if (g_previous_wavenum > wavenum)
            required_wavenum = (MAX_PWM_WAVENUM - g_previous_wavenum) + wavenum;
        else
            required_wavenum = wavenum - g_previous_wavenum;        

        if (required_wavenum != 0)
        {
            disp_bls_get_mutex();
            
            // re-enable PWM 
            DISP_REG_SET(DISP_REG_BLS_EN, 0x80000000);
            disp_bls_release_mutex();
            DISP_REG_SET(DISP_REG_PWM_WAVE_NUM, required_wavenum);


            // poll for wave num to be generated completely
            if(disp_poll_for_reg(DISP_REG_PWM_SEND_WAVENUM, required_wavenum, 0xFFFFFFFF, POLLING_TIME_OUT))
            {
                printf("[DDP] fail to set wavenum! PWM_SEND_WAVENUM = %d\n", DISP_REG_GET(DISP_REG_PWM_SEND_WAVENUM));
                g_previous_wavenum = DISP_REG_GET(DISP_REG_PWM_SEND_WAVENUM);
            ret = -1;
                goto Exit;
            }
            
            printf("[DDP] send wavenum = %d\n", required_wavenum); 
        }
        
        g_previous_level = level;
        g_previous_wavenum = wavenum;
    }
Exit:
    return ret;    
}
#else
int disp_bls_set_backlight(unsigned int level)
{
    printf("[DDP] disp_bls_set_backlight: %d\n", level);
    disp_bls_get_mutex();
    DISP_REG_SET(DISP_REG_BLS_PWM_DUTY, brightness_mapping(level));
    disp_bls_release_mutex();
    return 0;    
}
#endif
