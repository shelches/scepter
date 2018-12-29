#include "scepter.h"

#ifdef	PROTO
static void	ModObject(ObjectPoint Object);
static void	SetClass(ObjectPoint Obj, ObClassType Class);
static void	BadClass(int WhichParm);
static ObjectPoint	InsertObject(ObjectPoint *Tail);
static void	ShowETList(int type, int Index);
#else
static ObjectPoint	InsertObject();
#endif

static PLType	ObjParms[] =
{
			NULL, 0,
/*  1 */	"NA", DOther,	"AR", DWord,	"CA", DFlag,	"WE", DNum,
/*  5 */	"VA", DNum,		"MA", DFlag,	"PE", DFlag,	"TY", DWord,
/*  9 */	"TO", DNum,		"MI", DNum,		"MH", DNum,		"SL", DNum,
/* 13 */	"WP", DNum,		"SF", DNum,		"SH", DNum,		"AF", DNum,
/* 17 */	"AH", DNum,		"DI", DNum,		"DR", DNum,		"IN", DFlag,
/* 21 */	"MU", DNum,		"SP", DNum,		"CC", DFlag,	"CT", DNum,
/* 25 */	"CI", DNum,		"XX", DNum,		"CL", DNum,		"DC", DFlag,
/* 29 */	"DT", DNum,		"DL", DNum,		"UL", DNum,		"CH", DNum,
/* 33 */	"RO", DNum,		"WT", DWord,	"DS", DFlag,	"LV", DNum,
/* 37 */	"CU", DFlag,	"TL", DFlag
};

