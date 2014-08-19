//for open()
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>


//errno
#include    <errno.h>

//SMI kernel include
#include    <linux/ioctl.h>
#include    "mt_smi.h"  

#include    "bandwidth_control.h"
#include    "bandwidth_control_private.h"

#ifdef FLAG_SUPPORT_MODEM_SCALE
//modem speed change
#include    <netutils/ifc.h>
#endif


/*=============================================================================
    SMI Bandwidth Control
  =============================================================================*/

int BWC::smi_bw_ctrl_set( BWC_PROFILE_TYPE profile_type , BWC_VCODEC_TYPE codec_type , bool bOn )
{
    
    int                     smi_fd = -1;    //smi device driver node file descriptor
    MTK_SMI_BWC_CONFIG      cfg;

    smi_fd = open("/dev/MTK_SMI", O_RDONLY);

    if(-1 == smi_fd)
    {
        BWC_ERROR("Open SMI(/dev/MTK_SMI) driver file failed.:%s\n", strerror(errno));
        return -1;
    }


    if( bOn )
    {
        switch( profile_type )
        {
        case BWCPT_NONE:
            cfg.scenario = SMI_BWC_SCEN_NORMAL;
            cfg.b_reduce_command_buffer = 0;
            cfg.b_gpu_od = 1;
            break;
            
        case BWCPT_VIDEO_NORMAL:
            cfg.scenario = SMI_BWC_SCEN_NORMAL;
            cfg.b_reduce_command_buffer = 0;
            cfg.b_gpu_od = 1;
            break;
            
        case BWCPT_CAMERA_PREVIEW:
        case BWCPT_CAMERA_ZSD:
            cfg.scenario = SMI_BWC_SCEN_VRCAMERA1066;   /*SMI special setting*/
            cfg.b_reduce_command_buffer = 1;
            cfg.b_gpu_od = 0;                           /*Camera relative scenario disable GPU OD*/
            break;

        case BWCPT_CAMERA_CAPTURE:
            cfg.scenario = SMI_BWC_SCEN_NORMAL;
            cfg.b_reduce_command_buffer = 0;
            cfg.b_gpu_od = 0;                           /*Camera relative scenario disable GPU OD*/
            break;
        
        case BWCPT_VIDEO_PLAYBACK:
            cfg.scenario = SMI_BWC_SCEN_VP1066;         /*SMI special setting*/
            cfg.b_reduce_command_buffer = 0;
            cfg.b_gpu_od = 1;
            break;

        case BWCPT_VIDEO_TELEPHONY:
            cfg.scenario = SMI_BWC_SCEN_NORMAL;
            cfg.b_reduce_command_buffer = 0;
            cfg.b_gpu_od = 0;                           /*Camera relative scenario disable GPU OD*/
            break;
        
        case BWCPT_VIDEO_RECORD:
            cfg.scenario = SMI_BWC_SCEN_VR1066;         /*SMI special setting*/
            cfg.b_reduce_command_buffer = 1;
            cfg.b_gpu_od = 1;
            break;
            
        case BWCPT_VIDEO_RECORD_CAMERA:
            cfg.scenario = SMI_BWC_SCEN_VRCAMERA1066;
            cfg.b_reduce_command_buffer = 1;
            cfg.b_gpu_od = 0;                           /*Camera relative scenario disable GPU OD*/
            break;

        case BWCPT_VIDEO_SNAPSHOT:
            cfg.scenario = SMI_BWC_SCEN_NORMAL;
            cfg.b_reduce_command_buffer = 0;
            cfg.b_gpu_od = 0;                           /*Camera relative scenario disable GPU OD*/
            break;
            
        default:
            cfg.scenario = SMI_BWC_SCEN_NORMAL;
            cfg.b_reduce_command_buffer = 0;
            cfg.b_gpu_od = 1;
            break;
            
        }
    }
    else
    {
        cfg.scenario = SMI_BWC_SCEN_NORMAL;
        cfg.b_reduce_command_buffer = 0;
        cfg.b_gpu_od = 1;
    }


    
    
    if( ioctl( smi_fd, MTK_IOC_SMI_BWC_CONFIG , &cfg ) < 0 )
    {
        BWC_ERROR("MTK_IOC_SMI_BWC_CONFIG failed.:%s\n" , strerror(errno) );
        close(smi_fd);
        return -1; 
    }

    BWC_INFO("smi_bw_ctrl_set: scen %d, reduce cmd %d\n", cfg.scenario, cfg.b_reduce_command_buffer );

    close(smi_fd);
    
    return 0;
}


/*=============================================================================
    EMI Bandwidth Control
  =============================================================================*/
