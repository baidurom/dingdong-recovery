#include <ddp_cmdq.h>
#include <ddp_reg.h>

spinlock_t              gCmdqTaskLock;
spinlock_t              gCmdqThreadLock;
spinlock_t              gCmdqExecLock;
spinlock_t              gCmdqRecordLock;
ContextStruct           gCmdqContext;
wait_queue_head_t       gCmdWaitQueue[CMDQ_MAX_THREAD_COUNT];
struct list_head        gCmdqFreeTask;
struct completion       gCmdqComplete;
struct proc_dir_entry   *gCmdqProcEntry;

extern void smi_dumpDebugMsg(void);

typedef struct {
    int              moduleType[cbMAX];
    CMDQ_TIMEOUT_PTR cmdqTimeout_cb[cbMAX];
    CMDQ_RESET_PTR   cmdqReset_cb[cbMAX];
} CMDQ_CONFIG_CB_ARRAY;

CMDQ_CONFIG_CB_ARRAY g_CMDQ_CB_Array = { {cbMDP, cbISP}, { NULL, NULL }, { NULL, NULL } };  //if cbMAX == 2

int32_t cmdqRegisterCallback(int32_t          index,
                             CMDQ_TIMEOUT_PTR pTimeoutFunc,
                             CMDQ_RESET_PTR   pResetFunc)
{
    if((index >= 2) || (NULL == pTimeoutFunc) || (NULL == pResetFunc))
    {
        printk("Warning! [Func]%s register NULL function : %p,%p\n", __func__ , pTimeoutFunc , pResetFunc);
        return -1;
    }

    g_CMDQ_CB_Array.cmdqTimeout_cb[index] = pTimeoutFunc;
    g_CMDQ_CB_Array.cmdqReset_cb[index]   = pResetFunc;

    return 0;
}


static int32_t cmdq_read_status_proc(char    *pPage,
                                     char    **ppStart,
                                     off_t   offset,
                                     int32_t count,
                                     int32_t *pEOF,
                                     void    *pData)
{
    u_long       flags;
    char         *pBuffer;
    EngineStruct *pEngine;
    TaskStruct   *pTask;
    ThreadStruct *pThread;
    int32_t      index;
    int32_t      inner;
    int32_t      begin;
    int32_t      curPos;
    int32_t      length;
    int32_t      isFull;

    pEngine = &(gCmdqContext.engine[0]);
    pBuffer = pPage;
    begin   = 0;
    curPos  = 0;
    length  = 0;
    *pEOF   = 0x1;
    isFull  = 0;

    spin_lock_irqsave(&gCmdqThreadLock, flags); 
    smp_mb();

    length += sprintf(&pBuffer[length], "====== Clock Status =======\n");
    length += sprintf(&pBuffer[length], "MT_CG_DISP0_MM_CMDQ: %d, MT_CG_DISP0_MUTEX: %d\n",
        clock_is_on(MT_CG_DISP0_MM_CMDQ), clock_is_on(MT_CG_DISP0_MUTEX));

    length += sprintf(&pBuffer[length], "====== Engine Usage =======\n");
    length += sprintf(&pBuffer[length], "IMGI: count %d, owner %d, fail: %d, reset: %d\n", 
        pEngine[tIMGI].userCount, pEngine[tIMGI].currOwner, pEngine[tIMGI].failCount, pEngine[tIMGI].resetCount);
    length += sprintf(&pBuffer[length], "RDMA0: count %d, owner %d, fail: %d, reset: %d\n", 
        pEngine[tRDMA0].userCount, pEngine[tRDMA0].currOwner, pEngine[tRDMA0].failCount, pEngine[tRDMA0].resetCount);
    length += sprintf(&pBuffer[length], "RSZ0: count %d, owner %d, fail: %d, reset: %d\n", 
        pEngine[tSCL0].userCount, pEngine[tSCL0].currOwner, pEngine[tSCL0].failCount, pEngine[tSCL0].resetCount);
    length += sprintf(&pBuffer[length], "RSZ1: count %d, owner %d, fail: %d, reset: %d\n", 
        pEngine[tSCL1].userCount, pEngine[tSCL1].currOwner, pEngine[tSCL1].failCount, pEngine[tSCL1].resetCount);   
    length += sprintf(&pBuffer[length], "TDSHP: count %d, owner %d, fail: %d, reset: %d\n", 
        pEngine[tTDSHP].userCount, pEngine[tTDSHP].currOwner, pEngine[tTDSHP].failCount, pEngine[tTDSHP].resetCount);
    length += sprintf(&pBuffer[length], "tWDMA1: count %d, owner %d, fail: %d, reset: %d\n", 
        pEngine[tWDMA1].userCount, pEngine[tWDMA1].currOwner, pEngine[tWDMA1].failCount, pEngine[tWDMA1].resetCount);  
    length += sprintf(&pBuffer[length], "tWROT: count %d, owner %d, fail: %d, reset: %d\n", 
        pEngine[tWROT].userCount, pEngine[tWROT].currOwner, pEngine[tWROT].failCount, pEngine[tWROT].resetCount); 

    curPos = begin + length;
    if (curPos < offset)
    {
        length = 0;
        begin  = curPos;
    }

    if (curPos > (offset + count))
    {
        *pEOF  = 0x0;
        isFull = 1;
    }

    for (index = 0; ((index < CMDQ_MAX_FIXED_TASK) && (0 == isFull)); index++)
    {
        pTask = &(gCmdqContext.taskInfo[index]);
        
        length += sprintf(&pBuffer[length], "====== Task %d Usage =======\n", index);

        length += sprintf(&pBuffer[length], "State %d, VABase: 0x%08x, MVABase: 0x%08x, Size: %d\n",
            pTask->taskState, (uint32_t)pTask->pVABase, pTask->MVABase, pTask->blockSize);
        length += sprintf(&pBuffer[length], "Scenario %d, Priority: %d, Flag: 0x%08x, VAEnd: 0x%08x\n",
            pTask->scenario, pTask->priority, pTask->engineFlag, (uint32_t)pTask->pCMDEnd);
        length += sprintf(&pBuffer[length], "Reorder: %d, Trigger %d:%d, IRQ: %d:%d, Wake Up: %d:%d\n",
            pTask->reorder,
            (uint32_t)pTask->trigger.tv_sec, (uint32_t)pTask->trigger.tv_nsec,
            (uint32_t)pTask->gotIRQ.tv_sec, (uint32_t)pTask->gotIRQ.tv_nsec, 
            (uint32_t)pTask->wakedUp.tv_sec, (uint32_t)pTask->wakedUp.tv_nsec);

        curPos = begin + length;
        if (curPos < offset)
        {
            length = 0;
            begin  = curPos;
        }

        if (curPos > (offset + count))
        {
            *pEOF  = 0x0;
            isFull = 1;
            break;
        }
    }

    for (index = 0; ((index < CMDQ_MAX_THREAD_COUNT) && (0 == isFull)); index++)
    {
        pThread = &(gCmdqContext.thread[index]);
        
        if (pThread->taskCount > 0)
        {
            length += sprintf(&pBuffer[length], "====== Thread %d Usage =======\n", index);
            length += sprintf(&pBuffer[length], "Wait Cookie %d, Next Cookie %d\n", pThread->waitCookie, pThread->nextCookie);

            curPos = begin + length;
            if (curPos < offset)
            {
                length = 0;
                begin  = curPos;
            }

            if (curPos > (offset + count))
            {
                *pEOF  = 0x0;
                isFull = 1;
            }

            for (inner = 0; ((inner < CMDQ_MAX_TASK_COUNT) && (0 == isFull)); inner++)
            {
                pTask = pThread->pCurTask[inner];
                if (NULL != pTask)
                {
                    length += sprintf(&pBuffer[length], "Slot: %d, Task: 0x%08x, MVABase: 0x%08x, Size: %d, Last Command 0x%08x, 0x%08x\n", 
                        index, (uint32_t)pTask, pTask->MVABase, pTask->blockSize, pTask->pCMDEnd[-1], pTask->pCMDEnd[0]);
                    
                    curPos = begin + length;
                    if (curPos < offset)
                    {
                        length = 0;
                        begin  = curPos;
                    }

                    if (curPos > (offset + count))
                    {
                        *pEOF  = 0x0;
                        isFull = 1;
                        break;
                    }
                }
            }
        }
    }
    spin_unlock_irqrestore(&gCmdqThreadLock, flags); 

    *ppStart  = pPage + (offset - begin);
    length   -= (offset - begin);

    return (length > count)? count: length;
}


static int32_t cmdq_read_error_proc(char    *pPage,
                                    char    **ppStart,
                                    off_t   offset,
                                    int32_t count,
                                    int32_t *pEOF,
                                    void    *pData)
{
    u_long      flags;
    int32_t     begin;
    int32_t     curPos;
    int32_t     length;
    int32_t     index;
#if 0
    int32_t     inner;
#endif // 0
    ErrorStruct *pError;
    TaskStruct  *pTask;

#if 0
    uint32_t    *pCMD;
    int32_t     size;
    uint32_t    MVABase;
    uint32_t    mask;
    uint32_t    value;
    uint32_t    addr;
#endif // 0

    uint32_t    isFull;

    begin   = 0;
    curPos  = 0;
    length  = 0;
    *pEOF   = 0x1;
    isFull  = 0;

    *ppStart  = pPage;
    length    = 0;
    return 0;

    spin_lock_irqsave(&gCmdqExecLock, flags);
    smp_mb();

    pError = &(gCmdqContext.error[0]);

    count = gCmdqContext.errNum;
    for (index = 0; ((index < count) && (0 == isFull)); index++)
    {
        pTask = pError[index].pTask;

        length += sprintf(&pPage[length], "====== Error Task 0x%08x =======\n", (uint32_t)pTask);
        length += sprintf(&pPage[length], "State %d, VABase: 0x%08x, MVABase: 0x%08x, Size: %d\n",
            pTask->taskState, (uint32_t)pTask->pVABase, pTask->MVABase, pTask->blockSize);
        length += sprintf(&pPage[length], "Scenario %d, Priority: %d, Flag: 0x%08x, VAEnd: 0x%08x\n",
            pTask->scenario, pTask->priority, pTask->engineFlag, (uint32_t)pTask->pCMDEnd);
        length += sprintf(&pPage[length], "Reorder: %d, Trigger %d:%d, IRQ: %d:%d, Wake Up: %d:%d\n",
            pTask->reorder,
            (uint32_t)pTask->trigger.tv_sec, (uint32_t)pTask->trigger.tv_nsec,
            (uint32_t)pTask->gotIRQ.tv_sec, (uint32_t)pTask->gotIRQ.tv_nsec, 
            (uint32_t)pTask->wakedUp.tv_sec, (uint32_t)pTask->wakedUp.tv_nsec);
        length += sprintf(&pPage[length], "================================\n");

        curPos = begin + length;
        if (curPos < offset)
        {
            length = 0;
            begin  = curPos;
        }

        if (curPos > (offset + count))
        {
            *pEOF  = 0x0;
            isFull = 1;
            continue;
        }

    #if 0
        if (pTask->engineFlag & (0x01 << tRDMA0))
        {
            for (inner = 0; inner < 1024; inner++)
            {
                length += sprintf(&pPage[length], "0x%08x: 0x%08x\n", (0xF4001000 + (inner << 2)), pError->regVal[tRDMA0][inner]);
        
                curPos = begin + length;
                if (curPos < offset)
                {
                    length = 0;
                    begin  = curPos;
                }

                if (curPos > (offset + count))
                {
                    *pEOF  = 0x0;
                    isFull = 1;
                    continue;
                }
            }
        }

        if (pTask->engineFlag & (0x01 << tWROT))
        {
            for (inner = 0; inner < 1024; inner++)
            {
                length += sprintf(&pPage[length], "0x%08x: 0x%08x\n", (0xF4005000 + (inner << 2)), pError->regVal[tWROT][inner]);
        
                curPos = begin + length;
                if (curPos < offset)
                {
                    length = 0;
                    begin  = curPos;
                }

                if (curPos > (offset + count))
                {
                    *pEOF  = 0x0;
                    isFull = 1;
                    continue;
                }
            }
        }

        if (pTask->engineFlag & (0x01 << tWDMA1))
        {
            for (inner = 0; inner < 1024; inner++)
            {
                length += sprintf(&pPage[length], "0x%08x: 0x%08x\n", (0xF4004000 + (inner << 2)), pError->regVal[tWDMA1][inner]);
        
                curPos = begin + length;
                if (curPos < offset)
                {
                    length = 0;
                    begin  = curPos;
                }

                if (curPos > (offset + count))
                {
                    *pEOF  = 0x0;
                    isFull = 1;
                    continue;
                }
            }
        }
    #endif // 0

    #if 0 // too huge
        pCMD    = pTask->pVABase;
        size    = pTask->blockSize;
        MVABase = pTask->MVABase;
        mask    = 0xFFFFFFFF;
        for (pCMD  = pTask->pVABase; size > 0; pCMD += 2, size -= 8, MVABase += 8)
        {
            switch(pCMD[1] >> 24)
            {
                case CMDQ_CODE_MOVE:
                    mask = ~pCMD[0];
                    length += sprintf(&pPage[length], "MVA: 0x%08x, MOVE 0x%08x\n", MVABase, pCMD[0]);
                    break;
                case CMDQ_CODE_WRITE:
                    value = (pCMD[1] >> 22) & 0x03;
                    addr  = (pCMD[1] & 0x3fffff);

                    if (0 != mask)
                    {
                        value &= mask;
                        addr  &= ~1;
                    }
                    else
                    {
                        mask = 0xFFFFFFFF;
                    }

                    if (0x00 == value)
                    {
                        addr |= (0x14 << 24);
                    }
                    else if (0x01 == value)
                    {
                        addr |= (0x15 << 24);
                    }

                    length += sprintf(&pPage[length], "MVA: 0x%08x, WRITE Addr: 0x%08x, Value: 0x%08x\n", MVABase, addr, pCMD[0]);
                    break;
                case CMDQ_CODE_POLL:
                    value = (pCMD[1] >> 22) & 0x03;
                    addr  = (pCMD[1] & 0x3fffff);
                    
                    if (0 != mask)
                    {
                        value &= mask;
                        addr  &= ~1;
                    }
                    else
                    {
                        mask = 0xFFFFFFFF;
                    }
                    
                    if (0x00 == value)
                    {
                        addr |= (0x14 << 24);
                    }
                    else if (0x01 == value)
                    {
                        addr |= (0x15 << 24);
                    }

                    length += sprintf(&pPage[length], "MVA: 0x%08x, POLL Addr: 0x%08x, Value: 0x%08x\n", MVABase, addr, pCMD[0]);
                    break;
                case CMDQ_CODE_JUMP:
                    length += sprintf(&pPage[length], "MVA: 0x%08x, JUMP 0x%08x\n", MVABase, pCMD[0]);
                    break;
                case CMDQ_CODE_WFE:
                    value = pCMD[1] & 0x00FFFFFF;
                    length += sprintf(&pPage[length], "MVA: 0x%08x, WFE 0x%08x\n", MVABase, value);
                    break;
                case CMDQ_CODE_EOC:
                    length += sprintf(&pPage[length], "MVA: 0x%08x, EOC 0x%08x\n", MVABase, pCMD[0]);
                    break;
                default:
                    length += sprintf(&pPage[length], "MVA: 0x%08x, 0x%08x:0x%08x\n", MVABase, pCMD[1], pCMD[0]);
                    break;
            }

            curPos = begin + length;
            if (curPos < offset)
            {
                length = 0;
                begin  = curPos;
            }

            if (curPos > (offset + count))
            {
                *pEOF  = 0x0;
                isFull = 1;
                break;
            }
        }
    #endif // 0
    }
    spin_unlock_irqrestore(&gCmdqExecLock, flags); 

    *ppStart  = pPage + (offset - begin);
    length   -= (offset - begin);

    return (length > count)? count: length;
}