static void	ModObject(Object)
ObjectPoint	Object;
{
	int		NP, WhichParm, Number, Flag;
	Alfa	Word;

	if (!Object)
	{
		QOut(Term, "0Object not found.");
		return;
	}

	memcpy(&Parms, &ObjParms, sizeof(ObjParms));
	for (NP = 0; WhichParm = GetEditParm(38, Word, &Number, &Flag); NP++)
		switch (WhichParm)
		{
			case 1:
				ModName(Object->Name);
				break;

			case 2:
				switch (Cap(*Word))
				{
					case 'A':
						Object->Article = Cap(Word[1]) == 'N' ? An : A;
						break;
					case 'N':
						Object->Article = None;
						break;
					case 'S':
						Object->Article = Some;
						break;
					case 'T':
						Object->Article = The;
						break;
				}
				break;

			case 3:
				Object->Carry = Flag;
				break;

			case 4:
				Object->Weight = Max(0,	Min(1000, Number));
				break;

			case 5:
				Object->Price = Max(0, Min(16000, Number));
				break;

			case 6:
				Object->Magic = Flag;
				break;

			case 7:
				Object->Permanent = Flag;
				break;

			case 8:
				CapAlfa(Word);
				switch (*Word)
				{
					case 'A':
						SetClass(Object, Armor);
						break;

					case 'C':
						SetClass(Object, Word[1] == 'A' ? Card
										: Word[1] == 'H' ? Chest : Coins);
						break;

					case 'D':
						SetClass(Object, Door);
						break;

					case 'K':
						SetClass(Object, Keys);
						break;

					case 'M':
						SetClass(Object, Word[1] == 'A' ? MagDevice : Misc);
						break;

					case 'P':
						SetClass(Object, Portal);
						break;

					case 'R':
						SetClass(Object, Ring);
						break;

					case 'S':
						SetClass(Object, Word[1] == 'C' ? Scroll : Shield);
						break;

					case 'T':
						SetClass(Object, Word[1] == 'E' ? Teleport : Treasure);
						break;

					case 'W':
						SetClass(Object, Weap);
						break;
				}
				break;

			case 9:
			{
				int		n = Max(0, Min(NumRooms, Number));

				if (Object->ObClass == Portal)
					Object->Object.Portal.ToWhere = n;
				else
				if (Object->ObClass == Door)
					Object->Object.Door.DToWhere = n;
				else
				if (Object->ObClass == Teleport)
					Object->Object.Teleport.TToWhere = n;
				else
					BadClass(WhichParm);
				break;
			}
			case 10:
				if (Object->ObClass == Weap)
					Object->Object.Weap.MinHp = Max(0, Min(127, Number));
				else
					BadClass(WhichParm);
				break;

			case 11:
				if (Object->ObClass == Weap)
					Object->Object.Weap.MaxHp = Max(0, Min(127, Number));
				else
					BadClass(WhichParm);
				break;

			case 12:
				if (Object->ObClass == Weap)
					Object->Object.Weap.Strikes = Max(0, Min(511, Number));
				else
					BadClass(WhichParm);
				break;

			case 13:
				if (Object->ObClass == Weap)
					Object->Object.Weap.WeaPlus = Max(-5, Min(10, Number));
				else
					BadClass(WhichParm);
				break;

			case 14:
			case 16:
			{
				int		n = Max(-5, Min(10, Number));

				if (Object->ObClass == Shield)
					Object->Object.Shield.ShPlus = n;
				else
				if (Object->ObClass == Armor)
					Object->Object.Armor.ArmPlus = n;
				else
					BadClass(WhichParm);
				break;
			}
			case 15:
			case 17:
			{
				int		n = Max(0, Min(511, Number));

				if (Object->ObClass == Shield)
					Object->Object.Shield.ShHits = n;
				else
				if (Object->ObClass == Armor)
					Object->Object.Armor.ArmHits = n;
				else
					BadClass(WhichParm);
				break;
			}
			case 18:
				Object->DescCode = Max(0, Min(StrPerSeg, Number));
				break;

			case 19:
				Object->DescRec = Max(0, Min(256, Number));
				break;

			case 20:
				Object->Invisible = Flag;
				break;

			case 21:
				if (Object->ObClass == Coins)
					Object->Object.Coins.Multiplier = Max(1, Min(127, Number));
				else
					BadClass(WhichParm);
				break;

			case 22:
			{
				int		n = Max(0, Min(Spellen, Number));

				if (Object->ObClass == Scroll)
					Object->Object.Scroll.Spell = n;
				else
				if (Object->ObClass == MagDevice)
					Object->Object.MagDevice.MSpell = n;
				else
				if (Object->ObClass == Ring)
					Object->Object.Ring.RSpell = n;
				else
					BadClass(WhichParm);
				break;
			}
			case 23:
				if (Object->ObClass == Chest)
					Object->Object.Chest.Closed = Flag;
				else
					BadClass(WhichParm);
				break;

			case 24:
				if (Object->ObClass == Chest)
					Object->Object.Chest.Trap = Max(0, Min(50, Number));
				else
					BadClass(WhichParm);
				break;

			case 25:
				if (Object->ObClass == Chest)
					Object->Object.Chest.NumInside = Max(0, Min(10, Number));
				else
					BadClass(WhichParm);
				break;

			case 26:
				QOut(Term, "0Not implemented yet.");
				break;

			case 27:
				if (Object->ObClass == Chest)
					Object->Object.Chest.Locked = Max(0, Min(1023, Number));
				else
					BadClass(WhichParm);
				break;

			case 28:
				if (Object->ObClass == Door)
					Object->Object.Door.DClosed = Flag;
				else
					BadClass(WhichParm);
				break;

			case 29:
				if (Object->ObClass == Door)
					Object->Object.Door.DTrap = Max(0, Min(50, Number));
				else
					BadClass(WhichParm);
				break;

			case 30:
				if (Object->ObClass == Door)
					Object->Object.Door.DLocked = Max(0, Min(1023, Number));
				else
					BadClass(WhichParm);
				break;

			case 31:
				if (Object->ObClass == Keys)
					Object->Object.Keys.UnLock = Max(0, Min(1023, Number));
				else
					BadClass(WhichParm);
				break;

			case 32:
				if (Object->ObClass == MagDevice)
					Object->Object.MagDevice.NumCharges =
						Max(0, Min(127, Number));
				else
				if (Object->ObClass == Keys)
					Object->Object.Keys.NumUses = Max(0, Min(127, Number));
				else
					BadClass(WhichParm);
				break;

			case 33:
				if (Object->ObClass == Teleport)
					Object->Object.Teleport.TActiveRm =
						Max(0, Min(NumRooms, Number));
				else
					BadClass(WhichParm);
				break;

			case 34:
				if (Object->ObClass == Weap)
					switch (Cap(Word[0]))
					{
						case 'B':
							Object->Object.Weap.WeapType = Blunt;
							break;
						case 'P':
							Object->Object.Weap.WeapType = Long;
							break;
						case 'S':
							Object->Object.Weap.WeapType = Sharp;
							break;
						case 'T':
							Object->Object.Weap.WeapType = Thrust;
							break;
						default:
							QOut(Term, "0Illegal weapon type.");
							break;
					}
				else
					BadClass(WhichParm);
				break;

			case 35:
				if (Object->ObClass == Door)
					Object->Object.Door.DSpring = Flag;
				else
					BadClass(WhichParm);
				break;

			case 36:
				if (Object->ObClass == MagDevice)
					Object->Object.MagDevice.MLevel = Max(0, Min(25, Number));
				else
				if (Object->ObClass == Ring)
					Object->Object.Ring.RLevel = Max(0, Min(25, Number));
				else
					BadClass(WhichParm);
				break;

			case 37:
				Object->Cursed = Flag;
				break;

			case 38:
				if (Object->ObClass == Door)
					Object->Object.Door.DToll = Flag;
				else
					BadClass(WhichParm);
				break;
		}

	if (Object->ObClass == Weap
	&&	Object->Object.Weap.MaxHp < Object->Object.Weap.MinHp)
		Object->Object.Weap.MaxHp = Object->Object.Weap.MinHp + 1;

	if (NP)
		QOut(Term, "0Object modified.");
}

