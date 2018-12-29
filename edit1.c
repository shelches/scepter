#include "scepter.h"

#ifdef	PROTO
static void	ModPlayer(UserPoint Player, int OnLine);
static void	EditPlayer(EdFunc EditCmd, char *Which);
static void	EditFile(EdFunc EditCmd, char *Which);
static void	EditRoom(EdFunc EditCmd, int Rm);
static void	EditMonster(EdFunc EditCmd, char *Which, int MNum, int WhatCode);
static void	AssocErr(void);
#endif

static PLType	UsrParms[] =
{
			NULL, 0,
/*  1 */	"NA", DWord,	"ZZ", DWord,	"WE", DNum,		"LV", DNum,
/*  5 */	"CL", DWord,	"VI", DNum,		"MV", DNum,		"MA", DNum,
/*  9 */	"MM", DNum,		"EX", DNum,		"AC", DNum,		"RO", DNum,
/* 13 */	"BR", DFlag,	"ST", DNum,		"IN", DNum,		"DX", DNum,
/* 17 */	"PT", DNum,		"EC", DFlag,	"CO", DNum,		"SE", DWord,
/* 21 */	"SP", DNum,		"PW", DWord,	"MO", DNum,		"AS", DNum,
/* 25 */	"IV", DFlag,	"FA", DNum,		"MF", DNum,		"PO", DFlag,
/* 29 */	"PL", DFlag,	"HD", DFlag,	"TG", DFlag,	"MB", DFlag,
/* 33 */	"AG", DFlag,	"EV", DFlag,	"SU", DFlag,	"PG", DFlag,
/* 37 */	"NO", DFlag,	"SS", DNum,		"TS", DNum,		"BS", DNum,
/* 41 */	"PS", DNum,		"IT", DFlag
};

static PLType	RoomParms[] =
{
			NULL, 0,
/*  1 */	"DI", DNum,		"DR", DNum,		 "N", DNum,		 "S", DNum,
/*  5 */	 "E", DNum,		 "W", DNum,		 "U", DNum,		 "D", DNum,
/*  9 */	"OU", DNum,		"EN", DNum,		"ET", DNum,		"NO", DFlag,
/* 13 */	"SA", DFlag,	"AM", DFlag,	"DA", DFlag
};

static PLType	MonParms[] =
{
			NULL, 0,
/*  1 */	"DE", DFlag,	"BL", DFlag,	"FO", DFlag,	"GU", DFlag,
/*  5 */	"AT", DFlag,	"SR", DFlag,	"MO", DFlag,	"FL", DFlag,
/*  9 */	"AS", DFlag,	"LV", DNum,		"HI", DNum,		"MH", DNum,
/* 13 */	"RA", DNum,		"PE", DFlag,	"MA", DFlag,	"TR", DNum,
/* 17 */	"NA", DOther,	"FR", DFlag,	"IN", DFlag,	"RE", DFlag,
/* 21 */	"EN", DFlag,	"PO", DFlag,	"AM", DFlag,	"UN", DFlag,
/* 25 */	"SP", DFlag,	"PA", DNum,		"AC", DNum,		"DR", DNum,
/* 29 */	"DI", DNum,		"SU", DFlag,	"IM", DFlag,	"AR", DWord,
/* 33 */	"NI", DFlag,	"WA", DFlag,	"UQ", DFlag
};

