#include "scepter.h"

#ifdef	PROTO
static void	AnnounceLogin(void);
static void	SetStats(ChType Cl, int Vit, int Fat, int Mag, int Mony);
#endif

static void	AnnounceLogin()
{
	User->Status = SNormal;
	*User->LastCmd = '\0';
	if (User->LastAccess != Today)
	{
		User->LastAccess = Today;
		if (User->TGuild || User->AGuild)
		{
			User->Money = MaxL(0L, User->Money - User->Lvl * 20);
			if (!User->Money)
			{
				QOut(Term, "0You have not the funds to pay your dues!");
				QOut(Term, "0You have been expelled from the guild.");
				User->AGuild = User->TGuild = FALSE;
			}
			else
			{
				sprintf(B1, "0Daily %d guild fee paid in full.  Thank you!",
						User->Lvl * 20);
				QOut(Term, B1);
			}
		}
	}

	CmdCode = 0;
	sprintf(B1, "0### Please welcome %s, the %s.",
			User->Name, CName[(int) User->Class]);
	AllMsg(B1);

	if (!User->SSJ && (User->Invisible || User->NonExistant))
		User->NonExistant = User->Invisible = FALSE;
	if (User->Invisible)
	{
		sprintf(B1, "0You are currently %s.",
				User->NonExistant ? "nonexistant" : "invisible");
		QOut(Term, B1);
	}

	RoomDisplay(User->RmCode, User->Brief);
}

UserPoint	Login()
{
	UserPoint	NewUser;

	NUsers++;
	NewUser = malloc(sizeof(UserType));
	*NewUser = ProtoUser;
	sprintf(B1, "0Scepter Fantasy Role-Playing Game  Ver 2.4.3  (Run %d)",
			NumRun);
	QOut(Term, B1);

	if (*Notice)
	{
		sprintf(B1, "0Notice:  %s", Notice);
		QOut(Term, B1);
	}

	NewUser->Entry		= XName;
	NewUser->Status		= SLogin;
	NewUser->Trm		= Term;
	NewUser->XPid		= Pid;
	NewUser->LastCmd[0]	= '\0';
	NewUser->NextUser	= UserTail;
	UserTail = NewUser;

	return NewUser;
}

void	EnterSex()
{
	char	Ch;
	int		Num;
	Alfa	Word;

	GetWord(Word, &Num);
	Ch = Cap(*Word);
	User->Entry = XCmd;
	if (Ch == 'M')
		User->Sex = Male;
	else
	if (Ch == 'F')
		User->Sex = Female;
	else
	{
		User->Entry = XSex;
		Loc = LenBuf + 1;
		QOut(Term, "0Try again.");
		return;
	}

	PlacePlayer(User, 1);
	Loc = LenBuf + 1;
	AnnounceLogin();
}

void	NewPW()
{
	int		Dummy;
	Alfa	PassWord;

	GetWord(PassWord, &Dummy);
	CapAlfa(PassWord);
	if (*PassWord)
	{
		User->PW = Hash(PassWord);
		User->Entry = XSex;
	}
}

void	EnterStats()
{
	int		ILoop, Sum;
	int		Stat[5];

	for (Sum = ILoop = 0; ILoop < 5; ILoop++)
	{
		int		Num;
		Alfa	Dummy;

		GetWord(Dummy, &Num);
		Stat[ILoop] = Num;
		Sum += Num;
		if (Sum > 55 || Num < 5 || Num > 18)
		{
			QOut(Term, Sum > 55 ?
					"0Error.  Your average is greater than 11."
				  : "0Error.  A number is more than 18 or less than 5.");
			User->Entry = XStats;
			Loc	= LenBuf + 1;
			return;
		}
	}

	User->Str = Stat[0];
	User->Int = Stat[1];
	User->Dex = Stat[2];
	User->Pty = Stat[3];
	User->Con = Stat[4];
	User->Entry = XSkill;

	if (Loc >= LenBuf)
	{
		QOut(Term, "0Which weapon is your most skillful?");
		QOut(Term,
			 "0Choose from:  Sharp weapons, thrusting weapons, blunt weapons,");
		QOut(Term, "0or pole weapons.");
	}
}

void	EnterSkill()
{
	int		Ch, Num;
	Alfa	Word;

	User->Entry = XNewPW;
	GetWord(Word, &Num);
	Ch = Cap(Word[0]);
	switch (Ch)
	{
		case 'S': User->SSharp	= 1; break;
		case 'T': User->SThrust	= 1; break;
		case 'B': User->SBlunt	= 1; break;
		case 'P': User->SLong	= 1; break;
		default:
			User->Entry = XSkill;
			Loc = LenBuf + 1;
			QOut(Term, "0No such skill.  Try again.");
			break;
	}
}

