#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mt6575_irq.h>

#define GPT4_CON                    ((P_U32)(APMCU_GPTIMER_BASE+0x0040))
#define GPT4_CLK                    ((P_U32)(APMCU_GPTIMER_BASE+0x0044))
#define GPT4_DAT                    ((P_U32)(APMCU_GPTIMER_BASE+0x0048))

#define GPT4_EN                     0x0001
#define GPT4_FREERUN                0x0030
#define GPT4_SYS_CLK                0x0000


#define GPT4_MAX_TICK_CNT   ((U32)0xFFFFFFFF)

#ifndef CFG_MT6589_FPGA
// 13MHz setting
#define GPT4_MAX_US_TIMEOUT ((U32)330368884)    // 0xFFFFFFFF * 76.92ns / 1000
#define GPT4_MAX_MS_TIMEOUT ((U32)330368)       // 0xFFFFFFFF * 76.92ns / 1000000
#define GPT4_1US_TICK       ((U32)13)           //    1000 / 76.92ns = 13.000
#define GPT4_1MS_TICK       ((U32)13000)        // 1000000 / 76.92ns = 13000.520
// 13MHz: 1us = 13.000 ticks
#define TIME_TO_TICK_US(us) ((us)*GPT4_1US_TICK + ((us)*0 + (1000-1))/1000)
// 13MHz: 1ms = 13000.520 ticks
#define TIME_TO_TICK_MS(ms) ((ms)*GPT4_1MS_TICK + ((ms)*520 + (1000-1))/1000)
#else
// 12MHz setting
#define GPT4_MAX_US_TIMEOUT ((U32)357899624)    // 0xFFFFFFFF * 76.92ns / 1000
#define GPT4_MAX_MS_TIMEOUT ((U32)357899)       // 0xFFFFFFFF * 76.92ns / 1000000
#define GPT4_1US_TICK       ((U32)12)           //    1000 / 76.92ns = 13.000
#define GPT4_1MS_TICK       ((U32)12000)        // 1000000 / 76.92ns = 13000.520
// 13MHz: 1us = 13.000 ticks
#define TIME_TO_TICK_US(us) ((us)*GPT4_1US_TICK + ((us)*0 + (1000-1))/1000)
// 13MHz: 1ms = 13000.520 ticks
#define TIME_TO_TICK_MS(ms) ((ms)*GPT4_1MS_TICK + ((ms)*480 + (1000-1))/1000)
#endif

#define MS_TO_US            1000
#define CFG_HZ              100
#define MAX_REG_MS          GPT4_MAX_MS_TIMEOUT

#define GPT_SET_BITS(BS,REG)       ((*(volatile U32*)(REG)) |= (U32)(BS))
#define GPT_CLR_BITS(BS,REG)       ((*(volatile U32*)(REG)) &= ~((U32)(BS)))

static volatile U32 timestamp;
static volatile U32 lastinc;


//===========================================================================
// GPT4 fixed 13MHz counter
//===========================================================================
static void gpt_power_on (bool bPowerOn)
{
    #define AP_PERI_GLOBALCON_PDN0 (PERI_CON_BASE+0x10)
    if(!bPowerOn){
        GPT_SET_BITS(1<<13, AP_PERI_GLOBALCON_PDN0);
    }else{
    	GPT_CLR_BITS(1<<13, AP_PERI_GLOBALCON_PDN0);
    }
}

static void gpt4_start (void)
{
    *GPT4_CLK = (GPT4_SYS_CLK);
    *GPT4_CON = (GPT4_EN|GPT4_FREERUN);
}

static void gpt4_stop (void)
{
    *GPT4_CON = 0x0; // disable
    *GPT4_CON = 0x2; // clear counter
}

static void gpt4_init (bool bStart)
{
    // power on GPT
    gpt_power_on (TRUE);

    // clear GPT4 first
    gpt4_stop ();

    // enable GPT4 without lock
    if (bStart)
    {
        gpt4_start ();
    }
}