void	Edit()
{
	NameTList	EditList;
	Alfa		EditCmd, What, Which;
	EdFunc		EditNum;
	int			Dummy, WhatCode, RmOrMonNum;
	static char	*CmdList[] =
	{
		"",
		"MODIFY",	"DELETE",		"DISPLAY",	"CREATE",	"COPY",
		"MOVE",		"QUIT",			"XQ",		"END",		"ROOM",
		"OBJECT",	"MONSTER",		"PLAYER",	"FILE",		"MLIST",
		"OLIST",	"DESCRIPTIO",	"ELIST",	"TLIST",	NULL
	};

	for (Dummy = 1; CmdList[Dummy]; Dummy++)
		strcpy(EditList[Dummy], CmdList[Dummy]);
	GetWord(EditCmd, &Dummy);
	CapAlfa(EditCmd);
	EditNum = WordMatch(EditCmd, 9,	EditList);
	if (EditNum <= E_NULL)
	{
		if (*EditCmd)
			QOut(Term, "0Illegal edit command.");
		return;
	}
	if (EditNum >= E_QUIT && EditNum <= E_END)
	{
		User->Entry = XCmd;
		if (!User->Brief)
			QOut(Term, "0Exiting the editor.");
		return;
	}

	GetWord(What, &Dummy);
	CapAlfa(What);
	WhatCode = WordMatch(What, 19, EditList);
	if (WhatCode <= E_END)
	{
		sprintf(B1, "0Bad operand - %s", What);
		QOut(Term, B1);
		return;
	}

	GetWord(Which, &RmOrMonNum);
	switch (WhatCode)
	{
		case 10:	/* ROOM */
			if (EditNum != E_DISPLAY && User->Assoc)
				AssocErr();
			else
			{
				EditRoom(EditNum, RmOrMonNum);
				if (EditNum == E_DISPLAY)
				{
					GetWord(EditCmd, &Dummy);
					Dummy = Min(NumRooms, Dummy);
					while (RmOrMonNum < Dummy)
					{
						RmOrMonNum++;
						QOut(Term, "0");
						EditRoom(EditNum, RmOrMonNum);
					}
				}
			}
			break;

		case 11:	/* OBJECT */
			EditObject(EditNum, Which, RmOrMonNum);
			break;

		case 12:	/* MONSTER */
			EditMonster(EditNum, Which, RmOrMonNum, WhatCode);
			break;

		case 13:	/* PLAYER */
			EditPlayer(EditNum, Which);
			break;

		case 14:	/* FILE */
			if (EditNum != E_DISPLAY && User->Assoc)
				AssocErr();
			else
				EditFile(EditNum, Which);
			break;

		case 15:	/* MLIST */
			EditMonster(EditNum, Which, RmOrMonNum, WhatCode);
			if (EditNum == E_DISPLAY && RmOrMonNum)
			{
				GetWord(EditCmd, &Dummy);
				Dummy = Min(RanMonLen, Dummy);
				while (RmOrMonNum < Dummy)
				{
					RmOrMonNum++;
					QOut(Term, "0");
					sprintf(B1, "0MList #%d", RmOrMonNum);
					QOut(Term, B1);
					EditMonster(EditNum, Which, RmOrMonNum, WhatCode);
				}
			}
			break;

		case 16:	/* OLIST */
			EditOList(EditNum, RmOrMonNum);
			if (EditNum == E_DISPLAY && RmOrMonNum)
			{
				GetWord(EditCmd, &Dummy);
				Dummy = Min(RanObjLen, Dummy);
				while (RmOrMonNum < Dummy)
				{
					RmOrMonNum++;
					QOut(Term, "0");
					sprintf(B1, "0OList #%d", RmOrMonNum);
					QOut(Term, B1);
					EditOList(EditNum, RmOrMonNum);
				}
			}
			break;

		case 17:	/* DESCRIPTION */
			EditDscrpt(EditNum, RmOrMonNum);
			break;

		case 18:	/* ELIST */
		case 19:	/* TLIST */
			if (User->Assoc && EditNum != E_DISPLAY)
				AssocErr();
			else
			if (EditNum == E_DELETE || EditNum == E_CREATE)
				QOut(Term, "0List entries cannot be created or deleted.");
			else
			{
				EditETList(EditNum, WhatCode, RmOrMonNum);
				if (EditNum == E_DISPLAY && RmOrMonNum)
				{
					GetWord(EditCmd, &Dummy);
					Dummy = Min(LEncounter, Dummy);
					while (RmOrMonNum++ < Dummy)
						EditETList(EditNum, WhatCode, RmOrMonNum);
				}
			}
			break;
	}
}

