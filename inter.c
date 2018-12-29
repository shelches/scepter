#include <process.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dev.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/proxy.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <wm.h>

#include "common.h"

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define CTRL(x)   ((x) - 64)
#define UNUSED(x) x = x

static char Msg[256], Reply[256];
static pid_t Qid;
static unsigned Mode;

/*
** restore mode
*/

unsigned ResetMode(void)
{
	tcdrain(1);
	return dev_mode(0, Mode, Mode);
}

/*
** set mode
*/

unsigned SetMode(void)
{
	tcdrain(1);
	return dev_mode(0, 0, ~(_DEV_ISIG | _DEV_OPOST));
}

/*
** wait for character or timeout
*/

int WaitInput(void)
{
	int gotchar = FALSE;
	int timeout = FALSE;
	fd_set rfd;
	struct timeval tv;

	while (!timeout && !gotchar)
	{
		FD_ZERO(&rfd);
		FD_SET(0, &rfd);

		tv.tv_sec  = 0;
		tv.tv_usec = 500000L;

		switch (select(1, &rfd, NULL, NULL, &tv))
		{
			case -1:
				leave("Error in select.\n");

			case 0:
				timeout = TRUE;
				break;

			default:
				if (FD_ISSET(0, &rfd))
					gotchar = TRUE;
				break;
		}
	}

	return gotchar;
}

/*
** get string
*/

int GetString(char *s, int maxlen, int cool)
{
	char c;
	int done  = FALSE;
	int gotstr = FALSE;
	int pos = 0;

	fflush(stdout);

	while (!done && !gotstr)
	{
		if (feof(stdin) || read(0, &c, sizeof(c)) != sizeof(c))
			done = TRUE;
		else
		{
			c &= 0x7f;

			switch (c)
			{
				case '\b':
				case 127:
					if (pos)
					{
						pos--;
						printf("\b \b");
						fflush(stdout);
					}
					break;

				case CTRL('X'):
					while (pos)
					{
						pos--;
						printf("\b \b");
					}

					fflush(stdout);
					break;

				case '\r':
				case '\n':
					putchar('\n');
					fflush(stdout);

					s[pos] = '\0';
					gotstr = TRUE;
					break;

				default:
					if (c >= ' ' && c <= 126)
					{
						if (pos < maxlen)
						{
							s[pos++] = c;
							putchar(c);
						}
						else
							putchar(7);

						fflush(stdout);
					}
					break;
			}

			if (cool && !pos)
				done = TRUE;
		}
	}

	/*dev_mode(0, mode, _DEV_MODES);*/
	s[pos] = '\0';

	return gotstr;
}

void mygets(char *s, int i)
{
	char	*t = s;

	fgets(s, i, stdin);
	while (*t = *s++ & 0x7f)
		if (*t >= ' ' && *t <= '~')
			t++;
}

void leave(char *s)
{
	ResetMode();

	if (*s)
		printf("%s", s);

	exit(0);
}

static void Handler(int sig)
{
	switch (sig)
	{
		case SIGHUP:
			strcpy(Msg, "4");
			Send(Qid, Msg, Reply, sizeof(Msg), sizeof(Reply));
			leave("");

		case SIGINT:
			strcpy(Msg, "4");
			Reply[0] = '\0';
			Send(Qid, Msg, Reply, sizeof(Msg), sizeof(Reply));
			if ('2' == Reply[0])
				puts(Reply + 1);
			leave("");

		default:
			printf("Signal caught: %d\n", sig);
			exit(1);
	}
}

pid_t startup(char **arge)
{
	pid_t Qid;
	
	/*Qid = qnx_name_locate(getnid(), SERVER, 256, NULL);*/

	if ((Qid = qnx_name_locate(getnid(), SERVER, 256, NULL)) == -1)
	{
		char *argv[2];

		argv[0] = BINDIR "/scepterd";
		argv[1] = NULL;
		
   	Qid = qnx_spawn(0, NULL, getnid(), -1, -1, _SPAWN_NOZOMBIE,
					argv[0], argv, arge, NULL, -1);
					
		if (Qid != -1)
		{
			int		i;

			for (i = 0; i < 5; i++)
			{
				sleep(2);
				
				if ((Qid = qnx_name_locate(getnid(), SERVER, 256, NULL)) != -1)
					break;
			}
		}
	}
	
	return Qid;
}

void main(int argc, char **argv, char **arge)
{
	unsigned state;
	pid_t kb_proxy; /* get rid of this crap! */

	static char kb_msg[] = "1";

	UNUSED(argc);
	UNUSED(argv);

	Mode = SetMode();

	if ((Qid = startup(arge)) == -1)
		leave("Scepter daemon not available\n");

	signal(SIGHUP, Handler);
	signal(SIGINT, SIG_IGN);

	kb_proxy = qnx_proxy_attach(0, kb_msg, sizeof(kb_msg), 0);
	dev_state(0, 0, _DEV_EVENT_INPUT);
	/*dev_arm(0, kb_proxy, _DEV_EVENT_INPUT);*/

	sprintf(Msg, "0%d", WmPortNum());
	
	for (;;)
	{
		if (Send(Qid, Msg, Reply, 256, 256) == -1)
			break;

		switch (*Reply)
		{
			case '0':
				printf("%s\n", Reply + 1);
				fflush(stdout);
				strcpy(Msg, "2");
				break;

			case '1':
				/*state = dev_state(0, 0, _DEV_EVENT_INPUT);*/
				state = 0;

				printf("%s", Reply + 1);
				Msg[0] = '1';

				if (feof(stdin))
					leave("Could not open stdin\n");

				if (!Reply[1] && dev_ischars(0) < 1)
				{
					if (WaitInput())
						GetString(Msg + 1, 255, TRUE);
					else
						Msg[1] = '\0';
				}
				else if (Reply[1])
					GetString(Msg + 1, 255, FALSE);
				else
					GetString(Msg + 1, 255, TRUE);

				if (state & _DEV_EVENT_INPUT)
				{
					puts(">>> arming device");
					dev_arm(0, kb_proxy, _DEV_EVENT_INPUT);
				}
				break;

			case '2':
				leave(Reply + 1);

			case '3':
				leave("End - aborted\n");
		}
	}

	leave("Premature task abort\n");
}
