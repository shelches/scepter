#include "scepter.h"

void	InitCmds()
{
	int		i;
	static struct
	{
		char	*cmd;
		int		num;
	} cmdarray[] =
	{
		"*ABORT",		21,	"*CHANGENAM",	58,	"*CLOSE",		18,
		"*DAYFILE",		80,	"*DEBUG",		27,	"*DELETENEW",	53,
		"*EDIT",		16,	"*INVISIBLE",	30,	"*MONSPEED",	54,
		"*NEWS",		52,	"*NONEXISTA",	83,	"*NOTICE",		47,
		"*OFF",			82,	"*OPEN",		19,	"*PURGEDEAT",	50,
		"*SAY",			36,	"*SEND",		38,	"*TEST",		51,
		"*UPDATE",		23,	"*YELL",		37,

		"ACCEPT",		61,
		"ACT",			89,	"APPEAL",		45,	"ATTACK",		11,
		"BACKSTAB",		72,	"BLOCK",		76,	"BREAK",		70,
		"BRIEF",		25,	"BUY",			46,	"CAST",			44,
		"CATALOG",		48,	"CHANGEPW",		59,	"CIRCLE",		86,
		"CLIMB",		9,	"CLOCK",		26,	"CLOSE",		66,
		"CONDITION",	88,	"D",			6,	"DOWN",			6,
		"DRAW",			10,	"DRINK",		10,	"DROP",			8,
		"E",			3,	"ECHO",			34,	"END",			17,
		"ENTER",		9,	"EXAMINE",		24,	"EXIT",			7,
		"EXPERIENCE",	79,	"FEINT",		85,	"FOLLOW",		55,
		"GET",			63,	"GO",			9,	"HELP",			28,
		"HIDE",			73,	"HINT",			32,	"HIT",			11,
		"HURL",			80,
		"IDENTIFY",		31,	"INFORMATIO",	22,	"INVENTORY",	41,
		"KILL",			11,	"LEAVE",		7,	"LOCK",			67,
		"LOOK",			24,	"LOSE",			56,	"N",			1,
		"NUSERS",		20,	"OFFER",		60,	"OPEN",			65,
		"OUT",			7,	"PACK",			29,	"PANIC",		78,
		"PARLEY",		81,	"PARRY",		42,	"PAWN",			49,
		"PICKLOCK",		69,	"PUT",			8,	"QUIT",			17,
		"READ",			64,	"REPAIR",		57,	"RETURN",		29,
		"RUN",			78,	"S",			2,	"SAVE",			40,
		"SAY",			12,	"SEARCH",		74,	"SELL",			49,
		"SEND",			14,	"SMASH",		70,	"STATUS",		33,
		"STEAL",		84,	"SUICIDE",		39,	"TAG",			77,
		"TAKE",			63,	"TALK",			81,	"THRUST",		43,
		"TOUCH",		10,	"TRACK",		71,	"TRAIN",		75,
		"TURN",			62,	"U",			5,	"UNLOCK",		68,
		"UP",			5,	"USE",			10,	"USERS",		35,
		"W",			4,	"WEAR",			10,	"WHO",			87,
		"WIELD",		10,	"YELL",			13,	NULL,			0
	};

	for (i = 0; cmdarray[i].cmd; i++)
	{
		CmdList[i + 1] = cmdarray[i].cmd;
		CmdNum[i + 1] = cmdarray[i].num;
	}
}

int		Min(a, b)
int		a, b;
{
	return a < b ? a : b;
}

int		Max(a, b)
int		a, b;
{
	return a > b ? a : b;
}

long MinL(a, b)
long	a, b;
{
	return a < b ? a : b;
}

long MaxL(a, b)
long	a, b;
{
	return a > b ? a : b;
}

void	Abort(Error)
char	*Error;
{
	FILE	*fp;

	if (fp = fopen(ERRLOG, "a"))
	{
		fprintf(fp, "%s%s\n", ctime(&RealTime), Error);
		fclose(fp);
	}
	exit(1);
}

void	SetRandom(a, b)
int		a, b;
{
	Srand(a, b, a + b);
}

/* Random number from 1 to X, inclusive. */
int		Rnd(X)
{
	if (X)
		return (Rand() % X) + 1;
	return 1;
}

/* See if *Part* is an abbreviation of *Full* */
int		SubSet(Part, Full)
char	*Part, *Full;
{
	int		I;

	for (I = 0; I < sizeof(Alfa); I++)
		if (Part[I] != Full[I])
			return (Part[I] == '\0');
	return TRUE;
}

char	*Pb(X)
int		X;
{
	return X ? " TRUE" : "FALSE";
}

