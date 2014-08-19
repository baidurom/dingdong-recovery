//#define HALL_DEBUG_CODE

#define HALL_DEVICE "HALL"
#ifdef HALL_DEBUG_CODE
#undef HALL_DEBUG
#define HALL_DEBUG(a,arg...) printk(HALL_DEVICE ": " a, ##arg)
#define HALL_FUNC()	printk(HALL_DEVICE ": %s line=%d\n", __func__, __LINE__)
#else
#define HALL_DEBUG(arg...)
#define HALL_FUNC()
#endif

enum hall_report_state
{
	HALL_NEAR =0,
	HALL_FAR = 1,
};