static int32_t cmdq_read_record_proc(char    *pPage,
                                     char    **ppStart,
                                     off_t   offset,
                                     int32_t count,
                                     int32_t *pEOF,
                                     void    *pData)
{
    u_long       flags;
    int32_t      begin;
    int32_t      curPos;
    int32_t      length;
    int32_t      index;
    int32_t      numRec;
    RecordStruct *pRecord;
    int32_t      IRQTime;
    int32_t      execTime;
    int32_t      totalTime;

    begin   = 0;
    curPos  = 0;
    length  = 0;
    *pEOF   = 0x1;

    spin_lock_irqsave(&gCmdqRecordLock, flags);
    smp_mb();

    numRec  = gCmdqContext.recNum;
    if (numRec >= CMDQ_MAX_RECORD_COUNT)
    {
        index = gCmdqContext.lastID;
    }
    else
    {
        index = gCmdqContext.lastID - numRec;
        if (index < 0)
        {
            index = CMDQ_MAX_RECORD_COUNT - index;
        }
    }

    for (;numRec > 0; numRec--, index++)
    {
        if (index >= CMDQ_MAX_RECORD_COUNT)
        {
            index = 0;
        }
        
        pRecord = &(gCmdqContext.record[index]);

        CMDQ_GET_TIME_DURATION(pRecord->trigger, pRecord->gotIRQ, IRQTime)
        CMDQ_GET_TIME_DURATION(pRecord->trigger, pRecord->wakedUp, execTime);
        CMDQ_GET_TIME_DURATION(pRecord->start,   pRecord->done, totalTime);

        length += sprintf(&pPage[length], "Index: %d, Scenario: %d, Priority: %d, Reorder: %d, trigger: %d:%d IRQ Time: %d, Exec Time: %d, Total Time: %d\n",
            index, pRecord->scenario, pRecord->priority, pRecord->reorder, (uint32_t)pRecord->trigger.tv_sec, (uint32_t)pRecord->trigger.tv_nsec, IRQTime, execTime, totalTime);

        curPos = begin + length;
        if (curPos < offset)
        {
            length = 0;
            begin  = curPos;
        }

        if (curPos > (offset + count))
        {
            *pEOF = 0x0;
            break;
        }
    }

    spin_unlock_irqrestore(&gCmdqRecordLock, flags); 

    *ppStart  = pPage + (offset - begin);
    length   -= (offset - begin);

    return (length > count)? count: length;
}


void cmdqInitialize()
{
    //u_long                flags;
    uint8_t               *pVABase;
    uint32_t              MVABase;
    EngineStruct          *pEngine;
    TaskStruct            *pTask;
    int32_t               index;
    struct proc_dir_entry *pEntry; 

    spin_lock_init(&gCmdqTaskLock);
    spin_lock_init(&gCmdqThreadLock);
    spin_lock_init(&gCmdqExecLock);
    spin_lock_init(&gCmdqRecordLock);

    //spin_lock_irqsave(&gCmdqTaskLock, flags); 
    //smp_mb();

    pVABase = dma_alloc_coherent(NULL, CMDQ_MAX_DMA_BUF_SIZE, &MVABase, GFP_ATOMIC);
    if(NULL == pVABase)
    {
        CMDQ_AEE("CMDQ", "Allocate command buffer failed\n");
        //spin_unlock_irqrestore(&gCmdqTaskLock, flags); 
        return;
    }

    memset(pVABase, 0, CMDQ_MAX_DMA_BUF_SIZE);
    CMDQ_MSG("Command buffer VA:%x PA:%x \n", (uint32_t)pVABase, MVABase);

    for (index = 0; index < CMDQ_MAX_THREAD_COUNT; index++)
    {
        init_waitqueue_head(&gCmdWaitQueue[index]);
    }

    init_completion(&gCmdqComplete);

    // Reset overall context
    memset(&gCmdqContext, 0x0, sizeof(ContextStruct));

    // Reset engine status
    pEngine = &(gCmdqContext.engine[0]);
    for (index = 0; index < CMDQ_MAX_ENGINE_COUNT; index++)
    {
        pEngine[index].currOwner = CMDQ_INVALID_THREAD;
    }

    // Reset task status
    INIT_LIST_HEAD(&gCmdqFreeTask);
    pTask = &(gCmdqContext.taskInfo[0]);
    for (index = 0; index < CMDQ_MAX_FIXED_TASK; index++)
    {
        INIT_LIST_HEAD(&(pTask[index].listEntry));

        pTask[index].taskType  = TASK_TYPE_FIXED;
        pTask[index].taskState = TASK_STATE_IDLE;
        pTask[index].pVABase   = (uint32_t*)pVABase;
        pTask[index].MVABase   = MVABase;
        pTask[index].bufSize   = CMDQ_MAX_BLOCK_SIZE;
        list_add_tail(&(pTask[index].listEntry), &gCmdqFreeTask);

        pVABase += CMDQ_MAX_BLOCK_SIZE;
        MVABase += CMDQ_MAX_BLOCK_SIZE;
    }

    // Clear CMDQ event for engines
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x14);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x15);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x16);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x17);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x18);

    // Mout proc entry for debug
    gCmdqProcEntry = proc_mkdir("cmdq", NULL);
    if (NULL != gCmdqProcEntry)
    {
        pEntry = create_proc_entry("status", 0660,gCmdqProcEntry);    
        if (NULL != pEntry)
        {
            pEntry->read_proc = cmdq_read_status_proc;
        }

        pEntry = create_proc_entry("error", 0660, gCmdqProcEntry);    
        if (NULL != pEntry)
        {
            pEntry->read_proc = cmdq_read_error_proc;
        }

        pEntry = create_proc_entry("record", 0660, gCmdqProcEntry);    
        if (NULL != pEntry)
        {
            pEntry->read_proc = cmdq_read_record_proc;
        }
    }

    //spin_unlock_irqrestore(&gCmdqTaskLock, flags); 
}


int32_t cmdqSuspendTask()
{
    u_long       flags;
    EngineStruct *pEngine;

    pEngine = &(gCmdqContext.engine[0]);

    spin_lock_irqsave(&gCmdqThreadLock, flags); 
    smp_mb();

    if((0 != pEngine[tIMGI].userCount) ||
       (0 != pEngine[tRDMA0].userCount) ||
       (0 != pEngine[tSCL0].userCount) ||
       (0 != pEngine[tSCL1].userCount) ||
       (0 != pEngine[tTDSHP].userCount) ||
       (0 != pEngine[tWROT].userCount) ||
       (0 != pEngine[tWDMA1].userCount))
    {
        spin_unlock_irqrestore(&gCmdqThreadLock, flags); 
        return -EFAULT;
    }

    spin_unlock_irqrestore(&gCmdqThreadLock, flags); 

    return 0;
}


int32_t cmdqResumeTask()
{
    u_long flags;

    spin_lock_irqsave(&gCmdqThreadLock, flags); 

    // Clear CMDQ event for engines
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x14);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x15);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x16);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x17);
    DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_UPDATE, 0x18);

    spin_unlock_irqrestore(&gCmdqThreadLock, flags); 

    return 0;
}


void cmdq_dump_task_usage_impl(TaskStruct *pTask)
{
    if(NULL == pTask)
    {
        return;
    }

    printk("pTask: 0x%08x, State: %d, VABase: 0x%08x, MVABase: 0x%08x, Size: %d\n",
        pTask, pTask->taskState, (uint32_t)pTask->pVABase, pTask->MVABase, pTask->blockSize);
    printk("SCE %d, PRI: %d, ENG: 0x%08x, VAEnd: 0x%08x\n",
        pTask->scenario, pTask->priority, pTask->engineFlag, (uint32_t)pTask->pCMDEnd);
    printk("Reorder: %d, Trigger %d:%d, IRQ: %d:%d, Wake Up: %d:%d\n",
        pTask->reorder,
        (uint32_t)pTask->trigger.tv_sec, (uint32_t)pTask->trigger.tv_nsec,
        (uint32_t)pTask->gotIRQ.tv_sec, (uint32_t)pTask->gotIRQ.tv_nsec, 
        (uint32_t)pTask->wakedUp.tv_sec, (uint32_t)pTask->wakedUp.tv_nsec);
}


static void cmdq_dump_task_usage(void)
{
    int32_t    index;
    TaskStruct *pTask;

    for (index = 0; index < CMDQ_MAX_FIXED_TASK; index++)
    {
        pTask = &(gCmdqContext.taskInfo[index]);
        if (TASK_STATE_IDLE != pTask->taskState)
        {
            printk("====== Task %d Usage =======\n", index);
            cmdq_dump_task_usage_impl(pTask);
        }
    }
}


static TaskStruct* cmdq_acquire_task(int32_t  scenario,
                                     int32_t  priority,
                                     uint32_t engineFlag,
                                     void     *pCMDBlock,
                                     uint32_t blockSize)
{
    u_long     flags;
    TaskStruct *pTask;
    uint32_t   *pVABase;
    uint32_t   MVABase;

    CMDQ_MSG("Allocate task structure begin\n");

    CMDQ_MSG("Got task flag 0x%08x, CMD 0x%08x, size %d\n", engineFlag, (uint32_t)pCMDBlock, blockSize);

    pTask = NULL;

    spin_lock_irqsave(&gCmdqTaskLock, flags); 
    smp_mb();

    if (list_empty(&gCmdqFreeTask) ||
        (blockSize >= CMDQ_MAX_BLOCK_SIZE))
    {
        spin_unlock_irqrestore(&gCmdqTaskLock, flags); 

        pTask = (TaskStruct*)kmalloc(sizeof(TaskStruct), GFP_KERNEL);
        if (NULL != pTask)
        {
            pVABase = dma_alloc_coherent(NULL, blockSize + CMDQ_EXTRA_MARGIN, &MVABase, GFP_ATOMIC);
            if (NULL == pVABase)
            {
                kfree(pTask);
                pTask = NULL;
                CMDQ_ERR("Can't allocate DMA buffer\n");
            }
            else
            {
                pTask->pVABase   = pVABase;
                pTask->MVABase   = MVABase;
                pTask->bufSize   = blockSize + CMDQ_EXTRA_MARGIN;
                pTask->taskType  = TASK_TYPE_DYNAMIC;
            }
        }
    }
    else
    {
        pTask = list_first_entry(&(gCmdqFreeTask), TaskStruct, listEntry);
        list_del(&(pTask->listEntry));
        spin_unlock_irqrestore(&gCmdqTaskLock, flags);
    }
    
    if (NULL != pTask)
    {
        pTask->scenario   = scenario;
        pTask->priority   = priority;
        pTask->engineFlag = engineFlag;
        pTask->pCMDEnd    = pTask->pVABase + (blockSize >> 2) - 1;
        pTask->blockSize  = blockSize;
        pTask->firstEOC   = pTask->MVABase + blockSize - 16;
        pTask->taskState  = TASK_STATE_BUSY;
        pTask->reorder    = 0;

        memset(&(pTask->trigger), 0x0, sizeof(struct timespec));
        memset(&(pTask->gotIRQ), 0x0, sizeof(struct timespec));
        memset(&(pTask->wakedUp), 0x0, sizeof(struct timespec));

        CMDQ_MSG("Copy command buffer begin\n");

        if (copy_from_user(pTask->pVABase, pCMDBlock, blockSize))
        {
            if (TASK_TYPE_DYNAMIC != pTask->taskType)
            {
                spin_lock_irqsave(&gCmdqTaskLock, flags); 
                list_add_tail(&(pTask->listEntry), &gCmdqFreeTask);
                spin_unlock_irqrestore(&gCmdqTaskLock, flags); 
            }

            CMDQ_AEE("CMDQ", "Copy commands buffer failed\n");
            CMDQ_ERR("Source: 0x%08x, target: 0x%08x, size: %d\n", (uint32_t)pCMDBlock, (uint32_t)pTask->pVABase, blockSize);

            return NULL;
        }

        CMDQ_MSG("Copy command buffer end\n");
    }
    else
    {
        CMDQ_AEE("CMDQ", "Can't acquire task info\n");
        cmdq_dump_task_usage();
    }

    CMDQ_MSG("Allocate task structure end\n");

    return pTask;
}