U32 gpt4_get_current_tick (void)
{
    return (*GPT4_DAT);
}

bool gpt4_timeout_tick (U32 start_tick, U32 timeout_tick)
{
    register U32 cur_tick;
    register U32 elapse_tick;

    // get current tick
    cur_tick = gpt4_get_current_tick ();

    // check elapse time
    if (start_tick <= cur_tick)
    {
        elapse_tick = cur_tick - start_tick;
    }
    else
    {
        elapse_tick = (GPT4_MAX_TICK_CNT - start_tick) + cur_tick;
    }

    // check if timeout
    if (timeout_tick <= elapse_tick)
    {
        // timeout
        return TRUE;
    }

    return FALSE;
}

//===========================================================================
// us interface
//===========================================================================
static U32 gpt4_tick2time_us (U32 tick)
{
    return ((tick + (GPT4_1US_TICK - 1)) / GPT4_1US_TICK);
}

U32 gpt4_time2tick_us (U32 time_us)
{
    if (GPT4_MAX_US_TIMEOUT <= time_us)
    {
        return GPT4_MAX_US_TIMEOUT;
    }
    else
    {
        return TIME_TO_TICK_US (time_us);
    }
}

//===========================================================================
// ms interface
//===========================================================================
static U32 gpt4_tick2time_ms (U32 tick)
{
    return ((tick + (GPT4_1MS_TICK - 1)) / GPT4_1MS_TICK);
}

static U32 gpt4_time2tick_ms (U32 time_ms)
{
    if (GPT4_MAX_MS_TIMEOUT <= time_ms)
    {
        return GPT4_MAX_MS_TIMEOUT;
    }
    else
    {
        return TIME_TO_TICK_MS (time_ms);
    }
}

//===========================================================================
// bust wait
//===========================================================================
void gpt_busy_wait_us (U32 timeout_us)
{
    U32 start_tick, timeout_tick;

    // get timeout tick
    timeout_tick = gpt4_time2tick_us (timeout_us);
    start_tick = gpt4_get_current_tick ();

    // wait for timeout
    while (!gpt4_timeout_tick (start_tick, timeout_tick));
}

void gpt_busy_wait_ms (U32 timeout_ms)
{
    U32 start_tick, timeout_tick;

    // get timeout tick
    timeout_tick = gpt4_time2tick_ms (timeout_ms);
    start_tick = gpt4_get_current_tick ();

    // wait for timeout
    while (!gpt4_timeout_tick (start_tick, timeout_tick));
}

//======================================================================


void reset_timer_masked (void)
{
    lastinc = gpt4_tick2time_ms (*GPT4_DAT);
    timestamp = 0;
}

ulong get_timer_masked (void)
{
    volatile U32 now;

    now = gpt4_tick2time_ms (*GPT4_DAT);

    if (now >= lastinc)
    {
        timestamp = timestamp + now - lastinc;        /* normal */
    }
    else
    {
        timestamp = timestamp + MAX_REG_MS - lastinc + now;   /* overflow */
    }
    lastinc = now;

    return timestamp;
}

void reset_timer (void)
{
    reset_timer_masked ();
}

#define MAX_TIMESTAMP_MS  0xffffffff

ulong get_timer (ulong base)
{
    ulong current_timestamp = 0;
    ulong temp = 0;

    current_timestamp = get_timer_masked ();

    if (current_timestamp >= base)
    {                         /* timestamp normal */
        return (current_timestamp - base);
    }
    /* timestamp overflow */
    //dbg_print("return = 0x%x\n",MAX_TIMESTAMP_MS - ( base - current_timestamp ));
    temp = base - current_timestamp;

    return (MAX_TIMESTAMP_MS - temp);
}

void set_timer (ulong ticks)
{
    timestamp = ticks;
}