static void	ModPlayer(Player, OnLine)
UserPoint	Player;
int			OnLine;
{
	int		NP, WhichParm, Number, Flag;
	Alfa	Word, OldName;

	memcpy(&Parms, &UsrParms, sizeof(UsrParms));
	*OldName = '\0';
	for (NP = 0; WhichParm = GetEditParm(42, Word, &Number, &Flag); NP++)
	{
		if (User->Assoc
		&& (WhichParm ==  3 || WhichParm == 11 || WhichParm == 20
		||	WhichParm == 21 || WhichParm == 25 || WhichParm == 29
		||	WhichParm == 34 || WhichParm == 37))
		{
			AssocErr();
			Loc = LenBuf + 1;
			continue;
		}

		switch (WhichParm)
		{
			case 1:
				strcpy(OldName, Player->Name);
				strcpy(Player->Name, Word);
				break;

			case 2:
				if (!User->Master)
					QOut(Term, "0Only master DMs may alter DM privileges.");
				else
					switch (Cap(*Word))
					{
						case 'A':	/* should be used along with AS=# */
						case 'D':
							Player->SSJ		= TRUE;
							Player->Assoc	= 0;
							Player->Master	= FALSE;
							break;

						case 'M':
							Player->SSJ		= TRUE;
							Player->Assoc	= 0;
							Player->Master	= TRUE;
							break;

						case 'N':
							Player->SSJ		= FALSE;
							Player->Assoc	= 0;
							Player->Master	= FALSE;
							break;

						default:
							QOut(Term,
								"0ZZ must be Associate, DM, Master, or None.");
							break;
					}
				break;

			case 3:
				Player->Weight = Max(0,	Min(5000, Number));
				break;

			case 4:
				Player->Lvl = Max(0, Min(25, Number));
				break;

			case 5:
				switch(Cap(*Word))
				{
					case 'B':
						Player->Class = Barbarian;
						break;
					case 'F':
						Player->Class = Fighter;
						break;
					case 'T':
						Player->Class = Thief;
						break;
					case 'C':
						Player->Class = Cleric;
						break;
					case 'M':
						Player->Class = MagicUser;
						break;
					case 'R':
						Player->Class = Ranger;
						break;
					case 'P':
						Player->Class = Paladin;
						break;
					case 'D':
						Player->Class = DM;
						break;
					default:
						QOut(Term, "0Illegal class type.");
						break;
				}
				break;

			case 6:
				Player->Hits = Max(0, Min(2500, Number));
				break;

			case 7:
				Player->MaxHits = Max(0, Min(2500, Number));
				break;

			case 8:
				Player->Magic = Max(0, Min(2500, Number));
				break;

			case 9:
				Player->MaxMagic = Max(0, Min(2500, Number));
				break;

			case 10:
				Player->Experience = MaxL(0L, MinL(999999999L, (long)Number));

			case 11:
				Player->AC = Max(-32, Min(31, Number));
				break;

			case 12:
				Number = Max(1, Min(NumRooms, Number));
				if (OnLine)
				{
					StopFollow(Player);
					DeletePlayer(Player, Player->RmCode);
					PlacePlayer(Player, Number);
				}
				else
					Player->RmCode = Number;
				break;

			case 13:
				Player->Brief = Flag;
				break;

			case 14:
				Player->Str = Max(-10, Min(25, Number));
				break;

			case 15:
				Player->Int = Max(-10, Min(25, Number));
				break;

			case 16:
				Player->Dex = Max(-10, Min(25, Number));
				break;

			case 17:
				Player->Pty = Max(-10, Min(25, Number));
				break;

			case 18:
				Player->Echo = Flag;
				break;

			case 19:
				Player->Con = Max(-10, Min(25, Number));
				break;

			case 20:
				switch (Cap(*Word))
				{
					case 'M':
						Player->Sex = Male;
						break;
					case 'F':
						Player->Sex = Female;
						break;
					default:
						QOut(Term, "0Sex must be male or female.");
						break;
				}
				break;

			case 21:
				Player->Spells = Number;
				break;

			case 22:
				if (User->Master)
				{
					CapAlfa(Word);
					Player->PW = Hash(Word);
				}
				else
					QOut(Term, "0Only master DMs may alter passwords.");
				break;

			case 23:
				Player->Money = MaxL(0L, MinL(999999999L, (long)Number));
				break;

			case 24:
				if (User->Master)
					Player->Assoc = Number;
				else
					QOut(Term, "0Only master DMs may alter DM privileges.");
				break;

			case 25:
				Player->Invisible = Flag;
				break;

			case 26:
				Player->Fatigue = Max(0, Min(2500, Number));
				break;

			case 27:
				Player->MaxFatigue = Max(0,	Min(2500, Number));
				break;

			case 28:
				Player->Poisoned = Flag;
				break;

			case 29:
				Player->PlayTester = Flag;
				break;

			case 30:
				Player->Hidden = Flag;
				break;

			case 31:
				Player->TGuild = Flag;
				break;

			case 32:
				Player->MesBlock = Flag;
				break;

			case 33:
				Player->AGuild = Flag;
				break;

			case 34:
				Player->Evil = Flag;
				break;

			case 35:
				Player->Suck = Flag;
				break;

			case 36:
				Player->Plague = Flag;
				break;

			case 37:
				Player->NonExistant = Flag;
				break;

			case 38:
				Player->SSharp = Min(7,	Max(0,	Number / 10));
				break;

			case 39:
				Player->SThrust = Min(7, Max(0, Number / 10));
				break;

			case 40:
				Player->SBlunt = Min(7,	Max(0,	Number / 10));
				break;

			case 41:
				Player->SLong = Min(7, Max(0, Number / 10));
				break;

			case 42:
				Player->It = Flag;
				break;
		}
	}

	if (NP)
	{
		if (*OldName)
			ChangeUsrName(OldName, Player->Name);
		QOut(Term, "0Player modified.");
	}
}