void	Pn(N, Str)
int		N;
char	*Str;
{
	static char	*NumString[] =
	{
		"zero",		"one",		"two",			"three",		"four",
		"five",		"six",		"seven",		"eight",		"nine",
		"ten",		"eleven",	"twelve",		"thirteen",		"fourteen",
		"fifteen",	"sixteen",	"seventeen",	"eighteen",		"nineteen",
		"twenty"
	};

	if (N >= 0 && N <= 20)
		sprintf(Str, "%s ", NumString[N]);
	else
		sprintf(Str, "%d ", N);
}

void	PNth(N, Str)
int		N;
char	*Str;
{
	static char	*NTh[] =
	{
		"zeroth",		"first",		"second",		"third",
		"fourth",		"fifth",		"sixth",		"seventh",
		"eighth",		"ninth",		"tenth",		"eleventh",	
		"twelfth",		"thirteenth",	"fourteenth",	"fifteenth",
		"sixteenth",	"seventeenth",	"eighteenth",	"ninteenth",
		"twentieth"
	};

	if (N >= 0 && N <= 20)
		strcpy(Str, NTh[N]);
	else
		switch (N % 10)
		{
			case 1:
				sprintf(Str, "%dst", N);
				break;
			case 2:
				sprintf(Str, "%dnd", N);
				break;
			case 3:
				sprintf(Str, "%drd", N);
				break;
			default:
				sprintf(Str, "%dth", N);
				break;
		}
}

char	*Pa(First)
char	First;
{
	return (strchr("AEIOU", Cap(First)) ? "an" : "a");
}

int		Cap(Ch)
int		Ch;
{
	if (Ch >= 'a' && Ch <= 'z')
		return Ch - ' ';
	else
		return Ch;
}

void	CapAlfa(a)
char	*a;
{
	while (*a)
	{
		*a = Cap(*a);
		a++;
	}
}

void	GetWord(Word, Num)
char	*Word;
int		*Num;
{
	*Word = '\0';
	*Num = 0;
	while (Loc < LenBuf)
	{
		if (Buf[Loc] == ',')
			Loc++;
		while (Buf[Loc] == ' ' && Loc < LenBuf)
			Loc++;

		if (Loc < LenBuf)
		{
			int		ILoop, LbSign;
			Alfa	TWord;

			ILoop = 0;
			LbSign = FALSE;
			do
			{
				if (Buf[Loc] == '#')
				{
					LbSign = TRUE;
					Loc++;
				}
				if (ILoop < sizeof(Alfa) - 1 && !LbSign)
					Word[ILoop++] = Buf[Loc];
				else
				if (LbSign && Buf[Loc] >= '0' && Buf[Loc] <= '9')
					*Num = *Num * 10 + Buf[Loc] - '0';
				Loc++;
			} while (Buf[Loc] != ' ' && Buf[Loc] != ',' && Loc < LenBuf);
			Word[ILoop] = '\0';

			if (*Word >= '0' && *Word <= '9' || *Word == '-')
			{
				int		Digit;

				*Word = '\0';
				ILoop = Loc - 1;
				Digit = 1;
				while (ILoop >= 0)
					if (Buf[ILoop] >= '0' && Buf[ILoop] <= '9'
					||	Buf[ILoop] == '-')
					{
						if (Buf[ILoop] == '-')
							*Num = -*Num;
						else
						{
							*Num += (Buf[ILoop] - '0') * Digit;
							Digit *= 10;
						}
						--ILoop;
					}
					else
						ILoop = -1;
			}

			strcpy(TWord, Word);
			CapAlfa(TWord);
			if (strcmp(TWord, "AT")		&& strcmp(TWord, "FOR")
			&&	strcmp(TWord, "FROM")	&& strcmp(TWord, "IN")
			&&	strcmp(TWord, "OF")		&& strcmp(TWord, "ON")
			&&	strcmp(TWord, "SPELL")	&& strcmp(TWord, "TO")
			&&	strcmp(TWord, "THE")	&& strcmp(TWord, "WITH"))
				break;
			*Word = '\0';
			*Num = 0;
		}
	}
}

void	DoInput()
{
	Loc = 0;
	LenBuf = Min(256, strlen(C1) - 1);
	if (LenBuf)
	{
		int		ILoop;

		for (ILoop = 0; ILoop < LenBuf; ILoop++)
			Buf[ILoop] = C1[ILoop + 1];
	}
}