static void cmdq_enable_clock(uint32_t engineFlag,
                              int32_t  thread)
{
    EngineStruct *pEngine;

    CMDQ_MSG("Enable hardware clock begin\n");

    pEngine = &(gCmdqContext.engine[0]);

    if((0 == pEngine[tIMGI].userCount) &&
       (0 == pEngine[tRDMA0].userCount) &&
       (0 == pEngine[tSCL0].userCount) &&
       (0 == pEngine[tSCL1].userCount) &&
       (0 == pEngine[tTDSHP].userCount) &&
       (0 == pEngine[tWROT].userCount) &&
       (0 == pEngine[tWDMA1].userCount))
    {
        enable_clock(MT_CG_DISP0_SMI_COMMON, "SMI_COMMON");
        enable_clock(MT_CG_DISP0_SMI_LARB0, "SMI_LARB0");
        enable_clock(MT_CG_DISP0_MM_CMDQ, "MM_CMDQ");
        enable_clock(MT_CG_DISP0_MUTEX, "MUTEX");
    }

    if (engineFlag & (0x1 << tIMGI))
    {
        if(!clock_is_on(MT_CG_DISP0_CAM_MDP))
        {
            enable_clock(MT_CG_IMAGE_CAM_SMI, "CAMERA");
            enable_clock(MT_CG_IMAGE_CAM_CAM, "CAMERA");
            enable_clock(MT_CG_IMAGE_SEN_TG,  "CAMERA");
            enable_clock(MT_CG_IMAGE_SEN_CAM, "CAMERA");
            enable_clock(MT_CG_IMAGE_LARB2_SMI, "CAMERA");

            enable_clock(MT_CG_DISP0_CAM_MDP, "CAM_MDP"); 
        
            pEngine[tIMGI].currOwner = thread;
        }

        pEngine[tIMGI].userCount++;
    }
    
    if (engineFlag & (0x1 << tRDMA0))
    {
        if(!clock_is_on(MT_CG_DISP0_MDP_RDMA))
        {
            enable_clock(MT_CG_DISP0_MDP_RDMA, "MDP_RDMA");
        
            pEngine[tRDMA0].currOwner = thread;
        }

        pEngine[tRDMA0].userCount++;
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        if(!clock_is_on(MT_CG_DISP0_MDP_RSZ0))
        {
            enable_clock(MT_CG_DISP0_MDP_RSZ0, "MDP_RSZ0");
        
            pEngine[tSCL0].currOwner = thread;
        }

        pEngine[tSCL0].userCount++;
    }

    if (engineFlag & (0x1 << tSCL1))
    {
        if(!clock_is_on(MT_CG_DISP0_MDP_RSZ1))
        {
            enable_clock(MT_CG_DISP0_MDP_RSZ1, "MDP_RSZ1");
        
            pEngine[tSCL1].currOwner = thread;
        }

        pEngine[tSCL1].userCount++;
    }

    if (engineFlag & (0x1 << tTDSHP))
    {
        if(!clock_is_on(MT_CG_DISP0_MDP_TDSHP))
        {
            enable_clock(MT_CG_DISP0_MDP_TDSHP, "MDP_TDSHP");

            pEngine[tTDSHP].currOwner = thread;
        }

        pEngine[tTDSHP].userCount++;
    }

    if (engineFlag & (0x1 << tWROT))
    {
        if(!clock_is_on(MT_CG_DISP0_MDP_WROT))
        {
            enable_clock(MT_CG_DISP0_MDP_WROT, "MDP_WROT");

            pEngine[tWROT].currOwner = thread;
        }

        pEngine[tWROT].userCount++;
    }

    if (engineFlag & (0x1 << tWDMA1))
    {
        if(!clock_is_on(MT_CG_DISP0_MDP_WDMA))
        {
            enable_clock(MT_CG_DISP0_MDP_WDMA, "MDP_WDMA");
        
            pEngine[tWDMA1].currOwner = thread;
        }

        pEngine[tWDMA1].userCount++;
    }

    CMDQ_MSG("Enable hardware clock end\n");
}


static int32_t cmdq_acquire_thread(uint32_t engineFlag)
{
    EngineStruct *pEngine;
    ThreadStruct *pThread;
    u_long       flags;
    uint32_t     engine;
    uint32_t     free;
    int32_t      index;
    int32_t      thread;

    pEngine = &(gCmdqContext.engine[0]);
    pThread = &(gCmdqContext.thread[0]);
    do
    {
        spin_lock_irqsave(&gCmdqThreadLock, flags); 
        smp_mb();

        // Default values
        engine = engineFlag;
        free   = 0xFFFFFFFF;
        thread = CMDQ_INVALID_THREAD;

        for (index = 0; ((index < tTotal) && (engine != 0)); index++)
        {
            if (engine & (0x1 << index))
            {               
                if (CMDQ_INVALID_THREAD == pEngine[index].currOwner)
                {
                    continue;
                }
                else if (CMDQ_INVALID_THREAD == thread)
                {
                    thread = pEngine[index].currOwner;
                    free   &= ~(0x1 << thread);
                }
                else if (thread != pEngine[index].currOwner)
                {
                    thread = CMDQ_INVALID_THREAD;
                    spin_unlock_irqrestore(&gCmdqThreadLock, flags);
                                       
                    wait_for_completion(&gCmdqComplete);
                    break;
                }
                engine &= ~(0x1 << index);
            }
        }
        
        if ((0xFFFFFFFF == free) &&
            (CMDQ_INVALID_THREAD == thread))
        {
            thread = 0;
            break;
        }

    } while(index == CMDQ_INVALID_THREAD);

    cmdq_enable_clock(engineFlag,
                      thread);

    spin_unlock_irqrestore(&gCmdqThreadLock, flags);

    return thread;
}


