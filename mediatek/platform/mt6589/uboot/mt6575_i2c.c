//#include <mt6575.h>
//#include <mt6575_typedefs.h>
//#include <mt6575_pdn_sw.h>
//#include <mt6575_i2c.h>

#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>
#include <asm/arch/mt6575_i2c.h>

/*-----------------------------------------------------------------------
 * Set I2C Speend interface:    Set internal I2C speed, 
 *                              Goal is that get sample_cnt_div and step_cnt_div
 *   clock: Depends on the current MCU/AHB/APB clock frequency   
 *   mode:  ST_MODE. (fixed setting for stable I2C transaction)
 *   khz:   MAX_ST_MODE_SPEED. (fixed setting for stable I2C transaction) 
 *
 *   Returns: ERROR_CODE
 */
U32 i2c_v1_set_speed (unsigned long clock, I2C_SPD_MODE mode, unsigned long khz)
{
    U32 ret_code = I2C_OK;
    unsigned short sample_cnt_div, step_cnt_div;
    unsigned short max_step_cnt_div = (mode == HS_MODE) ? MAX_HS_STEP_CNT_DIV : MAX_STEP_CNT_DIV;
    unsigned long tmp, sclk;
    
    {
        unsigned long diff, min_diff = I2C_CLK_RATE;
        unsigned short sample_div = MAX_SAMPLE_CNT_DIV;
        unsigned short step_div = max_step_cnt_div;
        for (sample_cnt_div = 1; sample_cnt_div <= MAX_SAMPLE_CNT_DIV; sample_cnt_div++) {
        
            for (step_cnt_div = 1; step_cnt_div <= max_step_cnt_div; step_cnt_div++) {
                sclk = (clock >> 1) / (sample_cnt_div * step_cnt_div);
                if (sclk > khz) 
                    continue;
                diff = khz - sclk;
                
                if (diff < min_diff) {
                    min_diff = diff;
                    sample_div = sample_cnt_div;
                    step_div   = step_cnt_div;
                }
            }
        }
        sample_cnt_div = sample_div;
        step_cnt_div   = step_div;
    }

    sclk = clock / (2 * sample_cnt_div * step_cnt_div);
    if (sclk > khz) {
	  ret_code = I2C_SET_SPEED_FAIL_OVER_SPEED;
        return ret_code;
    }

    step_cnt_div--;
    sample_cnt_div--;

    if (mode == HS_MODE) {
        tmp  = __raw_readw(MT_I2C_HS) & ((0x7 << 12) | (0x7 << 8));
        tmp  = (sample_cnt_div & 0x7) << 12 | (step_cnt_div & 0x7) << 8 | tmp;
        __raw_writew(tmp, MT_I2C_HS);
        I2C_SET_HS_MODE(1);
    }
    else {
        tmp  = __raw_readw(MT_I2C_TIMING) & ~((0x7 << 8) | (0x3f << 0));
        tmp  = (0/*sample_cnt_div*/ & 0x7) << 8 | (5/*step_cnt_div*/ & 0x3f) << 0 | tmp;
        __raw_writew(tmp, MT_I2C_TIMING);
        I2C_SET_HS_MODE(0);
    }
    
    printf("[i2c_set_speed] Set sclk to %d khz (orig: %d khz)\n", sclk, khz);
    printf("[i2c_set_speed] I2C Timing parameter sample_cnt_div(%d),  step_cnt_div(%d)\n", sample_cnt_div, step_cnt_div);
    
    return ret_code;
}

/*-----------------------------------------------------------------------
* Initializa the HW I2C module
*    Returns: ERROR_CODE 
*/
U32 i2c_v1_init (void) 
{
    U32 ret_code = I2C_OK;

    /* Power On I2C Duel */
    //PDN_Power_CONA_DOWN(PDN_PERI_I2C, KAL_FALSE); // wait PLL API release
    printf("\n[i2c_init] Start...................\n");

    /* Reset the HW I2C module */
    I2C_SOFTRESET;

    /* Set I2C control register */
    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT);

    /* Sset I2C speed mode */
    ret_code = i2c_v1_set_speed(I2C_CLK_RATE, ST_MODE, MAX_ST_MODE_SPEED);
    if( ret_code !=  I2C_OK)
    {
        printf("[i2c_init] i2c_v1_set_speed error (%d)\n", ret_code);
        return ret_code;
    }

    /* Clear Interrupt status */ 
    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* Double Reset the I2C START bit*/
    __raw_writel(0, MT_I2C_START);	

    printf("[i2c_init] Done\n");
    
    return ret_code;
}

/*-----------------------------------------------------------------------
* De-Initializa the HW I2C module
*    Returns: ERROR_CODE 
*/
U32 i2c_v1_deinit (void)
{
    U32 ret_code = I2C_OK;

    /* Reset the HW I2C module */
    I2C_SOFTRESET;

    printf("[i2c_deinit] Done\n");
    
    return ret_code;
}

/*-----------------------------------------------------------------------
 * Read interface: Read bytes
 *   chip:    I2C chip address, range 0..127
 *              e.g. Smart Battery chip number is 0xAA
 *   buffer:  Where to read/write the data (device address is regarded as data)
 *   len:     How many bytes to read/write
 *
 *   Returns: ERROR_CODE
 */
