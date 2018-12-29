#include "scepter.h"

#ifdef	PROTO
static void	WhyDie(void);
static void	EnterWhyDie(void);
static void	DispStatus(UserPoint Plr);
#endif

void	Appeal()
{
	UserPoint	Dm;

	if (Dm = FindDm(UserTail))
	{
		QOut(Term, "0Ding dong!  You wake up the DM.");
		User->Pty = Max(-10, User->Pty - 1);
	}
	else
	{
		int		Exit, ILoop;

		Exit = 0;
		for (ILoop = 0; ILoop < 6; ILoop++)
			Exit += Room[User->RmCode].Adjoin[ILoop];
		Exit += Room[User->RmCode].Out;
		if (!Exit && !Room[User->RmCode].RmObjectTail)
		{
			QOut(Term, "0Sigh, very well...");
			StopFollow(User);
			DeletePlayer(User, User->RmCode);
			PlacePlayer(User, 1);
			QOut(Term, "0*** Poof *** You are elsewhere..");
		}
		else
			QOut(Term, "0Sorry, the DM isn't here right now.  Try again later.");
	}

	for (; Dm; Dm = FindDm(Dm->NextUser))
		if (!Dm->NonExistant)
		{
			char	*p, *s;

			sprintf(B1, "0*** %s summons your assistance.", User->Name);
			QOut(Dm->Trm, B1);
			sprintf(B1, "0*** Piety rating is %d.", User->Pty);
			QOut(Dm->Trm, B1);
			sprintf(B1, "0*** Recommended reaction:  ");

			p = Pro[(int)User->Sex];
			if (User->Pty < -2)
				s = "Put %s in a LOT of trouble.";
			else
			if (User->Pty < 3)
				s = "Make the situation even worse for %s.";
			else
			if (User->Pty < 6)
			{
				s = "Laugh in %s face.";
				p = User->Sex == Male ? "his" : "her";
			}
			else
			if (User->Pty < 9)
				s = "Neutral.  Ask %s what's wrong.";
			else
			if (User->Pty < 12)
				s = "Tepid.  Help %s, but not too much.";
			else
			if (User->Pty < 15)
				s = "Positive.  Help %s out of situation.";
			else
			if (User->Pty < 19)
				s = "Very positive.  Save %s immediately.";
			else
				s = "Save player and then send %s on a quest.";

			sprintf(B2, s, p);
			strcat(B1, B2);
			QOut(Dm->Trm, B1);
		}
}

void	Run()
{
	int		ILoop, Dir, GoodDir = FALSE;

	for (ILoop = 0; !GoodDir && ILoop < 4; ILoop++)
	{
		Dir = Rnd(7);
		if (Dir < 7)
			GoodDir = (Room[User->RmCode].Adjoin[Dir - 1]);
		else
			GoodDir = (Room[User->RmCode].Out);
	}

	if (!GoodDir)
		QOut(Term, "0You fail to escape!");
	else
	{
		int		DrWeap = FALSE, DrShield = FALSE;

		if (User->USWeap)
		{
			DeleteObject(User->USWeap, &User->ObjectTail);
			User->Weight -= User->USWeap->Weight;
			User->USWeap->Next = Room[User->RmCode].RmObjectTail;
			Room[User->RmCode].RmObjectTail = User->USWeap;
			User->USWeap = NULL;
			DrWeap = TRUE;
		}

		if (User->USShield)
		{
			DeleteObject(User->USShield, &User->ObjectTail);
			User->Weight -= User->USShield->Weight;
			User->USShield->Next = Room[User->RmCode].RmObjectTail;
			Room[User->RmCode].RmObjectTail = User->USShield;
			StopUsing(User, User->USShield);
			DrShield = TRUE;
		}

		sprintf(B1, "0%s ", User->Name);
		if (DrWeap || DrShield)
		{
			strcat(B1, User->Sex == Male ? "drops his " : "drops her ");
			if (DrWeap)
				strcat(B1, "weapon and ");
			if (DrShield)
				strcat(B1, "shield and ");
		}
		strcat(B1, "runs away like a blithering idiot!");

		LocalMsg(B1);
		QOut(Term, "0Aarggghhh!  You run like crazy!");

		StopFollow(User);
		DeletePlayer(User, User->RmCode);
		Room[User->RmCode].LastDir = Dir;
		if (Dir < 7)
			PlacePlayer(User, Room[User->RmCode].Adjoin[Dir - 1]);
		else
			PlacePlayer(User, Room[User->RmCode].Out);
		RoomDisplay(User->RmCode, FALSE);
	}
}

