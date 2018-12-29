#include "scepter.h"

#ifdef	PROTO
static void	ReadSeg(int SlotNum, int SegNum);
static void	EraseTail(ObjectPoint Object);
static void	StopPlyrAtk(MonsterPoint Monster, RmCodeType Rm);
static void	StopOtherAtk(UserPoint Player, RmCodeType Rm);
static void	CleanRm(RmCodeType Rm);
static void	InActive(int Rm);
static int	OkSend(UserPoint Usr);
static UserPoint	MatchUser(UserPoint Usr);
#else
static UserPoint	MatchUser();
#endif

void	OpenCave()
{
	if (!(ECaves = fopen(EDUNGEON, FOPEN)))
		Abort(" Sce910 - Error opening dungeon file.");
}

void	CloseCave()
{
	if (fclose(ECaves))
		Abort(" Sce911 - Error closing dungeon file.");
}

int		W(Rm)
int		Rm;
{
	int		Where = 0;

	if (Rm >= 0 && Rm < RmLimit)
	{
		Where = SlotTbl[Rm / 10] * 10 + Rm % 10;
		if (Where < 0 || Where > NumRooms)
			Where = 0;
	}
	return Where;
}

void	Condition(Plyr)
UserPoint	Plyr;
{
	sprintf(B1,
		"0You have %d/%d vitality pts, %d/%d fatigue pts, and %d/%d magic pts.",
			Plyr->Hits, Plyr->MaxHits, Plyr->Fatigue, Plyr->MaxFatigue,
			Plyr->Magic, Plyr->MaxMagic);
	QOut(Term, B1);
}

void	PlayerDisplay(Plyer)
UserPoint	Plyer;
{
	ObjectPoint	Obj;
	int			Count;

	PNth(Plyer->Lvl, B2);
	sprintf(B1, "0%s, the %s level %s",
			Plyer->Name, B2, CName[(int)Plyer->Class]);
	if (Plyer->PlayTester)
		strcat(B1, " (PlayTester)");
	if (Plyer->SSJ)
	{
		if (Plyer->Assoc)
			sprintf(B1, " (Assoc DM %d)", Plyer->Assoc);
		else
			strcat(B1, Plyer->Master ? " (Master DM)" : " (DM)");
	}
	if (User->SSJ)
	{
		sprintf(B2, " in room %d/%d", Plyer->RmCode, W(Plyer->RmCode));
		strcat(B1, B2);
	}
	strcat(B1, ".");
	QOut(Term, B1);

	if (Plyer->Invisible)
		QOut(Term, Plyer->NonExistant ? "0You are nonexistant."
									  : "0You are currently invisible.");
	if (Plyer->TGuild)
		QOut(Term, "0You are a member of the Thieves Guild.");
	if (Plyer->AGuild)
		QOut(Term, "0You are a member of the Assassins Guild.");
	if (Plyer->Evil)
		QOut(Term, "0You have fallen into the forces of evil.");
	if (Plyer->It)
		QOut(Term, "0You are It.");
	if (Plyer->Suck)
		QOut(Term, "0You suck!");

	sprintf(B1,
		"0With %d/%d vitality pts, %d/%d fatigue pts, and %d/%d magic pts.",
		Plyer->Hits, Plyer->MaxHits, Plyer->Fatigue, Plyer->MaxFatigue,
		Plyer->Magic, Plyer->MaxMagic);
	QOut(Term, B1);

	if (Plyer->Plague)
		QOut(Term, "0You are infected with the plague!");
	if (Plyer->Poisoned)
		QOut(Term, "0You are dying from poison!");
	if (Plyer->Follow)
	{
		sprintf(B1, "0You are following %s.", Plyer->Follow->Name);
		QOut(Term, B1);
	}
	sprintf(B1, "0You have a defensive armor class of %d.", Plyer->AC);
	QOut(Term, B1);
	sprintf(B1, "0Str=%d, Int=%d, Dex=%d, Pty=%d, Con=%d.",
			Plyer->Str, Plyer->Int, Plyer->Dex, Plyer->Pty, Plyer->Con);
	QOut(Term, B1);
	sprintf(B1,
			"0Weapon skills:  Sharp-%d%%, Thrust-%d%%, Blunt-%d%%, Pole-%d%%.",
			Plyer->SSharp * 10, Plyer->SThrust * 10,
			Plyer->SBlunt * 10, Plyer->SLong * 10);
	QOut(Term, B1);

	PrintExp(Plyer);
	QOut(Term, "0");
	sprintf(B1, "0You are carrying %d lbs of items:", Plyer->Weight);
	QOut(Term, B1);

	for (Count = 0, Obj = Plyer->ObjectTail; Obj; Obj = Obj->Next)
	{
		PrintObj(Obj, FALSE, TRUE, B2);
		sprintf(B1, "0 %c %d) %s",
			Obj == Plyer->USWeap
		||	Obj == Plyer->USShield
		||	Obj == Plyer->USArm
		||	Obj == Plyer->USRingL
		||	Obj == Plyer->USRingR ? '*' : ' ', ++Count, B2);
		QOut(Term, B1);
	}
	if (Count > MaxObjs)
	{
		sprintf(B1, "0Only the first %d items will be saved when you log out.",
				MaxObjs);
		QOut(Term, B1);
	}

	if (User->SSJ)
	{
		sprintf(B1, "0Last logged in on %d/%d.",
				Plyer->LastAccess >> 7, Plyer->LastAccess & 0x3f);
		QOut(Term, B1);
	}
}

