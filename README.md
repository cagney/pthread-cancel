# pthread-cancel

## trying to get pthread_cancel() to work with pthread_cleanup_push()

The program main.c tries pthread_cancel() + pthread_cleanup_push() as follows:

- thread directly calls sleep()
- thread calls sleep via an external file
- thread calls sleep via a shared object

the program is then built with and without -fexceptions:

- main-exceptions - built with -fexceptions so relies on a working unwinder
- main-longjump - no -fexceptions so uses "slower" longjmp

In all cases the cleanup should be called but with the shared object it and -fexceptions it doesn't.
It seems that glibc's unwinder fails when unwinding __sleep.  It ends up back in
__sleep when it should be in do_sleep()
