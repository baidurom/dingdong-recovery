#include "../dd_inter.h"
#include "../dd.h"

#define BUFFER_SIZE 4096
#define TTF_PATH_SYSTEM "/system/fonts"
#define TTF_PATH_EXTSD "/sdcard/dingdong/recovery/fonts"
#define TTF_PATH_INTSD "/sdcard2/dingdong/recovery/fonts"
#define TTF_PATH_RES "/res/fonts/DroidSansFallback.ttf"
#define TTF_NAME "DroidSansFallback.ttf"
#define TTF_CN "fonts/DroidSansFallback.ttf"
#define TTF_EN "fonts/DroidSerif-Regular.ttf"

static int copy_file(const char *to, const char *from) {
    int from_fd, to_fd;
    int bytes_read, bytes_write;
    char *ptr;
    
    char* buffer = malloc(BUFFER_SIZE);
		if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate buffer\n");
        return -1;
    }
    
    if ((from_fd = open (from, O_RDONLY)) == -1) {
       fprintf(stderr, "Open %s Error:%s\n", from, strerror(errno));
       return from_fd;
    }
    
    if ((to_fd = open (to, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) == -1) {
       fprintf(stderr, "Open %s Error:%s\n", to, strerror(errno));
       return to_fd;
    }
    
    while (bytes_read = read (from_fd, buffer, BUFFER_SIZE)) {
        if ((bytes_read == -1) && (errno != EINTR)) 
            break;
        else if (bytes_read > 0) {
            ptr = buffer;
            while (bytes_write = write (to_fd, ptr, bytes_read)) {
                if ((bytes_write == -1) && (errno != EINTR))
                    break;
                else if (bytes_write == bytes_read) 
                    break;
                else if (bytes_write > 0) {
                    ptr += bytes_write;
                    bytes_read -= bytes_write;
                }
            }
            if (bytes_write == -1)
                break;
        }
    }
    close(from_fd);
    close(to_fd);
    if((bytes_write == -1) || (bytes_read == -1)) {
        fprintf(stderr, "Open %s Error:%s\n", to, strerror(errno));
        return -1;
    }
    return 0;
}

static int copy_cn_fonts() {
    struct stat st;
    char file_path[BUFFER_SIZE];
    int rtn = 1, i = 0;

    if(!stat(TTF_PATH_RES, &st))
        return 0;

    do {
        switch(i) {
            case 0:
                 snprintf(file_path, BUFFER_SIZE, "%s/%s", TTF_PATH_SYSTEM, TTF_NAME);
                 break;
            case 1:
                 snprintf(file_path, BUFFER_SIZE, "%s/%s", TTF_PATH_EXTSD, TTF_NAME);
                 break;
            case 2:
                 snprintf(file_path, BUFFER_SIZE, "%s/%s", TTF_PATH_INTSD, TTF_NAME);
                 break;
            default:
                 break;
        }

        i++;
        if (!ensure_path_mounted(file_path)) {
            if (!stat(file_path, &st)) {
                rtn = copy_file(TTF_PATH_RES, file_path);
                if(rtn) { fprintf(stderr, "Copy from %s Error\n", file_path); }
                else {
                    ensure_path_unmounted(file_path);
                    break;
                }
            } else {
                fprintf(stderr, "%s not found\n", file_path);
            }
            ensure_path_unmounted(file_path);
        } else {
            fprintf(stderr, "%s mount failed\n", file_path);
        }
    } while(i < 3);
   
    return rtn;
}

int dd_lang_init() {
    int rtn=0;

    LOGI("Language init for recovery...\n");
    rtn = copy_cn_fonts();
    if(!rtn) {
        dd_loadlang("values/strings.cn");
        dd_font( "0", TTF_CN";"TTF_EN, "9" );
        dd_font( "1", TTF_CN";"TTF_EN, "12" );
        dd_font( "2", TTF_CN";"TTF_EN, "18" );
    } else {
        dd_loadlang("values/strings.en");
        dd_font( "0", TTF_EN, "9" );
        dd_font( "1", TTF_EN, "12" );
        dd_font( "2", TTF_EN, "14" );
    }

    return rtn;
}
