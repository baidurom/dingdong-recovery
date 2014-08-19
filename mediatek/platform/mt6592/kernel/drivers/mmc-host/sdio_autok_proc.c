#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>

#ifdef CHANGE_SCHED_POLICY
//#include <linux/sched.h>
#include <linux/syscalls.h>
#endif

#include "mt_sd.h"
#include "sdio_autok.h"

#define PROC_AUTOK_NAME "autok"
#define PROC_BUF_SIZE 256

struct proc_dir_entry *s_proc = NULL;

#ifdef USE_KERNEL_THREAD
struct sdio_autok_thread_data g_autok_thread_data;
struct task_struct *task;
#else   // USE_KERNEL_THREAD
struct workqueue_struct             *g_autok_wq;
struct sdio_autok_workqueue_data    g_autok_thread_data;
#endif  // USE_KERNEL_THREAD

extern int sdio_autok_processed;

extern void mmc_set_clock(struct mmc_host *host, unsigned int hz);

#ifdef USE_KERNEL_THREAD
static int autok_thread_func(void *data);
#else   // USE_KERNEL_THREAD
static void autok_thread_func(struct work_struct *data);
#endif  // USE_KERNEL_THREAD

static int autok_writeproc(struct file *file,const char *buffer,
                           unsigned long count, void *data)
{
    //int err = 0;
    //char *envp[2];
    char stage;
    char bufferContent[PROC_BUF_SIZE];
    char *bufContIdx;
    //char sdiofunc_addr[11];
    //char autok_stage1_result[256];
    struct msdc_host *host;
    struct sdio_func *sdioFunc;
    struct mmc_host *mmc;
    int len;
    int procParamsOffset = 0;
    int i;
    
    printk(KERN_INFO "[%s] (/proc/%s) called\n", __func__, PROC_AUTOK_NAME);
    
    if(count >= PROC_BUF_SIZE)
    {
        printk(KERN_INFO "[%s] proc input size (%ld) is larger than buffer size (%d) \n", __func__, count, PROC_BUF_SIZE);
        return -EFAULT;
    }
    
    if (copy_from_user(bufferContent, buffer, count))
        return -EFAULT;
        
    bufferContent[count] = '\0';
    printk(KERN_INFO "[%s] bufferContent: (count = %ld)\n", __func__, count);
    for(i = 0; i < count; i++)
        printk(" %x ", (unsigned int)bufferContent[i]);
    printk("\n");
    
    // Parsing bufferContent
    bufContIdx = bufferContent;
    sdioFunc = (struct sdio_func *)(*(int *)bufContIdx);
    bufContIdx += 4;
    procParamsOffset += 4;
    stage = *bufContIdx;
    bufContIdx += 1;
    procParamsOffset += 1;
    if(count <= procParamsOffset)
    {
        stage = 1;
    }
    else
    {
        memcpy(&len, bufContIdx, sizeof(int));
        bufContIdx = bufContIdx + sizeof(int);
        procParamsOffset += sizeof(int);
        if(len > count - procParamsOffset)
        {
            printk(KERN_INFO "[%s] autok stage 1 result len (%d) is larger than actual proc input size (%ld) \n", __func__, len, count - procParamsOffset);
            return -EFAULT;
        }
        
		memcpy(g_autok_thread_data.autok_stage1_result, bufContIdx, len);
        g_autok_thread_data.len = len;
        
		printk(KERN_INFO "[%s] autok_stage1_result: (len = %d)\n", __func__, len);
        for(i = 0; i < len; i++)
            printk(" %x ", (unsigned int)g_autok_thread_data.autok_stage1_result[i]);
        printk("\n");
    }
    
    printk(KERN_INFO "[%s] stage = %d\n", __func__, stage);
    
    if(sdioFunc == NULL)
    {
        printk(KERN_INFO "[%s] sdioFunc = NULL\n", __func__);
        return -EFAULT;
    }
        
    mmc = sdioFunc->card->host;
    host = mmc_priv(mmc);
    
    // Set clock to card max clock
    sdio_autok_processed = 1;
    printk(KERN_INFO "[%s] mmc->ios.clock = %d, mmc->ios.power_mode = %d\n", __func__, mmc->ios.clock, mmc->ios.power_mode);
    mmc_set_clock(mmc, mmc->ios.clock);
    
    g_autok_thread_data.host = host;
    g_autok_thread_data.stage = stage;

#ifdef USE_KERNEL_THREAD    
    task = kthread_run(&autok_thread_func,(void *)(&g_autok_thread_data),"autokp");
    //if(!IS_ERR(task))
    //    wake_up_process(task);
#else   // USE_KERNEL_THREAD
    queue_delayed_work_on(0, g_autok_wq, (struct delayed_work *)&g_autok_thread_data, msecs_to_jiffies(0));
#endif  // USE_KERNEL_THREAD
    
    return count;
}


static int autok_readproc(char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
    void *param;
    int len;
    int i;
    char *p = page;
    
    printk(KERN_INFO "[%s] (/proc/%s) called\n", __func__, PROC_AUTOK_NAME);
    
    // read auto-K result from auto-K callback function
    msdc_autok_stg1_data_get(&param, &len);
    
    memcpy(p, &len, sizeof(int));
    p = p + sizeof(int);
    memcpy(p, param, len);
    
    printk(KERN_INFO "[%s] page = (len = %d)\n", __func__, len);
    for(i = 0; i < len + sizeof(int); i++)
    {
        printk(" %x ", (unsigned int)page[i]);
    }
    
    printk("\n");
    
    return len + sizeof(int);
}

