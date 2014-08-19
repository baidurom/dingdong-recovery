/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#if 0
#include <asm/arch/mt6589_irq.h>
#endif 
#include <asm/arch/mt6589_sleep.h>

//#define HAVE_SLEEP_MODE

#if 0
static void sc_go_to_sleep(u32 timeout, kal_bool en_deep_idle)
{
    struct mtk_irq_mask mask;

    __asm__ __volatile__("cpsid i @ arch_local_irq_disable" : : : "memory", "cc"); // set i bit to disable interrupt

    mt6589_irq_mask_all(&mask); // mask all interrupts

    gpt_irq_ack();

    mt6589_irq_unmask(MT6589_GPT_IRQ_ID);

    gpt_one_shot_irq(timeout);

    __asm__ __volatile__("dsb" : : : "memory");
    __asm__ __volatile__("wfi"); // enter wfi

    gpt_irq_ack();
    mt6589_irq_ack(MT6575_GPT_IRQ_ID);

    mt6589_irq_mask_restore(&mask); // restore all interrupts

    __asm__ __volatile__("cpsie i @ arch_local_irq_enable" : : : "memory", "cc"); // clear i bit to enable interrupt
}
#endif 

void mtk_sleep(u32 timeout, kal_bool en_deep_idle)
{
    printf("enter mtk_sleep, timeout = %d\n", timeout);

#ifndef HAVE_SLEEP_MODE
    gpt_busy_wait_ms(timeout);
#else
    sc_go_to_sleep(timeout, en_deep_idle);
#endif

    return 0;
}

void power_saving_init(void) {}

void sc_mod_init(void)
{
    #if 0
    mt6589_init_irq();

    gpt_irq_init();

    power_saving_init();
    #endif

    printf("[BATTERY] sc_hw_init : Done\r\n");
}

void sc_mod_exit(void) {}