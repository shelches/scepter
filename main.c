#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unix.h>

#include "common.h"
#include "scepter.h"

static void	Init_Scepter(void);
static void	DoEntry(void);

static unsigned GetDate(time_t t)
{
	struct tm *tp = localtime(&t);

	return ((tp->tm_mon + 1) << 7) | tp->tm_mday;	/* mmmm0dddddd */
}

static unsigned GetMinute(time_t t)
{
	return (unsigned)((t / 60) % 1440);
}

void Boom(int sig)
{
	switch (sig)
	{
		case SIGSEGV:
			Abort(" SIGSEGV");

		default:
			Abort(" Signal caught.");
	}
}

void main(void)
{
	unsigned	minute;
	int Aborted = FALSE;

	switch (fork())
	{
		case -1:
			Abort(" Unable to fork.");

		case 0:
			close(1);
			close(2);
			open(TMPDIR "/scepterd.out", O_APPEND | O_CREAT, 0666);
			open(TMPDIR "/scepterd.out", O_APPEND | O_CREAT, 0666);
			close(0);

			setsid();
			setlogin("System");
			break;

		default:
			exit(0);
	}

	QCre();
	Init_Scepter();
	InitCmds();

	if ((EDesc = fopen(DSCRPT, FOPEN)) == NULL)
		Abort(" File open error on DSCRPT.");

	CurrentRec = 0;

	if ((OList = fopen(OLIST, FOPEN)) == NULL)
		Abort(" File open error on DLIST.");

	if ((MList = fopen(MLIST, FOPEN)) == NULL)
		Abort(" File open error on MLIST.");

	InitSpells();
	FolCount = 0;

	Init();

	if ((EDBMon = fopen(DBMON, FOPEN)) == NULL)
		Abort(" File open error on DBMON.");

	if ((EDBObj = fopen(DBOBJ, FOPEN)) == NULL)
		Abort(" File open error on DBOBJ.");

	OpenCave();

	InitEvents();

	RealTime = time(NULL);
	minute = GetMinute(RealTime);
	Today = GetDate(RealTime);

	signal(SIGSEGV, Boom);

	do
	{
		QRea(C1, &Pid);
		RealTime = time(NULL);

		if (minute != GetMinute(RealTime))
		{
			minute = GetMinute(RealTime);
			Today = GetDate(RealTime);	/* gotta do it someplace */
			CheckHup();
		}

		MonAttack();

		switch (C1[0])
		{
			case '0':
				QAss(C1 + 1, &Term);
				Term = Pid;
				User = Login();
				User->LastInput = RealTime;
				Loc	= LenBuf + 1;
				break;

			case '1':
				User = FindPid(Pid);
				if (User)
				{
					User->LastInput = RealTime;
					Term = User->Trm;
					DoInput();
					DoEntry();
				}
				break;

			default:
				break;
		}

		if (User)
		{
			PromptUser();
		}	

		GetNextMsg();

	} while (!Aborted);

	UpdateF();
	Off();

	CurrentRec = 0;
	fclose(EDesc);
	fclose(OList);
	fclose(MList);
	fclose(EDBMon);
	fclose(EDBObj);
	CloseCave();
	exit(0);
}

static void Init_Scepter(void)
{
	int		ILoop;

	if ((EFile = fopen(EFILE, "r")) == NULL)
		Abort(" File open error on EFILE.");

	fgets(Notice, 256, EFile);
	Notice[strlen(Notice) - 1] = '\0';

	for (ILoop = 0; ILoop < 5; ILoop++)
	{
		fgets(NewsBuf[ILoop], 256, EFile);
		NewsBuf[ILoop][strlen(NewsBuf[ILoop]) - 1] = '\0';
	}
	for (ILoop = 0; ILoop < 3; ILoop++)
	{
		fgets(B1, 256, EFile);
		TName[ILoop][0] = TClass[ILoop][0] = '\0';
		sscanf(B1, "%d%s%s", &TLvl[ILoop], TName[ILoop], TClass[ILoop]);
		fgets(TBuf[ILoop], 256, EFile);
		TBuf[ILoop][strlen(TBuf[ILoop]) - 1] = '\0';
	}

	fscanf(EFile, "%c%d%d", &TaskClosed, &NumRun, &MSpeed);

	for (ILoop = 1; ILoop <= LEncounter; ILoop++)
	{
		int j, e[8];

		fscanf(EFile, "%d%d%d%d%d%d%d%d",
				  &e[0], &e[1], &e[2], &e[3], &e[4], &e[5], &e[6], &e[7]);

		for (j = 0; j < 8; j++)
			PutEncIndex(ILoop, j, e[j]);
	}
	for (ILoop = 1; ILoop <= ObjListLen; ILoop++)
	{
		int		j, o[8];

		fscanf(EFile, "%d%d%d%d%d%d%d%d",
				  &o[0], &o[1], &o[2], &o[3], &o[4], &o[5], &o[6], &o[7]);
		for (j = 0; j < 8; j++)
			PutObjIndex(ILoop, j, o[j]);
	}

	fclose(EFile);

	RealTime = time((long *) 0);
	SetRandom((int) (RealTime & 0x7fff), getpid());

	NumSegs	= 200;
	NumRooms = NumSegs * 10 - 1;
	UserTail = NULL;
	NUsers = 0;
	LenBuf = 0;
	Loc	= 0;
	LastUpdate = RealTime;
	TradeTime = RealTime - 60;
	NTPlyr[0] = NFPlyr[0] = '\0';

	for	(ILoop = 0; ILoop <= MaxUsers; ILoop++)
	{
		Active[ILoop] =	0;
		SlotTbl[ILoop] = -1;
	}
}

static void DoEntry(void)
{
	switch ((int) User->Entry)
	{
		case XCmd:
		case XDead:
		case XSpell:
		case XSure:
			DoCmd();
			break;

		case XEdit:
			Edit();
			Loc = LenBuf + 1;
			break;

		case XNotice:
			EnterNotice();
			break;

		case XNews:
			EnterNews();
			break;

		case XParley:
		case XRepair:
		case XSell:
			YesNo();
			break;

		case XName:
			GetName();
			break;

		case XPassword:
			MatchPW();
			break;

		case XNewPW:
			NewPW();
			break;

		case XNewClass:
			ChooseClass();
			break;

		case XSex:
			EnterSex();
			break;

		case XStats:
			EnterStats();
			break;

		case XSkill:
			EnterSkill();
			break;

		case XDesc:
			EnterDesc();
			break;

		default:
			sprintf(B1,	"0 Sce107 - Undefined entry type = %d", User->Entry);
			QOut(Term, B1);
			User->Entry = XCmd;
			break;
	}
}