static void	EditPlayer(EditCmd, Which)
EdFunc	EditCmd;
char	*Which;
{
	UserPoint	Player;

	if (Player = FindUser(Which))
		switch (EditCmd)
		{
			case E_MODIFY:
				ModPlayer(Player, TRUE);
				break;

			case E_DELETE:
				if (User->Assoc)
					AssocErr();
				else
				{
					int		ILoop, Count;

					sprintf(B1, "0### A lightning bolt hits %s and turns %s into ashes!",
							Player->Name, Pro[(int) Player->Sex]);
					Count = MsgTerm(SysMsg);
					for (ILoop = 0; ILoop < Count; ILoop++)
						QOut(TermList[ILoop], B1);
					Player->Dead = TRUE;
					Player->Hits = 0;
				}
				break;

			case E_DISPLAY:
				PlayerDisplay(Player);
				break;

			case E_CREATE:
				QOut(Term, "0You can't create players.");
				break;
		}
	else
		QOut(Term, "0Player not found.");
}

static void	EditFile(EditCmd, Which)
EdFunc	EditCmd;
char	*Which;
{
	UserType	Player;
	int			Exists;

	ReadPlayer(&Player, Which);
	if (!*Player.Name)
	{
		QOut(Term, "0Entry not found.");
		Exists = FALSE;
		strcpy(Player.Name, Which);
	}
	else
		Exists = TRUE;

	switch (EditCmd)
	{
		case E_MODIFY:
			ModPlayer(&Player, FALSE);
			WritePlayer(&Player);
			break;

		case E_DELETE:
			if (Exists)
			{
				ChangeUsrName(Player.Name, "");
				QOut(Term, "0Player erased.");
			}
			break;

		case E_DISPLAY:
			if (Exists)
				PlayerDisplay(&Player);
			break;

		case E_CREATE:
			if (Exists)
				QOut(Term, "0Player already exists in file.");
			else
			{
				Player.LastAccess = Today;
				QOut(Term, "0Entry created.");
				ModPlayer(&Player, FALSE);
				WritePlayer(&Player);
			}
			break;
	}
}