static int32_t cmdq_disable_clock(uint32_t engineFlag)
{
    EngineStruct *pEngine;
    int32_t      loopCount;

    CMDQ_MSG("Disable hardware clock begin\n");

    pEngine = &(gCmdqContext.engine[0]);
    
    if (engineFlag & (0x1 << tWDMA1))
    {
        pEngine[tWDMA1].userCount--;
        if((pEngine[tWDMA1].userCount <= 0) &&
           clock_is_on(MT_CG_DISP0_MDP_WDMA))
        {
            DISP_REG_SET(0xF400400C, 0x1);

            loopCount = 0;
            while(loopCount < CMDQ_MAX_LOOP_COUNT)
            {
                if (0x1 == (DISP_REG_GET(0xF40040A0) & 0x3FF))
                {
                    break;
                }

                loopCount++;
            }
        
            if (loopCount >= CMDQ_MAX_LOOP_COUNT)
            {
                CMDQ_ERR("Reset WDMA engine failed\n");
                return -EFAULT;
            }
        
            DISP_REG_SET(0xF400400C, 0x0);

            disable_clock(MT_CG_DISP0_MDP_WDMA, "MDP_WDMA");
            pEngine[tWDMA1].currOwner = CMDQ_INVALID_THREAD;
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        pEngine[tWROT].userCount--;
        if((pEngine[tWROT].userCount <= 0) &&
           clock_is_on(MT_CG_DISP0_MDP_WROT))
        {
            DISP_REG_SET(0xF4005010, 0x1);

            loopCount = 0;
            while(loopCount < CMDQ_MAX_LOOP_COUNT)
            {
                if (0x0 == (DISP_REG_GET(0xF4005014) & 0x1))
                {
                    break;
                }

                loopCount++;
            }

            if (loopCount >= CMDQ_MAX_LOOP_COUNT)
            {
                CMDQ_ERR("Reset WROT engine failed\n");
                return -EFAULT;
            }

            DISP_REG_SET(0xF4005010, 0x0);
            
            disable_clock(MT_CG_DISP0_MDP_WROT, "MDP_WROT");
            pEngine[tWROT].currOwner = CMDQ_INVALID_THREAD;
        }
    }

    if (engineFlag & (0x1 << tTDSHP))
    {
        pEngine[tTDSHP].userCount--;
        if((pEngine[tTDSHP].userCount <= 0) &&
           clock_is_on(MT_CG_DISP0_MDP_TDSHP))
        {
            DISP_REG_SET(0xF4006100, 0x0);
            DISP_REG_SET(0xF4006100, 0x2);
            DISP_REG_SET(0xF4006100, 0x0);

            disable_clock(MT_CG_DISP0_MDP_TDSHP, "MDP_TDSHP");
            pEngine[tTDSHP].currOwner = CMDQ_INVALID_THREAD;
        }
    }

    if (engineFlag & (0x1 << tSCL1))
    {
        pEngine[tSCL1].userCount--;
        if((pEngine[tSCL1].userCount <= 0) &&
           clock_is_on(MT_CG_DISP0_MDP_RSZ1))
        {
            DISP_REG_SET(0xF4003000, 0x0);
            DISP_REG_SET(0xF4003000, 0x10000);
            DISP_REG_SET(0xF4003000, 0x0);

            disable_clock(MT_CG_DISP0_MDP_RSZ1, "MDP_RSZ1");
            pEngine[tSCL1].currOwner = CMDQ_INVALID_THREAD;
        }
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        pEngine[tSCL0].userCount--;
        if((pEngine[tSCL0].userCount <= 0) &&
           clock_is_on(MT_CG_DISP0_MDP_RSZ0))
        {
            DISP_REG_SET(0xF4002000, 0x0);
            DISP_REG_SET(0xF4002000, 0x10000);
            DISP_REG_SET(0xF4002000, 0x0);

            disable_clock(MT_CG_DISP0_MDP_RSZ0, "MDP_RSZ0");
            pEngine[tSCL0].currOwner = CMDQ_INVALID_THREAD;
        }
    }

    if (engineFlag & (0x1 << tRDMA0))
    {
        pEngine[tRDMA0].userCount--;

        if((pEngine[tRDMA0].userCount <= 0) &&
           clock_is_on(MT_CG_DISP0_MDP_RDMA))
        {
            DISP_REG_SET(0xF4001008, 0x1);

            loopCount = 0;
            while(loopCount < CMDQ_MAX_LOOP_COUNT)
            {
                if (0x100 == (DISP_REG_GET(0xF4001408) & 0x7FF00))
                {
                    break;
                }
                loopCount++;
            }

            if (loopCount >= CMDQ_MAX_LOOP_COUNT)
            {
                CMDQ_ERR("Reset RDMA engine failed\n");
                return -EFAULT;
            }

            DISP_REG_SET(0xF4001008, 0x0);            
            
            disable_clock(MT_CG_DISP0_MDP_RDMA, "MDP_RDMA");
            pEngine[tRDMA0].currOwner = CMDQ_INVALID_THREAD;
        }
    }

    if (engineFlag & (0x1 << tIMGI))
    {
        pEngine[tIMGI].userCount--;
        if(pEngine[tIMGI].userCount <= 0)
        {
            disable_clock(MT_CG_DISP0_CAM_MDP, "CAM_MDP");

            disable_clock(MT_CG_IMAGE_CAM_SMI, "CAMERA");
            disable_clock(MT_CG_IMAGE_CAM_CAM, "CAMERA");
            disable_clock(MT_CG_IMAGE_SEN_TG,  "CAMERA");
            disable_clock(MT_CG_IMAGE_SEN_CAM, "CAMERA");
            disable_clock(MT_CG_IMAGE_LARB2_SMI, "CAMERA");
            
            pEngine[tIMGI].currOwner = CMDQ_INVALID_THREAD;
        }
    }

    if((0 == pEngine[tIMGI].userCount) &&
       (0 == pEngine[tRDMA0].userCount) &&
       (0 == pEngine[tSCL0].userCount) &&
       (0 == pEngine[tSCL1].userCount) &&
       (0 == pEngine[tTDSHP].userCount) &&
       (0 == pEngine[tWROT].userCount) &&
       (0 == pEngine[tWDMA1].userCount))
    {
        disable_clock(MT_CG_DISP0_MUTEX, "MUTEX");
        disable_clock(MT_CG_DISP0_MM_CMDQ, "MM_CMDQ");
        disable_clock(MT_CG_DISP0_SMI_LARB0, "SMI_LARB0");
        disable_clock(MT_CG_DISP0_SMI_COMMON, "SMI_COMMON");
    }    

    CMDQ_MSG("Disable hardware clock end\n");

    return 0;
}


static void cmdq_release_thread(int32_t   thread,
                                uint32_t  engineFlag)
{
    int32_t status;
    u_long  flags;

    spin_lock_irqsave(&gCmdqThreadLock, flags);
   
    status = cmdq_disable_clock(engineFlag);

    spin_unlock_irqrestore(&gCmdqThreadLock, flags);

    if (-EFAULT == status)
    {
        CMDQ_AEE("CMDQ", "Can't disable clock\n");
    }

    complete_all(&gCmdqComplete);
}


static int32_t cmdq_reset_hw_engine(int32_t engineFlag)
{
    EngineStruct *pEngine;
    int32_t      loopCount;
    uint32_t     regValue;

    printk("Reset hardware engine begin\n");

    pEngine = &(gCmdqContext.engine[0]);

    if (engineFlag & (0x01 << tIMGI))
    {
        printk("Reset ISP pass2 start\n");
      
        if(NULL != g_CMDQ_CB_Array.cmdqReset_cb[cbISP])
        {
            g_CMDQ_CB_Array.cmdqReset_cb[cbISP](0);
        }

        printk("Reset ISP pass2 end\n");
    }

    if (engineFlag & (0x1 << tRDMA0))
    {
        if(clock_is_on(MT_CG_DISP0_MDP_RDMA))
        {
            DISP_REG_SET(0xF4001008, 0x1);
        
            loopCount = 0;
            while(loopCount < CMDQ_MAX_LOOP_COUNT)
            {
                if (0x100 == (DISP_REG_GET(0xF4001408) & 0x7FF00))
                {
                    break;
                }

                loopCount++;
            }

            if (loopCount >= CMDQ_MAX_LOOP_COUNT)
            {
                CMDQ_ERR("Reset RDMA engine failed\n");
                return -EFAULT;
            }

            pEngine[tRDMA0].resetCount++;

            DISP_REG_SET(0xF4001008, 0x0);
        }
    }

    if (engineFlag & (0x1 << tSCL0))
    {
        if(clock_is_on(MT_CG_DISP0_MDP_RSZ0))
        {
            DISP_REG_SET(0xF4002000, 0x0);
            DISP_REG_SET(0xF4002000, 0x10000);
            DISP_REG_SET(0xF4002000, 0x0);
        
            pEngine[tSCL0].resetCount++;
        }
    }

    if (engineFlag & (0x1 << tSCL1))
    {
        if(clock_is_on(MT_CG_DISP0_MDP_RSZ1))
        {
            DISP_REG_SET(0xF4003000, 0x0);
            DISP_REG_SET(0xF4003000, 0x10000);
            DISP_REG_SET(0xF4003000, 0x0);
        
            pEngine[tSCL1].resetCount++;
        }
    }

    if (engineFlag & (0x1 << tTDSHP))
    {
        if(clock_is_on(MT_CG_DISP0_MDP_TDSHP))
        {
            DISP_REG_SET(0xF4006100, 0x0);
            DISP_REG_SET(0xF4006100, 0x2);
            DISP_REG_SET(0xF4006100, 0x0);
        
            pEngine[tTDSHP].resetCount++;
        }
    }

    if (engineFlag & (0x1 << tWROT))
    {
        if(clock_is_on(MT_CG_DISP0_MDP_WROT))
        {
            DISP_REG_SET(0xF4005010, 0x1);

            loopCount = 0;
            while(loopCount < CMDQ_MAX_LOOP_COUNT)
            {
                if (0x0 == (DISP_REG_GET(0xF4005014) & 0x1))
                {
                    break;
                }

                loopCount++;
            }

            DISP_REG_SET(0xF4005010, 0x0);

            if (loopCount >= CMDQ_MAX_LOOP_COUNT)
            {
                CMDQ_ERR("WROT 2nd pass reset start\n");
                
                regValue = DISP_REG_GET(0xF4000138);
                regValue &= 0xFFFFFBFF;
                DISP_REG_SET(0xF4000138, regValue);

                loopCount = 0;
                while(loopCount < CMDQ_MAX_LOOP_COUNT)
                {
                    loopCount++;
                }

                CMDQ_ERR("WROT 2nd pass reset count %d\n", loopCount);

                regValue |= 0x00000400;
                DISP_REG_SET(0xF4000138, regValue);
            
                if (0x0 != (DISP_REG_GET(0xF4005014) & 0x1))
                {
                    CMDQ_ERR("Reset WROT engine failed\n");
                    return -EFAULT;
                }
            
                CMDQ_ERR("WROT 2nd pass reset end\n");
            }

            pEngine[tWROT].resetCount++;
        }
    }

    if (engineFlag & (0x1 << tWDMA1))
    {
        if(clock_is_on(MT_CG_DISP0_MDP_WDMA))
        {
            DISP_REG_SET(0xF400400C, 0x1);

            loopCount = 0;
            while(loopCount < CMDQ_MAX_LOOP_COUNT)
            {
                if (0x1 == (DISP_REG_GET(0xF40040A0) & 0x3FF))
                {
                    break;
                }

                loopCount++;
            }
        
            if (loopCount >= CMDQ_MAX_LOOP_COUNT)
            {
                CMDQ_ERR("Reset WDMA engine failed\n");
                return -EFAULT;
            }

            pEngine[tWDMA1].resetCount++;

            DISP_REG_SET(0xF400400C, 0x0);
        }
    }

    printk("Reset hardware engine end\n");

    return 0;
}


void cmdq_dump_mmsys()
{
    printk(KERN_DEBUG "=============== [CMDQ] MM ===============\n");
    printk(KERN_DEBUG "CAM_MDP_MOUT_EN: 0x%08x, MDP_RDMA_MOUT_EN: 0x%08x, MDP_RSZ0_MOUT_EN: 0x%08x\n", 
        DISP_REG_GET(0xF4000000 + 0x01c), 
        DISP_REG_GET(0xF4000000 + 0x020), 
        DISP_REG_GET(0xF4000000 + 0x024));
    printk(KERN_DEBUG "MDP_RSZ1_MOUT_EN: 0x%08x, MDP_TDSHP_MOUT_EN: 0x%08x, DISP_OVL_MOUT_EN: 0x%08x\n", 
        DISP_REG_GET(0xF4000000 + 0x028), 
        DISP_REG_GET(0xF4000000 + 0x02c), 
        DISP_REG_GET(0xF4000000 + 0x030));
    printk(KERN_DEBUG "MDP_RSZ0_SEL: 0x%08x, MDP_RSZ1_SEL: 0x%08x, MDP_TDSHP_SEL: 0x%08x\n", 
        DISP_REG_GET(0xF4000000 + 0x038), 
        DISP_REG_GET(0xF4000000 + 0x03c), 
        DISP_REG_GET(0xF4000000 + 0x040));
    printk(KERN_DEBUG "MDP_WROT_SEL: 0x%08x, MDP_WDMA_SEL: 0x%08x, DISP_OUT_SEL: 0x%08x\n", 
        DISP_REG_GET(0xF4000000 + 0x044), 
        DISP_REG_GET(0xF4000000 + 0x048), 
        DISP_REG_GET(0xF4000000 + 0x04c));    
    printk(KERN_DEBUG "MMSYS_DL_VALID_0: 0x%08x, MMSYS_DL_VALID_1: 0x%08x, MMSYS_DL_READY0: 0x%08x, MMSYS_DL_READY1: 0x%08x\n",
        DISP_REG_GET(0xF4000000 + 0x860), 
        DISP_REG_GET(0xF4000000 + 0x864), 
        DISP_REG_GET(0xF4000000 + 0x868), 
        DISP_REG_GET(0xF4000000 + 0x86c));  
}


static void cmdq_attach_error_task(TaskStruct *pTask,
                                   int32_t    thread)
{
    ThreadStruct *pThread;
    int32_t      index;
#if 0
    int32_t      inner;
#endif // 0
    EngineStruct *pEngine;
    uint32_t     engineFlag;
    ErrorStruct  *pError;
    uint32_t     value[40];

    if (NULL != pTask)
    {
        printk(KERN_DEBUG "=============== [CMDQ] Task Status ===============\n");
        printk(KERN_DEBUG "Task: 0x%08x, Scenario: %d, State: %d, Priority: %d, Reorder: %d, Flag: 0x%08x, VABase: 0x%08x\n",
            (uint32_t)pTask, pTask->scenario, pTask->taskState, pTask->priority, pTask->reorder, pTask->engineFlag, (uint32_t)pTask->pVABase);
        printk(KERN_DEBUG "CMDEnd: 0x%08x, MVABase: 0x%08x, Size: %d, Last Inst: 0x%08x:0x%08x, 0x%08x:0x%08x\n",
            (uint32_t)pTask->pCMDEnd, pTask->MVABase, pTask->blockSize, pTask->pCMDEnd[-3], pTask->pCMDEnd[-2], pTask->pCMDEnd[-1], pTask->pCMDEnd[0]);
        printk(KERN_DEBUG "Trigger: %d:%d, Got IRQ: %d:%d, Finish: %d:%d\n", (uint32_t)pTask->trigger.tv_sec, (uint32_t)pTask->trigger.tv_nsec,
            (uint32_t)pTask->gotIRQ.tv_sec, (uint32_t)pTask->gotIRQ.tv_nsec, (uint32_t)pTask->wakedUp.tv_sec, (uint32_t)pTask->wakedUp.tv_nsec);
    }

    cmdq_dump_mmsys();

    printk(KERN_DEBUG "=============== [CMDQ] SMI Status ===============\n");

    for (index = 0; index < 5; index++)
    {
        printk(KERN_DEBUG "=============== [CMDQ] SMI Dump %d ===============\n", index);
        //SMI Dump
        smi_dumpDebugMsg();
    }

    printk(KERN_DEBUG "=============== [CMDQ] Thread Status ===============\n");

    value[0] = DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(thread));
    value[1] = DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(thread));
    value[2] = DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(thread));
    value[3] = DISP_REG_GET(DISP_REG_CMDQ_THRx_WAIT_EVENTS0(thread));
    value[4] = DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(thread)) & 0x0FFFF;

    pThread = &(gCmdqContext.thread[thread]);

    printk(KERN_DEBUG "Enabled: %d, Thread PC: 0x%08x, End: 0x%08x, Wait Event: 0x%02x, Curr Cookie: %d, Wait Cookie: %d, Next Cookie: %d, Task Count %d\n",
        value[0], value[1], value[2], value[3], value[4], pThread->waitCookie, pThread->nextCookie, pThread->taskCount);

    for (index = 0; index < CMDQ_MAX_TASK_COUNT; index++)
    {
        if (NULL != pThread->pCurTask[index])
        {
            printk(KERN_DEBUG "Slot %d, Task: 0x%08x, VABase: 0x%08x, MVABase 0x%08x, Size: %d, Last Inst 0x%08x:0x%08x, 0x%08x:0x%08x\n", 
                index, (uint32_t)(pThread->pCurTask[index]), (uint32_t)(pThread->pCurTask[index]->pVABase), pThread->pCurTask[index]->MVABase, pThread->pCurTask[index]->blockSize,
                pThread->pCurTask[index]->pCMDEnd[-3], pThread->pCurTask[index]->pCMDEnd[-2], pThread->pCurTask[index]->pCMDEnd[-1], pThread->pCurTask[index]->pCMDEnd[0]);
        }
    }

    printk(KERN_DEBUG "=============== [CMDQ] Mutex Status ===============\n");
    value[0] = DISP_REG_GET(0xF400E004);
    value[1] = DISP_REG_GET(0xF400E00C);
    printk(KERN_DEBUG "[CMDQ] DISP_MUTEX_INTSTA: 0x%08x, DISP_REG_COMMIT: 0x%08x\n", value[0], value[1]);

    for (index = 0; index < 8; index++)
    {        
        printk(KERN_DEBUG "[CMDQ] DISP_MUTEX%d_RST: 0x%08x\n", index, DISP_REG_GET(DISP_REG_CONFIG_MUTEX_RST(index)));
        printk(KERN_DEBUG "[CMDQ] DISP_MUTEX%d_MOD: 0x%08x\n", index, DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(index)));
    }

    printk(KERN_DEBUG "=============== [CMDQ] CMDQ Status ===============\n");
    printk(KERN_DEBUG "[CMDQ] CMDQ_DEBUG_1: 0x%08x\n", DISP_REG_GET((DISPSYS_CMDQ_BASE + 0x70)));
    printk(KERN_DEBUG "[CMDQ] CMDQ_DEBUG_2: 0x%08x\n", DISP_REG_GET((DISPSYS_CMDQ_BASE + 0x74)));
    printk(KERN_DEBUG "[CMDQ] CMDQ_DEBUG_3: 0x%08x\n", DISP_REG_GET((DISPSYS_CMDQ_BASE + 0x78)));
    printk(KERN_DEBUG "[CMDQ] CMDQ_DEBUG_4: 0x%08x\n", DISP_REG_GET((DISPSYS_CMDQ_BASE + 0x7C)));

    printk(KERN_DEBUG "ISPSys clock state %d\n", subsys_is_on(SYS_ISP));
    printk(KERN_DEBUG "DisSys clock state %d\n", subsys_is_on(SYS_DIS));

    printk(KERN_DEBUG "=============== [CMDQ] Clock Status ===============\n");

    value[0] = DISP_REG_GET(0xF4000100);
    value[1] = DISP_REG_GET(0xF4000110);
    printk(KERN_DEBUG "[CMDQ] MMSYS_CG_CON0: 0x%08x, MMSYS_CG_CON1: 0x%08x\n", value[0], value[1]);

    if (NULL != pTask)
    {
        pEngine    = &(gCmdqContext.engine[0]);
        engineFlag = pTask->engineFlag;

        // Setup error info
        pError = &(gCmdqContext.error[0]);

        // CMDQ_MAX_ERROR_COUNT is 1 now, replace the old one instead of using gCmdqContext.errNum as the index init value
        // otherwise, the corruption may happen while there are more than one error tasks
        index  = (CMDQ_MAX_ERROR_COUNT <= gCmdqContext.errNum) ? (0) : (gCmdqContext.errNum);

        if (engineFlag & (0x01 << tIMGI))
        {
            value[0] = DISP_REG_GET(CLK_CFG_0);
            value[1] = DISP_REG_GET(CLK_CFG_3);
            value[2] = DISP_REG_GET(0xF5000000);
            printk(KERN_DEBUG "[CMDQ] CLK_CFG_0: 0x%08x, CLK_CFG_3: 0x%08x, ISP_CLK_CG: 0x%08x\n", value[0], value[1], value[2]);

            pEngine[tIMGI].failCount++;

            printk(KERN_DEBUG "=============== [CMDQ] ISP Status ====================================\n");

            if (NULL != g_CMDQ_CB_Array.cmdqTimeout_cb[cbISP])
            {
                g_CMDQ_CB_Array.cmdqTimeout_cb[cbISP](pTask->engineFlag & tIMGI);
            }
        }

        if (engineFlag & (0x01 << tRDMA0))
        {
            pEngine[tRDMA0].failCount++;

        #if 0
            for (inner = 0; inner < 1024; inner += 1)
            {
                pError[index].regVal[tRDMA0][index] = DISP_REG_GET(0xF4001000 + (index << 2));
            }
        #endif // 0

            value[0] = DISP_REG_GET(0xF4001030);
            value[1] = DISP_REG_GET(0xF4001040);
            value[2] = DISP_REG_GET(0xF4001060);
            value[3] = DISP_REG_GET(0xF4001070);
            value[4] = DISP_REG_GET(0xF4001078);
            value[5] = DISP_REG_GET(0xF4001080);
            value[6] = DISP_REG_GET(0xF4001100);
            value[7] = DISP_REG_GET(0xF4001118);
            value[8] = DISP_REG_GET(0xF4001130);
            value[9] = DISP_REG_GET(0xF4001400);
            value[10] = DISP_REG_GET(0xF4001408);
            value[11] = DISP_REG_GET(0xF4001410);
            value[12] = DISP_REG_GET(0xF4001420);
            value[13] = DISP_REG_GET(0xF4001430);
            value[14] = DISP_REG_GET(0xF40014D0);

            printk(KERN_DEBUG "=============== [CMDQ] RDMA Status ====================================\n");
            printk(KERN_DEBUG "[CMDQ] RDMA_SRC_CON: 0x%08x, RDMA_SRC_BASE_0: 0x%08x, RDMA_MF_BKGD_SIZE_IN_BYTE: 0x%08x\n", 
                value[0], value[1], value[2]);
            printk(KERN_DEBUG "[CMDQ] RDMA_MF_SRC_SIZE: 0x%08x, RDMA_MF_CLIP_SIZE: 0x%08x, RDMA_MF_OFFSET_1: 0x%08x\n", 
                value[3], value[4], value[5]);
            printk(KERN_DEBUG "[CMDQ] RDMA_SRC_END_0: 0x%08x, RDMA_SRC_OFFSET_0: 0x%08x, RDMA_SRC_OFFSET_W_0: 0x%08x\n", 
                value[6], value[7], value[8]);
            printk(KERN_DEBUG "[CMDQ] RDMA_MON_STA_0: 0x%08x, RDMA_MON_STA_1: 0x%08x, RDMA_MON_STA_2: 0x%08x\n", 
                value[9], value[10], value[11]);
            printk(KERN_DEBUG "[CMDQ] RDMA_MON_STA_4: 0x%08x, RDMA_MON_STA_6: 0x%08x, RDMA_MON_STA_26: 0x%08x\n", 
                value[12], value[13], value[14]);
        }

        if (engineFlag & (0x01 << tSCL0))
        {
            pEngine[tSCL0].failCount++;

        #if 0
            for (inner = 0; inner < 1024; inner += 1)
            {
                pError[index].regVal[tSCL0][index] = DISP_REG_GET(0xF4002000 + (index << 2));
            }
        #endif // 0

            value[0] = DISP_REG_GET(0xF4002004);
            value[1] = DISP_REG_GET(0xF400200C);
            value[2] = DISP_REG_GET(0xF4002010);
            value[3] = DISP_REG_GET(0xF4002014);
            value[4] = DISP_REG_GET(0xF4002018);
            DISP_REG_SET(0xF4002040, 0x00000001);
            value[5] = DISP_REG_GET(0xF4002044);
            DISP_REG_SET(0xF4002040, 0x00000002);
            value[6] = DISP_REG_GET(0xF4002044);
            DISP_REG_SET(0xF4002040, 0x00000003);
            value[7] = DISP_REG_GET(0xF4002044);

            printk(KERN_DEBUG "=============== [CMDQ] RSZ0 Status ====================================\n");
            printk(KERN_DEBUG "[CMDQ] RSZ0_CONTROL: 0x%08x, RSZ0_INPUT_IMAGE: 0x%08x RSZ0_OUTPUT_IMAGE: 0x%08x\n", 
                value[0], value[1], value[2]);
            printk(KERN_DEBUG "[CMDQ] RSZ0_HORIZONTAL_COEFF_STEP: 0x%08x, RSZ0_VERTICAL_COEFF_STEP: 0x%08x\n", 
                value[3], value[4]);
            printk(KERN_DEBUG "[CMDQ] RSZ0_DEBUG_1: 0x%08x, RSZ0_DEBUG_2: 0x%08x, RSZ0_DEBUG_3: 0x%08x\n", 
                value[5], value[6], value[7]);
        }

        if (engineFlag & (0x01 << tSCL1))
        {
            pEngine[tSCL1].failCount++;

        #if 0
            for (inner = 0; inner < 1024; inner += 1)
            {
                pError[index].regVal[tSCL1][index] = DISP_REG_GET(0xF4003000 + (index << 2));
            }
        #endif // 0

            value[0] = DISP_REG_GET(0xF4003004);
            value[1] = DISP_REG_GET(0xF400300C);
            value[2] = DISP_REG_GET(0xF4003010);
            value[3] = DISP_REG_GET(0xF4003014);
            value[4] = DISP_REG_GET(0xF4003018);
            DISP_REG_SET(0xF4003040, 0x00000001);
            value[5] = DISP_REG_GET(0xF4003044);
            DISP_REG_SET(0xF4003040, 0x00000002);
            value[6] = DISP_REG_GET(0xF4003044);
            DISP_REG_SET(0xF4003040, 0x00000003);
            value[7] = DISP_REG_GET(0xF4003044);

            printk(KERN_DEBUG "=============== [CMDQ] RSZ1 Status ====================================\n");
            printk(KERN_DEBUG "[CMDQ] RSZ1_CONTROL: 0x%08x, RSZ1_INPUT_IMAGE: 0x%08x RSZ1_OUTPUT_IMAGE: 0x%08x\n", 
                value[0], value[1], value[2]);
            printk(KERN_DEBUG "[CMDQ] RSZ1_HORIZONTAL_COEFF_STEP: 0x%08x, RSZ1_VERTICAL_COEFF_STEP: 0x%08x\n", 
                value[3], value[4]);
            printk(KERN_DEBUG "[CMDQ] RSZ1_DEBUG_1: 0x%08x, RSZ1_DEBUG_2: 0x%08x, RSZ1_DEBUG_3: 0x%08x\n", 
                value[5], value[6], value[7]);
        }

        if (engineFlag & (0x01 << tTDSHP))
        {
            pEngine[tTDSHP].failCount++;
            value[0] = DISP_REG_GET((0xF4006000 + 0x10C));
            value[1] = DISP_REG_GET((0xF4006000 + 0x114));
            value[2] = DISP_REG_GET((0x14006000 + 0x11C));

            printk(KERN_DEBUG "=============== [CMDQ] tTDSHP Status ====================================\n");
            printk(KERN_DEBUG "[CMDQ] status: 0x%08x, INPUT_COUNT: 0x%08x, OUTPUT_COUNT: 0x%08x\n", value[0], value[1], value[2]);
        }

        if (engineFlag & (0x01 << tWROT))
        {
            pEngine[tWROT].failCount++;

        #if 0
            for (inner = 0; inner < 1024; inner += 1)
            {
                pError[index].regVal[tWROT][index] = DISP_REG_GET(0xF4005000 + (index << 2));
            }
        #endif // 0

            value[0] = DISP_REG_GET(0xF4005000);
            value[1] = DISP_REG_GET(0xF4005008);
            value[2] = DISP_REG_GET(0xF400500C);
            value[3] = DISP_REG_GET(0xF4005024);
            value[4] = DISP_REG_GET(0xF4005028);
            value[5] = DISP_REG_GET(0xF400502C);
            value[6] = DISP_REG_GET(0xF4005004);
            value[7] = DISP_REG_GET(0xF4005030);
            value[8] = DISP_REG_GET(0xF4005078);
            value[9] = DISP_REG_GET(0xF4005070);
            DISP_REG_SET(0xF4005018, 0x00000100);
            value[10] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000200);
            value[11] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000300);
            value[12] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000400);
            value[13] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000500);
            value[14] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000600);
            value[15] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000700);
            value[16] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000800);
            value[17] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000900);
            value[18] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000A00);
            value[19] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000B00);
            value[20] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000C00);
            value[21] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000D00);
            value[22] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000E00);
            value[23] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00000F00);
            value[24] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00001000);
            value[25] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00001100);
            value[26] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00001200);
            value[27] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00001300);
            value[28] = DISP_REG_GET(0xF40050D0);
            DISP_REG_SET(0xF4005018, 0x00001400);
            value[29] = DISP_REG_GET(0xF40050D0);

            value[30] = DISP_REG_GET(0xF4005010);
            value[31] = DISP_REG_GET(0xF4005014);
            value[32] = DISP_REG_GET(0xF400501c);
            value[33] = DISP_REG_GET(0xF4005020);

            printk(KERN_DEBUG "=============== [CMDQ] ROT Status ====================================\n");
            printk(KERN_DEBUG "[CMDQ] ROT_CTRL: 0x%08x, ROT_MAIN_BUF_SIZE: 0x%08x, ROT_SUB_BUF_SIZE: 0x%08x\n", 
                value[0], value[1], value[2]);
            printk(KERN_DEBUG "[CMDQ] ROT_TAR_SIZE: 0x%08x, ROT_BASE_ADDR: 0x%08x, ROT_OFST_ADDR: 0x%08x\n", 
                value[3], value[4], value[5]);
            printk(KERN_DEBUG "[CMDQ] ROT_DMA_PERF: 0x%08x, ROT_STRIDE: 0x%08x, ROT_IN_SIZE: 0x%08x\n", 
                value[6], value[7], value[8]);
            printk(KERN_DEBUG "[CMDQ] ROT_EOL: 0x%08x, ROT_DBUGG_1: 0x%08x, ROT_DEBUBG_2: 0x%08x\n", 
                value[9], value[10], value[11]);
            printk(KERN_DEBUG "[CMDQ] ROT_DBUGG_3: 0x%08x, ROT_DBUGG_4: 0x%08x, ROT_DEBUBG_5: 0x%08x\n", 
                value[12], value[13], value[14]);
            printk(KERN_DEBUG "[CMDQ] ROT_DBUGG_6: 0x%08x, ROT_DBUGG_7: 0x%08x, ROT_DEBUBG_8: 0x%08x\n", 
                value[15], value[16], value[17]);
            printk(KERN_DEBUG "[CMDQ] ROT_DBUGG_9: 0x%08x, ROT_DBUGG_A: 0x%08x, ROT_DEBUBG_B: 0x%08x\n", 
                value[18], value[19], value[20]);
            printk(KERN_DEBUG "[CMDQ] ROT_DBUGG_C: 0x%08x, ROT_DBUGG_D: 0x%08x, ROT_DEBUBG_E: 0x%08x\n", 
                value[21], value[22], value[23]);
            printk(KERN_DEBUG "[CMDQ] ROT_DBUGG_F: 0x%08x, ROT_DBUGG_10: 0x%08x, ROT_DEBUBG_11: 0x%08x\n", 
                value[24], value[25], value[26]);
            printk(KERN_DEBUG "[CMDQ] ROT_DEBUG_12: 0x%08x, ROT_DBUGG_13: 0x%08x, ROT_DBUGG_14: 0x%08x\n", 
                value[27], value[28], value[29]);

            printk(KERN_DEBUG "[CMDQ] ROT_RST: 0x%08x, ROT_RST_STAT: 0x%08x, ROT_INIT: 0x%08x, [CMDQ] ROT_CROP_OFST: 0x%08x\n", 
                value[30], value[31], value[32], value[33]);
                
            value[0] = DISP_REG_GET(0xF4005034);
            value[1] = DISP_REG_GET(0xF4005038);
            value[2] = DISP_REG_GET(0xF400503c);
            value[3] = DISP_REG_GET(0xF4005054);
            value[4] = DISP_REG_GET(0xF4005064);                
            value[5] = DISP_REG_GET(0xF4005068);
            value[6] = DISP_REG_GET(0xF400506C);
            value[7] = DISP_REG_GET(0xF4005074);
            value[8] = DISP_REG_GET(0xF400507C);
            value[9] = DISP_REG_GET(0xF4005080);
            
            value[10] = DISP_REG_GET(0xF4005084);
            value[11] = DISP_REG_GET(0xF4005088);
            value[12] = DISP_REG_GET(0xF400508C);            
            value[13] = DISP_REG_GET(0xF4005090);
            value[14] = DISP_REG_GET(0xF4005094);  
            value[15] = DISP_REG_GET(0xF4005098);
            value[16] = DISP_REG_GET(0xF400509C);
            value[17] = DISP_REG_GET(0xF40050A0);
            value[18] = DISP_REG_GET(0xF40050A4);
            value[19] = DISP_REG_GET(0xF40050AC);
            value[20] = DISP_REG_GET(0xF40050B0);
            value[21] = DISP_REG_GET(0xF40050B4);
            value[22] = DISP_REG_GET(0xF40050B8);            
            value[23] = DISP_REG_GET(0xF40050BC);
            value[24] = DISP_REG_GET(0xF40050C0);
            value[25] = DISP_REG_GET(0xF40050C4);
            value[26] = DISP_REG_GET(0xF40050C8);
            value[27] = DISP_REG_GET(0xF40050CC);
            
            printk(KERN_DEBUG "[CMDQ] ROT_BASE_ADDR_C: 0x%08x, ROT_OFST_ADDR_C: 0x%08x, ROT_STRIDE_C: 0x%08x, ROT_DITHER: 0x%08x\n", 
                value[0], value[1], value[2], value[3]);            
            printk(KERN_DEBUG "[CMDQ] ROT_BASE_ADDR_V: 0x%08x, ROT_OFST_ADDR_V: 0x%08x, ROT_STRIDE_V: 0x%08x", 
                value[4], value[5], value[6]);
            printk(KERN_DEBUG "[CMDQ] DMA_PREULTRA: 0x%08x, ROT_ROT_EN: 0x%08x, ROT_FIFO_TEST: 0x%08x\n", 
                value[7], value[8], value[9]);
                
            printk(KERN_DEBUG "[CMDQ] ROT_MAT_CTRL: 0x%08x, ROT_MAT_RMY: 0x%08x, ROT_MAT_RMV: 0x%08x\n", 
                value[10], value[11], value[12]);
            printk(KERN_DEBUG "[CMDQ] ROT_GMY: 0x%08x, ROT_BMY: 0x%08x, ROT_BMV: 0x%08x\n", 
                value[13], value[14], value[15]);
            printk(KERN_DEBUG "[CMDQ] ROT_MAT_PREADD: 0x%08x, ROT_MAT_POSTADD: 0x%08x, DITHER_00: 0x%08x\n", 
                value[16], value[17], value[18]);
            printk(KERN_DEBUG "[CMDQ] ROT_DITHER_02: 0x%08x, ROT_DITHER_03: 0x%08x, ROT_DITHER_04: 0x%08x\n", 
                value[19], value[20], value[21]);
            printk(KERN_DEBUG "[CMDQ] ROT_DITHER_05: 0x%08x, ROT_DITHER_06: 0x%08x, ROT_DITHER_07: 0x%08x\n", 
                value[22], value[23], value[24]);
            printk(KERN_DEBUG "[CMDQ] ROT_DITHER_08: 0x%08x, ROT_DITHER_09: 0x%08x, ROT_DITHER_10: 0x%08x\n", 
                value[25], value[26], value[27]);

        }

        if (engineFlag & (0x01 << tWDMA1))
        {
            pEngine[tWDMA1].failCount++;

        #if 0
            for (inner = 0; inner < 1024; inner += 1)
            {
                pError[index].regVal[tWDMA1][index]= DISP_REG_GET(0xF4004000 + (index << 2));
            }
        #endif // 0

            value[0] = DISP_REG_GET(0xF4004014);
            value[1] = DISP_REG_GET(0xF4004018);
            value[2] = DISP_REG_GET(0xF4004028);
            value[3] = DISP_REG_GET(0xF4004024);
            value[4] = DISP_REG_GET(0xF4004078);
            value[5] = DISP_REG_GET(0xF4004080);
            value[6] = DISP_REG_GET(0xF40040A0);
            value[7] = DISP_REG_GET(0xF40040A8);

            DISP_REG_SET(0xF4004014, (value[0] & (0x0FFFFFFF)));
            value[8] = DISP_REG_GET(0xF4004014);
            value[9] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x10000000  | (value[0] & (0x0FFFFFFF)));
            value[10] = DISP_REG_GET(0xF4004014);
            value[11] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x20000000  | (value[0] & (0x0FFFFFFF)));
            value[12] = DISP_REG_GET(0xF4004014);
            value[13] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x30000000  | (value[0] & (0x0FFFFFFF)));
            value[14] = DISP_REG_GET(0xF4004014);
            value[15] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x40000000  | (value[0] & (0x0FFFFFFF)));
            value[16] = DISP_REG_GET(0xF4004014);
            value[17] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x50000000  | (value[0] & (0x0FFFFFFF)));
            value[18] = DISP_REG_GET(0xF4004014);
            value[19] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x60000000  | (value[0] & (0x0FFFFFFF)));
            value[20] = DISP_REG_GET(0xF4004014);
            value[21] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x70000000  | (value[0] & (0x0FFFFFFF)));
            value[22] = DISP_REG_GET(0xF4004014);
            value[23] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x80000000  | (value[0] & (0x0FFFFFFF)));
            value[24] = DISP_REG_GET(0xF4004014);
            value[25] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0x90000000  | (value[0] & (0x0FFFFFFF)));
            value[26] = DISP_REG_GET(0xF4004014);
            value[27] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0xA0000000  | (value[0] & (0x0FFFFFFF)));
            value[28] = DISP_REG_GET(0xF4004014);
            value[29] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0xB0000000  | (value[0] & (0x0FFFFFFF)));
            value[30] = DISP_REG_GET(0xF4004014);
            value[31] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0xC0000000  | (value[0] & (0x0FFFFFFF)));
            value[32] = DISP_REG_GET(0xF4004014);
            value[33] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0xD0000000  | (value[0] & (0x0FFFFFFF)));
            value[34] = DISP_REG_GET(0xF4004014);
            value[35] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0xE0000000  | (value[0] & (0x0FFFFFFF)));
            value[36] = DISP_REG_GET(0xF4004014);
            value[37] = DISP_REG_GET(0xF40040AC);
            DISP_REG_SET(0xF4004014, 0xF0000000  | (value[0] & (0x0FFFFFFF)));
            value[38] = DISP_REG_GET(0xF4004014);
            value[39] = DISP_REG_GET(0xF40040AC);

            printk(KERN_DEBUG "=============== [CMDQ] WDMA Status ====================================\n");
            printk(KERN_DEBUG "[CMDQ]WDMA_CFG: 0x%08x, WDMA_SRC_SIZE: 0x%08x, WDMA_DST_W_IN_BYTE = 0x%08x\n", 
                value[0], value[1], value[2]);
            printk(KERN_DEBUG "[CMDQ]WDMA_DST_ADDR0: 0x%08x, WDMA_DST_UV_PITCH: 0x%08x, WDMA_DST_ADDR_OFFSET0 = 0x%08x\n", 
                value[3], value[4], value[5]);
            printk(KERN_DEBUG "[CMDQ]WDMA_STATUS: 0x%08x, WDMA_INPUT_CNT: 0x%08x\n", 
                value[6], value[7]);

            //Dump Addtional WDMA debug info
            printk(KERN_DEBUG "WDMA_DEBUG_0 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[8], value[9]);
            printk(KERN_DEBUG "WDMA_DEBUG_1 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[10], value[11]);
            printk(KERN_DEBUG "WDMA_DEBUG_2 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[12], value[13]);
            printk(KERN_DEBUG "WDMA_DEBUG_3 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[14], value[15]);
            printk(KERN_DEBUG "WDMA_DEBUG_4 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[16], value[17]);
            printk(KERN_DEBUG "WDMA_DEBUG_5 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[18], value[19]);
            printk(KERN_DEBUG "WDMA_DEBUG_6 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[20], value[21]);
            printk(KERN_DEBUG "WDMA_DEBUG_7 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[22], value[23]);
            printk(KERN_DEBUG "WDMA_DEBUG_8 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[24], value[25]);
            printk(KERN_DEBUG "WDMA_DEBUG_9 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[26], value[27]);
            printk(KERN_DEBUG "WDMA_DEBUG_A 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[28], value[29]);
            printk(KERN_DEBUG "WDMA_DEBUG_B 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[30], value[31]);
            printk(KERN_DEBUG "WDMA_DEBUG_C 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[32], value[33]);
            printk(KERN_DEBUG "WDMA_DEBUG_D 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[34], value[35]);
            printk(KERN_DEBUG "WDMA_DEBUG_E 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[36], value[37]);
            printk(KERN_DEBUG "WDMA_DEBUG_F 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[38], value[39]);
        }

        pError[index].pTask = pTask;
        gCmdqContext.errNum++;

        // Set task state to error state
        pTask->taskState = TASK_STATE_ERROR;
    }
}


static int32_t cmdq_exec_task_sync(TaskStruct *pTask,
                                   int32_t    thread)
{
    int32_t      status;
    ThreadStruct *pThread;
    u_long       flags;
    int32_t      loop;
    int32_t      count;
    int32_t      waitQ;
    int32_t      minimum;
    int32_t      cookie;
    int32_t      prev;
    TaskStruct   *pPrev;
    TaskStruct   *pLast;
    int32_t      index;
    int32_t      currPC;
    uint8_t      *pInst;
    uint32_t     insts[4];
    uint32_t     type;
    char         msg[64];

    CMDQ_MSG("Execute the task 0x%08x on thread %d begin\n", (uint32_t)pTask, thread);

    pThread = &(gCmdqContext.thread[thread]);

    spin_lock_irqsave(&gCmdqExecLock, flags);
    smp_mb();

    if (pThread->taskCount <= 0)
    {
        CMDQ_MSG("Allocate new HW thread(%d)\n", thread);
   
        DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(thread), 0x01);
        
        loop = 0;
        while(0x1 == (DISP_REG_GET(DISP_REG_CMDQ_THRx_RESET(thread))))
        {
            if(loop > CMDQ_MAX_LOOP_COUNT)
            {
                spin_unlock_irqrestore(&gCmdqExecLock, flags);
                CMDQ_AEE("CMDQ", "Reset HW thread %d failed\n", thread);
                return -EFAULT;
            }
            loop++;
        }

        DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(thread), 0x01);  //Enable Each IRQ
        DISP_REG_SET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(thread), CMDQ_MAX_INST_CYCLE);
        DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(thread), pTask->MVABase);
        DISP_REG_SET(DISP_REG_CMDQ_THRx_END_ADDR(thread), pTask->MVABase + pTask->blockSize);  

        minimum = DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(thread)) & 0x0FFFF;

        pThread->nextCookie = minimum + 1;
        if (pThread->nextCookie >= 65536)  // Reach the maximum cookie
        {
            pThread->nextCookie = 0;
        }

        cookie = pThread->nextCookie;
        pThread->waitCookie = cookie;

        pThread->pCurTask[cookie % CMDQ_MAX_TASK_COUNT] = pTask;
        pThread->taskCount++;

        pThread->nextCookie += 1;
        if (pThread->nextCookie >= 65536)  // Reach the maximum cookie
        {
            pThread->nextCookie = 0;
        }

        CMGQ_GET_CURRENT_TIME(pTask->trigger);

        DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(thread), 0x01);
    }
    else
    {
        CMDQ_MSG("Reuse original HW thread(%d)\n", thread);
        
        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(thread), 0x01);

        loop = 0;
        while(0x0 == (DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(thread)) & 0x2))
        {
            if(loop > CMDQ_MAX_LOOP_COUNT)
            {
                spin_unlock_irqrestore(&gCmdqExecLock, flags);
                CMDQ_AEE("CMDQ", "Suspend HW thread %d failed\n", thread);
                return -EFAULT;
            }
            loop++;
        }

        DISP_REG_SET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(thread), CMDQ_MAX_INST_CYCLE);

        cookie  = pThread->nextCookie;

        // Boundary case tested: EOC have been executed, but JUMP is not executed
        // Thread PC: 0x9edc0dd8, End: 0x9edc0de0, Curr Cookie: 1, Next Cookie: 2
        if((DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(thread)) == (DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(thread)) - 8)) ||  // PC = END - 8, EOC is executed
           (DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(thread)) == (DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(thread)) - 0)))    // PC = END - 0, All CMDs are executed
        {
            CMDQ_MSG("Set new task's MVA to thread current PC\n");

            DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(thread), pTask->MVABase);
            DISP_REG_SET(DISP_REG_CMDQ_THRx_END_ADDR(thread), pTask->MVABase + pTask->blockSize); 

            pThread->pCurTask[cookie % CMDQ_MAX_TASK_COUNT] = pTask;
            pThread->taskCount++;
        }
        else
        {
            CMDQ_MSG("Connect new task's MVA to previous one\n");

            // Current task that shuld be processed
            minimum = (DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(thread)) & 0x0FFFF) + 1;
            if (minimum >= 65536)
            {
                minimum = 0;
            }

            // Calculate loop count to adjust the tasks' order
            if (minimum <= cookie)
            {
                loop = cookie - minimum;
            }
            else
            {
                // Counter wrapped
                loop = (65535 - minimum + 1) + cookie;
            }

            CMDQ_MSG("Reorder task in range [%d, %d] with count %d\n", minimum, cookie, loop);

            if (loop < 1)
            {
                CMDQ_AEE("CMDQ", "Invalid task count(%d) for reorder\n", loop);
                spin_unlock_irqrestore(&gCmdqExecLock, flags);
                return -EFAULT;
            }
            else
            {
                CMDQ_MSG("Reorder %d tasks for performance begin\n", loop);

                pLast = pTask;  // Default last task

                // Adjust tasks' order according to their priorities
                for (index = (cookie % CMDQ_MAX_TASK_COUNT); loop > 0; loop--, index--)
                {
                    if (index < 0)
                    {
                        index = CMDQ_MAX_TASK_COUNT - 1;
                    }
                
                    prev = index - 1;
                    if (prev < 0)
                    {
                        prev = CMDQ_MAX_TASK_COUNT - 1;
                    }

                    pPrev = pThread->pCurTask[prev];

                    // Maybe the job is killed, search a new one
                    count = CMDQ_MAX_TASK_COUNT;
                    while((NULL == pPrev) && (count > 0))
                    {
                        prev = prev - 1;
                        if (prev < 0)
                        {
                            prev = CMDQ_MAX_TASK_COUNT - 1;
                        }

                        pPrev = pThread->pCurTask[prev];
                        loop--;
                        index--;
                        count--;
                   }

                    if (NULL != pPrev)
                    {
                        if (loop > 1)
                        {
                            if (pPrev->priority < pTask->priority)
                            {
                                CMDQ_MSG("Switch prev(%d) and current(%d) order\n", prev, index);

                                pThread->pCurTask[index] = pPrev;
                                pPrev->pCMDEnd[ 0] = pTask->pCMDEnd[ 0];
                                pPrev->pCMDEnd[-1] = pTask->pCMDEnd[-1];

                                // Boot priority for the task
                                pPrev->priority += CMDQ_MIN_AGE_VALUE;
                                pPrev->reorder++;

                                pThread->pCurTask[prev]  = pTask;
                                pTask->pCMDEnd[ 0] = 0x10000001;     //Jump: Absolute
                                pTask->pCMDEnd[-1] = pPrev->MVABase; //Jump to here

                                if (pLast == pTask)
                                {
                                    pLast = pPrev;
                                }
                            }
                            else
                            {
                                CMDQ_MSG("Set current(%d) order for the new task\n", index);
                            
                                CMDQ_MSG("Original PC 0x%08x, end 0x%08x\n", pPrev->MVABase, pPrev->MVABase + pPrev->blockSize);
                                CMDQ_MSG("Original instruction 0x%08x, 0x%08x\n", pPrev->pCMDEnd[0], pPrev->pCMDEnd[-1]);
                
                                pThread->pCurTask[index] = pTask;
                                pPrev->pCMDEnd[ 0] = 0x10000001;     //Jump: Absolute
                                pPrev->pCMDEnd[-1] = pTask->MVABase; //Jump to here
                                break;
                            }
                        }
                        else
                        {
                            CMDQ_MSG("Set current(%d) order for the new task\n", index);
                        
                            CMDQ_MSG("Original PC 0x%08x, end 0x%08x\n", pPrev->MVABase, pPrev->MVABase + pPrev->blockSize);
                            CMDQ_MSG("Original instruction 0x%08x, 0x%08x\n", pPrev->pCMDEnd[0], pPrev->pCMDEnd[-1]);
                
                            pThread->pCurTask[index] = pTask;
                            pPrev->pCMDEnd[ 0] = 0x10000001;     //Jump: Absolute
                            pPrev->pCMDEnd[-1] = pTask->MVABase; //Jump to here
                            break;
                        }
                    }
                    else
                    {
                        spin_unlock_irqrestore(&gCmdqExecLock, flags);
                        cmdq_attach_error_task(pTask, index);
                        CMDQ_AEE("CMDQ", "Invalid task state for reorder %d %d\n", index, loop);
                        return -EFAULT;
                    }
                }
            }

            CMDQ_MSG("Reorder %d tasks for performance end\n", loop);

            DISP_REG_SET(DISP_REG_CMDQ_THRx_END_ADDR(thread), pLast->MVABase + pLast->blockSize); 
            pThread->taskCount++;
        }

        pThread->nextCookie += 1;
        if (pThread->nextCookie >= 65536)  // Reach the maximum cookie
        {
            pThread->nextCookie = 0;
        }
 
        CMGQ_GET_CURRENT_TIME(pTask->trigger);
 
        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(thread), 0x00);
    }

    spin_unlock_irqrestore(&gCmdqExecLock, flags);

    waitQ = wait_event_interruptible_timeout(gCmdWaitQueue[thread], (TASK_STATE_BUSY != pTask->taskState), HZ);

    status = 0;  // Default status

    CMGQ_GET_CURRENT_TIME(pTask->wakedUp);

    spin_lock_irqsave(&gCmdqExecLock, flags); 
    smp_mb();

    if (TASK_STATE_ERROR == pTask->taskState)  // CMDQ error
    {
        cmdq_attach_error_task(pTask, thread);
        CMDQ_AEE("CMDQ", "Execute user commands error\n");
        status = -EFAULT;
    }
    else if (0 > waitQ)  // Task be killed
    {
        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(thread), 0x01);

        loop = 0;
        while(0x0 == (DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(thread)) & 0x2))
        {
            if(loop > CMDQ_MAX_LOOP_COUNT)
            {
                spin_unlock_irqrestore(&gCmdqExecLock, flags);
                cmdq_attach_error_task(pTask, thread);
                CMDQ_AEE("CMDQ", "Suspend HW thread %d failed\n", thread);
                return -EFAULT;
            }
            loop++;
        }

        DISP_REG_SET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(thread), CMDQ_MAX_INST_CYCLE);

        // The cookie of the task waitting to be processed
        cookie = (DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(thread)) & 0x0FFFF) + 1;

        pPrev = pThread->pCurTask[cookie % CMDQ_MAX_TASK_COUNT];
        if (pPrev == pTask)
        {           
            // The task is executed now, set the PC to EOC for bypass       
            DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(thread), pTask->firstEOC);

            status = cmdq_reset_hw_engine(pTask->engineFlag);
            if (-EFAULT == status)
            {
                spin_unlock_irqrestore(&gCmdqExecLock, flags);
                cmdq_attach_error_task(NULL, thread);
                CMDQ_AEE("MDP", "Reset hardware engine failed\n");
                return status;
            }

            CMDQ_ERR("Delete current execute task\n");

            pThread->pCurTask[cookie % CMDQ_MAX_TASK_COUNT] = NULL;
            pThread->taskCount--;

            pTask->taskState = TASK_STATE_KILLED;
        }
        else
        {           
            loop = pThread->taskCount;
            for (index = (cookie % CMDQ_MAX_TASK_COUNT); loop > 0; loop--, index++)
            {
                if (index >= CMDQ_MAX_TASK_COUNT)
                {
                    index = 0;
                }

                pPrev = pThread->pCurTask[index];
                if (NULL != pPrev)
                {
                    if ((0x10000000 == pPrev->pCMDEnd[ 0]) &&
                        (0x00000008 == pPrev->pCMDEnd[-1]))
                    {
                        // We reached the last task
                        CMDQ_ERR("Task reached command end\n");
                        break;
                    }
                    else if (pPrev->pCMDEnd[-1] == pTask->MVABase)
                    {
                        // Fake EOC command
                        pPrev->pCMDEnd[-3]  = 0x00000000;
                        pPrev->pCMDEnd[-2]  = 0x40000000;
                        pPrev->pCMDEnd[-1]  = 0x00000001;
                        pPrev->pCMDEnd[ 0]  = 0x40000000;
                    
                        // Bypass the task
                        pPrev->pCMDEnd[ 1]  = pTask->pCMDEnd[-1];
                        pPrev->pCMDEnd[ 2]  = pTask->pCMDEnd[ 0];

                        // Adjust command size
                        pPrev->pCMDEnd      = &(pPrev->pCMDEnd[ 2]);
                        pPrev->blockSize   += 8;

                        index += 1;
                        if (index >= CMDQ_MAX_TASK_COUNT)
                        {
                            index = 0;
                        }

                        pThread->pCurTask[index] = NULL;
                        pThread->taskCount--;

                        pTask->taskState = TASK_STATE_KILLED;
                    
                        CMDQ_ERR("Task for cookie %d is killed\n", index);
                    
                        status = -EFAULT;
                        break;
                    }
                }
            }
        }

        DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(thread), 0x00);   
    }
    else if (0 == waitQ)  // SW timeout
    {        
        if (TASK_STATE_DONE != pTask->taskState)
        {
            type   = 2;
            currPC = DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(thread));
            
            printk("[CMDQ] thread %d curPC: 0x%08x\n", thread, currPC);
            if((currPC >= pTask->MVABase) && (currPC <= (pTask->MVABase + pTask->blockSize)))
            {
                pInst  = (uint8_t*)pTask->pVABase + (currPC - pTask->MVABase);               
                if ((uint32_t*)pInst != pTask->pCMDEnd)
                {
                    // Prevent page fault
                    if ((currPC - pTask->MVABase) >= 8)
                    {
                        insts[0] = DISP_REG_GET(pInst - 8);
                    }
                    else
                    {
                        insts[0] = 0x0;
                    }

                    if ((currPC - pTask->MVABase) >= 4)
                    {
                        insts[1] = DISP_REG_GET(pInst - 4);
                    }
                    else
                    {
                        insts[1] = 0x0;
                    }

                    insts[2] = DISP_REG_GET(pInst + 0);
                    insts[3] = DISP_REG_GET(pInst + 4);
                }
                else
                {
                    // Prevent page fault
                    if ((currPC - pTask->MVABase) >= 16)
                    {
                        insts[0] = DISP_REG_GET(pInst - 16);
                    }
                    else
                    {
                        insts[0] = 0x0;
                    }

                    if ((currPC - pTask->MVABase) >= 12)
                    {
                        insts[1] = DISP_REG_GET(pInst - 12);
                    }
                    else
                    {
                        insts[1] = 0x0;
                    }
                    
                    if ((currPC - pTask->MVABase) >= 8)
                    {
                        insts[2] = DISP_REG_GET(pInst - 8);
                    }
                    else
                    {
                        insts[2] = 0x0;
                    }
                
                    if ((currPC - pTask->MVABase) >= 4)
                    {
                        insts[3] = DISP_REG_GET(pInst - 4);
                    }
                    else
                    {
                        insts[3] = 0x0;
                    }
                }

                printk("[CMDQ] insts[0] = 0x%08x, insts[1] = 0x%08x\n",  insts[0] ,  insts[1]);                
                printk("[CMDQ] insts[2] = 0x%08x, insts[3] = 0x%08x\n",  insts[2] ,  insts[3]);
                if((insts[3] & 0x08000000) != 0) // Polling some register
                {
                    if((insts[3] & 0x00400000) != 0) //22 bit == 1 -> ISP
                    {
                        type = 0;
                        sprintf(msg, "Polling ISP reg timeout, inst = 0x%08x, 0x%08x\n", insts[2], insts[3]);
                    }
                    else  //22 bit == 0 -> MDP
                    {
                        type = 1;
                        sprintf(msg, "Polling MDP reg timeout, inst = 0x%08x, 0x%08x\n", insts[2], insts[3]);
                    }
                }
                else if((insts[3] & 0x20000000) != 0)  // Waiting some event
                {
                    if((0x17 == (insts[3] & 0xFFFFFF)) || (0x18 == (insts[3] & 0xFFFFFF)))
                    {
                        type = 0;
                        sprintf(msg, "Wait ISP event timeout, inst = 0x%08x, 0x%08x\n", insts[2], insts[3]);
                    }
                    else
                    {
                        type = 1;
                        sprintf(msg, "Wait MDP event timeout, inst = 0x%08x, 0x%08x\n", insts[2], insts[3]);
                    }
                }
                else
                {
                    type = 2;
                    sprintf(msg, "Wait MM command queue timeout, inst = 0x%08x, 0x%08x\n", insts[2], insts[3]);
                }
            }
            else
            {
                type = 2;
                sprintf(msg, "MM command queue driver internal state error, pTask[0x%08x], MVABase[0x%08x], blocksize[%d], curPC[0x%08x]\n", 
                	pTask,
                	pTask->MVABase, 
                	pTask->blockSize, 
                	currPC);
            }

            spin_unlock_irqrestore(&gCmdqExecLock, flags);
            cmdq_attach_error_task(pTask, thread);

            switch (type)
            {
                case 0:
                    CMDQ_AEE("ISP", "%s", msg);
                    break;
                case 1:
                    CMDQ_AEE("MDP", "%s", msg);
                    break;
                default:
                    CMDQ_AEE("CMDQ", "%s", msg);
                    break;
            }

            if((currPC >= pTask->MVABase) && (currPC <= (pTask->MVABase + pTask->blockSize)))
            {
                CMDQ_ERR("Resume CMDQ thread executeion\n");
                
                spin_lock_irqsave(&gCmdqExecLock, flags); 

                pThread->taskCount--;
                
                // Resume executing remain tasks
                DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(thread), 0x01);

                loop = 0;
                while(0x0 == (DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(thread)) & 0x2))
                {
                    if(loop > CMDQ_MAX_LOOP_COUNT)
                    {
                        spin_unlock_irqrestore(&gCmdqExecLock, flags);
                        cmdq_attach_error_task(NULL, thread);
                        CMDQ_AEE("CMDQ", "Suspend thread %d after timeout failed\n", thread);
                        return -EFAULT;
                    }
                    loop++;
                }

                status = cmdq_reset_hw_engine(pTask->engineFlag);
                if (-EFAULT == status)
                {
                    spin_unlock_irqrestore(&gCmdqExecLock, flags);
                    cmdq_attach_error_task(NULL, thread);
                    CMDQ_AEE("MDP", "Reset hardware engine 0x%08x after timeout failed\n", pTask->engineFlag);
                    return status;
                }

                // The task is in timeout state, set the PC to EOC for bypass       
                DISP_REG_SET(DISP_REG_CMDQ_THRx_PC(thread), pTask->firstEOC);
    
                DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(thread), 0x0);    
                
                spin_unlock_irqrestore(&gCmdqExecLock, flags);
            }
            
            return -EFAULT;
        }        
    }

    if (pThread->taskCount <= 0)
    {
        DISP_REG_SET(DISP_REG_CMDQ_THRx_RESET(thread), 0x01);
        
        loop = 0;
        while(0x1 == (DISP_REG_GET(DISP_REG_CMDQ_THRx_RESET(thread))))
        {
            if(loop > CMDQ_MAX_LOOP_COUNT)
            {
                spin_unlock_irqrestore(&gCmdqExecLock, flags);
                cmdq_attach_error_task(NULL, thread);
                CMDQ_AEE("CMDQ", "Reset HW thread %d failed\n", thread);
                return -EFAULT;
            }
            loop++;
        }
    
        DISP_REG_SET(DISP_REG_CMDQ_THRx_EN(thread), 0x00);
    }

    spin_unlock_irqrestore(&gCmdqExecLock, flags);

    CMDQ_MSG("Execute the task 0x%08x on thread %d end\n", (uint32_t)pTask, thread);

    return status;
}