static void	ReadSeg(SlotNum, SegNum)
int		SlotNum, SegNum;
{
	int		RLoop, T;

	SlotTbl[SlotNum] = SegNum;
	fseek(ECaves, 10L * SegNum * sizeof(RoomType), 0);

	T = SlotNum * 10;
	for (RLoop = 0; RLoop < 10; RLoop++)
	{
		int		Rm, WRm;

		Rm = RLoop + T;
		fread((char *)&Room[Rm], sizeof(RoomType), 1, ECaves);
		Room[Rm].RmPlayerTail = NULL;
		WRm = W(Rm);
		if (Room[Rm].RmObjectTail)
			ReadUObj(WRm, &Room[Rm].RmObjectTail, 0);
		if (Room[Rm].RmMonsterTail)
			ReadUMon(WRm, &Room[Rm].RmMonsterTail);
	}
}

void	WriteSeg(SlotNum, SegNum, Erase)
int		SlotNum, SegNum;
int		Erase;
{
	int		RLoop, T;

	fseek(ECaves, 10L * SegNum * sizeof(RoomType), 0);
	T = SlotNum * 10;
	for (RLoop = 0; RLoop < 10; RLoop++)
	{
		int		Rm, WRm;

		Rm = RLoop + T;
		fwrite((char *)&Room[Rm], sizeof(RoomType), 1, ECaves);
		fflush(ECaves);
		WRm = W(Rm);

		if (Room[Rm].RmObjectTail)
		{
			WriteUObj(WRm, Room[Rm].RmObjectTail, 0);
			if (Erase)
				EraseTail(Room[Rm].RmObjectTail);
		}
		else
			DeleteUObj(WRm);

		if (Room[Rm].RmMonsterTail)
		{
			WriteUMon(WRm, Room[Rm].RmMonsterTail);
			if (Erase)
			{
				MonsterPoint	Monster, Mon2;

				for (Monster = Room[Rm].RmMonsterTail; Monster; Monster = Mon2)
				{
					EraseTail(Monster->ObjectTail);
					Mon2 = Monster->Next;
					free(Monster);
				}
			}
		}
		else
			DeleteMon(WRm);
	}
}

static void	EraseTail(Object)
ObjectPoint	Object;
{
	while (Object)
	{
		ObjectPoint	Obj2;

		Obj2 = Object->Next;
		ODestroy(Object);
		Object = Obj2;
	}
}