U32 i2c_v1_read(U8 chip, U8 *buffer, int len)
{
    U32 ret_code = I2C_OK;
    U8 *ptr = buffer;
    unsigned short status;    
    int ret = len;
    long tmo;    
    U32 timeout_ms = I2C_TIMEOUT_TH;
    //U32 timeout_ms = 4000; // 4s
    U32 start_tick=0, timeout_tick=0;
	unsigned int time_out_val=0;
    
    /* CHECKME. mt65xx doesn't support len = 0. */
    if (!len) {
        printf("[i2c_read] I2C doesn't support len = 0.\n");
        return I2C_READ_FAIL_ZERO_LENGTH;
    }

    /* for read, bit 0 is to indicate read REQ or write REQ */
    chip = (chip | 0x1);

    /* control registers */
    I2C_SET_SLAVE_ADDR(chip);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT | STOP_FLAG);

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* set timer to calculate time avoid timeout without any reaction */
    //tmo = get_timer(0);    
#if 0    
    timeout_tick = gpt4_time2tick_ms(timeout_ms);
    start_tick = gpt4_get_current_tick();
#endif	

    /* polling mode : see if transaction complete */
    while (1) 
    {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP && (!I2C_FIFO_IS_EMPTY) )
        {
            ret = 0;
            ret_code = I2C_OK; // 0
            break;
        }
        else if ( status & I2C_HS_NACKERR) 
        {
            ret = 1;
            ret_code = I2C_READ_FAIL_HS_NACKERR;
            printf("[i2c_read] transaction NACK error (%x)\n", status);
            break;
        }
        else if ( status & I2C_ACKERR) 
        {
            ret = 2;
            ret_code = I2C_READ_FAIL_ACKERR;
            printf("[i2c_read] transaction ACK error (%x)\n", status);
            break;
        }
#if 1
        //else if (get_timer(tmo) > I2C_TIMEOUT_TH /* ms */) {           
        else if (time_out_val > 100000) { 
            ret = 3;
            ret_code = I2C_READ_FAIL_TIMEOUT;
            printf("[i2c_read] transaction timeout:%d\n", time_out_val);
            break;
        }
		time_out_val++;
#endif
    }

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);
    
    if (!ret) 
    {
        while (len--) 
        {   
            I2C_READ_BYTE(*ptr);
            //dbg_print("[i2c_read] read byte = 0x%x\n", *ptr);
            ptr++;
        }
    }

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

    //dbg_print("[i2c_read] Done\n");
    
    return ret_code;
}

/*-----------------------------------------------------------------------
 * Read interface: Write bytes
 *   chip:    I2C chip address, range 0..127
 *              e.g. Smart Battery chip number is 0xAA
 *   buffer:  Where to read/write the data (device address is regarded as data)
 *   len:     How many bytes to read/write
 *
 *   Returns: ERROR_CODE
 */
U32 i2c_v1_write (U8 chip, U8 *buffer, int len)
{
    U32 ret_code = I2C_OK;
    U8 *ptr = buffer;
    int ret = len;
    long tmo;    
    unsigned short status;
    U32 timeout_ms = I2C_TIMEOUT_TH;
    U32 start_tick=0, timeout_tick=0;
	unsigned int time_out_val=0;
    
    /* CHECKME. mt65xx doesn't support len = 0. */
    if (!len)
    {   printf("[i2c_read] I2C doesn't support len = 0.\n");
        return I2C_WRITE_FAIL_ZERO_LENGTH;
    }

    /* bit 0 is to indicate read REQ or write REQ */
    chip = (chip & ~0x1);

    /* control registers */
    I2C_SET_SLAVE_ADDR(chip);
    I2C_SET_TRANS_LEN(len);
    I2C_SET_TRANSAC_LEN(1);
    I2C_SET_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);
    I2C_FIFO_CLR_ADDR;

    I2C_SET_TRANS_CTRL(ACK_ERR_DET_EN | CLK_EXT | STOP_FLAG);

    /* start to write data */
    while (len--) {
        I2C_WRITE_BYTE(*ptr);
        //dbg_print("[i2c_write] write byte = 0x%x\n", *ptr);
        ptr++;
    }

    /* start trnasfer transaction */
    I2C_START_TRANSAC;

    /* set timer to calculate time avoid timeout without any reaction */
    //tmo = get_timer(0);
#if 0    
    timeout_tick = gpt4_time2tick_ms(timeout_ms);
    start_tick = gpt4_get_current_tick();
#endif	

    /* polling mode : see if transaction complete */
    while (1) {
        status = I2C_INTR_STATUS;

        if ( status & I2C_TRANSAC_COMP) {
            ret = 0;
            ret_code = I2C_OK;			
            break;
        }
        else if ( status & I2C_HS_NACKERR) {
            ret = 1;
            ret_code = I2C_WRITE_FAIL_HS_NACKERR;			
            printf("[i2c_write] transaction NACK error\n");
            break;
        }
        else if ( status & I2C_ACKERR) {
            ret = 2;
            ret_code = I2C_WRITE_FAIL_ACKERR;			
            printf("[i2c_write] transaction ACK error\n");
            break;
        }
#if 1		
        //else if (get_timer(tmo) > I2C_TIMEOUT_TH /* ms */ ) {
        else if (time_out_val > 100000) {
            ret = 3;
            ret_code	= I2C_WRITE_FAIL_TIMEOUT;		
            printf("[i2c_write] transaction timeout:%d\n", time_out_val);
            break;
        }
		time_out_val++;
#endif	
    }

    I2C_CLR_INTR_STATUS(I2C_TRANSAC_COMP | I2C_ACKERR | I2C_HS_NACKERR);

    /* clear bit mask */
    I2C_CLR_INTR_MASK(I2C_HS_NACKERR | I2C_ACKERR | I2C_TRANSAC_COMP);

    return ret_code;
}