static void cmdq_release_task(TaskStruct *pTask)
{
    u_long flags;

    CMDQ_MSG("Release task structure begin\n");

    if ((TASK_STATE_ERROR != pTask->taskState) &&
        (TASK_STATE_BUSY  != pTask->taskState))
    {
        if (TASK_TYPE_FIXED == pTask->taskType)
        {
            spin_lock_irqsave(&gCmdqTaskLock, flags);            
            pTask->taskState = TASK_STATE_IDLE;
            list_add_tail(&(pTask->listEntry), &gCmdqFreeTask);
            spin_unlock_irqrestore(&gCmdqTaskLock, flags);
        }
        else
        {
            dma_free_coherent(NULL, pTask->bufSize, pTask->pVABase, pTask->MVABase);
            kfree(pTask);
        }
    }

    CMDQ_MSG("Release task structure end\n");
}


void cmdqHandleError(int32_t index)
{
    u_long       flags;
    uint32_t     status;
    ThreadStruct *pThread;
    int32_t      cookie;
    int32_t      count;
    int32_t      inner;
 
    pThread = &(gCmdqContext.thread[index]);

    spin_lock_irqsave(&gCmdqExecLock, flags);
    smp_mb();

    status = DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(index));
    if (0x0 == status)
    {
        spin_unlock_irqrestore(&gCmdqExecLock, flags);
        //cmdq_attach_error_task(NULL, index);
        //CMDQ_AEE("CMDQ", "Incorrect thread state\n");
        return;
    }

    DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(index), 0x01);

    inner = 0;
    while(0x0 == (DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(index)) & 0x2))
    {
        if(inner > CMDQ_MAX_LOOP_COUNT)
        {
            spin_unlock_irqrestore(&gCmdqExecLock, flags);
            CMDQ_AEE("CMDQ", "Suspend HW thread %d failed0\n", index);
            return;
        }
        inner++;
    }

    cookie = (DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(index)) & 0x0FFFF) + 1;
    if (cookie >= 65536)
    {
        cookie -= 65536;
    }

    // Set the issued task to error state
    if (NULL != pThread->pCurTask[cookie % CMDQ_MAX_TASK_COUNT])
    {
        CMGQ_GET_CURRENT_TIME(pThread->pCurTask[inner]->gotIRQ);
        
        pThread->pCurTask[cookie % CMDQ_MAX_TASK_COUNT]->taskState = TASK_STATE_ERROR;
        pThread->pCurTask[cookie % CMDQ_MAX_TASK_COUNT] = NULL;
        pThread->taskCount--;
    }

    // Set the remain tasks to done state
    if (pThread->waitCookie <= cookie)
    {
        count = cookie - pThread->waitCookie + 1;
    }
    else
    {
        // Counter wrapped
        count = (65535 - pThread->waitCookie + 1) + (cookie + 1);
    }

    for (inner = (pThread->waitCookie % CMDQ_MAX_TASK_COUNT); count > 0; count--, inner++)
    {
        if (inner >= CMDQ_MAX_TASK_COUNT)
        {
            inner = 0;
        }
            
        if (NULL != pThread->pCurTask[inner])
        {
            CMGQ_GET_CURRENT_TIME(pThread->pCurTask[inner]->gotIRQ);

            pThread->pCurTask[inner]->taskState = TASK_STATE_DONE;
            pThread->pCurTask[inner] = NULL;
            pThread->taskCount--;
        }
    }    

    pThread->waitCookie = cookie + 1;
    if (pThread->waitCookie >= 65536)
    {
        pThread->waitCookie -= 65536;
    }

    DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG(index), ~(0x012));

    DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(index), 0x00);

    spin_unlock_irqrestore(&gCmdqExecLock, flags);

    smp_mb();
    wake_up_interruptible(&gCmdWaitQueue[index]);
}