#ifdef USE_KERNEL_THREAD
static int autok_thread_func(void *data)
#else   // USE_KERNEL_THREAD
static void autok_thread_func(struct work_struct *data)
#endif  // USE_KERNEL_THREAD
{
    int err = 0;
    
    #ifdef USE_KERNEL_THREAD
    struct sdio_autok_thread_data *autok_thread_data = (struct sdio_autok_thread_data *)data;
    #else   // USE_KERNEL_THREAD
    struct sdio_autok_workqueue_data *autok_thread_data = (struct sdio_autok_workqueue_data *)data;
    #endif  // USE_KERNEL_THREAD
    
    struct msdc_host *host = autok_thread_data->host;
    char stage = autok_thread_data->stage;
    char *envp[2];
    char *lteprocenvp[2];
#ifdef CHANGE_SCHED_POLICY    
    struct sched_param param;
    int sched_policy;
    
    #ifdef SCHED_POLICY_INFO
    sched_policy = sys_sched_getscheduler(0);
    printk("[%s] orig. sched policy: %d\n", __func__, sched_policy);
    
    param.sched_priority = sys_sched_get_priority_max(SCHED_FIFO);
    if( sys_sched_setscheduler( 0, SCHED_FIFO, &param ) == -1 )
    {
    	printk("[%s] sched_setscheduler fail\n", __func__);
    }
    
    sched_policy = sys_sched_getscheduler(0);
    printk("[%s] sched policy FIFO: %d\n", __func__, sched_policy);
    #endif
    
    //param.sched_priority = sched_get_priority_max(SCHED_RR);
    param.sched_priority = 1;
    if( sys_sched_setscheduler( 0, SCHED_RR, &param ) == -1 )
    {
    	printk("[%s] sched_setscheduler fail\n", __func__);
    }
    
    #ifdef SCHED_POLICY_INFO
    sched_policy = sys_sched_getscheduler(0);
    printk("[%s] modified sched policy: %d\n", __func__, sched_policy);
    #endif
#endif    
    if(stage == 1) {
        // call stage 1 auto-K callback function
        msdc_autok_stg1_cal(host);
        
        envp[0] = "FROM=sdio_autok";
        envp[1] = NULL;
        err = kobject_uevent_env(&host->mmc->class_dev.kobj, KOBJ_ONLINE, envp);
        if(err < 0)
            printk(KERN_INFO "[%s] kobject_uevent_env error = %d\n", __func__, err);
    } else if(stage == 2) {
        // call stage 2 auto-K callback function
        msdc_autok_stg2_cal(host, autok_thread_data->autok_stage1_result, autok_thread_data->len);
    } else {
        printk(KERN_INFO "[%s] stage %d doesn't support in auto-K\n", __func__, stage);
#ifdef USE_KERNEL_THREAD
        return -EFAULT;
#else   // USE_KERNEL_THREAD
        return;
#endif  // USE_KERNEL_THREAD
    }
    
    lteprocenvp[0] = "FROM=autok_done";
    lteprocenvp[1] = NULL;
    err = kobject_uevent_env(&host->mmc->class_dev.kobj, KOBJ_ONLINE, lteprocenvp);
    if(err < 0)
        printk(KERN_INFO "[%s] kobject_uevent_env error = %d\n", __func__, err);
    
#ifdef USE_KERNEL_THREAD
    return 0;
#else   // USE_KERNEL_THREAD
    return;
#endif  // USE_KERNEL_THREAD
}

static int autok_module_init(void)
{
    s_proc = create_proc_entry(PROC_AUTOK_NAME, 0660, NULL);
    
    if (s_proc == NULL) {
		remove_proc_entry(PROC_AUTOK_NAME, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROC_AUTOK_NAME);
		return -ENOMEM;
	}
	
    s_proc->write_proc = autok_writeproc;
    s_proc->read_proc = autok_readproc;
    
    printk(KERN_INFO "/proc/%s created\n", PROC_AUTOK_NAME);

#ifdef USE_KERNEL_THREAD    
    //task = kthread_create(&autok_thread_func,(void *)(&g_autok_thread_data),"autokp");
#else   // USE_KERNEL_THREAD
    g_autok_wq = create_workqueue("autok_queue");
    INIT_DELAYED_WORK((struct delayed_work *)(&g_autok_thread_data), autok_thread_func);
#endif  // USE_KERNEL_THREAD
    
	return 0;	/* everything is ok */
}

static void autok_module_exit(void)
{
	//int ret;
	
    remove_proc_entry(PROC_AUTOK_NAME, NULL);

#ifdef USE_KERNEL_THREAD
	//ret = kthread_stop(task);
#else   // USE_KERNEL_THREAD
    flush_workqueue(g_autok_wq);
    destroy_workqueue(g_autok_wq);
#endif  // USE_KERNEL_THREAD

    printk(KERN_INFO "/proc/%s removed\n", PROC_AUTOK_NAME);
}  

module_init(autok_module_init);
module_exit(autok_module_exit);

MODULE_AUTHOR("MediaTek Inc.");
MODULE_DESCRIPTION("MediaTek SDIO Auto-K Proc");
MODULE_LICENSE("GPL");