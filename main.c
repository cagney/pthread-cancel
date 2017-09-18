#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <dlfcn.h>

volatile bool running = false;
volatile bool called = false;

void
thread_cancel(void *name)
{
  printf("thread %s: cancel called\n", (char *)name);
  called = true;
}

void *
thread_sleep(void *name)
{
  pthread_cleanup_push(thread_cancel, name);
  {
    printf("thread %s: running\n", (char*)name);
    running = true;
    sleep(100);
  }
  pthread_cleanup_pop(0);
  printf("thread %s: finished\n", (char*)name);
  return NULL;
}

extern void do_sleep(void);

void *
thread_call(void *name)
{
  pthread_cleanup_push(thread_cancel, name);
  {
    printf("thread %s: running\n", (char*)name);
    running = true;
    do_sleep();
  }
  pthread_cleanup_pop(0);
  printf("thread %s: finished\n", (char*)name);
  return NULL;
}

void *
thread_dl(void *name)
{
  pthread_cleanup_push(thread_cancel, name);
  {
    printf("thread %s: running\n", (char*)name);
    void *handle = dlopen("./dl.so", RTLD_NOW);
    if (handle == NULL) {
      printf("thread %s: dlopen failed\n", (char*)name);
      return NULL;
    }
    void (*func)(void) = (void (*)(void)) dlsym(handle, "do_sleep");
    if (func == NULL) {
      printf("thread %s: dlsym failed\n", (char*)name);
      return NULL;
    }
    running = true;
    printf("thread %s: calling func\n", (char*)name);
    func();
  }
  pthread_cleanup_pop(0);
  printf("thread %s: finished\n", (char*)name);
  return NULL;
}

void
test(char *name, void *(*thread)(void*))
{
  pthread_t tid;
  running = false;
  called = false;
  int count = 0;

  printf("%s: creating thread\n", name);
  pthread_create(&tid, NULL, thread, name);
  while (!running && count < 5) {
    printf("%s: sleeping a bit\n", name);
    sleep(1);
    count++;
  }
  printf("%s: sleeping some more\n", name);
  sleep(1);
  printf("%s: canceling thread\n", name);
  pthread_cancel(tid);
  printf("%s: joining thread\n", name);
  pthread_join(tid, NULL);
  if (called) {
    printf("%s: all worked - cancel called\n", name);
  } else {
    printf("%s: huh? - cancel never called\n", name);
  }

  printf("\n");
}

int
main(int argc, char **argv)
{
  test("sleep", thread_sleep);
  test("call", thread_call);
  test("dl", thread_dl);
}