void	Pawn(Word, Num)
char	*Word;
int		Num;
{
	ObjectPoint	Object;

	if (W(User->RmCode) != 8)
	{
		QOut(Term, "0Go to the pawn shop first.");
		return;
	}

	if (Object = FindObject(Word, Num, User->ObjectTail))
	{
		unsigned	Amount;

		StopUsing(User, Object);
		Amount = Object->Price;
		switch ((int)Object->ObClass)
		{
			case Weap:
				if (Object->Object.Weap.Strikes < 100)
					Amount = (Amount * Object->Object.Weap.Strikes) / 100;
				break;
			case Shield:
				if (Object->Object.Shield.ShHits < 50)
					Amount = (Amount * Object->Object.Shield.ShHits) / 50;
				break;
			case Armor:
				if (Object->Object.Armor.ArmHits < 50)
					Amount = (Amount * Object->Object.Armor.ArmHits) / 50;
				break;
			case MagDevice:
				if (!Object->Object.MagDevice.NumCharges)
					Amount = 5;
				break;
		}

		if (User->It)
			Amount >>= 1;

		PrintObj(Object, TRUE, FALSE, B2);
		sprintf(B1, "0\"Well, I'll give you %d shillings for %s.", Amount, B2);
		QOut(Term, B1);
		QOut(Term, "0Is this to your satisfaction?\"");
		User->Entry = XSell;
		User->Data = Amount;
		User->USWeap = Object;
	}
	else
		QOut(Term, "0You don't own that item.");
}

void	Buy(Num, Rm)
int		Num, Rm;
{
	switch (W(Rm))
	{
		case 3:		/* bar */
			if (User->Money)
			{
				int		SecsDk;

				--User->Money;
				if (User->Drunk < RealTime)
					User->Drunk = RealTime;
				User->Drunk += 30;
				SecsDk = User->Drunk - RealTime;
				if (SecsDk <= 60)
					QOut(Term, "0That was good!");
				else
				if (SecsDk <= 120)
					QOut(Term, "0That's good! (HIC)");
				else
				if (SecsDk <= 180)
					QOut(Term, "0Thash gud!");
				else
				{
					StopFollow(User);
					DeletePlayer(User, Rm);
					Rm = Rnd(10);
					PlacePlayer(User, Rm);
					QOut(Term,
						"0You seem to have staggered off somewhere... You find");
					RoomDisplay(User->RmCode, FALSE);
				}
			}
			else
				QOut(Term, "0Bartender says, \"Your credit's no good here!\"");
			break;

		case 4:		/* newsstand */
			if (User->Money)
			{
				int		ILoop;

				--User->Money;
				QOut(Term, "0");
				sprintf(B1, "0((( The County Press.  Issue #%d, %d/%d. )))",
						NumRun, Today >> 7, Today & 0x3f);
				QOut(Term, B1);
				QOut(Term, "0");

				for (ILoop = 0; ILoop < 5; ILoop++)
					if (NewsBuf[ILoop][0])
					{
						sprintf(B1, "0%s", NewsBuf[ILoop]);
						QOut(Term, B1);
					}
				QOut(Term, "0");

				for (ILoop = 0; ILoop < 3; ILoop++)
					if (TLvl[ILoop])
					{
						static char	*adv[6] =
						{
							"unexpectedly", "suddenly", "heinously",
							"mysteriously", "horribly", "viciously"
						};
						static char	*ver[6] =
						{
							"murdered", "killed", "slain", "butchered",
							"\"terminated\"", "disemboweled"
						};

						PNth(TLvl[ILoop], B2);
						sprintf(B1, "0%s the %s level %s was %s %s yesterday.",
								TName[ILoop], B2, TClass[ILoop],
								adv[Rnd(6) - 1], ver[Rnd(6) - 1]);
						QOut(Term, B1);

						if (*TBuf[ILoop])
						{
							sprintf(B1, "0%s", TBuf[ILoop]);
							QOut(Term, B1);
						}
						QOut(Term, "0");
					}
			}
			else
				QOut(Term, "0\"Sorry buddy, you got no money.\"");
			break;

		case 10:	/* weapon shop */
			if (Num < 1 || Num > Min(RanObjLen, 20))
				QOut(Term, "0Item number out of bounds.");
			else
			{
				ObjectPoint	Object;

				Object = (ObjectPoint)malloc(sizeof(ObjectType));
				fseek(OList, (long)Num * sizeof(ObjectType), 0);
				fread((char *)Object, sizeof(ObjectType), 1, OList);
				if (Object->Price > User->Money)
				{
					QOut(Term, "0You don't have enough cash to buy that.");
					free(Object);
				}
				else
				{
					User->Money -= Object->Price;
					Object->Next = Room[User->RmCode].RmObjectTail;
					Room[User->RmCode].RmObjectTail = Object;
					sprintf(B1, "0\"Here you are, %s.  Thank you!\"",
							User->Sex == Male ? "sir" : "ma'am");
					QOut(Term, B1);
					QOut(Term,
						"0\"Oh, be sure and take it with you as you leave!\"");
				}
			}
			break;

		default:
			QOut(Term, "0Nothing to purchase here.");
			break;
	}
}

