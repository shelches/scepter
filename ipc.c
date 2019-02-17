#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <wm.h>

#include "common.h"
#include "scepter.h"

static char *QSegment;
static short *EncSegment;
static short *ObjSegment;

static unsigned QBuffer;
static int	QUser;

static struct
{
	int Hup;
	int Tty;
	pid_t Pid;
	unsigned QBufPtr;
} QUsers[MaxUsers];

int QFind(int pid)
{
	int i;

	for (i = 0; i < MaxUsers; i++)
		if (QUsers[i].Pid == pid)
			return i;

	return -1;
}

unsigned QAdjust(unsigned *value)
{
	unsigned ret = *value;
	
	*value = (*value + 1) % 32768;
	return ret;
}	

char QSegUpdate(char c)
{
	QSegment[QBuffer] = c;
	QAdjust(&QBuffer);
	return c;
}

void QAss(char *Mbx, int *Chn)
{
	UNUSED(Mbx);
	UNUSED(Chn);
}

void QCre(void)
{
	/* SPS
	if (!(Cmd_flags & RUN_BACKGROUND))
		abort("usage: scepterd &\n");
	*/	
	if (qnx_name_attach(getnid(), SERVER) == 0)
	{
		fprintf(stderr, "Could not register name\n");
		abort();
	}
	/*QSegment = alloc_segment(4096);*/
	QSegment = (char *)malloc(32768);
	if (QSegment == NULL)
		printf("ipc: QSegment malloc failed\n");
	QBuffer = 0;
	/*set_extra_segment(QSegment);*/
	for (QUser = 0; QUser < MaxUsers; QUser++)
		QUsers[QUser].Pid = 0;

	/*EncSegment = alloc_segment(LEncounter + 1);*/
	EncSegment = (short *)malloc((LEncounter + 1) * 8 * sizeof(short));
	/*ObjSegment = alloc_segment(ObjListLen + 1);*/
	ObjSegment = (short *)malloc((ObjListLen + 1) * 8 * sizeof(short));
}

int GetETIndex(int table, int row, int col)
{
	int	val;
	short	*Index;

	/*set_extra_segment(table ? ObjSegment : EncSegment);*/
	if (table)
		Index = ObjSegment;
	else
		Index = EncSegment;
	/*val = @(Index + row * 8 + col);*/
	val = Index[row * 8 + col];
	/*set_extra_segment(QSegment);*/
	return val;
}

void PutETIndex(int table, int row, int col, int val)
{
	short *Index;

	/*set_extra_segment(table ? ObjSegment : EncSegment);*/
	if (table)
		Index = ObjSegment;
	else
		Index = EncSegment;
	/*@(Index + row * 8 + col) = val;*/
	Index[row * 8 + col] = (short)val;
	/*set_extra_segment(QSegment);*/
}

void QDea(int Chn)
{
	UNUSED(Chn);
}

static void QHang(int Chn)
{
	int i;

	for (i = 0; i < MaxUsers; i++)
		if (QUsers[i].Pid == Chn)
			QUsers[i].Pid = 0;
}

void QRea(char *Str, int *Chn)
{
	do
	{
		*Chn = Receive(0, Str, 256);
		if (*Str == '0')
		{
			for (QUser = 0; QUser < MaxUsers; QUser++)
				if (QUsers[QUser].Pid == 0)
					break;
			if (QUser == MaxUsers)
				Reply(*Chn, "2Too many users", 256);
			else
			{
				QUsers[QUser].Hup = 0;
				QUsers[QUser].Pid = *Chn;
				QUsers[QUser].Tty = atoi(Str + 1);
				QUsers[QUser].QBufPtr = QBuffer;
			}
		}
		else
		{
			for (QUser = 0; QUser < MaxUsers; QUser++)
				if (QUsers[QUser].Pid == *Chn)
					break;
			if (QUser == MaxUsers)
				Reply(*Chn, "2Task not registered", 256);
			else if (*Str == '2')
			{
				GetNextMsg();
				QUser = MaxUsers;
			}
			else if (*Str == '4')
			{
				QUsers[QUser].Hup = 1;
				Reply(*Chn, "2That's not a nice way to quit.", 256);
				QUser = MaxUsers;
			}
		}
	} while (QUser == MaxUsers);
}

void QOut(int Chn, char *Str)
{
	if (*Str < '0' || *Str > '4')
		printf("Error in QOut(): %s\n", Str);
	else
	{
		QSegUpdate(Chn & 0xff);
		QSegUpdate(Chn >> 8);
		while (QSegUpdate(*Str++))
			;
	}
}