static void	EditRoom(EditCmd, Rm)
EdFunc	EditCmd;
int		Rm;
{
	if (Rm < 1 || Rm > NumRooms)
	{
		QOut(Term, "0Room number out of bounds.");
		return;
	}

	switch (EditCmd)
	{
		case E_MODIFY:
		{
			int			NP, WhichParm, Num, Flag;
			Alfa		Word;
			RoomType	*RmPt;

			RmPt = &Room[S(Rm)];
			memcpy(&Parms, &RoomParms, sizeof(RoomParms));
			for (NP = 0; WhichParm = GetEditParm(15, Word, &Num, &Flag); NP++)
				switch (WhichParm)
				{
					case 1:
						RmPt->DescCode = Max(0, Min(StrPerSeg, Num));
						break;

					case 2:
						RmPt->DescRec = Max(0, Min(255, Num));
						break;

					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
						Num = Max(0, Min(NumRooms, Num));
						RmPt->Adjoin[WhichParm - 3] = Num;
						break;

					case 9:
						RmPt->Out = Max(0, Min(NumRooms, Num));
						break;

					case 10:
						RmPt->WhichEncounter = Max(0, Min(LEncounter, Num));
						break;

					case 11:
						RmPt->EncounterTime = Max(0, Min(255, Num));
						break;

					case 12:
						RmPt->NotifyDM = Flag;
						break;

					case 13:
						RmPt->Safe = Flag;
						break;

					case 14:
						RmPt->AntiMagic = Flag;
						break;

					case 15:
						RmPt->Dark = Flag;
						break;
				}

			if (NP)
				QOut(Term, "0Room modified.");
			break;
		}

		case E_DELETE:
		{
			RoomType	*RmPt;

			RmPt = &Room[S(Rm)];
			if (RmPt->RmMonsterTail || RmPt->RmPlayerTail || RmPt->RmObjectTail)
				QOut(Term, "0Room not empty!");
			else
			{
				*RmPt = ProtoRoom;
				QOut(Term, "0Room initialized.");
			}
			break;
		}

		case E_COPY:
		{
			Alfa	Dummy;
			int		Rm2;

			GetWord(Dummy, &Rm2);
			if (Rm2 < 1 || Rm2 > NumRooms)
				QOut(Term, "0Room number out of bounds.");
			else
			{
				RoomType	Tmp;

				Tmp = Room[S(Rm)];
				Tmp.RmMonsterTail = NULL;
				Tmp.RmPlayerTail  = NULL;
				Tmp.RmObjectTail  = NULL;
				Room[S(Rm2)] = Tmp;
			}
			break;
		}

		case E_DISPLAY:
		{
			int			ILoop;
			RoomType	*RmPt;

			Rm = S(Rm);
			RoomDisplay(Rm, User->Brief);

			RmPt = &Room[Rm];
			strcpy(B1, "0");
			for (ILoop = 0; ILoop < 6; ILoop++)
			{
				sprintf(B2, "%c=%d, ",
						Cap(*DirList[ILoop]), RmPt->Adjoin[ILoop]);
				strcat(B1, B2);
			}

			if (RmPt->Out)
			{
				sprintf(B2, "OUT=%d, ", RmPt->Out);
				strcat(B1, B2);
			}

			sprintf(B2, "DR=%d, DI=%d, EN=%d, ET=%d.",
					RmPt->DescRec, RmPt->DescCode,
					RmPt->WhichEncounter, RmPt->EncounterTime);
			strcat(B1, B2);
			QOut(Term, B1);

			if (RmPt->NotifyDM)
				QOut(Term, "0DM notified upon entry.");
			if (RmPt->Safe)
				QOut(Term, "0Room is haven against attack.");
			if (RmPt->AntiMagic)
				QOut(Term, "0Room nullifies all forms of magic.");
			break;
		}

		case E_CREATE:
			if (!User->Master)
				QOut(Term, "0Only master DMs may create additional roomsegs.");
			else
				AddSeg();
			break;
	}
}