void cmdqHandleDone(int32_t index)
{
    u_long       flags;
    uint32_t     status;
    ThreadStruct *pThread;
    int32_t      cookie;
    int32_t      count;
    int32_t      inner;

    pThread = &(gCmdqContext.thread[index]);

    spin_lock_irqsave(&gCmdqExecLock, flags);
    smp_mb();

    status = DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(index));
    if (0x0 == status)
    {
        spin_unlock_irqrestore(&gCmdqExecLock, flags);
        //cmdq_attach_error_task(NULL, index);
        //CMDQ_AEE("CMDQ", "Incorrect thread state\n");
        return;
    }
    
    DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(index), 0x01);

    inner = 0;
    while(0x0 == (DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(index)) & 0x2))
    {
        if(inner > CMDQ_MAX_LOOP_COUNT)
        {
            spin_unlock_irqrestore(&gCmdqExecLock, flags);
            cmdq_attach_error_task(NULL, index);
            CMDQ_AEE("CMDQ", "Suspend HW thread %d failed1\n", index);
            return;
        }
        inner++;
    }
    
    cookie = DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(index)) & 0x0FFFF;
    if (pThread->waitCookie <= cookie)
    {
        count = cookie - pThread->waitCookie + 1;
    }
    else
    {
        // Counter wrapped
        count = (65535 - pThread->waitCookie + 1) + (cookie + 1);
    }

    for (inner = (pThread->waitCookie % CMDQ_MAX_TASK_COUNT); count > 0; count--, inner++)
    {
        if (inner >= CMDQ_MAX_TASK_COUNT)
        {
            inner = 0;
        }
            
        if (NULL != pThread->pCurTask[inner])
        {
            CMGQ_GET_CURRENT_TIME(pThread->pCurTask[inner]->gotIRQ);
            
            pThread->pCurTask[inner]->taskState = TASK_STATE_DONE;
            pThread->pCurTask[inner] = NULL;
            pThread->taskCount--;
        }
    }    

    pThread->waitCookie = cookie + 1;
    if (pThread->waitCookie >= 65536)
    {
        pThread->waitCookie -= 65536;
    }

    DISP_REG_SET(DISP_REG_CMDQ_THRx_IRQ_FLAG(index), ~(0x01));

    DISP_REG_SET(DISP_REG_CMDQ_THRx_SUSPEND(index), 0x00);

    spin_unlock_irqrestore(&gCmdqExecLock, flags);

    smp_mb();    
    wake_up_interruptible(&gCmdWaitQueue[index]);
}


