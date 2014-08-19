#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>

#include "tpd_custom_msg2133.h"

#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include "cust_gpio_usage.h"

/*Open OR Close Debug Info*/
#define __TPD_DEBUG__ 

/*Ctp Power Off In Sleep ? */
//#define TPD_CLOSE_POWER_IN_SLEEP

 
extern struct tpd_device *tpd;

/*Use For Get CTP Data By I2C*/ 
struct i2c_client *i2c_client = NULL;

/*Use For Firmware Update By I2C*/
static struct i2c_client     *msg21xx_i2c_client = NULL;

//struct task_struct *thread = NULL;
 
static DECLARE_WAIT_QUEUE_HEAD(waiter);
//static DEFINE_MUTEX(i2c_access);

typedef struct
{
    u16 X;
    u16 Y;
} TouchPoint_t;

/*CTP Data Package*/
typedef struct
{
    u8 nTouchKeyMode;
    u8 nTouchKeyCode;
    u8 nFingerNum;
    TouchPoint_t Point[MAX_TOUCH_FINGER];
} TouchScreenInfo_t;

 
static void tpd_eint_interrupt_handler(void);
static struct work_struct    msg21xx_wq;

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif

 extern void mt65xx_eint_unmask(unsigned int line);
 extern void mt65xx_eint_mask(unsigned int line);
 extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
 extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
 extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
									  kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
									  kal_bool auto_umask);

#ifdef MT6577
	extern void mt65xx_eint_unmask(unsigned int line);
	extern void mt65xx_eint_mask(unsigned int line);
	extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
	extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
	extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif

 
static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);
 

static int tpd_flag = 0;
static int tpd_halt=0;
static int point_num = 0;
static int p_point_num = 0;



#define TPD_OK 0

 
 static const struct i2c_device_id msg2133_tpd_id[] = {{"msg2133",0},{}};

 static struct i2c_board_info __initdata msg2133_i2c_tpd={ I2C_BOARD_INFO("msg2133", (0x26))};
 
 
 static struct i2c_driver tpd_i2c_driver = {
  .driver = {
	 .name = "msg2133",//.name = TPD_DEVICE,
//	 .owner = THIS_MODULE,
  },
  .probe = tpd_probe,
  .remove = __devexit_p(tpd_remove),
  .id_table = msg2133_tpd_id,
  .detect = tpd_detect,
//  .address_data = &addr_data,
 };
 //start for update firmware //msz   for update firmware 20121126
#define __FIRMWARE_UPDATE__
#ifdef __FIRMWARE_UPDATE__

#define FW_ADDR_MSG21XX   (0xC4>>1)
#define FW_ADDR_MSG21XX_TP   (0x4C>>1)
#define FW_UPDATE_ADDR_MSG21XX   (0x92>>1)

static  char *fw_version;
static u8 temp[94][1024];
static int FwDataCnt;
struct class *firmware_class;
struct device *firmware_cmd_dev;

static void HalTscrCReadI2CSeq( u8 addr, u8* read_data, u8 size )
{
    //according to your platform.
    int rc;

    struct i2c_msg msgs[] =
    {
        {
            .addr = addr,
            .flags = I2C_M_RD,
            .len = size,
            .buf = read_data,
        },
    };

    rc = i2c_transfer( msg21xx_i2c_client->adapter, msgs, 1 );
    if( rc < 0 )
    {
        TPD_DEBUG( "HalTscrCReadI2CSeq error %d\n", rc );
    }
}

static void HalTscrCDevWriteI2CSeq( u8 addr, u8* data, u16 size )
{
    //according to your platform.
    int rc;

    struct i2c_msg msgs[] =
    {
        {
            .addr = addr,
            .flags = 0,
            .len = size,
            .buf = data,
        },
    };
    rc = i2c_transfer( msg21xx_i2c_client->adapter, msgs, 1 );
    if( rc < 0 )
    {
        TPD_DEBUG( "HalTscrCDevWriteI2CSeq error %d,addr = %d\n", rc, addr );
    }
}

static void dbbusDWIICEnterSerialDebugMode( void )
{
    u8 data[5];

    // Enter the Serial Debug Mode
    data[0] = 0x53;
    data[1] = 0x45;
    data[2] = 0x52;
    data[3] = 0x44;
    data[4] = 0x42;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, data, 5 );
}

static void dbbusDWIICStopMCU( void )
{
    u8 data[1];

    // Stop the MCU
    data[0] = 0x37;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, data, 1 );
}

static void dbbusDWIICIICUseBus( void )
{
    u8 data[1];

    // IIC Use Bus
    data[0] = 0x35;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, data, 1 );
}