void	Catalog()
{
	int			ILoop;

	if (W(User->RmCode) != 10)
	{
		QOut(Term, "0I don't understand");
		return;
	}

	QOut(Term, "0Inventory of armaments for sale");
	QOut(Term, "0 To purchase, enter: BUY <Item Number>");
	QOut(Term, "0");
	QOut(Term, "0 # Type    Cost Weight Name");
	for (ILoop = 1; ILoop <= Min(20, RanObjLen); ILoop++)
	{
		ObjectType	ObjTemp;
		char		*p;

		fseek(OList, (long)ILoop * sizeof(ObjectType), 0);
		fread((char *)&ObjTemp, sizeof(ObjectType), 1, OList);

		switch ((int)ObjTemp.ObClass)
		{
			case Weap:	 p = "Weapon"; break;
			case Shield: p = "Shield"; break;
			case Armor:	 p = "Armor "; break;
			default:	 p = "Other "; break;
		}
		PrintObj(&ObjTemp, FALSE, TRUE, B2);
		sprintf(B1, "0%2d %s %5d %6d %s",
				ILoop, p, ObjTemp.Price, ObjTemp.Weight, B2);
		if (ObjTemp.ObClass == Weap)
			sprintf(B2, ", %d-%d hits.",
					ObjTemp.Object.Weap.MinHp, ObjTemp.Object.Weap.MaxHp);
		else
			strcpy(B2, ".");
		strcat(B1, B2);
		QOut(Term, B1);
	}
}

void	TagPlayer(Word)
char	*Word;
{
	UserPoint	Plyr;

	if (!User->It && !User->Master)
	{
		QOut(Term, "0You aren't It.");
		return;
	}

	Plyr = FindPlayer(Word, Room[User->RmCode].RmPlayerTail);
	if (!Plyr)
		QOut(Term, "0Player not here.");
	else
	if (Plyr == User)
		QOut(Term, "0You can't tag yourself!");
	else
	{
		User->Hidden = FALSE;
		User->It = FALSE;
		Plyr->It = TRUE;
		sprintf(B1, "0You tag %s!", Pro[(int)Plyr->Sex]);
		QOut(Term, B1);
		sprintf(B1, "0%s tags you!", User->Name);
		QOut(Plyr->Trm, B1);
		QOut(Plyr->Trm, "0You are It!");
	}
}

void	FollowPlayer(Word)
char	*Word;
{
	UserPoint	Leader;

	Leader = FindPlayer(Word, Room[User->RmCode].RmPlayerTail);
	if (!Leader || Leader->Invisible && !User->SSJ)
		QOut(Term, "0Player not here.");
	else
	if (Leader == User)
		QOut(Term, "0You can't follow yourself!");
	else
	{
		User->Follow = Leader;
		QOut(Term, "0Ok.");
		if (!User->Hidden && !User->Invisible)
		{
			sprintf(B1, "0%s follows you!", User->Name);
			QOut(Leader->Trm, B1);
		}
	}
}

void	Lose(Word)
char	*Word;
{
	UserPoint	FollowPlyr;

	FollowPlyr = FindPlayer(Word, Room [User->RmCode].RmPlayerTail);
	if (!FollowPlyr || FollowPlyr->Invisible && !User->SSJ)
		QOut(Term, "0Player not here.");
	else
	if (FollowPlyr->Follow != User)
		QOut(Term, "0Player not following you anyway.");
	else
	if (FollowPlyr->Class != Ranger
	&& (Rnd(3) == 1 || User->Weight < FollowPlyr->Weight))
	{
		FollowPlyr->Follow = NULL;
		sprintf(B1, "0You lose %s!", Pro[(int)FollowPlyr->Sex]);
		QOut(Term, B1);
	}
	else
	{
		QOut(Term, "0Didn't work!");
		sprintf(B1, "0%s fails to lose you!", User->Name);
		QOut(FollowPlyr->Trm, B1);
	}
}