/* Return physical loc of room in room list */
S(Rm)
int		Rm;
{
	int		ILoop, SegNum;
	static int	LastOut;

	if (Rm < 1 || Rm > NumRooms)
		Abort(" Sce205 - Room # out of bounds!");

	SegNum = Rm / 10;
	for (ILoop = 0; ILoop <= MaxUsers; ILoop++)
		if (SlotTbl[ILoop] == SegNum)
			return (ILoop * 10 + Rm - SegNum * 10);

	for (ILoop = 0; ILoop <= MaxUsers && SlotTbl[ILoop] > -1; ILoop++)
		;
	if (ILoop > MaxUsers)
		for (ILoop = 0; ILoop <= MaxUsers; ILoop++)
			if (Active[(ILoop + LastOut) % MaxPlusOne] == 0)
			{
				ILoop = (ILoop + LastOut) % MaxPlusOne;
				LastOut = ILoop + 1;
				break;
			}
	if (ILoop > MaxUsers)
		Abort(" Sce206 - Room buffer full!");
	if (SlotTbl[ILoop] > -1)
	{
		WriteSeg(ILoop, SlotTbl[ILoop], TRUE);
		SlotTbl[ILoop] = -1;
	}

	ReadSeg(ILoop, SegNum);
	return (ILoop * 10 + Rm - SegNum * 10);
}

void	AddSeg()
{
	if (NumSegs > MaxSegs)
		QOut(Term, "0 Sorry, max room limit reached.");
	else
	{
		int		ILoop;

		for (ILoop = 0; ILoop <= MaxUsers && Active[ILoop] > 0; ILoop++)
			;
		if (ILoop > MaxUsers)
			QOut(Term, "0 Sce209 - Room buffer full.");
		else
		{
			int		JLoop, T;

			if (SlotTbl[ILoop] > -1)
				WriteSeg(ILoop, SlotTbl[ILoop], TRUE);

			T = ILoop * 10;
			for (JLoop = 0; JLoop < 10; JLoop++)
				Room[JLoop + T] = ProtoRoom;

			SlotTbl[ILoop] = NumSegs;
			T = NumSegs * 10;
			sprintf(B1, "0New rooms %d to %d created.", T, T + 9);
			QOut(Term, B1);
			NumSegs++;
			WriteSeg(ILoop, NumSegs - 1, FALSE);
		}
	}

	NumRooms = NumSegs * 10 - 1;
}

static void	StopPlyrAtk(Monster, Rm)
MonsterPoint	Monster;
RmCodeType		Rm;
{
	UserPoint	Plyer;

	for (Plyer = Room[Rm].RmPlayerTail; Plyer; Plyer = Plyer->Next)
		if (Plyer->DefMon == Monster)
			Plyer->DefMon = NULL;
}

static void	StopOtherAtk(Player, Rm)
UserPoint	Player;
RmCodeType	Rm;
{
	MonsterPoint	Mon;
	UserPoint		OtherPly;

	Player->DefMon = NULL;
	Player->DefPlayer = NULL;

	for (OtherPly = Room[Rm].RmPlayerTail; OtherPly; OtherPly = OtherPly->Next)
		if (OtherPly->DefPlayer == Player)
			OtherPly->DefPlayer = NULL;

	for (Mon = Room[Rm].RmMonsterTail; Mon; Mon = Mon->Next)
	{
		if (Mon->DefPlayer == Player)
		{
			Mon->DefPlayer = NULL;
			if (Mon->MReact >= 5)
				Mon->MReact = 0;
		}
		if (Mon->CharmPlayer == Player)
			Mon->CharmPlayer = NULL;
	}
}

void	DeleteMonster(Pt, Rm)
MonsterPoint	Pt;
RmCodeType		Rm;
{
	if (!Pt)
		Abort(" Sce86 - Cannot delete NULL monster!");

	if (Pt == Room[Rm].RmMonsterTail)
		Room[Rm].RmMonsterTail = Room[Rm].RmMonsterTail->Next;
	else
	{
		MonsterPoint	Pt2;

		for (Pt2 = Room[Rm].RmMonsterTail; Pt2; Pt2 = Pt2->Next)
			if (Pt2->Next == Pt)
			{
				Pt2->Next = Pt->Next;	/* remove from list */
				break;
			}

		if (!Pt2)
			Abort(" Sce05 - Deleted mon not found");
	}

	FixMonCount(Room[Rm].RmMonsterTail, Pt->Name);
	if (Room[Rm].RmPlayerTail)
		StopPlyrAtk(Pt, Rm);
}