void	ModName(Str)
char	*Str;
{
	while (Buf[Loc] != '"' && Loc < LenBuf)
		Loc++;
	if (Loc < LenBuf - 1)
	{
		int		ILoop;
		char	TempName[21];

		Loc++;
		*TempName = '\0';
		ILoop = 0;
		while (Buf[Loc] != '"' && Loc < LenBuf)
		{
			if (ILoop < 20)
			{
				TempName[ILoop++] = Buf[Loc];
				TempName[ILoop] = '\0';
			}
			Loc++;
		}

		Loc++;
		if (!strchr(TempName, '*') || !strchr(TempName, ',') || *TempName == ' ')
			QOut(Term, "0Illegal name.");
		else
			strcpy(Str, TempName);
	}
}

static void	SetClass(Obj, Class)
ObjectPoint	Obj;
ObClassType	Class;
{
	Obj->ObClass = Class;
	switch ((int) Class)
	{
		case Portal:
			Obj->Object.Portal.ToWhere = 1;
			Obj->Carry = FALSE;
			break;

		case Weap:
			Obj->Object.Weap.MinHp = 1;
			Obj->Object.Weap.MaxHp = 1;
			Obj->Object.Weap.Strikes = 1;
			Obj->Object.Weap.WeaPlus = 0;
			Obj->Object.Weap.WeapType = Sharp;
			break;

		case Shield:
			Obj->Object.Shield.ShPlus = 1;
			Obj->Object.Shield.ShHits = 5;
			break;

		case Armor:
			Obj->Object.Armor.ArmPlus = 1;
			Obj->Object.Armor.ArmHits = 5;
			break;

		case Coins:
			Obj->Object.Coins.Multiplier = 1;
			break;

		case Scroll:
			Obj->Object.Scroll.Spell = 1;
			break;

		case Chest:
			Obj->Object.Chest.Closed = FALSE;
			Obj->Object.Chest.Locked = 0;
			Obj->Object.Chest.Trap = 0;
			Obj->Object.Chest.NumInside = 0;
			Obj->Object.Chest.ObjectTail = NULL;
			break;

		case Door:
			Obj->Carry = FALSE;
			Obj->Object.Door.DToWhere = 1;
			Obj->Object.Door.DClosed = FALSE;
			Obj->Object.Door.DSpring = FALSE;
			Obj->Object.Door.DTrap = 0;
			Obj->Object.Door.DLocked = 0;
			break;

		case Keys:
			Obj->Object.Keys.UnLock = 0;
			Obj->Object.Keys.NumUses = 0;
			break;

		case MagDevice:
			Obj->Object.MagDevice.MSpell = 1;
			Obj->Object.MagDevice.NumCharges = 1;
			Obj->Object.MagDevice.MLevel = 1;
			break;

		case Teleport:
			Obj->Object.Teleport.TToWhere = 1;
			Obj->Object.Teleport.TActiveRm = 1;
			break;

		case Ring:
			Obj->Object.Ring.RSpell = 1;
			Obj->Object.Ring.RLevel = 1;
			break;
	}
}