static void	WhyDie()
{
	User->Status = SLogin;
	StopFollow(User);
	DeletePlayer(User, User->RmCode);
	if (Min(Min(TLvl[0], TLvl[1]), TLvl[2]) < User->Lvl)
	{
		int		ILoop, Indx;

		Indx = 2;
		for (ILoop = 1; ILoop >= 0; --ILoop)
			if (TLvl[ILoop] < TLvl[Indx])
				Indx = ILoop;
		User->Entry = XDead;
		TLvl[Indx] = User->Lvl;
		strcpy(TName[Indx], User->Name);
		strcpy(TClass[Indx], CName[(int)User->Class]);
	}
	else
		Dead();
}

static void	EnterWhyDie()
{
	int		ILoop, Found;

	ILoop = -1;
	Found = FALSE;
	while (!Found && ILoop < 2)
	{
		ILoop++;
		Found = TLvl[ILoop] == User->Lvl && !strcmp(TName[ILoop], User->Name);
	}

	if (!Found)
		QOut(Term, "0On second thought, forget it!");
	else
	{
		int		i;

		i = Min(77, LenBuf);
		strncpy(TBuf[ILoop], Buf, i + 1);
		TBuf[ILoop][i] = '\0';
	}
	Dead();
}

void	Quit(QuitHow)
int		QuitHow;
{
	int			Num;
	TimeType	Delay;
	Alfa		Word;

	Delay = RealTime - User->HitAtTime;
	GetWord(Word, &Num);
	User->Entry = XCmd;
	if (Cap(*Word) != 'Y')
		return;

	if (!Delay)
		QOut(Term, "0Not while you're being attacked!");
	else
	if (Delay < QuitWait)
	{
		sprintf(B1,
			"0Sorry, you must wait at least %d more seconds before quitting.",
			QuitWait - Delay);
		QOut(Term, B1);
	}
	else
	{
		if (QuitHow == 39)	/* suicide */
			sprintf(B1,
				"0### %s just killed %sself.  We shall all miss %s dearly.",
				User->Name, Pro[(int)User->Sex], Pro[(int)User->Sex]);
		else
			sprintf(B1, "0### %s has quit adventuring.", User->Name);
		AllMsg(B1);

		if (QuitHow == 39)	/* suicide */
			KillPlayer();
		else
		{
			QOut(Term, "0Character file updated.");
			Logoff(User);
		}

		QOut(Term, "0End Scepter.");
		QOut(Term, "2");
		QDea(Term);
		User = NULL;
	}
}

static void	DispStatus(Plr)
UserPoint	Plr;
{
	char	*Cp;

	sprintf(B1, "0%-10s %c%c%-10s ", Plr->Name, Plr->MesBlock ? '*' : ' ',
			Plr->It && User->SSJ ? '+' : ' ',
			Plr->Status == SNormal ? Plr->LastCmd : "login");

	if (Plr->Lvl >= 10)
		switch ((int)Plr->Class)
		{
			case Fighter:	Cp = Plr->Sex == Male ? "Lord" : "Lady";	break;
			case Thief:		Cp = "Master Thief";						break;
			case MagicUser:	Cp = "Wizard";								break;
			case Cleric:	Cp = "High Priest";							break;
			case Barbarian:	Cp = "Barbarian Chieftain";					break;
			case Ranger:	Cp = Plr->Sex == Male ? "Ranger Lord"
												  : "Ranger Lady";		break;
			case Paladin:	Cp = Plr->Sex == Male ? "Knight" : "Dame";	break;
			default:		Cp = CName[(int)Plr->Class];				break;
		}
	else
		Cp = CName[(int)Plr->Class];
	strcat(B1, Cp);

	if (User->SSJ)
	{
		sprintf(B2, " %d", Plr->Lvl);
		strcat(B1, B2);
	}

	if (Plr->NonExistant)
		strcat(B1, " (NonExist)");
	else
	if (Plr->Invisible)
		strcat(B1, " (Inv)");

	if (User->Master && Plr->Sex == Female)
		strcat(B1, " (F)");

	if (User->SSJ && Plr->Status == SNormal)
	{
		sprintf(B2, " (%d)", W(Plr->RmCode));
		strcat(B1, B2);
	}

	QOut(Term, B1);
}