static void	EditMonster(EditCmd, Which, MNum, WhatCode)
EdFunc	EditCmd;
char	*Which;
int		MNum, WhatCode;
{
	MonsterPoint	Monster;
	int				Rm;
	Alfa			Dummy;

	if (WhatCode == 12)		/* 12 == monster */
		GetWord(Dummy, &Rm);
	else					/* 15 == mlist */
	{
		Rm = MNum;
		strcpy(Dummy, Which);
	}

	if (Rm <= 0 || Rm > NumRooms || *Dummy)
	{
		QOut(Term, "0Illegal location number.");
		return;
	}
	
	if (WhatCode == 12)
	{
		Rm = S(Rm);
		strcpy(Dummy, Which);
		Monster = FindMonster(Dummy, MNum, Room[Rm].RmMonsterTail);
		if (EditCmd == E_CREATE)
		{
			Monster = (MonsterPoint)malloc(sizeof(MonsterType));
			*Monster = ProtoMonster;
			strcpy(Monster->Name, Which);
			InsertMonster(Monster, Rm);
			QOut(Term, "0Monster created.");
		}
	}
	else
	if (Rm >= 1 && Rm <= RanMonLen)
	{
		Monster = (MonsterPoint)malloc(sizeof(MonsterType));
		fseek(MList, (long)Rm * sizeof(MonsterType), 0);
		fread((char *)Monster, sizeof(MonsterType), 1, MList);
	}
	else
	{
		QOut(Term, "0Bad MList number.");
		Monster = NULL;
	}

	if (!Monster)
	{
		QOut(Term, "0Monster not found.");
		return;
	}

	switch (EditCmd)
	{
		case E_COPY:
			if (WhatCode == 15)
			{
				GetWord(Dummy, &Rm);
				if (Rm < 1 || Rm > RanMonLen)
				{
					QOut(Term, "0Bad MList number.");
					free(Monster);
					EditCmd = E_NULL;
				}
			}
			break;

		case E_CREATE:
			if (WhatCode == 15 && User->Master)		/* master can init mlist */
			{
				*Monster = ProtoMonster;
				break;
			}
			/* else fall through */
		case E_MODIFY:
		{
			int		NP, WhichParm, Num, Flag;
			Alfa	Word;

			memcpy(&Parms, &MonParms, sizeof(MonParms));
			for (NP = 0; WhichParm = GetEditParm(35, Word, &Num, &Flag); NP++)
				switch (WhichParm)
				{
					case 1:
						Monster->Defend = Flag;
						break;
					case 2:
						Monster->Block = Flag;
						break;
					case 3:
						Monster->Follow = Flag;
						break;
					case 4:
						Monster->Guard = Flag;
						break;
					case 5:
						Monster->AtkLastAggr = Flag;
						break;
					case 6:
						if (Monster->SlowReact = Flag)
							Monster->Nice = FALSE;
						break;
					case 7:
						Monster->MoralReact = Flag;
						break;
					case 8:
						Monster->Flee = Flag;
						break;
					case 9:
						Monster->Assistance = Flag;
						break;
					case 10:
						Monster->Lvl = Max(0, Min(25, Num));
						break;
					case 11:
						Monster->Hits = Max(0, Min(2500, Num));
						break;
					case 12:
						Monster->MaxHits = Max(0, Min(2500, Num));
						break;
					case 13:
						Monster->AtkSpeed = Max(-10, Min(10, Num));
						break;
					case 14:
						Monster->Permanent = Flag;
						break;
					case 15:
						Monster->Magic = Flag;
						break;
					case 16:
						Monster->WhichObj = Max(0, Min(Num, ObjListLen));
						break;
					case 17:
					{
						char	TempName[21];

						strcpy(TempName, Monster->Name);
						ModName(Monster->Name);
						if (WhatCode == 12)		/* not mlist */
						{
							FixMonCount(Room[Rm].RmMonsterTail, TempName);
							FixMonCount(Room[Rm].RmMonsterTail, Monster->Name);
						}
						break;
					}
					case 18:
						if (Monster->FastReact = Flag)
							Monster->Nice = FALSE;
						break;
					case 19:
						Monster->Invisible = Flag;
						break;
					case 20:
						Monster->Regenerate = Flag;
						break;
					case 21:
						Monster->Drain = Flag;
						break;
					case 22:
						Monster->Poison = Flag;
						break;
					case 23:
						Monster->AntiMagic = Flag;
						break;
					case 24:
						Monster->Undead = Flag;
						break;
					case 25:
						Monster->MonSpells = Flag;
						break;
					case 26:
						Monster->MParley = Max(0, Min(30, Num));
						break;
					case 27:
						Monster->AC = Max(-25, Min(25, Num));
						break;
					case 28:
						Monster->DescRec = Max(0, Min(255, Num));
						break;
					case 29:
						Monster->DescCode = Max(0, Min(StrPerSeg, Num));
						break;
					case 30:
						if (Monster->SummonHelp = Flag)
							Monster->Unique = FALSE;
						break;
					case 31:
						if (Monster->ImmoralReact = Flag)
							Monster->Nice = FALSE;
						break;
					case 32:
						switch (Cap(*Word))
						{
							case 'A':
								Monster->Article = Cap(Word[1]) == 'N' ? An : A;
								break;
							case 'N':
								Monster->Article = None;
								break;
							case 'S':
								Monster->Article = Some;
								break;
							case 'T':
								Monster->Article = The;
								break;
						}
						break;
					case 33:
						if (Monster->Nice = Flag)
							Monster->SlowReact = Monster->FastReact =
							Monster->ImmoralReact = FALSE;
						break;
					case 34:
						Monster->Watch = Flag;
						break;
					case 35:
						if (Monster->Unique = Flag)
							Monster->SummonHelp = FALSE;
						break;
				}

			if (Monster->MaxHits < Monster->Hits)
				Monster->MaxHits = Monster->Hits;
			if (NP)
				QOut(Term, "0Monster modified.");
			break;
		}

		case E_DELETE:
			if (WhatCode == 12)
			{
				DeleteMonster(Monster, Rm);
				Destroy(Monster);
				QOut(Term, "0Monster disposed.");
			}
			else
				QOut(Term, "0MList cannot be disposed.");
			break;

		case E_DISPLAY:
			MonDisplay(Monster);
			break;
	}

	if (WhatCode == 15
	&& (EditCmd == E_MODIFY || EditCmd == E_CREATE || EditCmd == E_COPY))
	{
		FILE	*hdMList;

		fseek(MList, (long)Rm * sizeof(MonsterType), 0);
		fwrite((char *)Monster, sizeof(MonsterType), 1, MList);
		fflush(MList);

		if (hdMList = fopen(HDMLIST, FOPEN))
		{
			fseek(hdMList, (long)Rm * sizeof(MonsterType), 0);
			fwrite((char *)Monster, sizeof(MonsterType), 1, hdMList);
			fclose(hdMList);
			QOut(Term, "0File modified.");
		}
		free(Monster);
	}
}