static void	BadClass(WhichParm)
int		WhichParm;
{
	sprintf(B1, "0%s - bad parameter for this obtype.  Mod not made.",
			Parms[WhichParm].PName);
	QOut(Term, B1);
}

static ObjectPoint	InsertObject(Tail)
ObjectPoint	*Tail;
{
	ObjectPoint	Obj;

	Obj = (ObjectPoint)malloc(sizeof(ObjectType));
	*Obj = ProtoObject;
	Obj->Next = *Tail;
	*Tail = Obj;
	QOut(Term, "0Object created.");
	return Obj;
}

void	EditObject(EditCmd, Which, ONum)
EdFunc	EditCmd;
char	*Which;
int		ONum;
{
	ObjectPoint		Object, Container, PTemp;
	UserPoint		Player;
	MonsterPoint	Monster;
	Alfa			Word1, Word2;
	int				Rm;
	enum eEditT
	{
		InRoom, OnPlayerX, InContainer, OnMonster, InFile, ENone
	} EditT;

	EditT = ENone;
	Object = NULL;
	GetWord(Word1, &Rm);
	CapAlfa(Word1);
	if (!*Word1 && Rm)
	{
		if (Rm > NumRooms)
			QOut(Term, "0Illegal location number.");
		else
		{
			Rm = S(Rm);
			EditT = InRoom;
			if (EditCmd == E_CREATE)
			{
				PTemp = Room[Rm].RmObjectTail;
				Object = InsertObject(&PTemp);
				Room[Rm].RmObjectTail = PTemp;
			}
			else
				Object = FindObject(Which, ONum, Room[Rm].RmObjectTail);
		}
	}
	else
	if (!strcmp(Word1, "FILE"))
	{
		int		Num2;

		EditT = InFile;
		GetWord(Word2, &Num2);
		Player = (UserPoint)malloc(sizeof(UserType));
		ReadPlayer(Player, Word2);
		if (!*Player->Name)
		{
			free(Player);
			QOut(Term, "0Player entry not found.");
			EditT = ENone;
		}
		else
		if (EditCmd == E_CREATE)
		{
			PTemp = Player->ObjectTail;
			Object = InsertObject(&PTemp);
			Player->ObjectTail = PTemp;
		}
		else
			Object = FindObject(Which, ONum, Player->ObjectTail);
	}
	else
	{
		Player = FindUser(Word1);
		if (Player)
		{
			EditT = OnPlayerX;
			if (EditCmd == E_CREATE)
			{
				PTemp = Player->ObjectTail;
				Object = InsertObject(&PTemp);
				Player->ObjectTail = PTemp;
			}
			else
				Object = FindObject(Which, ONum, Player->ObjectTail);
		}
		else
		{
			GetWord(Word2, &Rm);
			if (Rm < 1 || Rm > NumRooms || *Word2)
				QOut(Term, "0Illegal room number.");
			else
			{
				EditT = OnMonster;
				Rm = S(Rm);
				Monster = FindMonster(Word1, ONum, Room[Rm].RmMonsterTail);
				if (Monster)
				{
					if (EditCmd == E_CREATE)
					{
						PTemp = Monster->ObjectTail;
						Object = InsertObject(&PTemp);
						Monster->ObjectTail = PTemp;
					}
					else
						Object = FindObject(Which, ONum, Monster->ObjectTail);
				}
				else
				{
					EditT = InContainer;
					Container = FindObject(Word1, ONum, Room[Rm].RmObjectTail);
					if (Container && Container->ObClass == Chest)
					{
						if (EditCmd == E_CREATE)
						{
							PTemp = Container->Object.Chest.ObjectTail;
							Object = InsertObject(&PTemp);
							Container->Object.Chest.ObjectTail = PTemp;
						}
						else
							Object = FindObject(Which, ONum,
										Container->Object.Chest.ObjectTail);
					}
				}
			}
		}
	}

	if (!Object || EditT == ENone)
	{
		QOut(Term, "0Object not found.");
		return;
	}

	if (EditCmd == E_MODIFY || EditCmd == E_CREATE)
	{
		int		OWeight = EditCmd == E_CREATE ? 0 : Object->Weight;

		ModObject(Object);
		if (EditT == OnPlayerX || EditT == InFile)
		{
			Player->Weight += Object->Weight - OWeight;
			if (Player->Weight > Player->Str * 10)
				QOut(Term,
					"0Warning:  Weight carried exceeds player's capacity.");
		}
	}
	else
	if (EditCmd == E_DISPLAY)
		ObjDisplay(Object);
	else
		switch ((int)EditT)
		{
			case InRoom:
				DeleteObject(Object, &Room[Rm].RmObjectTail);
				break;

			case OnMonster:
				DeleteObject(Object, &Monster->ObjectTail);
				break;

			case InContainer:
				DeleteObject(Object, &Container->Object.Chest.ObjectTail);
				break;

			case OnPlayerX:
			case InFile:
				StopUsing(Player, Object);
				DeleteObject(Object, &Player->ObjectTail);
				Player->Weight -= Object->Weight;
				break;
		}

	if (EditT == InFile)
	{
		WritePlayer(Player);
		FreePlayer(Player);
	}
	else
	if (EditT == InRoom && Object->ObClass == Door && EditCmd != E_DISPLAY)
	{
		sprintf(B1, "0Don't forget to %s companion door.",
				EditCmd == E_MODIFY ? "modify the"
			  : EditCmd == E_DELETE ? "delete the" : "create a");
		QOut(Term, B1);
	}

	if (EditCmd == E_DELETE)
	{
		free(Object);
		QOut(Term, "0Object deleted.");
	}
}