void	InsertMonster(Monster, Rm)
MonsterPoint	Monster;
RmCodeType		Rm;
{
	Monster->Next = Room[Rm].RmMonsterTail;
	Room[Rm].RmMonsterTail = Monster;
	FixMonCount(Room[Rm].RmMonsterTail, Monster->Name);
}

/* remove old monsters and objects that don't have the permanent bit set */
static void	CleanRm(Rm)
RmCodeType	Rm;
{
	MonsterPoint	NextMon, OldMon;
	ObjectPoint		NextObj, OldObj;

	for (OldObj = Room[Rm].RmObjectTail; OldObj; OldObj = NextObj)
	{
		NextObj = OldObj->Next;
		if (!OldObj->Permanent)
		{
			DeleteObject(OldObj, &Room[Rm].RmObjectTail);
			ODestroy(OldObj);
		}
	}

	for (OldMon = Room[Rm].RmMonsterTail; OldMon; OldMon = NextMon)
	{
		NextMon = OldMon->Next;
		if (OldMon->Permanent)
			OldMon->Hits = OldMon->MaxHits;
		else
		{
			DeleteMonster(OldMon, Rm);
			free(OldMon);
		}
	}
}

void	PlacePlayer(Plyr, Rm)
UserPoint	Plyr;
RmCodeType	Rm;
{
	int		NewRm;

	NewRm = S(Rm);
	Active[NewRm / 10]++;
	Plyr->Next = Room[NewRm].RmPlayerTail;
	Room[NewRm].RmPlayerTail = Plyr;
	Plyr->RmCode = NewRm;
}

void	DeletePlayer(Pt, Rm)
UserPoint	Pt;
RmCodeType	Rm;
{
	if (!Pt)
		Abort(" Sce87 - Cannot delete NULL player!");

	if (Pt == Room[Rm].RmPlayerTail)
	{
		Room[Rm].RmPlayerTail = Room[Rm].RmPlayerTail->Next;
		if (!Room[Rm].RmPlayerTail)
			CleanRm(Pt->RmCode);
		else
		if (!Room[Rm].RmPlayerTail->Hits)
			CleanRm(Pt->RmCode);
	}
	else
	{
		UserPoint	Pt2;

		for (Pt2 = Room[Rm].RmPlayerTail; Pt2; Pt2 = Pt2->Next)
			if (Pt2->Next == Pt)
			{
				Pt2->Next = Pt->Next;
				break;
			}

		if (!Pt2)
			QOut(Term, "0 Sce06 - Deleted player not found!");
	}

	StopOtherAtk(Pt, Rm);
	Pt->Follow = NULL;
	InActive(Rm);
}

static void	InActive(Rm)
int		Rm;
{
	int		Seg;

	Seg = Rm / 10;
	if (Active[Seg] <= 0)
		Abort(" Sce222 - Non active seg assumed active!");
	--Active[Seg];
}

void	DeleteUser(Pt)
UserPoint	Pt;
{
	if (!Pt || !UserTail)
		Abort(" Sce88 - Cannot delete NULL user!");

	if (Pt == UserTail)
		UserTail = UserTail->NextUser;
	else
	{
		UserPoint	Pt2;

		for (Pt2 = UserTail; Pt2; Pt2 = Pt2->NextUser)
			if (Pt2->NextUser == Pt)
			{
				Pt2->NextUser = Pt->NextUser;
				break;
			}

		if (!Pt2)
			Abort(" Sce08 - Deleted record not found!");
	}

	if (Pt->Status != SLogin)
		DeletePlayer(Pt, Pt->RmCode);

	Pt->RmCode = W(Pt->RmCode);
	StopFollow(Pt);
}

void	StopFollow(Plyr)
UserPoint	Plyr;
{
	UserPoint	FollowPlyr;

	for (FollowPlyr = UserTail; FollowPlyr; FollowPlyr = FollowPlyr->NextUser)
		if (FollowPlyr->Follow == Plyr)
			FollowPlyr->Follow = NULL;
}