int		GetEditParm(LPList, Word, Num, Flag)
int		LPList;
char	*Word;
int		*Num, *Flag;
{
	char	Parm[3];
	int		WhichParm;

	Parm[0] = Parm[1] = Parm[2] = '\0';
	*Word = '\0';
	*Num = 0;
	*Flag = FALSE;
	if (Buf[Loc] == ',')
		Loc++;
	while (Buf[Loc] == ' ' && Loc < LenBuf)
		Loc++;

	if (Loc >= LenBuf)
		return 0;

	Parm[0] = Cap(Buf[Loc++]);
	if (Buf[Loc] != '=' && Loc < LenBuf)
		Parm[1] = Cap(Buf[Loc++]);
	while (Cap(Buf[Loc]) >= 'A' && Cap(Buf[Loc]) <= 'Z')
		Loc++;
	if (Buf[Loc] != '=' || Loc >= LenBuf - 1)
	{
		sprintf(B1, "0Bad parameter: %s", Parm);
		QOut(Term, B1);
		return 0;
	}

	Loc++;

	for (WhichParm = 1; WhichParm <= LPList; WhichParm++)
		if (!strcmp(Parm, Parms[WhichParm].PName))
			break;

	/*****
	WhichParm = 0;
	while (WhichParm <= LPList)
		if (!strcmp(Parm, Parms[++WhichParm].PName))
			break;
	*****/

	if (WhichParm > LPList)
	{
		sprintf(B1, "0Unknown parameter: %s", Parm);
		QOut(Term, B1);
		return 0;
	}

	switch ((int)Parms[WhichParm].PType)
	{
		case DFlag:
			Buf[Loc] = Cap(Buf[Loc]);
			if (Buf[Loc] == 'T')
				*Flag = TRUE;
			else
			if (Buf[Loc] == 'F')
				*Flag = FALSE;
			else
			{
				WhichParm = 0;
				sprintf(B1, "0%s - must be true or false.", Parm);
				QOut(Term, B1);
			}

			while (Buf[Loc] != ' ' && Buf[Loc] != ',' && Loc < LenBuf)
				Loc++;
			break;

		case DNum:
			GetWord(Word, Num);
			if (*Word)
			{
				WhichParm = 0;
				sprintf(B1, "0%s - must be a number.", Parm);
				QOut(Term, B1);
			}
			break;

		case DWord:
			GetWord(Word, Num);
			if (!*Word && *Num)
			{
				WhichParm = 0;
				sprintf(B1, "0%s - only string allowed.", Parm);
				QOut(Term, B1);
			}
			break;
	}

	return WhichParm;
}

static void	AssocErr()
{
	QOut(Term, "0Sorry, associate DMs can't do that.");
}