void	EditOList(EditCmd, OIndex)
EdFunc	EditCmd;
int		OIndex;
{
	if (EditCmd == E_DELETE)
		QOut(Term, "0OList cannot be disposed of.");
	else
	if (OIndex < 1 || OIndex > RanObjLen)
		QOut(Term, "0Illegal OList number.");
	else
	{
		ObjectType	Obj;

		fseek(OList, (long)OIndex * sizeof(ObjectType), 0);
		fread((char *)&Obj, sizeof(ObjectType), 1, OList);

		switch (EditCmd)
		{
			case E_CREATE:
				if (User->Master)
					Obj = ProtoObject;
				/* else fall through */
			case E_MODIFY:
			{
				FILE	*hdOList;

				ModObject(&Obj);
				fseek(OList, (long)OIndex * sizeof(ObjectType), 0);
				fwrite((char *)&Obj, sizeof(ObjectType), 1, OList);
				fflush(OList);

				if (hdOList = fopen(HDOLIST, FOPEN))
				{
					fseek(hdOList, (long)OIndex * sizeof(ObjectType), 0);
					fwrite((char *)&Obj, sizeof(ObjectType), 1, hdOList);
					fclose(hdOList);
					QOut(Term, "0File modified.");
				}
				QOut(Term, "0OList updated.");
				break;
			}

			case E_DISPLAY:
				ObjDisplay(&Obj);
				break;
		}
	}
}