void	RoomDisplay(Rm, Brief)
int		Rm, Brief;
{
	int				ILoop, Count;
	ObjectPoint		Object;
	MonsterPoint	Monster;
	UserPoint		Player;
	Alfa			Temp;

	if (Room[Rm].Dark && !User->SSJ)
	{
		QOut(Term, "0It's too dark to see anything.");
		return;
	}

	PrintDesc(Room[Rm].DescRec, Room[Rm].DescCode, 0, Brief, "0", B1);
	if (User->SSJ)
	{
		sprintf(B2, "  (%d)", W(Rm));
		strcat(B1, B2);
		if (Room[Rm].Dark)
			strcat(B1, "  (Dark)");
	}
	QOut(Term, B1);

	Count = 0;
	for (ILoop = 0; ILoop < 6; ILoop++)
		if (Room[Rm].Adjoin[ILoop])
			Count++;
	if (Room[Rm].Out)
		Count++;

	if (Count)
	{
		strcpy(B1, "0Obvious exits are ");
		for (ILoop = 0; ILoop < 7; ILoop++)
			if (ILoop == 6 && Room[Rm].Out || Room[Rm].Adjoin[ILoop])
			{
				strcat(B1, DirList[ILoop]);
				if (Count == 1)
				{
					strcat(B1, ".");
					QOut(Term, B1);
				}
				else
					strcat(B1, Count > 2 ? ", " : " and ");
				--Count;
			}
	}

	Count = 0;
	for (Object = Room[Rm].RmObjectTail; Object; Object = Object->Next)
		if (!Object->Invisible || User->SSJ)
			Count++;

	for (Monster = Room[Rm].RmMonsterTail; Monster; Monster = Monster->Next)
		if (Monster->Top && (!Monster->Invisible || User->SSJ))
			Count++;

	if (Count)
	{
		strcpy(B1, "0You see ");
		for (ILoop = 0, Monster = Room[Rm].RmMonsterTail;
			ILoop < Count && Monster;
			Monster = Monster->Next)
			if (Monster->Top && (!Monster->Invisible || User->SSJ))
			{
				if (Monster->Num == 1)
				{
					PrintMon(Monster, FALSE, FALSE, B2);
					strcat(B1, B2);
				}
				else
				{
					Pn(Monster->Num, B2);
					strcat(B1, B2);
					ExpandName(Monster->Name, B2);
					strcat(B1, B2);
					strcat(B1, "s");
					if (Monster->Invisible && User->SSJ)
						strcat(B1, " (INV)");
				}

				ILoop++;
				Punctuate(ILoop, Count, B1);
			}

		for (Object = Room[Rm].RmObjectTail;
			ILoop < Count && Object;
			Object = Object->Next)
			if (!Object->Invisible || User->SSJ)
			{
				ILoop++;
				PrintObj(Object, FALSE, FALSE, B2);
				strcat(B1, B2);
				Punctuate(ILoop, Count, B1);
			}

		strcat(B1, ".");
		QOut(Term, B1);
		QOut(Term, "0");
	}

	Count = 0;
	for (Player = Room[Rm].RmPlayerTail; Player; Player = Player->Next)
		if (Player != User
		&&	!Player->Invisible
		&&	(!Player->Hidden || User->SSJ))
			Count++;

	if (Count)
	{
		strcpy(B1, "0");
		ILoop = 0;
		for (Player = Room[Rm].RmPlayerTail; Player; Player = Player->Next)
			if (Player != User
			&&	!Player->Invisible
			&&	(!Player->Hidden || User->SSJ))
			{
				strcat(B1, Player->Name);
				if (Player->Hidden)
					strcat(B1, " (HID)");
				ILoop++;
				Punctuate(ILoop, Count, B1);
			}

		strcat(B1, Count == 1 ? " is also here." : " are also here.");
		QOut(Term, B1);
	}

	strcpy(Temp, User->Name);
	strcpy(User->Name, "you");

	for (Monster = Room[Rm].RmMonsterTail; Monster; Monster = Monster->Next)
		if (Monster->DefPlayer)
		{
			PM(Monster, TRUE, B2);
			sprintf(B1, "0%s is attacking %s!", B2, Monster->DefPlayer->Name);
			QOut(Term, B1);
		}

	for (Player = Room[Rm].RmPlayerTail; Player; Player = Player->Next)
	{
		if (Player->DefMon)
		{
			/*
			if (Player->DefMon->DefPlayer != Player)
			{
			*/
				PM(Player->DefMon, FALSE, B2);
				if (Player == User)
					sprintf(B1, "0You're attacking %s!", B2);
				else
					sprintf(B1, "0%s is attacking %s!", Player->Name, B2);
				QOut(Term, B1);
			/*
			}
			*/
		}
		if (Player->DefPlayer)
		{
			if (Player == User)
				sprintf(B1, "0You're attacking %s!", Player->DefPlayer->Name);
			else
				sprintf(B1, "0%s is attacking %s!",
						Player->Name, Player->DefPlayer->Name);
			QOut(Term, B1);
		}
	}

	strcpy(User->Name, Temp);
}