static void dbbusDWIICIICReshape( void )
{
    u8 data[1];

    // IIC Re-shape
    data[0] = 0x71;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, data, 1 );
}

static void dbbusDWIICIICNotUseBus( void )
{
    u8 data[1];

    // IIC Not Use Bus
    data[0] = 0x34;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, data, 1 );
}

static void dbbusDWIICNotStopMCU( void )
{
    u8 data[1];

    // Not Stop the MCU
    data[0] = 0x36;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, data, 1 );
}

static void dbbusDWIICExitSerialDebugMode( void )
{
    u8 data[1];

    // Exit the Serial Debug Mode
    data[0] = 0x45;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, data, 1 );

    // Delay some interval to guard the next transaction
    //udelay ( 200 );        // delay about 0.2ms
}

static void drvISP_EntryIspMode( void )
{
    u8 bWriteData[5] =
    {
        0x4D, 0x53, 0x54, 0x41, 0x52
    };

    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 5 );
}

static u8 drvISP_Read( u8 n, u8* pDataToRead )  //First it needs send 0x11 to notify we want to get flash data back.
{
    u8 Read_cmd = 0x11;
    unsigned char dbbus_rx_data[2] = {0xFF, 0xFF};
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &Read_cmd, 1 );
    msleep(1);         // delay about 1000us*****
    if( n == 1 )
    {
        HalTscrCReadI2CSeq( FW_UPDATE_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );

        // Ideally, the obtained dbbus_rx_data[0~1] stands for the following meaning:
        //  dbbus_rx_data[0]  |  dbbus_rx_data[1]  | status
        // -------------------+--------------------+--------
        //       0x00         |       0x00         |  0x00
        // -------------------+--------------------+--------
        //       0x??         |       0x00         |  0x??
        // -------------------+--------------------+--------
        //       0x00         |       0x??         |  0x??
        //                 
        // Therefore, we build this field patch to return the status to *pDataToRead.
        *pDataToRead = ( (dbbus_rx_data[0] >= dbbus_rx_data[1] )? \
                          dbbus_rx_data[0]  : dbbus_rx_data[1] );
    }
    else
    {
        HalTscrCReadI2CSeq( FW_UPDATE_ADDR_MSG21XX, pDataToRead, n );
    }

    return 0;
}

static void drvISP_WriteEnable( void )
{
    u8 bWriteData[2] =
    {
        0x10, 0x06
    };
    u8 bWriteData1 = 0x12;
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 2 );
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );
}


static void drvISP_ExitIspMode( void )
{
    u8 bWriteData = 0x24;
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData, 1 );
}

static u8 drvISP_ReadStatus( void )
{
    u8 bReadData = 0;
    u8 bWriteData[2] =
    {
        0x10, 0x05
    };
    u8 bWriteData1 = 0x12;

    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 2 );
    udelay(100);         // delay about 100us*****
    drvISP_Read( 1, &bReadData );
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );
    return bReadData;
}


static void drvISP_ChipErase()
{
    u8 bWriteData[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    u8 bWriteData1 = 0x12;
    u32 timeOutCount = 0;
    drvISP_WriteEnable();

    //Enable write status register
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x50;
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 2 );
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );

    //Write Status
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x01;
    bWriteData[2] = 0x00;
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 3 );
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );

    //Write disable
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x04;
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 2 );
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );
    udelay( 100 );         // delay about 100us*****
    timeOutCount = 0;
    while( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
    {
        timeOutCount++;
        if( timeOutCount >= 100000 ) break;  /* around 1 sec timeout */
    }
    drvISP_WriteEnable();

    bWriteData[0] = 0x10;
    bWriteData[1] = 0xC7;

    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 2 );
    HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );
    udelay( 100 );         // delay about 100us*****
    timeOutCount = 0;
    while( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
    {
        timeOutCount++;
        if( timeOutCount >= 500000 ) break;  /* around 5 sec timeout */
    }
}

static void drvISP_Program( u16 k, u8* pDataToWrite )
{
    u16 i = 0;
    u16 j = 0;
    //u16 n = 0;
    u8 TX_data[133];
    u8 bWriteData1 = 0x12;
    u32 addr = k * 1024;
    u32 timeOutCount = 0;
    for( j = 0; j < 8; j++ )  //128*8 cycle
    {
        TX_data[0] = 0x10;
        TX_data[1] = 0x02;// Page Program CMD
        TX_data[2] = ( addr + 128 * j ) >> 16;
        TX_data[3] = ( addr + 128 * j ) >> 8;
        TX_data[4] = ( addr + 128 * j );
        for( i = 0; i < 128; i++ )
        {
            TX_data[5 + i] = pDataToWrite[j * 128 + i];
        }
        udelay( 100 );         // delay about 100us*****

        timeOutCount = 0;
        while( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
        {
            timeOutCount++;
            if( timeOutCount >= 100000 ) break;  /* around 1 sec timeout */
        }



        drvISP_WriteEnable();
        HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, TX_data, 133 ); //write 133 byte per cycle
        HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );
    }
}


