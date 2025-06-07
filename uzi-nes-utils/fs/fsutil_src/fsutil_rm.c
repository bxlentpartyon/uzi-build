#include <lib/string.h>
#include <unix.h>

#define lstat _stat

int stat(), unlink();

char *basename(char *name)
{
    char *base;

    base = rindex(name, '/');
    return base ? base + 1 : name;
}


int rm_main(int argc, char **argv)
{
    int i /*, recurse = 0, interact =0 */ ;
    struct stat sbuf;

/*
    if (((argv[1][0] == '-') && (argv[1][1] == 'r')) ||
        ((argv[2][0] == '-') && (argv[2][1] == 'r'))) 
	recurse = 1;

    if (((argv[1][0] == '-') && (argv[1][1] == 'i')) ||
        ((argv[2][0] == '-') && (argv[2][1] == 'i')))
	interact = 1;        
*/

    fsutil_printf("DEBUG argc %d removing %s\n", argc, argv[1]);

    for (i = 1; i < argc; i++) {
	fsutil_printf("argv[%d] = %s\n", i, argv[i]);
	if (argv[i][0] != '-') {
	    if (!lstat(argv[i], &sbuf)) {
		if (_unlink(argv[i])) {
		    fsutil_printf("rm: could not remove %s\n", argv[i]);
		}
	    }
	}
    }

    return 0;
}