void	EditDscrpt(EditCmd, Rec)
EdFunc	EditCmd;
int		Rec;
{
	int		Line, Found, ILoop;
	Alfa	Dummy;

	if (Rec < 1)
	{
		QOut(Term, "0Illegal EDesc record number.");
		return;
	}

	GetWord(Dummy, &Line);

	fseek(EDesc, (long)Rec * sizeof(DescBuf), 0);
	Found = fread((char *)DescBuf, sizeof(DescBuf), 1, EDesc);
	CurrentRec = Found ? Rec : 0;

	switch (EditCmd)
	{
		case E_MODIFY:
			if (!Found)
				QOut(Term, "0No such EDesc record.");
			else
			if (Line < 1 || Line > StrPerSeg)
				QOut(Term, "0Illegal EDesc line.");
			else
			{
				User->Entry = XDesc;
				User->Data = Rec * StrPerSeg + (Line - 1);
			}
			break;

		case E_DELETE:
			QOut(Term, "0EDesc records cannot be deleted.");
			break;

		case E_DISPLAY:
			if (!Found)
				QOut(Term, "0No such EDesc record.");
			else
			{
				sprintf(B1, "0EDesc record %d:", Rec);
				QOut(Term, B1);
				if (Line)
				{
					if (Line <= StrPerSeg)
					{
						sprintf(B1, "0%2d:  %s", Line, DescBuf[Line - 1]);
						QOut(Term, B1);
					}
					else
						QOut(Term, "0Illegal line number.");
				}
				else
					for (Line = 0; Line < StrPerSeg; Line++)
					{
						sprintf(B1, "0%2d:  %s", Line + 1, DescBuf[Line]);
						QOut(Term, B1);
					}
			}
			break;

		case E_CREATE:
			if (Found)
			{
				QOut(Term, "0EDesc record already exists.");
				break;
			}

			if (Rec > 1)
			{
				fseek(EDesc, (long)(Rec - 1) * sizeof(DescBuf), 0);
				if (!fread((char *)DescBuf, sizeof(DescBuf), 1, EDesc))
					QOut(Term, "0Record beyond end of file.");
				else
				{
					fseek(EDesc, (long)Rec * sizeof(DescBuf), 0);
					if (!fread((char *)DescBuf, sizeof(DescBuf), 1, EDesc))
					{
						FILE	*hdEDesc;

						memset((char *)DescBuf, ' ', sizeof(DescBuf));
						for (ILoop = 0; ILoop < StrPerSeg; ILoop++)
							*DescBuf[ILoop] = '\0';
						fseek(EDesc, (long)Rec * sizeof(DescBuf), 0);
						fwrite((char *)DescBuf, sizeof(DescBuf), 1, EDesc);
						fflush(EDesc);

						if (hdEDesc = fopen(HDDSCRPT, FOPEN))
						{
							fseek(hdEDesc, (long)Rec * sizeof(DescBuf), 0);
							fwrite((char *)DescBuf, sizeof(DescBuf), 1, hdEDesc);
							fclose(hdEDesc);
							QOut(Term, "0File modified.");
						}
						QOut(Term, "0EDesc record created.");
					}
					else
						QOut(Term, "0EDesc record already exists.");
				}
			}
			else
				QOut(Term, "0Illegal EDesc record.");

			CurrentRec = 0;
			break;
	}
}