static int	OkSend(Usr)
UserPoint	Usr;
{
	return (Usr != User && Usr->Status != SLogin);
}

UserPoint	NextOkSend(Usr)
UserPoint	Usr;
{
	for (; Usr; Usr = Usr->Next)
		if (OkSend(Usr))
			return Usr;

	return NULL;
}

static UserPoint	MatchUser(Usr)
UserPoint	Usr;
{
	for (; Usr; Usr = Usr->NextUser)
		if (OkSend(Usr))
			return Usr;

	return NULL;
}

int		MsgTerm(TalkHow)
TalkHowType	TalkHow;
{
	int		Count = 0;

	if (!User->Invisible || TalkHow == SysMsg
	||	CmdCode == 12 || CmdCode == 13 || CmdCode == 14 || CmdCode == 16
	||	CmdCode == 36 || CmdCode == 37 || CmdCode == 38)
		if (TalkHow == Local || TalkHow == Yell)
		{
			RoomType	*Rm = &Room[User->RmCode];

			RoomMsg(Rm, &Count);

			if (TalkHow == Yell)
			{
				int			ILoop;
				ObjectPoint	Obj;

				for (ILoop = 0; ILoop < 6; ILoop++)
					if (Rm->Adjoin[ILoop])
						RoomMsg(&Room[S(Rm->Adjoin[ILoop])], &Count);
				if (Rm->Out)
					RoomMsg(&Room[S(Rm->Out)], &Count);
				for (Obj = Rm->RmObjectTail; Obj; Obj = Obj->Next)
				{
					int		ToWhere = 0;

					if (Obj->ObClass == Portal)
						ToWhere = Obj->Object.Portal.ToWhere;
					else
					if (Obj->ObClass == Door)
						ToWhere = Obj->Object.Door.DToWhere;
					if (ToWhere && ToWhere != W(User->RmCode))
						RoomMsg(&Room[S(ToWhere)], &Count);
				}
			}
		}
		else
		if (TalkHow == All || TalkHow == Others || TalkHow == NoBlock
		||	TalkHow == BrAll || TalkHow == BrOthers || TalkHow == SysMsg)
		{
			UserPoint	OtherPlayer;

			for (OtherPlayer = MatchUser(UserTail);
				OtherPlayer && Count < MaxPlusOne;
				OtherPlayer = MatchUser(OtherPlayer->NextUser))
				if ((!OtherPlayer->Brief
				||	TalkHow != BrOthers && TalkHow != BrAll)
				&& (!OtherPlayer->MesBlock || TalkHow != NoBlock))
					TermList[Count++] = OtherPlayer->Trm;

			if ((TalkHow == SysMsg || TalkHow == All || TalkHow == BrAll
			||	User->Echo) && Count < MaxPlusOne)
				TermList[Count++] = Term;
		}
		else
			Abort(" Sce10 - Bad talk/term function!");

	if (Count > MaxPlusOne)
		QOut(Term, "0 Sce11 - Term msg table overflow!");

	return Count;
}

void	RoomMsg(Rm, Count)
RoomType	*Rm;
int			*Count;
{
	UserPoint	OtherPlayer;

	OtherPlayer = NextOkSend(Rm->RmPlayerTail);
	while (OtherPlayer && *Count < MaxPlusOne)
	{
		TermList[(*Count)++] = OtherPlayer->Trm;
		OtherPlayer = NextOkSend(OtherPlayer->Next);
	}
}
