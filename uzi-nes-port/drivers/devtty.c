int tty_open(int minor)
{
	if (minor == 0)
		return minor;
    return(0);
}

int tty_close(int minor)
{
	return(0);
}