void	EnterDesc()
{
	int		Count, Slash, ILoop;

	memset(B1, ' ', 80);
	Slash = -1;
	for (ILoop = 0; ILoop < Min(80, LenBuf); ILoop++)
	{
		B1[ILoop] = Buf[ILoop];
		if (Buf[ILoop] == '/')
			Slash++;
	}
	B1[ILoop] = '\0';

	Count = Buf[0] - '0';
	if ((Count != Slash || Buf[1] != '/') && LenBuf)
		QOut(Term, "0Bad description.");
	else
	{
		int		Rec, Line;
		FILE	*hdEDesc;

		Rec = User->Data / StrPerSeg;
		Line = User->Data % StrPerSeg + 1;

		if (Rec != CurrentRec)
		{
			fseek(EDesc, (long)Rec * sizeof(DescBuf), 0);
			fread((char *)DescBuf, sizeof(DescBuf), 1, EDesc);
		}

		strcpy(DescBuf[Line - 1], B1);
		fseek(EDesc, (long)Rec * sizeof(DescBuf), 0);
		fwrite((char *)DescBuf, sizeof(DescBuf), 1, EDesc);
		fflush(EDesc);

		if (hdEDesc = fopen(HDDSCRPT, FOPEN))
		{
			fseek(hdEDesc, (long)Rec * sizeof(DescBuf), 0);
			fwrite((char *)DescBuf, sizeof(DescBuf), 1, hdEDesc);
			fclose(hdEDesc);
			QOut(Term, "0File modified.");
		}
		CurrentRec = 0;

		QOut(Term, "0EDesc record updated.");
	}

	User->Entry = XEdit;
	Loc = LenBuf + 1;
}

void	EditETList(EditCmd, WhatCode, Index)
EdFunc	EditCmd;
int		WhatCode, Index;
{
	int		ILoop;

	/* EditCmd can only be E_MODIFY or E_DISPLAY */
	if (WhatCode == 18)		/* ELIST */
	{
		if (EditCmd == E_MODIFY)
		{
			if (Index > 0 && Index <= LEncounter)
			{
				Alfa	Dummy;
				int		e;

				for (ILoop = 0; ILoop < 8; ILoop++)
				{
					GetWord(Dummy, &e);
					PutEncIndex(Index, ILoop, e);
				}
				QOut(Term, "0EList modified.");
			}
			else
				QOut(Term, "0Illegal EList number.");
		}
		else
		if (!Index)
			for (ILoop = 1; ILoop <= LEncounter; ILoop++)
				ShowETList(0, ILoop);
		else
		if (Index > 0 && Index <= LEncounter)
			ShowETList(0, Index);
		else
			QOut(Term, "0Illegal EList number.");
	}
	else					/* TLIST */
	{
		if (EditCmd == E_MODIFY)
		{
			if (Index > 0 && Index <= ObjListLen)
			{
				Alfa	Dummy;
				int		o;

				for (ILoop = 0; ILoop < 8; ILoop++)
				{
					GetWord(Dummy, &o);
					PutObjIndex(Index, ILoop, o);
				}
				QOut(Term, "0TList modified.");
			}
			else
				QOut(Term, "0Illegal TList number.");
		}
		else
		if (!Index)
			for (ILoop = 1; ILoop <= ObjListLen; ILoop++)
				ShowETList(1, ILoop);
		else
		if (Index > 0 && Index <= ObjListLen)
			ShowETList(1, Index);
		else
			QOut(Term, "0Illegal TList number.");
	}
}

static void	ShowETList(type, Index)
int		type, Index;
{
	int		ILoop, Entry[8];

	for (ILoop = 0; ILoop < 8; ILoop++)
		Entry[ILoop] = GetETIndex(type, Index, ILoop);
	sprintf(B1, "0%3d.  %3d %3d %3d %3d %3d %3d %3d %3d.", Index,
		Entry[0], Entry[1], Entry[2], Entry[3],
		Entry[4], Entry[5], Entry[6], Entry[7]);
	QOut(Term, B1);
}