static void drvISP_Verify( u16 k, u8* pDataToVerify )
{
    u16 i = 0, j = 0;
    u8 bWriteData[5] =
    {
        0x10, 0x03, 0, 0, 0
    };
    u8 RX_data[256];
    u8 bWriteData1 = 0x12;
    u32 addr = k * 1024;
    u8 index = 0;
    u32 timeOutCount;
    for( j = 0; j < 8; j++ )  //128*8 cycle
    {
        bWriteData[2] = ( u8 )( ( addr + j * 128 ) >> 16 );
        bWriteData[3] = ( u8 )( ( addr + j * 128 ) >> 8 );
        bWriteData[4] = ( u8 )( addr + j * 128 );
        udelay( 100 );         // delay about 100us*****


        timeOutCount = 0;
        while( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
        {
            timeOutCount++;
            if( timeOutCount >= 100000 ) break;  /* around 1 sec timeout */
        }



        HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 5 );  //write read flash addr
        udelay( 100 );          // delay about 100us*****
        drvISP_Read( 128, RX_data );
        HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );  //cmd end
        for( i = 0; i < 128; i++ )  //log out if verify error
        {
            if( ( RX_data[i] != 0 ) && index < 10 )
            {
                //TPD_DEBUG("j=%d,RX_data[%d]=0x%x\n",j,i,RX_data[i]);
                index++;
            }
            if( RX_data[i] != pDataToVerify[128 * j + i] )
            {
                TPD_DEBUG( "k=%d,j=%d,i=%d===============Update Firmware Error================", k, j, i );
            }
        }
    }
}

static ssize_t firmware_update_show( struct device *dev,
                                     struct device_attribute *attr, char *buf )
{
    return sprintf( buf, "%s\n", fw_version );
}

static void _HalTscrHWReset( void )
{
#if 0
    gpio_direction_output( MSG21XX_RESET_GPIO, 1 );
    gpio_set_value( MSG21XX_RESET_GPIO, 1 );
    gpio_set_value( MSG21XX_RESET_GPIO, 0 );
    mdelay( 10 ); /* Note that the RST must be in LOW 10ms at least */
    gpio_set_value( MSG21XX_RESET_GPIO, 1 );
    /* Enable the interrupt service thread/routine for INT after 50ms */
    mdelay( 50 );
#endif
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	msleep(10);
	TPD_DMESG(" msg2133 reset\n");
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(50);

}
static ssize_t firmware_update_store( struct device *dev,
                                      struct device_attribute *attr, const char *buf, size_t size )
{
    u8 i;
    u8 dbbus_tx_data[4];
    unsigned char dbbus_rx_data[2] = {0};


    _HalTscrHWReset();
    //1.Erase TP Flash first
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();
    msleep( 300 );