/* delay msec mseconds */
void mdelay (unsigned long msec)
{
    ulong start_time = 0;

    start_time = get_timer (0);
    while (get_timer (start_time) < msec);
}

/* delay usec useconds */
void udelay (unsigned long usec)
{
    ulong tmo, tmp;

    if (usec >= 1000)
    {                         /* if "big" number, spread normalization to seconds */
        tmo = usec / 1000;    /* start to normalize for usec to ticks per sec */
        tmo *= MS_TO_US;      /* find number of "ticks" to wait to achieve target */
        tmo /= 1000;          /* finish normalize. */
    }
    else
    {                         /* else small number, don't kill it prior to HZ multiply */
        tmo = usec * MS_TO_US;
        tmo /= (1000 * 1000);
    }

    tmp = get_timer (0);        /* get current timestamp */
    if ((tmo + tmp + 1) < tmp)  /* if setting this fordward will roll time stamp */
        reset_timer_masked ();  /* reset "advancing" timestamp to 0, set lastdec value */
    else
        tmo += tmp;             /* else, set advancing stamp wake up time */

    while (get_timer_masked () < tmo)   /* loop till event */
        /*NOP*/;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
    return (unsigned long long) get_timer (0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
    ulong tbclk;
    tbclk = CFG_HZ;
    return tbclk;
}

void mtk_timer_init (void)
{
    gpt4_init (TRUE);
    // init timer system
    reset_timer ();
}

/*********************************************************************************************
 * This following is used to wake up system for battery charge in u-boot.
 * Note, the maximux "ms" value of function "gpt_one_shot_irq" is 131071
 *********************************************************************************************/
#define GPT6_CON		(APMCU_GPTIMER_BASE+0x0060)
#define GPT6_CLK		(APMCU_GPTIMER_BASE+0x0064)
#define GPT6_CMP_L		(APMCU_GPTIMER_BASE+0x006C)
#define GPT6_CMP_H		(APMCU_GPTIMER_BASE+0x007C)
#define GPT_IRQ_EN		(APMCU_GPTIMER_BASE+0x0000)
#define GPT_IRQ_ACK		(APMCU_GPTIMER_BASE+0x0008)

#define GPT6_ONE_SHOT_EN	0x0001
#define GPT6_RTC_CLK		0x0010
#define GPT6_IRQ_BIT		(1<<5)
#define GPT6_STOP_CLEAR		(1<<1)
void gpt_one_shot_irq(unsigned int ms)
{
    // Using GPT6 as trigger source

    // 1. Stop and clear GPT
    DRV_WriteReg32(GPT6_CON, GPT6_STOP_CLEAR);

    // 2. Clear pending irq
    DRV_WriteReg32(GPT_IRQ_ACK, GPT6_IRQ_BIT);

    // 3. Configure GPT divider to 1 and using 32K clock source
    DRV_WriteReg32(GPT6_CLK, GPT6_RTC_CLK);

    // 4. Calculate and Set compare value
    DRV_WriteReg32(GPT6_CMP_L, 32768*ms/1000);

    // 5. Enabel IRQ En
    DRV_WriteReg32(GPT_IRQ_EN, GPT6_IRQ_BIT);

    // 6. Start GPT one-shot
    DRV_WriteReg32(GPT6_CON, GPT6_ONE_SHOT_EN);
}

int gpt_irq_init(void)
{
    //1. Disable all gpt irq bits
    DRV_WriteReg32(GPT_IRQ_EN, 0);

    //2. Ack all gpt irq if needed
    DRV_WriteReg32(GPT_IRQ_ACK, 0x3F);

    //3. Register gpt irq for GIC
    mt6575_irq_set_sens(MT6575_GPT_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    mt6575_irq_set_polarity(MT6575_GPT_IRQ_ID, MT65xx_POLARITY_LOW);

    return 0;
}

void gpt_irq_ack()
{
    DRV_WriteReg32(GPT_IRQ_ACK, GPT6_IRQ_BIT);
}
