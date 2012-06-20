#define __attribute__(x)
#define __extension__

namespace a {
  typedef int b;
}

/*/ Get This working first
using namespace a;
b c;

/*/// Then get this working so your life doesn't suck later
a::b c;
/* */

// Then this will follow
template<typename a> class b {
  a c;
}; /* */

//#include <string>

#include <GL/gl.h>
#include <GL/glu.h>
#include <ncurses.h>
#include <X11/Xlib.h>
#include <AL/al.h>
#include <zlib.h>

#include <stdio.h> //printf, NULL
#include <stdlib.h> //malloc
#include <unistd.h> //usleep
#include <time.h> //clock

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <float.h>
#include <iso646.h>
#include <math.h>
#include <locale.h>
#include <stdarg.h>
#include <wchar.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <malloc.h>

#include <cpio.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <tar.h>
#include <termios.h>
#include <utime.h>

#include <unistd.h>
#include <stdint.h>

#include <time.h>

#include <fcntl.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include <inttypes.h>
#include <sys/utsname.h>

// Once affected by lack of '?' operator token
#include <ctype.h>
#include <wctype.h>
#include <limits.h>
#include <assert.h>

// Once affected by lack of support for public/private/protected, and by lack of support for constructors.
#include <pthread.h>

#include <sys/stat.h>
#include <sys/time.h>

#include <fenv.h>
#include <tgmath.h>
#include <complex.h>