    // Disable the Watchdog
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x60;
    dbbus_tx_data[3] = 0x55;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x61;
    dbbus_tx_data[3] = 0xAA;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    //Stop MCU
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x0F;
    dbbus_tx_data[2] = 0xE6;
    dbbus_tx_data[3] = 0x01;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //set FRO to 50M
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x11;
    dbbus_tx_data[2] = 0xE2;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    HalTscrCReadI2CSeq( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TPD_DEBUG( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );



    //set MCU clock,SPI clock =FRO
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x22;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x23;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    // Enable slave's ISP ECO mode
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x08;
    dbbus_tx_data[2] = 0x0c;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    //Enable SPI Pad
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x02;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    HalTscrCReadI2CSeq( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TPD_DEBUG( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = ( dbbus_rx_data[0] | 0x20 ); //Set Bit 5
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //WP overwrite
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x0E;
    dbbus_tx_data[3] = 0x02;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //set pin high
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x10;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbusDWIICIICNotUseBus();
    dbbusDWIICNotStopMCU();
    dbbusDWIICExitSerialDebugMode();



    drvISP_EntryIspMode();
    drvISP_ChipErase();
    _HalTscrHWReset();
    mdelay( 300 );

    //2.Program and Verify
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();



    // Disable the Watchdog
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x60;
    dbbus_tx_data[3] = 0x55;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x61;
    dbbus_tx_data[3] = 0xAA;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    //Stop MCU
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x0F;
    dbbus_tx_data[2] = 0xE6;
    dbbus_tx_data[3] = 0x01;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //set FRO to 50M
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x11;
    dbbus_tx_data[2] = 0xE2;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    HalTscrCReadI2CSeq( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TPD_DEBUG( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );



    //set MCU clock,SPI clock =FRO
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x22;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x23;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // Enable slave's ISP ECO mode
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x08;
    dbbus_tx_data[2] = 0x0c;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    //Enable SPI Pad
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x02;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    HalTscrCReadI2CSeq( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TPD_DEBUG( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = ( dbbus_rx_data[0] | 0x20 ); //Set Bit 5
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //WP overwrite
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x0E;
    dbbus_tx_data[3] = 0x02;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //set pin high
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x10;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbusDWIICIICNotUseBus();
    dbbusDWIICNotStopMCU();
    dbbusDWIICExitSerialDebugMode();

    ///////////////////////////////////////
    // Start to load firmware
    ///////////////////////////////////////
    drvISP_EntryIspMode();

    for( i = 0; i < 94; i++ )  // total  94 KB : 1 byte per R/W
    {
        drvISP_Program( i, temp[i] );  // program to slave's flash
        drvISP_Verify( i, temp[i] );  //verify data
    }
    TPD_DEBUG( "update OK\n" );
    drvISP_ExitIspMode();
    FwDataCnt = 0;
    return size;
}

static DEVICE_ATTR( update, 0777, firmware_update_show, firmware_update_store );

/*test=================*/
static ssize_t firmware_clear_show( struct device *dev,
                                    struct device_attribute *attr, char *buf )
{
    u16 k = 0, i = 0, j = 0;
    u8 bWriteData[5] =
    {
        0x10, 0x03, 0, 0, 0
    };
    u8 RX_data[256];
    u8 bWriteData1 = 0x12;
    u32 addr = 0;
    u32 timeOutCount = 0;
    for( k = 0; k < 94; i++ )  // total  94 KB : 1 byte per R/W
    {
        addr = k * 1024;
        for( j = 0; j < 8; j++ )  //128*8 cycle
        {
            bWriteData[2] = ( u8 )( ( addr + j * 128 ) >> 16 );
            bWriteData[3] = ( u8 )( ( addr + j * 128 ) >> 8 );
            bWriteData[4] = ( u8 )( addr + j * 128 );
            udelay( 100 );         // delay about 100us*****

            timeOutCount = 0;
            while( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
            {
                timeOutCount++;
                if( timeOutCount >= 100000 ) break;  /* around 1 sec timeout */
            }


            HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, bWriteData, 5 );  //write read flash addr
            udelay( 100 );         // delay about 100us*****
            drvISP_Read( 128, RX_data );
            HalTscrCDevWriteI2CSeq( FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1 );  //cmd end
            for( i = 0; i < 128; i++ )  //log out if verify error
            {
                if( RX_data[i] != 0xFF )
                {
                    TPD_DEBUG( "k=%d,j=%d,i=%d===============erase not clean================", k, j, i );
                }
            }
        }
    }
    TPD_DEBUG( "read finish\n" );
    return sprintf( buf, "%s\n", fw_version );
}

static ssize_t firmware_clear_store( struct device *dev,
                                     struct device_attribute *attr, const char *buf, size_t size )
{

    u8 dbbus_tx_data[4];
    unsigned char dbbus_rx_data[2] = {0};

    _HalTscrHWReset();
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();



    // Disable the Watchdog
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x60;
    dbbus_tx_data[3] = 0x55;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x61;
    dbbus_tx_data[3] = 0xAA;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    //Stop MCU
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x0F;
    dbbus_tx_data[2] = 0xE6;
    dbbus_tx_data[3] = 0x01;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //set FRO to 50M
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x11;
    dbbus_tx_data[2] = 0xE2;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    HalTscrCReadI2CSeq( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TPD_DEBUG( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );



    //set MCU clock,SPI clock =FRO
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x22;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x23;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    // Enable slave's ISP ECO mode
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x08;
    dbbus_tx_data[2] = 0x0c;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
    //Enable SPI Pad
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x02;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    HalTscrCReadI2CSeq( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TPD_DEBUG( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = ( dbbus_rx_data[0] | 0x20 ); //Set Bit 5
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //WP overwrite
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x0E;
    dbbus_tx_data[3] = 0x02;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );


    //set pin high
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x10;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbusDWIICIICNotUseBus();
    dbbusDWIICNotStopMCU();
    dbbusDWIICExitSerialDebugMode();


    ///////////////////////////////////////
    // Start to load firmware
    ///////////////////////////////////////
    drvISP_EntryIspMode();
    TPD_DEBUG( "chip erase+\n" );
    drvISP_ChipErase();
    TPD_DEBUG( "chip erase-\n" );
    drvISP_ExitIspMode();
    return size;
}

static DEVICE_ATTR( clear, 0777, firmware_clear_show, firmware_clear_store );

/*test=================*/
/*Add by Tracy.Lin for update touch panel firmware and get fw version*/

static ssize_t firmware_version_show( struct device *dev,
                                      struct device_attribute *attr, char *buf )
{
    TPD_DEBUG( "*** firmware_version_show fw_version = %s***\n", fw_version );
    return sprintf( buf, "%s\n", fw_version );
}

static ssize_t firmware_version_store( struct device *dev,
                                       struct device_attribute *attr, const char *buf, size_t size )
{
    unsigned char dbbus_tx_data[3];
    unsigned char dbbus_rx_data[4] ;
    unsigned short major = 0, minor = 0;

    fw_version = kzalloc( sizeof( char ), GFP_KERNEL );
    //SM-BUS GET FW VERSION
    dbbus_tx_data[0] = 0x53;
    dbbus_tx_data[1] = 0x00;
    dbbus_tx_data[2] = 0x74;
    HalTscrCDevWriteI2CSeq( FW_ADDR_MSG21XX_TP, &dbbus_tx_data[0], 3 );
    HalTscrCReadI2CSeq( FW_ADDR_MSG21XX_TP, &dbbus_rx_data[0], 4 );

    major = ( dbbus_rx_data[1] << 8 ) + dbbus_rx_data[0];
    minor = ( dbbus_rx_data[3] << 8 ) + dbbus_rx_data[2];

    TPD_DEBUG( "***major = %d ***\n", major );
    TPD_DEBUG( "***minor = %d ***\n", minor );
    sprintf( fw_version, "%03d%03d", major, minor );
    TPD_DEBUG( "***fw_version = %s ***\n", fw_version );


    return size;
}
static DEVICE_ATTR( version, 0777, firmware_version_show, firmware_version_store );

static ssize_t firmware_data_show( struct device *dev,
                                   struct device_attribute *attr, char *buf )
{
    return FwDataCnt;
}

static ssize_t firmware_data_store( struct device *dev,
                                    struct device_attribute *attr, const char *buf, size_t size )
{

    int i;
    TPD_DEBUG( "***FwDataCnt = %d ***\n", FwDataCnt );
    memcpy( temp[FwDataCnt], buf, 1024 );
    FwDataCnt++;
    return size;
}
static DEVICE_ATTR( data, 0777, firmware_data_show, firmware_data_store );
#endif

//end for update firmware

 static u8 Calculate_8BitsChecksum( u8 *msg, s32 s32Length )
 {
	 s32 s32Checksum = 0;
	 s32 i;
 
	 for( i = 0 ; i < s32Length; i++ )
	 {
		 s32Checksum += msg[i];
	 }
 
	 return ( u8 )( ( -s32Checksum ) & 0xFF );
 }

 static int tpd_touchinfo(TouchScreenInfo_t *touchData)
 {

    u8 val[8] = {0};
    u8 Checksum = 0;
    u8 i;
    u32 delta_x = 0, delta_y = 0;
    u32 u32X = 0;
    u32 u32Y = 0;
    

    TPD_DEBUG(KERN_ERR "[msg2133]==tpd_touchinfo() \n");


#ifdef SWAP_X_Y
    int tempx;
    int tempy;
#endif

    /*Get Touch Raw Data*/
    i2c_master_recv( i2c_client, &val[0], REPORT_PACKET_LENGTH );
    TPD_DEBUG(KERN_ERR"[tpd_touchinfo]--val[0]:%x, REPORT_PACKET_LENGTH:%x \n",val[0], REPORT_PACKET_LENGTH);
    Checksum = Calculate_8BitsChecksum( &val[0], 7 ); //calculate checksum
    TPD_DEBUG(KERN_ERR"[tpd_touchinfo]--Checksum:%x, val[7]:%x, val[0]:%x \n",Checksum, val[7], val[0]);

    if( ( Checksum == val[7] ) && ( val[0] == 0x52 ) ) //check the checksum  of packet
    {
        u32X = ( ( ( val[1] & 0xF0 ) << 4 ) | val[2] );   //parse the packet to coordinates
        u32Y = ( ( ( val[1] & 0x0F ) << 8 ) | val[3] );

        delta_x = ( ( ( val[4] & 0xF0 ) << 4 ) | val[5] );
        delta_y = ( ( ( val[4] & 0x0F ) << 8 ) | val[6] );
		TPD_DEBUG(KERN_ERR"[tpd_touchinfo]--u32X:%d, u32Y:%d, delta_x:%d, delta_y:%d \n",u32X, u32Y,delta_x, delta_y);

#ifdef SWAP_X_Y
        tempy = u32X;
        tempx = u32Y;
        u32X = tempx;
        u32Y = tempy;

        tempy = delta_x;
        tempx = delta_y;
        delta_x = tempx;
        delta_y = tempy;
#endif
#ifdef REVERSE_X
        u32X = 2047 - u32X;
        delta_x = 4095 - delta_x;
#endif
#ifdef REVERSE_Y
        u32Y = 2047 - u32Y;
        delta_y = 4095 - delta_y;
#endif

		TPD_DEBUG(KERN_ERR"[tpd_touchinfo]--u32X:%d, u32Y:%d, delta_x:%d, delta_y:%d \n",u32X, u32Y,delta_x, delta_y);

        if( ( val[1] == 0xFF ) && ( val[2] == 0xFF ) && ( val[3] == 0xFF ) && ( val[4] == 0xFF ) && ( val[6] == 0xFF ) )
        {  
            touchData->Point[0].X = 0; // final X coordinate
            touchData->Point[0].Y = 0; // final Y coordinate

            if( ( val[5] == 0x0 ) || ( val[5] == 0xFF ) )
            {
                touchData->nFingerNum = 0; //touch end
                touchData->nTouchKeyCode = 0; //TouchKeyMode
                touchData->nTouchKeyMode = 0; //TouchKeyMode
            }
            else
            {
                touchData->nTouchKeyMode = 1; //TouchKeyMode
                touchData->nTouchKeyCode = val[5]; //TouchKeyCode
                touchData->nFingerNum = 1;
            }
        }
        else
        {
            touchData->nTouchKeyMode = 0; //Touch on screen...

            if(
#ifdef REVERSE_X
                ( delta_x == 4095 )
#else
                ( delta_x == 0 )
#endif
                &&
#ifdef REVERSE_Y
                ( delta_y == 4095 )
#else
                ( delta_y == 0 )
#endif
            )
            {
                touchData->nFingerNum = 1; //one touch
                touchData->Point[0].X = ( u32X * MS_TS_MSG21XX_X_MAX ) / 2048;
                touchData->Point[0].Y = ( u32Y * MS_TS_MSG21XX_Y_MAX ) / 2048;
				TPD_DEBUG(KERN_ERR"[tpd_touchinfo]--FingerNum = 1 \n");
            }
            else
            {
                u32 x2, y2;

                touchData->nFingerNum = 2; //two touch

                /* Finger 1 */
                touchData->Point[0].X = ( u32X * MS_TS_MSG21XX_X_MAX ) / 2048;
                touchData->Point[0].Y = ( u32Y * MS_TS_MSG21XX_Y_MAX ) / 2048;

                /* Finger 2 */
                if( delta_x > 2048 )    //transform the unsigh value to sign value
                {
                    delta_x -= 4096;
                }
                if( delta_y > 2048 )
                {
                    delta_y -= 4096;
                }

                x2 = ( u32 )( u32X + delta_x );
                y2 = ( u32 )( u32Y + delta_y );

                touchData->Point[1].X = ( x2 * MS_TS_MSG21XX_X_MAX ) / 2048;
                touchData->Point[1].Y = ( y2 * MS_TS_MSG21XX_Y_MAX ) / 2048;
				TPD_DEBUG(KERN_ERR"[tpd_touchinfo]--FingerNum = 2 \n");
            }
        }

       
    }
    else
    {
        //DBG("Packet error 0x%x, 0x%x, 0x%x", val[0], val[1], val[2]);
        //DBG("             0x%x, 0x%x, 0x%x", val[3], val[4], val[5]);
        //DBG("             0x%x, 0x%x, 0x%x", val[6], val[7], Checksum);
        TPD_DEBUG( KERN_ERR "err status in tp\n" );
    }

    //enable_irq( msg21xx_irq );
  ///
	 return true;

 };
 
 static  void tpd_down(int x, int y, int p) {
 	
	 // input_report_abs(tpd->dev, ABS_PRESSURE, p);
	  input_report_key(tpd->dev, BTN_TOUCH, 1);
	  input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 25);
	  input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	  input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);

	  /* track id Start 0 */
		//input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p); 
	  input_mt_sync(tpd->dev);
	  if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
	  {   
		tpd_button(x, y, 1);  
	  }
	  if(y > TPD_RES_Y) //virtual key debounce to avoid android ANR issue
	  {
		  msleep(50);
		  TPD_DEBUG("D virtual key \n");
	  }
	  TPD_EM_PRINT(x, y, x, y, p-1, 1);
  }
  
 static  void tpd_up(int x, int y,int *count) {

	  input_report_key(tpd->dev, BTN_TOUCH, 0);
	  input_mt_sync(tpd->dev);
	  TPD_EM_PRINT(x, y, x, y, 0, 0);
		  
	  if(FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
	  {   
	  	 TPD_DEBUG(KERN_ERR "[msg2133]--tpd_up-BOOT MODE--X:%d, Y:%d; \n", x, y);
		 tpd_button(x, y, 0); 
	  } 		  
 
  }

 static int touch_event_handler(void *unused)
 {
  
    TouchScreenInfo_t touchData;
	u8 touchkeycode = 0;
	static u32 preKeyStatus = 0;
	int i=0;
 
    TPD_DEBUG(KERN_ERR "[msg2133]touch_event_handler() do while \n");

	touchData.nFingerNum = 0;
	TPD_DEBUG(KERN_ERR "[msg2133]touch_event_handler() do while \n");
	 
	if (tpd_touchinfo(&touchData)) 
	{
	 
		TPD_DEBUG(KERN_ERR "[msg2133]--KeyMode:%d, KeyCode:%d, FingerNum =%d \n", touchData.nTouchKeyMode, touchData.nTouchKeyCode, touchData.nFingerNum );
	 
		//key...
		if( touchData.nTouchKeyMode )
		{
	    	//key mode change virtual key mode
			touchData.nFingerNum = 1;
			if( touchData.nTouchKeyCode == 1 )
			{
				//touchkeycode = KEY_MENU;
				touchData.Point[0].X = 80;
				touchData.Point[0].Y = 850;
			}
			if( touchData.nTouchKeyCode == 2 )
			{
				//touchkeycode = KEY_HOMEPAGE ;
				touchData.Point[0].X = 240;
				touchData.Point[0].Y = 850;

			}
			if( touchData.nTouchKeyCode == 4 )
			{
				//touchkeycode = KEY_BACK;
				touchData.Point[0].X = 400;
				touchData.Point[0].Y = 850;

			}
			if( touchData.nTouchKeyCode == 8 )
			{
				//touchkeycode = KEY_SEARCH;
				//touchData.Point[0].X = 560;
				//touchData.Point[0].Y = 850;

			}
					
		}
				//report
		{
	 
			if( ( touchData.nFingerNum ) == 0 ) //touch end
			{
				TPD_DEBUG("------DOWN------ \n");
				TPD_DEBUG(KERN_ERR "[msg2133]---X:%d, Y:%d; \n", touchData.Point[0].X, touchData.Point[0].Y);
				tpd_up(touchData.Point[0].X, touchData.Point[0].Y, 0);
				input_sync( tpd->dev );
			}
			else //touch on screen
			{
	 
				for( i = 0; i < ( (int)touchData.nFingerNum ); i++ )
				{
				    TPD_DEBUG("------DOWN------ \n");
					tpd_down(touchData.Point[i].X, touchData.Point[i].Y, 1);
					TPD_DEBUG(KERN_ERR "[msg2133]---X:%d, Y:%d; i=%d \n", touchData.Point[i].X, touchData.Point[i].Y, i);
				}
	 
				input_sync( tpd->dev );
			}
		}//end if(touchData->nTouchKeyMode)
	 
			}

     mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
	 return 0;
 }
 
 static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info) 
 {
	 strcpy(info->type, TPD_DEVICE);	
	  return 0;
 }
 
 static void tpd_eint_interrupt_handler(void)
 {
	 mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	 schedule_work( &msg21xx_wq );
 }

 static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
 {	 
 
	int retval = TPD_OK;
	char data;
	u8 report_rate=0;
	int err=0;
	int reset_count = 0;

	i2c_client = client;
	msg21xx_i2c_client = client;
	
	/*reset I2C clock*/
    //i2c_client->timing = 0;
    
   INIT_WORK( &msg21xx_wq, touch_event_handler );
//power on, need confirm with SA
#ifdef TPD_POWER_SOURCE_CUSTOM
	hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#else
	hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
	hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif 


#ifdef TPD_CLOSE_POWER_IN_SLEEP	 
	hwPowerDown(TPD_POWER_SOURCE,"TP");
	hwPowerOn(TPD_POWER_SOURCE,VOL_2800,"TP");
	msleep(100);
#else

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(10);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	msleep(50);
	TPD_DMESG(" msg2133 reset\n");
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(50);
	
#endif
	

	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
   	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_DOWN);
		

    msleep(10);

	mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1); 
	msleep(50);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	msleep(200);
    tpd_load_status = 1;

	TPD_DMESG("msg2133 Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");
	
	TPD_DEBUG("msg2133--frameware upgrade \n");

    /*frameware upgrade*/	
#ifdef __FIRMWARE_UPDATE__
		firmware_class = class_create( THIS_MODULE, "ms-touchscreen-msg20xx" );
		if( IS_ERR( firmware_class ) )
			pr_err( "Failed to create class(firmware)!\n" );
		firmware_cmd_dev = device_create( firmware_class,
										  NULL, 0, NULL, "device" );
		if( IS_ERR( firmware_cmd_dev ) )
			pr_err( "Failed to create device(firmware_cmd_dev)!\n" );
	
		// version
		if( device_create_file( firmware_cmd_dev, &dev_attr_version ) < 0 )
			pr_err( "Failed to create device file(%s)!\n", dev_attr_version.attr.name );
		// update
		if( device_create_file( firmware_cmd_dev, &dev_attr_update ) < 0 )
			pr_err( "Failed to create device file(%s)!\n", dev_attr_update.attr.name );
		// data
		if( device_create_file( firmware_cmd_dev, &dev_attr_data ) < 0 )
			pr_err( "Failed to create device file(%s)!\n", dev_attr_data.attr.name );
		// clear
		if( device_create_file( firmware_cmd_dev, &dev_attr_clear ) < 0 )
			pr_err( "Failed to create device file(%s)!\n", dev_attr_clear.attr.name );
	
		dev_set_drvdata( firmware_cmd_dev, NULL );
#endif

   return 0;
   
 }

 static int __devexit tpd_remove(struct i2c_client *client)
 
 {
   
	 TPD_DEBUG("TPD removed\n");
 
   return 0;
 }
 
 
 static int tpd_local_init(void)
 {

 
  	TPD_DMESG("Mstar msg2133 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);
 
 
    if(i2c_add_driver(&tpd_i2c_driver)!=0)
   	{
  		TPD_DMESG("msg2133 unable to add i2c driver.\n");
      	return -1;
    }
    if(tpd_load_status == 0) 
    {
    	TPD_DMESG("msg2133 add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    }
	
#ifdef TPD_HAVE_BUTTON     
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   
  
//#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
//WARP CHECK IS NEED --XB.PANG
//#endif 

	TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
		
    return 0; 
 }

 static void tpd_resume( struct early_suspend *h )
 {
 
   TPD_DMESG("TPD wake up\n");
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerOn(TPD_POWER_SOURCE,VOL_2800,"TP");
#endif
	msleep(100);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	msleep(50);
	TPD_DMESG(" msg2133 reset\n");
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(200);
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	TPD_DMESG("TPD wake up done\n");
	
 }

 static void tpd_suspend( struct early_suspend *h )
 {
 	
	TPD_DMESG("TPD enter sleep\n");
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	 
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerDown(TPD_POWER_SOURCE,"TP");
#else
	//TP enter sleep mode----XB.PANG NEED CHECK
	//if have sleep mode
#endif
    TPD_DMESG("TPD enter sleep done\n");
 } 


 static struct tpd_driver_t tpd_device_driver = {
		 .tpd_device_name = "msg2133",
		 .tpd_local_init = tpd_local_init,
		 .suspend = tpd_suspend,
		 .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		 .tpd_have_button = 1,
#else
		 .tpd_have_button = 0,
#endif		
 };
 /* called when loaded into kernel */
 static int __init tpd_driver_init(void) {
	 TPD_DEBUG("MediaTek MSG2133 touch panel driver init\n");
	   i2c_register_board_info(0, &msg2133_i2c_tpd, 1);
		 if(tpd_driver_add(&tpd_device_driver) < 0)
			 TPD_DMESG("add MSG2133 driver failed\n");
	 return 0;
 }
 
 /* should never be called */
 static void __exit tpd_driver_exit(void) {
	 TPD_DMESG("MediaTek MSG2133 touch panel driver exit\n");
	 tpd_driver_remove(&tpd_device_driver);
 }
 
 module_init(tpd_driver_init);
 module_exit(tpd_driver_exit);