/* Locate *Word* in first *Count* entries of *NameList* using binary search */
int		BinaryMatch(Word, Count, NameList)
char		*Word;
int			Count;
CmdListType	NameList;
{
	int		Lower, Upper, Posit, CmpVal;

	if (!*Word)
		return 0;

	Lower = 1;
	Upper = Count;
	do
	{
		Posit = (Lower + Upper) / 2;
		CmpVal = strcmp(NameList[Posit], Word);
		if (CmpVal <= 0)
			Lower = Posit + 1;
		if (CmpVal >= 0)
			Upper = Posit - 1;
	} while (Lower <= Upper);

	if (!CmpVal)
		return Posit;

	if (Posit == CmdListLen)
		return 0;

	if (!SubSet(Word, NameList[Posit]))
		Posit++;

	if (!SubSet(Word, NameList[Posit]))
		return 0;

	if (Posit <= CmdListLen && SubSet(Word, NameList[Posit + 1]))
	{
		sprintf(B1, "0%s is not unique.", Word);
		QOut(Term, B1);
		return -1;
	}

	return Posit;
}

/* Locate *Word* in first *Count* entries of *NameList* */
int		WordMatch(Word, Count, NameList)
char		*Word;
int			Count;
NameTList	NameList;
{
	int		ILoop, JLoop, Found;

	if (Count < 0 || Count > MaxNames)
		QOut(Term, "0 Sce03 - Bad parse count!");
	if (!Count || !*Word)
		return 0;

	for (ILoop = 1; ILoop <= Count; ILoop++)
		if (!strcmp(NameList[ILoop], Word))
			return ILoop;

	Found = FALSE;
	for (ILoop = 1; ILoop <= Count; ILoop++)
		if (Found = SubSet(Word, NameList[ILoop]))
			break;

	if (!Found)
		return 0;

	for (JLoop = ILoop + 1; JLoop <= Count; JLoop++)
		if (SubSet(Word, NameList[JLoop]))
			if (strcmp(NameList[ILoop], NameList[JLoop]))
			{
				sprintf(B1, "0%s is not unique.", Word);
				QOut(Term, B1);
				return -1;
			}

	return ILoop;
}

void	Punctuate(Num, Total, Str)
int		Num, Total;
char	*Str;
{
	if (Num != Total)
		strcat(Str, Total - Num < 2 ? " and " : ", ");
	if (!(Num % 3) && Num < Total)
	{
		QOut(Term, Str);
		strcpy(Str, "0");
	}
}

void	Pd(Rec, Line, PhraseNum, Brief, Cont, V)
int		Rec, Line, PhraseNum, Brief;
int		*Cont;
char	*V;
{
	int		NumPhrases, ILoop, LinePos, BrMark, VSub, Found;
	char	S[256];

	*Cont = FALSE;
	if (!Rec)
	{
		strcpy(V, "You're in an undefined room, report this!");
		return;
	}
	if (!Line)
	{
		*V = '\0';
		return;
	}

	Found = TRUE;
	BrMark = FALSE;
	if (Line > StrPerSeg)
	{
		strcpy(V, " Sce72 - Bad description index");
		return;
	}

	if (Rec != CurrentRec)
	{
		fseek(EDesc, (long)Rec * sizeof(DescBuf), 0);
		Found = fread((char *)DescBuf, sizeof(DescBuf), 1, EDesc);
		CurrentRec = Found ? Rec : 0;
	}

	if (Found)
		strcpy(S, DescBuf[Line - 1]);
	if (!Found || strlen(S) < 3)
		strcpy(S, "1//");

	LinePos = 1;
	NumPhrases = Min(9, Max(0, S[0] - '0'));
	if (!PhraseNum)
		PhraseNum = Rnd(NumPhrases);
	for (ILoop = 1; ILoop < PhraseNum; ILoop++)
		while (S[++LinePos] != '/')
			;
	S[LinePos] = ' ';

	VSub = 0;
	*V = '\0';

	while (LinePos < strlen(S) && S[LinePos] != '/')
		if (S[++LinePos] != '/')
			if (S[LinePos] == '#')
				BrMark = !BrMark;
			else
			if (!(BrMark && Brief))
			{
				if (S[LinePos] == '+')
				{
					*Cont = TRUE;
					S[LinePos] = '/';
				}
				else
				{
					V[VSub++] = S[LinePos];
					V[VSub] = '\0';
				}
			}
}

void	PrintDesc(Rec, Line, PhraseNum, Brief, Str1, Str2)
int		Rec, Line, PhraseNum, Brief;
char	*Str1, *Str2;
{
	int		Cont;
	char	Str3[256];

	Pd(Rec, Line, PhraseNum, Brief, &Cont, Str3);
	sprintf(Str2, "%s%s", Str1, Str3);
	while (Cont)
	{
		QOut(Term, Str2);
		Line++;
		Pd(Rec, Line, PhraseNum, Brief, &Cont, Str3);
		sprintf(Str2, "0%s", Str3);
	}
}
