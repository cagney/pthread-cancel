# pthread-cancel

## trying to get pthread_cancel() to work with pthread_cleanup_push()

The program main.c tries pthread_cancel() + pthread_cleanup_push() as follows:

- thread directly calls sleep()
- thread calls sleep via an external file
- thread calls sleep via a shared object

the program is then built with and without -fexceptions:

- main-exceptions - built with -fexceptions so relies on a working unwinder
- main-longjump - no -fexceptions so uses "slower" longjmp

In all cases the cleanup should be called but with the shared object
it and -fexceptions it doesn't.  It seems that glibc's unwinder fails
when unwinding __sleep.  It ends up back in __sleep when it should be
in do_sleep().

## Here's some debug output:

You want the third sigcancel_handler() call:

#0  sigcancel_handler (sig=32, si=0xb7dcdc8c, ctx=0xb7dcdd0c) at nptl-init.c:175
#1  <signal handler called>
#2  0xb7fdd424 in __kernel_vsyscall ()
#3  0xb7e893b6 in nanosleep () at ../sysdeps/unix/syscall-template.S:81
#4  0xb7e8918d in __sleep (seconds=0) at ../sysdeps/unix/sysv/linux/sleep.c:137
#5  0xb7fd81ce in do_sleep () at dl.c:5
#6  0x08048a13 in thread_dl (name=0x8048d70) at main.c:55
#7  0xb7fb0b2c in start_thread (arg=0xb7dceb40) at pthread_create.c:308
#8  0xb7ec80ae in clone () at ../sysdeps/unix/sysv/linux/i386/clone.S:131

after that, breakpoint unwind_stop(), it is eventually passed a
context for _sleep() twice, and after that nothing - indicative of the
unwinder failing.

(gdb) c
Continuing.

Breakpoint 2, unwind_stop (version=version@entry=1, actions=actions@entry=10, exc_class=0, exc_obj=exc_obj@entry=0xb7dced90, context=context@entry=0xb7dcdb90, stop_parameter=stop_parameter@entry=0xb7dce3d0)
    at unwind.c:44
44	{
(gdb) print *context
$7 = {reg = {0xb7dcdd4c, 0xb7dcdd48, 0xb7dcdd44, 0xb7dcdd44, 0x0, 0xb7dce148, 0xb7dcdd34, 0xb7dcdd30, 0xb7dce15c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, cfa = 0xb7dce160, ra = 0xb7e8918d <__sleep+205>, 
  lsda = 0x0, bases = {tbase = 0x0, dbase = 0xb7f8b000, func = 0xb7e890c0 <__sleep>}, flags = 1073741824, version = 0, args_size = 0, by_value = '\000' <repeats 17 times>}
(gdb) c
Continuing.

Breakpoint 2, unwind_stop (version=version@entry=1, actions=actions@entry=26, exc_class=0, exc_obj=exc_obj@entry=0xb7dced90, context=context@entry=0xb7dcdb90, stop_parameter=stop_parameter@entry=0xb7dce3d0)
    at unwind.c:44
44	{
(gdb) print *context
$8 = {reg = {0xb7dcdd4c, 0xb7dcdd48, 0xb7dcdd44, 0xb7dce32c, 0x0, 0xb7dce338, 0xb7dce330, 0xb7dce334, 0xb7dce33c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}, cfa = 0xb7dce340, ra = 0xb7fd81ce <do_sleep+30>, 
  lsda = 0x0, bases = {tbase = 0x0, dbase = 0xb7f8b000, func = 0xb7e890c0 <__sleep>}, flags = 1073741824, version = 0, args_size = 0, by_value = '\000' <repeats 17 times>}
(gdb) 