void	GetStatus(Word)
char		*Word;
{
	UserPoint	Player;

	QOut(Term, "0");
	if (!User->Brief)
		QOut(Term, "0Name       S LastCmd    Class");
	if (*Word)
	{
		Player = FindUser(Word);
		if (!Player || Player->NonExistant || Player->Invisible && Player->SSJ)
			QOut(Term, "0Player not active.");
		else
			DispStatus(Player);
	}
	else
		for (Player = UserTail; Player; Player = Player->NextUser)
			if (User->SSJ
			||	(!Player->Invisible || !Player->SSJ)
			&&	Player->Status == SNormal)
				DispStatus(Player);
}

void	TextLimits(MesStart, MesEnd)
int		*MesStart, *MesEnd;
{
	int		KLoop;

	for (KLoop = 0; KLoop < LenBuf && Buf[KLoop] != '"'; KLoop++)
		;

	if (KLoop == LenBuf)
		*MesStart = 0;
	else
	{
		*MesStart = KLoop + 1;
		*MesEnd = LenBuf - 1;
		if (Buf[*MesEnd] == '"')
			--*MesEnd;
	}
}

static void	SendMsg(Tm, Verb, MStart, MEnd, NoID, NoQuote)
int		Tm;
char	*Verb;
int		MStart, MEnd, NoID, NoQuote;
{
	int		LineLen, JLoop, Length;

	if (NoID)
	{
		strcpy(B1, "0");
		LineLen = 1000;
	}
	else
	{
		sprintf(B1, "0%s%s%s", User->Name, Verb, NoQuote ? "" : " \"");
		LineLen = 79 - strlen(B1);
	}

	for (JLoop = MStart; JLoop <= Min(MEnd, MStart + LineLen); JLoop++)
		if (NoID && Buf[JLoop] == '\\')
		{
			QOut(Tm, B1);
			strcpy(B1, "0");
		}
		else
		{
			Length = strlen(B1);
			B1[Length] = Buf[JLoop];
			B1[Length + 1] = '\0';
		}

	if (MStart + LineLen < MEnd)
	{
		QOut(Tm, B1);
		strcpy(B1, "0");
		for (JLoop = MStart + LineLen + 1; JLoop <= MEnd; JLoop++)
		{
			Length = strlen(B1);
			B1[Length] = Buf[JLoop];
			B1[Length + 1] = '\0';
		}
	}

	if (!NoID && !NoQuote)
		strcat(B1, "\"");

	QOut(Tm, B1);
}