void GetNextMsg(void)
{
	char	NextMsg[256];
	int		j;

	do
	{
		int		i;

		j = QSegment[ QAdjust(&QUsers[QUser].QBufPtr) ];
		j |= QSegment[ QAdjust(&QUsers[QUser].QBufPtr) ] << 8;
		for (i = 0; NextMsg[i] = QSegment[ QAdjust(&QUsers[QUser].QBufPtr) ]; i++)
			;
	} while (j != QUsers[QUser].Pid);
	Reply(QUsers[QUser].Pid, NextMsg, 256);
	if (*NextMsg > '1')
		QUsers[QUser].Pid = 0;
}

void Who(void)
{
	char *task;
	int i, j;
	int n;
	T_WHO *list;
	UserPoint Player;

	n = who(&list);

	QOut(Term, "0");

	if (!User->Brief)
		QOut(Term, "0T#  Task     idle-time  Name");

	for (i = 0; i < n; i++)
	{
		task = list[i].task;

		if (!strcmp(list[i].task, "scepter"))
		{
			for (Player = UserTail; Player; Player = Player->NextUser)
			{
				if (Player->NonExistant
					&& (j = QFind(Player->Trm)) >= 0
					&& QUsers[j].Tty == list[i].tty)
				{
					task = "talk";
					break;
				}
			}
		}

		sprintf(B1, "0%2d  %-8.8s  %s  %s",
			list[i].tty, task, list[i].idle, whois(list[i].uid));

		QOut(Term, B1);
	}

	free(list);

	/* SPS
	FILE	*fp;
	int		tty, tid;

	if (!(fp = fopen(PASSWD, "r")))
	{
		QOut(Term, "0file error");
		return;
	}

	QOut(Term, "0");
	if (!User->Brief)
		QOut(Term, "0tty name             task");
	for (tty = 0; tty < MAXTTY; tty++)
		if (tid = tty_tid(My_nid, tty))
		{
			int		uid;
			char	*p, name[24];
			struct
			{
				struct task_info_entry	tinfo;
				struct code_info_entry	cinfo;
			} tbuf;

			if ((uid = task_info(tid, 8) & 0xff) == 255)
				strcpy(name, "root");
			else
			{
				char	pwbuf[256];

				rewind(fp);
				while (uid-- >= 0)
					fgets(pwbuf, 255, fp);
				tsscanf(pwbuf, "%[^;]", name);
			}
			get_task_info(0, 1, tid, 3, &tbuf);
			if (p = strrchr(tbuf.cinfo.ci_code_name, '/'))
				p++;
			else
				p = tbuf.cinfo.ci_code_name;

			if (!strcmp(p, "scepter"))
			{
				UserPoint	Player;

				for (Player = UserTail; Player; Player = Player->NextUser)
					if (Player->Trm == tid && Player->NonExistant)
					{
						p = "talk";
						break;
					}
			}

			sprintf(B1, "0%2d  %-16s %s", tty, name, p);
			QOut(Term, B1);
		}
	fclose(fp);
	*/
}

void CheckHup(void)
{
	int Count;
	int ILoop;
	UserPoint Plyr;

	Plyr = UserTail;

	while (Plyr)
	{
		for (ILoop = 0; ILoop < MaxUsers; ILoop++)
			if (QUsers[ILoop].Hup && QUsers[ILoop].Pid == Plyr->Trm)
				break;

		if (ILoop == MaxUsers)
			Plyr = Plyr->NextUser;
		else
		{
			sprintf(B1, "0### %s was devoured by US West Communications.",
					Plyr->Name);
			CmdCode = 0;
			Term = Plyr->Trm;
			Count = MsgTerm(BrAll);
			for (ILoop = 0; ILoop < Count; ILoop++)
				if (TermList[ILoop] != Term)
					QOut(TermList[ILoop], B1);
			QHang(Plyr->Trm);
			Logoff(Plyr);
			Plyr = UserTail;
		}
	}
}

int IsGuest(int term)
{
	int found = FALSE;
	int u = QFind(term);

	if (u >= 0)
	{
		who_t *list;
		who_t *who;

		WmGetWho(FALSE, &list);

		for (who = list; !found && who; who = who->next)
			if (QUsers[u].Tty == who->tty && 0 == stricmp("guest", who->name))
				found = TRUE;

		WmFreeWho(list);
	}

	return found;
}