static int emi_ctrl_str_generate( BWC_PROFILE_TYPE profile_type , BWC_VCODEC_TYPE codec_type, bool bOn, char* out_str )
{
    char *p_cmdstr_profile = NULL;
    char *p_cmdstr_switch  = NULL;
    
    
    switch( profile_type )
    {
    case BWCPT_VIDEO_NORMAL:
        p_cmdstr_profile = (char*)"CON_SCE_NORMAL";
        break;

    case BWCPT_VIDEO_RECORD_CAMERA:
    case BWCPT_VIDEO_RECORD:
        if( codec_type == BWCVT_MPEG4 ){
            p_cmdstr_profile = (char*)"CON_SCE_VR_MP4";
        }
        else{
            p_cmdstr_profile = (char*)"CON_SCE_VR_H264";
        }
        break;
        
    case BWCPT_VIDEO_PLAYBACK:
        p_cmdstr_profile = (char*)"CON_SCE_VP";
        break;
                
    case BWCPT_VIDEO_SNAPSHOT:
        if( codec_type == BWCVT_MPEG4 ){   /*VSS use VR profile*/
            p_cmdstr_profile = (char*)"CON_SCE_VR_MP4"; 
        }
        else{
            p_cmdstr_profile = (char*)"CON_SCE_VR_H264";
        }
        break;
        
    case BWCPT_VIDEO_TELEPHONY:
        p_cmdstr_profile = (char*)"CON_SCE_VC";
        break;
                
    case BWCPT_CAMERA_PREVIEW:
        p_cmdstr_profile = (char*)"CON_SCE_NORMAL";/*Camera preview Use Normal Profile*/
        break;
        
    case BWCPT_CAMERA_CAPTURE:
        p_cmdstr_profile = (char*)"CON_SCE_IC";
        break;

    case BWCPT_CAMERA_ZSD:
        p_cmdstr_profile = (char*)"CON_SCE_ZSD";
        break;
        
    case BWCPT_NONE:
        p_cmdstr_profile = (char*)"CON_SCE_NORMAL";
        break;

    default:
        BWC_ERROR("Invalid profile_type = %d\n", (int)profile_type );
        return -1;
        
    }



    p_cmdstr_switch = ( bOn == true ) ? (char*)" ON":(char*)" OFF";

    strcpy( out_str, p_cmdstr_profile );
    strcat( out_str, p_cmdstr_switch );

    return 0;
    
    
}




int BWC::emi_bw_ctrl_set( BWC_PROFILE_TYPE profile_type , BWC_VCODEC_TYPE codec_type , bool bOn )
{
    const char  *con_sce_file    = "/sys/bus/platform/drivers/mem_bw_ctrl/concurrency_scenario";
    int         fd;
    char        emi_ctrl_str[128];


    if( emi_ctrl_str_generate( profile_type, codec_type, bOn, emi_ctrl_str ) < 0 ){
        BWC_ERROR("emi_ctrl_str_generate failed!\n");
        return -1;
    }
        

    fd = open(con_sce_file, O_WRONLY);
    
    if (fd == -1) 
    {
        BWC_ERROR("fail to open mem_bw_ctrl driver file\n");
        fsync(1);
        return -1;
    } 
    else 
    {
        BWC_INFO("emi_bw_ctrl_set: %s\n", emi_ctrl_str);
        
        /* enable my scenario before running my application*/
        write(fd, emi_ctrl_str, strlen(emi_ctrl_str));

    }

    close(fd);
    
    return 0;
}


/*=============================================================================
    EMI DDR TYPE Get
  =============================================================================*/
BWC::EMI_DDR_TYPE BWC::emi_ddr_type_get( void )
{
    const char      *ddr_type_file    = "/sys/bus/platform/drivers/ddr_type/ddr_type";
    int             fd;
    char            ddr_type_str[128];
    EMI_DDR_TYPE    ddr_type;


    fd = open(ddr_type_file, O_RDONLY);
    
    if (fd == -1) 
    {
        BWC_ERROR("fail to open ddr_type_file driver file\n");
        fsync(1);
        return EDT_NONE;
    } 
    else 
    {
        int i;

        for( i = 0; i < ((int)sizeof(ddr_type_str) - 1); i++ ) {
            if( read(fd, &(ddr_type_str[i]), 1 ) <= 0 ) {
                break;
            }

            if( (ddr_type_str[i] == 0xA) || (ddr_type_str[i] == 0xD) ){
                break;
            }
        }
        
        ddr_type_str[i] = '\0';

        BWC_INFO("Read DDR type string:%s\n", ddr_type_str);
    }

    close(fd);


    //Mapping DDR type
    if( strcmp( ddr_type_str , "LPDDR2" ) == 0 ){
        return EDT_LPDDR2;
    }

    if( strcmp( ddr_type_str , "DDR3" ) == 0 ){
        return EDT_DDR3;
    }
    
    if( strcmp( ddr_type_str , "LPDDR3" ) == 0 ){
        return EDT_LPDDR3;
    }

    if( strcmp( ddr_type_str , "mDDR" ) == 0 ){
        return EDT_mDDR;
    }
    
    return EDT_NONE;
    
}



/*=============================================================================
    Modem Speed Control
  =============================================================================*/

int BWC::modem_speed_profile_set( BWC::MODEM_SPEED_PROFILE profile )
{
    
#ifdef FLAG_SUPPORT_MODEM_SCALE


    switch( profile )
    {
    case MSP_NORMAL:
        BWC_INFO("ifc_set_throttle: %d %d (Normal)\n", -1, -1 );
        ifc_set_throttle( "ccmni0", -1, -1 );       //UnLimit
        break;

    case MSP_SCALE_DOWN:
        BWC_INFO("ifc_set_throttle: %d %d (Scale Down)\n", (int)21*1024, (int)5.7*1024 );
        ifc_set_throttle( "ccmni0", 21*1024, 5.7*1024 );  //Limit downlink to 21Mbps/uplink 5.7Mbps
        break;

    default:
        BWC_ERROR("Unknown modem speed profile:%d\n", profile );
        return -1;
    }



#endif
    
    return 0;
}