int32_t cmdqSubmitTask(int32_t  scenario,
                       int32_t  priority,
                       uint32_t engineFlag,
                       void     *pCMDBlock,
                       int32_t  blockSize)
{
    struct timespec start;
    struct timespec done;
    TaskStruct      *pTask;
    int32_t         thread;
    int32_t         retry;
    int32_t         status;
    u_long          flags;
    RecordStruct    *pRecord;
    int32_t         execTime;

    CMGQ_GET_CURRENT_TIME(start);

    pTask = cmdq_acquire_task(scenario,
                              priority,
                              engineFlag,
                              pCMDBlock,
                              blockSize);
    if (NULL == pTask)
    {
        CMDQ_ERR("Acquire task failed\n");
        return -EFAULT;
    }

    thread = cmdq_acquire_thread(engineFlag);
    if (CMDQ_INVALID_THREAD == thread)
    {
        CMDQ_ERR("Acquire thread failed\n");
        cmdq_release_task(pTask);
        return -EFAULT;
    }

    retry = 0;
    do
    {
        status = cmdq_exec_task_sync(pTask,
                                     thread);
        if ((status >= 0) ||
            (TASK_STATE_KILLED == pTask->taskState) ||
            (TASK_STATE_ERROR  == pTask->taskState))
        {
            break;
        }

        ++retry;
    } while(retry < 1);

    cmdq_release_thread(thread,
                        engineFlag);

    CMGQ_GET_CURRENT_TIME(done);

    spin_lock_irqsave(&gCmdqRecordLock, flags);
    smp_mb();

    pRecord = &(gCmdqContext.record[gCmdqContext.lastID]);
    gCmdqContext.lastID++;
    if (gCmdqContext.lastID >= CMDQ_MAX_RECORD_COUNT)
    {
        gCmdqContext.lastID = 0;
    }

    gCmdqContext.recNum++;
    if (gCmdqContext.recNum >= CMDQ_MAX_RECORD_COUNT)
    {
        gCmdqContext.recNum = CMDQ_MAX_RECORD_COUNT;
    }

    spin_unlock_irqrestore(&gCmdqRecordLock, flags);

    // Record scenario
    pRecord->scenario  = pTask->scenario;
    pRecord->priority  = pTask->priority;
    pRecord->reorder   = pTask->reorder;

    // Record time
    pRecord->start     = start;
    pRecord->trigger   = pTask->trigger;
    pRecord->gotIRQ    = pTask->gotIRQ;
    pRecord->wakedUp   = pTask->wakedUp;
    pRecord->done      = done;

    CMDQ_GET_TIME_DURATION(pRecord->trigger, pRecord->wakedUp, execTime);

    cmdq_release_task(pTask);

    return status;
}


void cmdqDeInitialize()
{
    TaskStruct *pTask;
    
    if (NULL != gCmdqProcEntry)
    {
        remove_proc_entry("record",  gCmdqProcEntry);
        remove_proc_entry("error", gCmdqProcEntry);
        remove_proc_entry("status", gCmdqProcEntry);
        remove_proc_entry("cmdq", NULL);
        gCmdqProcEntry = NULL;
    }

    pTask = &(gCmdqContext.taskInfo[0]);

    dma_free_coherent(NULL, CMDQ_MAX_DMA_BUF_SIZE, pTask->pVABase, pTask->MVABase);
}