void	Talk(Word)
char	*Word;
{
	int		NoID;

	NoID = CmdCode == 36 || CmdCode == 37 || CmdCode == 38 || CmdCode == 82;
	if (*Word)
	{
		UserPoint	Plyr;
		int			MsgStart, MsgEnd, ILoop, Count, NumHearing;

		User->Hidden = FALSE;
		TextLimits(&MsgStart, &MsgEnd);
		if (MsgStart <= 0)
			QOut(Term, "0Missing quotation marks.  Send <who> \"text\"");
		else
		if (*Word != '"')
			switch (CmdCode)
			{
				case 36:	/* say */
				case 12:
				case 89:
					Plyr = FindPlayer(Word, Room[User->RmCode].RmPlayerTail);
					if (Plyr && Plyr->NonExistant)
						Plyr = NULL;
					if (Plyr)
					{
						SendMsg(Plyr->Trm, CmdCode == 89 ? "" : " whispers,",
								MsgStart, MsgEnd, NoID, CmdCode == 89);

						if (Rnd(100) <= 10 && CmdCode != 36)
						{
							Count = MsgTerm(Local);
							for (ILoop = 0; ILoop < Count; ILoop++)
								if (TermList[ILoop] != Plyr->Trm)
								{
									sprintf(B1,
										"0You overhear %s whispering to %s!",
										User->Name, Plyr->Name);
									QOut(TermList[ILoop], B1);
									SendMsg(TermList[ILoop], " is whispering: ",
											MsgStart, MsgEnd, FALSE, FALSE);
								}
						}
					}
					else
					{
						sprintf(B1, "0%s is not here.", Word);
						QOut(Term, B1);
					}
					break;

				case 37:	/* yell */
				case 13:
					QOut(Term, "0You can't yell to just one player!");
					break;

				case 82:	/* send and *logoff */
				case 38:
				case 14:
					if (CmdCode == 14 && Room[User->RmCode].AntiMagic)
						QOut(Term, "0Such magic does not work here.");
					else
					if (SpellCost(1, -5, 5))
					{
						Plyr = FindUser(Word);
						if (Plyr && Plyr->NonExistant && !User->SSJ)
							Plyr = NULL;
						if (Plyr)
						{
							if (CmdCode == 82)
								QOut(Plyr->Trm, "0");
							SendMsg(Plyr->Trm, "# flashes:",
									MsgStart, MsgEnd, NoID, FALSE);

							if (CmdCode == 82)
							{
								int		LogTerm;

								LogTerm = Plyr->Trm;
								if (Plyr == User)
									User = NULL;
								Logoff(Plyr);
								QOut(LogTerm, "2");
								Plyr = NULL;
							}
						}
						else
						{
							sprintf(B1, "0%s cannot be found.", Word);
							QOut(Term, B1);
						}
					}
					break;
			}
		else
			switch (CmdCode)
			{
				case 36:	/* say */
				case 12:
				case 89:
					NumHearing = MsgTerm(Local);
					if (User->Echo)
						TermList[NumHearing++] = User->Trm;
					for (ILoop = 0; ILoop < NumHearing; ILoop++)
						SendMsg(TermList[ILoop], CmdCode == 89 ? "" : " says,",
								MsgStart, MsgEnd, NoID, CmdCode == 89);
					break;

				case 37:	/* yell */
				case 13:
					NumHearing = MsgTerm(Yell);
					if (User->Echo)
						TermList[NumHearing++] = User->Trm;
					for (ILoop = 0; ILoop < NumHearing; ILoop++)
						SendMsg(TermList[ILoop], " yells,",
								MsgStart, MsgEnd, NoID, FALSE);
					break;

				case 38:	/* send */
				case 14:
					if (CmdCode == 14 && Room[User->RmCode].AntiMagic)
						QOut(Term, "0Such magic does not work here.");
					else
					if (SpellCost(2, -5, 7))
					{
						TalkHowType	TalkHow;

						if (NoID)
							TalkHow = Others;
						else
							TalkHow = NoBlock;

						Count = MsgTerm(TalkHow);
						for (ILoop = 0; ILoop < Count; ILoop++)
							SendMsg(TermList[ILoop], ": ",
									MsgStart, MsgEnd, NoID, FALSE);
					}
					break;
			}
	}
	else
		QOut(Term, "0Missing text to send.");

	Loc = LenBuf + 1;
}

void	DoCmd()
{
	if (User->Entry == XDead)
	{
		EnterWhyDie();
		return;
	}
	if (User->Dead || !User->Hits)
	{
		WhyDie();
		return;
	}

	if (RealTime > LastUpdate + UpdatePeriod
	&&	NUsers && User->Status != SLogin)
	{
		UpdateF();	/* update all the data files */
		LastUpdate = RealTime;
	}

	if (User->Entry == XSpell)
		Spell(0);
	else
	if (User->Entry == XSure)
		Quit(User->Data);
	else
	{
		if (RealTime >= User->LastHeal + HealWait)
		{
			User->LastHeal = RealTime;
			if (User->Poisoned)
			{
				QOut(Term, "0You feel the poison coursing through your veins!");
				User->Hits = Max(0,	User->Hits - Rnd(2) * User->Lvl);
				if (!User->Hits || User->Dead)
				{
					sprintf(B1,
						"0### %s the %s finally died from %s poisonous wounds.",
						User->Name, CName[(int)User->Class],
						Pro[(int)User->Sex]);
					AllMsg(B1);
				}
			}
			else
			{
				User->Magic =
						Min(User->MaxMagic, User->Magic + Min(User->Lvl, 5));
				User->Hits = Min(User->MaxHits, User->Hits + User->Lvl / 2 + 1);
				User->Fatigue = RealTime - User->HitAtTime >= 90 ?
						User->MaxFatigue
						: Min(User->MaxFatigue, User->Fatigue + User->Lvl);
			}
		}

		ExecuteCommand();
	}

	if (User && (!User->Hits || User->Dead))
		WhyDie();
}
