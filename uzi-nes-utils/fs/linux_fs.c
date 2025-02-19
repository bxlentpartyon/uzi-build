#include <stdio.h>
#include <strings.h>

#include <unix.h>
#include "linux_fs.h"

int yes(void)
{
    char line[20];
    /* int  fgets(); - HP */

    if (!fgets(line, sizeof(line), stdin) || (*line != 'y' && *line != 'Y'))
	return (0);

    return (1);
}