static void	SetStats(Cl, Vit, Fat, Mag, Mony)
ChType	Cl;
int		Vit, Fat, Mag, Mony;
{
	User->Class = Cl;
	User->Hits = User->MaxHits = Vit;
	User->Fatigue = User->MaxFatigue = Fat;
	User->Magic = User->MaxMagic = Mag;
	User->Money = Mony;
}

void	ChooseClass()
{
	int		Num;
	Alfa	Word;

	User->Entry = XStats;
	GetWord(Word, &Num);
	CapAlfa(Word);
	if (!strcmp(Word, "STOP"))
	{
		Logoff(User);
		QOut(Term, "2");
		QDea(Term);
		return;
	}

	switch (*Word)
	{
		case 'F': SetStats(Fighter,	  8, 14, 2, 200); break;
		case 'T': SetStats(Thief,	  7, 10, 3, 130); break;
		case 'M': SetStats(MagicUser, 6,  9, 6, 150); break;
		case 'C': SetStats(Cleric,	  7, 11, 4,  80); break;
		case 'R': SetStats(Ranger,	  7, 11, 3, 150); break;
		case 'P': SetStats(Paladin,	 11,  8, 3, 140); break;
		default:
			User->Entry = XNewClass;
			Loc = LenBuf + 1;
			QOut(Term, "0No such class.  Try again.");
			return;
	}

	if (Loc >= LenBuf)
	{
		QOut(Term,
			"0Choose your character's attributes for strength, intelligence,");
		QOut(Term, "0  dexterity, piety and constitution.");
		QOut(Term,
			"0(Example:  15, 7, 9, 10, 9)  Each must be between 5 and 18, and");
		QOut(Term, "0the total average must be 11 or less.");
	}
}

void	MatchPW()
{
	int		Dummy;
	Alfa	PassWord;

	GetWord(PassWord, &Dummy);
	CapAlfa(PassWord);
	if (Hash(PassWord) != User->PW)
	{
		Logoff(User);
		QOut(Term, "0Wrong password, sorry.");
		QOut(Term, "2");
		QDea(Term);
	}
	else
	{
		PlacePlayer(User, User->RmCode);
		AnnounceLogin();
		User->Entry = XCmd;
		Loc = LenBuf + 1;
	}
}

void	GetName()
{
	UserPoint	TempNext, Other;
	UserType	DumUser;
	int			Dummy;
	Alfa		Nam, CapName;

	GetWord(Nam, &Dummy);
	strcpy(CapName, Nam);
	CapAlfa(CapName);

	if (!strcmp(CapName, "STOP"))
	{
		Logoff(User);
		QOut(Term, "2");
		QDea(Term);
		return;
	}

	if (Dummy || !*Nam)
	{
		QOut(Term, "0Bad characters in name.");
		Loc = LenBuf + 1;
		return;
	}

	for (Other = UserTail; Other; Other = Other->NextUser)
	{
		Alfa	Temp;

		strcpy(Temp, Other->Name);
		CapAlfa(Temp);
		if (!strcmp(Temp, CapName))
		{
			Logoff(User);
			QOut(Term, "0Name already in use!");
			QOut(Term, "2");
			QDea(Term);
			return;
		}
	}

	TempNext = User->NextUser;
	ReadPlayer(&DumUser, Nam);
	*User =	DumUser;

	User->Status	= SLogin;
	User->Trm		= Term;
	User->XPid		= Pid;
	User->NextUser	= TempNext;

	if (!User->SSJ
		&& (TaskClosed == 'C' || TaskClosed == 'T' && !User->PlayTester))
	{
		QOut(Term,
			"0As you approach the gates, a little gnome jumps out from behind");
		QOut(Term,
			"0a rock.  He whines, 'Go back!  The masters are not ready for");
		QOut(Term, "0you yet!'  The gnome then disappears into the brush.");
		Logoff(User);
		QOut(Term, "2");
		QDea(Term);
	}
	else if (!*User->Name)
	{
		strcpy(User->Name, Nam);
		User->Entry = XNewClass;
		if (Loc >= LenBuf)
		{
			QOut(Term, "0Please select your character's class:");
			QOut(Term,
				"0Fighter, Thief, Cleric, Paladin, Ranger, or Magic-User.");
		}
	}
	else if (IsGuest(User->Trm))
	{
		QOut(Term, "0Guests can't play saved characters, sorry.");
		Logoff(User);
		QOut(Term, "2");
		QDea(Term);
	}
	else
		User->Entry = XPassword;
}
