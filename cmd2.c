#include "scepter.h"

#ifdef	PROTO
static UserPoint	XXPFollow(UserPoint Usr);
static void	ShareExperience(MonsterPoint Mon, RmCodeType Rm);
static void	SharePlayerExp(UserPoint Plyr, RmCodeType Rm);
static void	MakeTreasure(MonsterPoint Monster, int Rm);
#else
static UserPoint	XXPFollow();
#endif

/*	GoDirection lets players move from room to room.  It moves players N, S, E,
	W, U, D and through portals and doors.  Movement is prohibited if a monster
	blocks the exit.  If the player is not blocked, following monsters have a
	66% chance of following the player into the next room. */

void	GoDirection(Word, Num)
char	*Word;
int		Num;
{
	int				Leave, Blocked, Followed, Following;
	RmCodeType		Rm, NewRm;
	ObjectPoint		Object = NULL;
	RoomType		*RmPt = NULL;
	UserPoint		FollowPlyr = NULL, Leader = NULL;
	MonsterPoint	Mon = NULL;

	Blocked = Followed = Leave = FALSE;
	Rm = User->RmCode;
	RmPt = &Room[Rm];

	if (CmdCode <= 6)
	{
		if (RmPt->Adjoin[CmdCode - 1])
		{
			Leave = TRUE;
			NewRm = RmPt->Adjoin[CmdCode - 1];
		}
		else
			QOut(Term, "0There is no way to go in that direction.");
	}
	else if (CmdCode == 7)
	{
		if (RmPt->Out)
		{
			Leave = TRUE;
			NewRm = RmPt->Out;
		}
		else
			QOut(Term, "0There is no obvious exit from here.");
	}
	else	/* CmdCode == 9.  Go thru door or passage. */
	{
		Object = FindObject(Word, Num, RmPt->RmObjectTail);
		if (!Object)
			QOut(Term, "0That object isn't here!");
		else if (Object->ObClass != Portal && Object->ObClass != Door)
			QOut(Term, "0You can't go in that direction.");
		else if (Object->ObClass == Door && Object->Object.Door.DClosed)
		{
			PrintObj(Object, TRUE, FALSE, B2);
			sprintf(B1, "0Clunk!  You bang your nose against %s.", B2);
			QOut(Term, B1);
		}
		else
		{
			Leave = TRUE;
			if (Object->ObClass == Door)
				NewRm = Object->Object.Door.DToWhere;
			else
				NewRm = Object->Object.Portal.ToWhere;
		}
	}

	if (!Leave)
		return;

	for (Mon = RmPt->RmMonsterTail; Mon; Mon = Mon->Next)
	{
		if (Mon->Block
			&& (CmdCode >= 7 || Mon->DefPlayer == User && Rnd(100) < 50))
		{
			PM(Mon, TRUE, B2);
			sprintf(B1, "0%s blocks your way!", B2);
			QOut(Term, B1);
			Blocked = TRUE;
			break;
		}
	}

	if (!Blocked)
	{
		int		I;

		if (CmdCode == 9 && Object->ObClass == Door && Object->Object.Door.DToll)
		{
			sprintf(B1, "0\"That'll be %d shillings, please.\"", Object->Price);
			QOut(Term, B1);
			if (User->Money >= Object->Price)
			{
				User->Money -= Object->Price;
				QOut(Term, "0\"Thank you, and have a nice day!\"");
			}
			else
			{
				QOut(Term, "0\"Sorry, buddy.  No dough, no go.\"");
				if (User->Follow && User->Follow->RmCode != User->RmCode)
					User->Follow = NULL;
				return;
			}
		}
		I = S(NewRm);
		if (!Room[I].Safe)
		{
			MonsterPoint	NextMon;

			for (Mon = RmPt->RmMonsterTail; Mon; Mon = NextMon)
			{
				NextMon = Mon->Next;
				if (Mon->CharmPlayer == User
					||	Mon->Follow && Mon->DefPlayer == User && Rnd(100) < 70)
				{
					PM(Mon, TRUE, B2);
					sprintf(B1, "0%s follows you!", B2);
					QOut(Term, B1);
					Followed = TRUE;
					DeleteMonster(Mon, Rm);
					InsertMonster(Mon, I);
					if (Mon->CharmPlayer == User)
						Mon->DefPlayer = NULL;
					else
						Mon->DefPlayer = User;
				}
			}
		}
	}

	if (Rnd(100) > 50)
		User->Hidden = FALSE;

	FollowPlyr = XXPFollow(RmPt->RmPlayerTail);

	if (!User->Hidden && (FolCount == 0 || Followed || Blocked))
	{
		sprintf(B1, "0%s%s just %s ", RmPt->Dark ? "Someone" : User->Name, FollowPlyr ? "'s group" : "",
				User->Drunk - RealTime > 30 ? "staggered" : "went");

		if (CmdCode == 9)
		{
			strcat(B1, "to ");
			PrintObj(Object, TRUE, FALSE, B2);
			strcat(B1, B2);
		}
		else
			strcat(B1, DirList[CmdCode - 1]);

		if (Followed)
			sprintf(B2, " and something followed %s!",
					FollowPlyr ? "them" : Pro[(int)User->Sex]);
		else if (Blocked)
			sprintf(B2, " but then something stopped %s!",
					FollowPlyr ? "them" : Pro[(int)User->Sex]);
		else
			strcpy(B2, ".");

		strcat(B1, B2);
		LocalMsg(B1);
	}

	if (Blocked)
		return;

	Leader = User->Follow;
	DeletePlayer(User, Rm);
	PlacePlayer(User, NewRm);
	if (CmdCode <= 7)
		RmPt->LastDir = CmdCode;
	else
		RmPt->LastDir = 8;

	if (!User->Hidden && !FolCount)
	{
		sprintf(B1, "0%s%s just arrived",
				Room[User->RmCode].Dark ? "Someone" : User->Name, FollowPlyr ? "'s group" : "");
		if (Followed)
			sprintf(B2, " with something following %s!",
					FollowPlyr ? "them" : Pro[(int)User->Sex]);
		else
			strcpy(B2, ".");
		strcat(B1, B2);
		LocalMsg(B1);
	}

	if (Room[User->RmCode].NotifyDM)
	{
		UserPoint	Dm;

		sprintf(B1, "0*** %s entered room %d.", User->Name, NewRm);
		for (Dm = FindDm(UserTail); Dm; Dm = FindDm(Dm->NextUser))
			QOut(Dm->Trm, B1);
	}

	Following = FolCount > 0;
	if (User->RmCode != Rm)
	{
		while (FollowPlyr && FolCount < MaxUsers)
		{
			UserPoint	Temp;

			FolCount++;
			if (FolCount >= MaxUsers)
				QOut(Term, "0 Sce432 - follow table overflow!");
			Temp = User;
			Term = FollowPlyr->Trm;
			User = FollowPlyr;
			FollowPlyr = FollowPlyr->Next;
			GoDirection(Word, Num);
			User = Temp;
			Term = User->Trm;
			FollowPlyr = XXPFollow(FollowPlyr);
		}
	}

	if (!Following || !User->Brief)
		RoomDisplay(User->RmCode, User->Brief);

	if (Following)
	{
		User->Follow = Leader;
		if (User->Brief)
		{
			strcpy(B2, "0");
			PrintDesc(Room[User->RmCode].DescRec, Room[User->RmCode].DescCode,
						0, User->Brief, B2, B1);
			QOut(Term, B1);
		}
	}
	else if (Object && Object->ObClass == Door && Object->Object.Door.DSpring)
	{
		ObjectPoint	OthrDoor;
		int			ILoop, Count;

		Object->Object.Door.DClosed = TRUE;
		for (OthrDoor = Room[S(Object->Object.Door.DToWhere)].RmObjectTail;
			OthrDoor;
			OthrDoor = OthrDoor->Next)
		{
			if (!strcmp(OthrDoor->Name, Object->Name))
			{
				OthrDoor->Object.Door.DClosed = TRUE;
				break;
			}
		}

		PrintObj(Object, TRUE, TRUE, B2);
		sprintf(B1, "0%s slams shut!", B2);
		Count = MsgTerm(Local);
		TermList[Count++] = User->Trm;
		for (ILoop = 0; ILoop < Count; ILoop++)
			QOut(TermList[ILoop], B1);
	}

	switch (NewRm)
	{
		case 3:
			QOut(Term, "0Bartender says, \"Buy a drink, mac?\"");
			break;
		case 4:
			QOut(Term, "0Paperboy says, \"Extra! Extra! Read all about it!\"");
			break;
		case 8:
			QOut(Term, "0Appraisor asks, \"What would you like to sell?\"");
			break;
		case 10:
			QOut(Term, "0Merchant asks, \"What would you like to buy?\"");
			break;
		case 18:
			QOut(Term, "0\"Hee hee.  So matey, your gear don't look too good.  Need fixin?\"");
			break;
	}
}

static UserPoint	XXPFollow(Usr)
UserPoint	Usr;
{
	for (; Usr; Usr = Usr->Next)
		if (Usr->Follow == User)
			return Usr;
	return NULL;
}

void	ReturnObj(Word, Num)
char	*Word;
int		Num;
{
	ObjectPoint	Object;

	if (Object = FindObject(Word, Num, User->ObjectTail))
	{
		StopUsing(User, Object);
		QOut(Term, "0Ok.");
	}
	else
		QOut(Term, "0Object not found.");
}

void	Track()
{
	int		Dir;

	if (!ReadyCheck(User->LastAtk))
		return;

	User->LastAtk = RealTime + 7;
	if (Room[User->RmCode].EncounterTime <= 60
		&&	Room[User->RmCode].EncounterTime > 0)
	{
		static char	*Melee[3] = {"battle", "struggle", "skirmish"};

		sprintf(B1, "0You see signs of a %s.",
				Melee[(Room[User->RmCode].EncounterTime - 1) / 20]);
		QOut(Term, B1);
	}

	Dir = User->Class == Ranger || Rnd(30) <= User->Int ?
		Room[User->RmCode].LastDir : 0;

	switch (Dir)
	{
		case 0:
			QOut(Term, "0You didn't find any tracks.");
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			sprintf(B1, "0You see some tracks leading %s.", DirList[Dir - 1]);
			QOut(Term, B1);
			break;

		case 7:
			QOut(Term, "0You see some tracks leading out.");
			break;

		case 8:
		{
			ObjectPoint	Obj;

			for (Obj = Room[User->RmCode].RmObjectTail; Obj; Obj = Obj->Next)
				if (Obj->ObClass == Portal || Obj->ObClass == Door)
				{
					PrintObj(Obj, TRUE, FALSE, B2);
					sprintf(B1, "0You see some tracks leading to %s.", B2);
					QOut(Term, B1);
					break;
				}

			if (Obj == NULL)
				QOut(Term, "0You didn't find any tracks.");
			break;
		}
	}
}

/*	use a weapon, shield, or other item  */
int		UseObject(Word, Num)
char	*Word;
int		Num;
{
	ObjectPoint	Object;
	int			RetVal, OnGround;
	static char	*NothingText = "0Nothing happens.";

	if (!(Object = FindObject(Word, Num, User->ObjectTail)))
	{
		MonsterPoint	Mon;

		if (!(Object = FindObject(Word, Num, Room[User->RmCode].RmObjectTail)))
		{
			QOut(Term, "0But you aren't carrying that!");
			return 0;
		}
		for (Mon = Room[User->RmCode].RmMonsterTail; Mon; Mon = Mon->Next)
			if (Mon->Guard)
			{
				PM(Mon, TRUE, B2);
				sprintf(B1, "0%s stops you from touching it.", B2);
				QOut(Term, B1);
				return 0;
			}
		if (Object->ObClass != MagDevice && Object->ObClass != Teleport
			&&	Object->ObClass != Card
			||	Object->Carry || !Object->Permanent)
		{
			QOut(Term, NothingText);
			return 0;
		}

		PrintObj(Object, TRUE, FALSE, B2);
		sprintf(B1, "0%s touches %s.", User->Name, B2);
		LocalMsg(B1);
		OnGround = TRUE;
	}
	else
		OnGround = FALSE;

	RetVal = 0;
	switch ((int)Object->ObClass)
	{
		case Weap:
			if (User->Class == Cleric
					&& (Object->Object.Weap.WeapType == Sharp
					||	Object->Object.Weap.WeapType == Thrust))
				QOut(Term, "0It is against your faith to use weapons that draw blood!");
			else
			{
				StopUsing(User, User->USWeap);
				User->USWeap = Object;
				PrintObj(Object, FALSE, FALSE, B2);
				sprintf(B1, "0%s pulls out %s!", User->Name, B2);
				LocalMsg(B1);
				QOut(Term, "0You ready your weapon!");
			}
			break;

		case Armor:
			if (User->Class == MagicUser && Object->Object.Armor.ArmPlus > 1)
				QOut(Term, "0Magic-users cannot wear such armor!");
			else
			{
				StopUsing(User, User->USArm);
				User->USArm = Object;
				User->AC =
					Max(-50, Min(50, User->AC - Object->Object.Armor.ArmPlus));
				PrintObj(Object, FALSE, FALSE, B2);
				sprintf(B1, "0%s puts on %s.", User->Name, B2);
				LocalMsg(B1);
				QOut(Term, "0You put on your armor.");
			}
			break;

		case Shield:
			if (User->Class == MagicUser && Object->Object.Shield.ShPlus > 1)
				QOut(Term, "0Magic-users cannot use such shields!");
			else
			{
				StopUsing(User, User->USShield);
				User->USShield = Object;
				User->AC =
					Max(-50, Min(50, User->AC - Object->Object.Shield.ShPlus));
				PrintObj(Object, FALSE, FALSE, B2);
				sprintf(B1, "0%s wields %s.", User->Name, B2);
				LocalMsg(B1);
				QOut(Term, "0You ready your shield.");
			}
			break;

		case MagDevice:
			if (Object->Object.MagDevice.NumCharges <= 0)
			{
				QOut(Term, NothingText);
				Object->Permanent = FALSE;
			}
			else if (ReadyCheck(User->LastAtk))
			{
				GetWord(Word, &Num);
				strcpy(User->WData, Word);
				User->Data = Num;
				if (!OnGround)
					Object->Object.MagDevice.NumCharges--;
				RetVal = -(Object->Object.MagDevice.MSpell
						 | Object->Object.MagDevice.MLevel << 8);
			}
			break;

		case Teleport:
			if (Object->Object.Teleport.TActiveRm
					&&	Object->Object.Teleport.TActiveRm != W(User->RmCode))
				QOut(Term, NothingText);
			else
			{
				sprintf(B1, "0%s suddenly vanishes in a blinding flash!",
						User->Name);
				LocalMsg(B1);
				StopFollow(User);
				DeletePlayer(User, User->RmCode);
				PlacePlayer(User, Object->Object.Teleport.TToWhere);
				QOut(Term, "0Everything swirls around in a blinding flash!  You find...");
				RoomDisplay(User->RmCode, FALSE);
				sprintf(B1, "0%s suddenly appears in a blinding flash!",
						User->Name);
				LocalMsg(B1);
			}
			break;

		case Card:
		{
			UserPoint	Plyr;

			if (!User->SSJ && Object->Carry)
			{
				QOut(Term, NothingText);
				break;
			}
			GetWord(Word, &Num);
			Plyr = FindUser(Word);
			if (!Plyr)
				QOut(Term, "0Player not found.");
			else if (User->RmCode == Plyr->RmCode)
				QOut(Term, NothingText);
			else
			{
				Alfa	Dummy;
				int		Num2;

				GetWord(Dummy, &Num2);
				CapAlfa(Dummy);
				if (!strcmp(Dummy, "GATE"))
				{
					ObjectPoint	Obj;

					if (Obj = (ObjectPoint)malloc(sizeof(ObjectType)))
					{
						*Obj = ProtoObject;
						strcpy(Obj->Name, "gateway,*");
						Obj->Carry = FALSE;
						Obj->Magic = TRUE;
						Obj->ObClass = Portal;
						Obj->Object.Portal.ToWhere = W(User->RmCode);
						Obj->Next = Room[Plyr->RmCode].RmObjectTail;
						Room[Plyr->RmCode].RmObjectTail = Obj;
						QOut(Plyr->Trm,
							"0A magic gateway appears next to you!");
					}
					else
						QOut(Term, "0malloc failed!");
				}
				else if (!strcmp(Dummy, "LOOK"))
					RoomDisplay(Plyr->RmCode, FALSE);
				else
				{
					StopFollow(User);
					DeletePlayer(User, User->RmCode);
					PlacePlayer(User, W(Plyr->RmCode));
					sprintf(B1, "0You teleport to player %s.", Plyr->Name);
					QOut(Term, B1);
					RoomDisplay(User->RmCode, FALSE);
				}
			}
			break;
		}

		case Ring:
			GetWord(Word, &Num);
			CapAlfa(Word);
			if (!strcmp(Word, "LEFT"))
			{
				StopUsing(User, User->USRingL);
				User->USRingL = Object;
			}
			else if (!strcmp(Word, "RIGHT"))
			{
				StopUsing(User, User->USRingR);
				User->USRingR = Object;
			}
			else
			{
				QOut(Term, "0You must specify which hand!");
				break;
			}
			PrintObj(Object, FALSE, FALSE, B2);
			sprintf(B1, "0%s puts on %s.", User->Name, B2);
			LocalMsg(B1);
			QOut(Term, "0You put on the ring.");
			break;

		default:
			QOut(Term, NothingText);
			break;
	}

	return RetVal;
}

void	Logoff(Usr)
UserPoint	Usr;
{
	if (Usr->Status == SLogin || Usr->Status == SInit || Usr->Entry == XDead)
	{
		Usr->RmCode = 0;
		DeleteUser(Usr);
	}
	else
	{
		DeleteUser(Usr);
		if (Usr->Lvl > 1)
			WritePlayer(Usr);
		else
			QOut(Term, "0Sorry, you must be at least second level to save your character.");
	}

	FreePlayer(Usr);
	--NUsers;
}

void	KillPlayer()
{
	DeleteUser(User);
	if (PlayerOnFile(User->Name))
	{
		ChangeUsrName(User->Name, "");
		QOut(Term, "0Player file space released.");
	}
	FreePlayer(User);
	--NUsers;
}

void	SaveChar()
{
	if (User->Lvl < 2)
		QOut(Term,
			"0Sorry, you must be at least 2nd level to save your character.");
	else if (IsGuest(User->Trm))
		QOut(Term, "0Guests can't save characters, sorry.");
	else
	{
		UserType	DumUsr;

		DumUsr = *User;
		DumUsr.USWeap = NULL;
		DumUsr.USRingL = NULL;
		DumUsr.USRingR = NULL;
		DumUsr.RmCode = W(DumUsr.RmCode);
		StopUsing(&DumUsr, DumUsr.USArm);
		StopUsing(&DumUsr, DumUsr.USShield);
		WritePlayer(&DumUsr);
		QOut(Term, "0Character file updated.");
	}
}

void	Off()
{
	while (User = UserTail)
	{
		Term = User->Trm;
		Logoff(User);
		QOut(Term, "3");
	}
}

void	Dead()
{
	ObjectPoint	Obj, Body, Money;
	RmCodeType	Rm;

	Rm = User->RmCode;
	Obj = User->ObjectTail;
	User->ObjectTail = NULL;

	if (Body = (ObjectPoint)malloc(sizeof(ObjectType)))
	{
		*Body = ProtoObject;
		Body->Weight = 150;
		Body->Article = None;
		sprintf(Body->Name, "body,%s's *", User->Name);
		if (User->Money > 0L
			&& (Money = (ObjectPoint)malloc(sizeof(ObjectType))))
		{
			*Money = ProtoObject;
			strcpy(Money->Name, "gold,bag of *");
			Money->Weight = Min(20, Max(1, (int)(User->Money / 1000)));
			Money->Price = User->Money > 32000L ? 32000 : (short)User->Money;
			Money->ObClass = Treasure;
		}
	}

	if (Room[Rm].RmObjectTail)
	{
		ObjectPoint	Obj2;

		for (Obj2 = Room[Rm].RmObjectTail; Obj2->Next; Obj2 = Obj2->Next)
			;
		if (Body)
		{
			Obj2->Next = Body;
			Obj2 = Body;
			if (Money)
			{
				Obj2->Next = Money;
				Obj2 = Money;
			}
		}
		Obj2->Next = Obj;
	}
	else
	{
		Room[Rm].RmObjectTail = Obj;
		if (Body)
		{
			if (Money)
			{
				Money->Next = Room[Rm].RmObjectTail;
				Body->Next = Money;
			}
			else
				Body->Next = Room[Rm].RmObjectTail;
			Room[Rm].RmObjectTail = Body;
		}
	}

	User->Weight = 0;
	User->AC = 10;
	if (User->Con * 5 >= Rnd(100))
	{
		User->USArm = User->USShield = User->USWeap = User->USRingL = User->USRingR = NULL;
		User->Entry = XCmd;
		User->Status = SNormal;
		QOut(Term, "0");
		QOut(Term, "0Everything goes dark...  You feel yourself floating away into space.");
		QOut(Term, "0After what seems like an eternity, you faintly hear voices!");
		QOut(Term, "0As you come to your senses, you find....");
		QOut(Term, "0");

		PlacePlayer(User, 17);
		User->Poisoned = FALSE;
		User->Con = Max(3, User->Con - 1);
		if (User->Lvl > 2)
		{
			short	*p;

			User->MaxHits =
					Max(3, User->MaxHits - User->MaxHits / User->Lvl);
			User->MaxMagic =
					Max(3, User->MaxMagic - User->MaxMagic / User->Lvl);
			User->MaxFatigue =
					Max(3, User->MaxFatigue - User->MaxFatigue / User->Lvl);
			User->Magic = User->MaxMagic;
			--User->Lvl;
			switch (Rnd(4))
			{
				case 1: p = &User->Str; break;
				case 2: p = &User->Int; break;
				case 3: p = &User->Dex; break;
				case 4: p = &User->Con; break;
			}
			*p = Max(3, *p - 1);
		}

		User->Hits = User->MaxHits;
		User->Fatigue = Min(10, User->MaxFatigue);
		User->Money = Max(3, User->Pty)	* 10;
		User->Experience = 0;
		RoomDisplay(User->RmCode, User->Brief);
		QOut(Term, "0The acolyte gives you a small bag of money and bids you go on your way.");
		User->Dead = FALSE;
		SaveChar();
		Loc = LenBuf + 1;
	}
	else
	{
		QOut(Term, "0Constitution failed.");
		User->Dead = FALSE;
		KillPlayer();
		User = NULL;
		QOut(Term, "0End Scepter.");
		QOut(Term, "2");
		QDea(Term);
	}
}

int		MonExperience(Mon)
MonsterPoint	Mon;
{
	int		Exp;

	Exp = Mon->MaxHits + (int)(Mon->Lvl - Mon->AC + Mon->AtkSpeed)
		+ Mon->Invisible + Mon->Follow + Mon->Block + Mon->Drain
		+ Mon->Magic + Mon->Regenerate + Mon->Poison + Mon->AntiMagic
		+ Mon->MonSpells + Mon->SummonHelp;
	if (Mon->FastReact)
		Exp += 2;
	else if (Mon->SlowReact || Mon->MoralReact || Mon->ImmoralReact)
		Exp++;
	return 5 * Max(1, Exp);
}

static void	ShareExperience(Mon, Rm)
MonsterPoint	Mon;
RmCodeType		Rm;
{
	int			NewExp, NumInvolved;
	UserPoint	Plyr;

	NumInvolved = 1;	/* extra share for slayer */
	for (Plyr = Room[Rm].RmPlayerTail; Plyr; Plyr = Plyr->Next)
		if (Plyr->DefMon == Mon)
			NumInvolved++;

	NewExp = MonExperience(Mon) / NumInvolved;
	for (Plyr = Room[Rm].RmPlayerTail; Plyr; Plyr = Plyr->Next)
		if (Plyr == User)
			Plyr->Experience += NewExp * 2;
		else if (Plyr->DefMon == Mon)
		{
			sprintf(B1, "0You earned %d exp for the melee.", NewExp);
			QOut(Plyr->Trm, B1);
			Plyr->Experience += NewExp;
		}
}

static void	SharePlayerExp(Plyr, Rm)
UserPoint	Plyr;
RmCodeType	Rm;
{
	int			NewExp, NumInvolved;
	UserPoint	Player;

	NumInvolved = 1;	/* extra share for slayer */
	for (Player = Room[Rm].RmPlayerTail; Player; Player = Player->Next)
		if (Player->DefPlayer == Plyr)
			NumInvolved++;

	NewExp = 5 * (Plyr->Lvl + Plyr->MaxHits + Plyr->MaxFatigue - Plyr->AC)
			/ NumInvolved;
	for (Player = Room[Rm].RmPlayerTail; Player; Player = Player->Next)
		if (Player == User)
			Player->Experience += NewExp * 2;
		else if (Player->DefPlayer == Plyr)
		{
			sprintf(B1, "0You earned %d exp for the melee.", NewExp);
			QOut(Player->Trm, B1);
			Player->Experience += NewExp;
		}
}

void	ProtectNPC(Player, Monster)
UserPoint		Player;
MonsterPoint	Monster;
{
#ifdef USEJAIL
	PrintMon(Monster, TRUE, FALSE, B2);
	sprintf(B1, "0As you step forward to attack, %s yells,", B2);
	QOut(Term, B1);
	QOut(Term, "0\"Help!  Help!  Guards!  Guards!\"... In seconds, several");
	QOut(Term, "0burly guards grab you and take away your weapon!");

	if (Player->USWeap)
	{
		ObjectPoint	Temp;

		Temp = Player->USWeap;
		Player->USWeap = NULL;
		DeleteObject(Temp, &Player->ObjectTail);
		Player->Weight = Max(0, Player->Weight - Temp->Weight);
		free(Temp);
	}

	if (Player->Lvl < 3)
	{
		QOut(Term, "0Since you are a young, inexperienced adventurer, they give");
		QOut(Term, "0you a stern warning and decide not to throw you in jail.");
		sprintf(B1, "0%s assaults %s, but the town guards stop %s.",
				Player->Name, B2, Pro[(int)Player->Sex]);
		LocalMsg(B1);
	}
	else
	{
		QOut(Term, "0Then they throw you in jail!");
		QOut(Term, "0");
		sprintf(B1, "0### %s was thrown in jail for assaulting %s.",
				Player->Name, B2);
		LocalMsg(B1);
		StopFollow(Player);
		DeletePlayer(Player, Player->RmCode);
		PlacePlayer(Player, 5);		/* jail */
		RoomDisplay(Player->RmCode,	FALSE);
	}

	Player->DefMon = NULL;
#else
	UNUSED(Player);
	UNUSED(Monster);	
#endif
}

static void	MakeTreasure(Monster, Rm)
MonsterPoint	Monster;
int				Rm;
{
	ObjectPoint	Obj, LastObj;
	int			Nothing;

	Nothing = TRUE;
	PM(Monster, FALSE, B2);
	sprintf(B1, "0On %s you find:", B2);
	QOut(Term, B1);

	if (Monster->ObjectTail)
	{
		Nothing = FALSE;
		LastObj = Monster->ObjectTail;
		while (LastObj->Next)
			LastObj = LastObj->Next;
		LastObj->Next = Room[Rm].RmObjectTail;
		LastObj = LastObj->Next;
		Room[Rm].RmObjectTail = Monster->ObjectTail;
		Monster->ObjectTail = NULL;
	}
	else
		LastObj = Room[Rm].RmObjectTail;

	if (Monster->WhichObj > ObjListLen)
	{
		QOut(Term, "0 Sce105 - Object list index out of bounds!");
		Monster->WhichObj = 0;
	}

	if (Monster->WhichObj)
	{
		int		NumObjs, ILoop, ObNum;

		for (ILoop = 0, Obj = Room[Rm].RmObjectTail; Obj; ILoop++, Obj = Obj->Next)
			;
		if (ILoop >= SavedItems)
			NumObjs = 0;
		else
			NumObjs = Rnd(4) == 1 ? Rnd(4) : 1;
		for (ILoop = 0; ILoop < NumObjs; ILoop++)
			if ((ObNum = ObjIndex(Monster->WhichObj, Rnd(8) - 1))
				&&	(Obj = (ObjectPoint)malloc(sizeof(ObjectType))))
			{
				Nothing = FALSE;
				fseek(OList, (long)ObNum * sizeof(ObjectType), 0);
				fread((char *)Obj, sizeof(ObjectType), 1, OList);
				Obj->Price = (int)(Obj->Price * (80.0 + Rnd(40)) / 100.0);
				Obj->Next = Room[Rm].RmObjectTail;
				Room[Rm].RmObjectTail = Obj;
			}
	}

	if (Nothing)
		QOut(Term, "0    Nothing.");
	else
	{
		for (Obj = Room[Rm].RmObjectTail; Obj != LastObj; Obj = Obj->Next)
		{
			PrintObj(Obj, FALSE, TRUE, B2);
			sprintf(B1, "0    %s", B2);
			QOut(Term, B1);
		}
	}
}

void	Turn(Word, Num)
char	*Word;
int		Num;
{
	MonsterPoint	Mon;

	if (!(Mon = FindMonster(Word, Num, Room[User->RmCode].RmMonsterTail)))
		QOut(Term, "0Monster not found here.");
	else if (!Mon->Undead)
	{
		PM(Mon, TRUE, B2);
		sprintf(B1, "0%s is not undead!", B2);
		QOut(Term, B1);
	}
	else if (User->Class != Cleric && User->Class != Paladin && User->Class != DM)
		QOut(Term, "0You can't turn away the undead!");
	else if (ReadyCheck(User->LastAtk))
	{
		int		Factor;
		char	*p;

		User->Hidden = FALSE;
		User->LastAtk = RealTime + 25;
		Factor = User->Lvl - Mon->Lvl - 2 - Rnd(5);
		if (User->Class == Paladin)
			Factor += 2;
		User->DefMon = Mon;
		Mon->DefPlayer = User;
		if (Mon->CharmPlayer == User)
			Mon->CharmPlayer = NULL;
		Mon->MReact = 6;
		if (Rnd(3) == 1)
			Factor = -1;

		PM(Mon, TRUE, B2);
		if (Factor < 0)
		{
			sprintf(B1, "0%s ignores your command!", B2);
			QOut(Term, B1);
			p = "fails to turn away";
		}
		else if (Factor < 4)
		{
			Mon->MaxHits >>= 1;	/* since hits are used to calculate exp */
			sprintf(B1, "0%s flees at your command!", B2);
			QOut(Term, B1);
			p = "turns away";
		}
		else
		{
			sprintf(B1, "0%s collapses and turns to dust!", B2);
			QOut(Term, B1);
			p = "damns and destroys";
			MakeTreasure(Mon, User->RmCode);
		}

		PM(Mon, FALSE, B2);
		sprintf(B1, "0%s %s %s!", User->Name, p, B2);
		LocalMsg(B1);

		if (Factor >= 0)
		{
			ShareExperience(Mon, User->RmCode);
			DeleteMonster(Mon, User->RmCode);
			Destroy(Mon);
		}
	}
}

void	HitPlayer(Damage)
int		Damage;
{
	UserPoint	Player;

	Player = User->DefPlayer;
	HitShArmor(Player, &Damage);
	PrintDmg(Player, Damage, User->Brief, B2);
	sprintf(B1, "0You hit %s for %s", Player->Name, B2);
	QOut(Term, B1);

	PrintDmg(Player, Damage, Player->Brief, B2);
	sprintf(B1, "0%s hits you for %s", User->Name, B2);
	QOut(Player->Trm, B1);

	if (Player->Hits + Player->Fatigue > Damage)
	{
		if (Damage > Player->Fatigue)
			Player->Hits = Max(0, Player->Hits - Damage + Player->Fatigue);
		Player->Fatigue = Max(0, Player->Fatigue - Damage);
	}
	else if (RealTime - Player->HitAtTime > 30)
	{
		Player->Fatigue = 0;
		Player->Hits = 1;
		QOut(Player->Trm, "0You hear a voice cry out, \"Run, fool, run!\"");
	}
	else
	{
		int				Merc, Cold;
		MonsterPoint	Revenent;

		sprintf(B1, "0You killed %s!", Pro[(int)Player->Sex]);
		QOut(Term, B1);
		sprintf(B1, "0You're dead, %s!", Player->Name);
		QOut(Player->Trm, B1);

		Merc = User->Lvl - Player->Lvl > 3;
		Cold = !Player->DefPlayer;
		sprintf(B1, "0### %s just killed %s%s%s!", User->Name, Player->Name,
				Merc ? " mercilessly" : "", Cold ? " in cold blood" : "");
		AllMsg(B1);

		SharePlayerExp(Player, User->RmCode);

		if (Merc)
			User->Pty = Max(-10, User->Pty - 1);
		if (Cold)
			User->Pty = Max(-10, User->Pty - 1);
		if (User->AGuild)
			User->Pty = Max(0, User->Pty - 1);
		if (User->Class == Paladin && (Merc || Cold) && Player->Class != Thief)
			User->Class = Fighter;

		if (Player->Con >= 18
			&& (Revenent = (MonsterPoint)malloc(sizeof(MonsterType))))
		{
			*Revenent = ProtoMonster;
			InsertMonster(Revenent, User->RmCode);
			strcpy(Revenent->Name, "Revenent,*");
			Revenent->Hits = Player->MaxHits;
			Revenent->Lvl = Player->Lvl;
			Revenent->DefPlayer = User;
			Revenent->MaxHits = Revenent->Hits;
			Revenent->Guard = TRUE;
			Revenent->Block = TRUE;
			Revenent->Permanent = TRUE;
			QOut(Term, "0From the body you see a gaseous shape form...");
		}

		Player->Dead = TRUE;
		Player->Fatigue = Player->Hits = 0;

		if ((Merc || Cold) && User->Pty * -10 > Rnd(100))
		{
			QOut(Term, "0Enough is enough!  The people rise up against you in an angry mob!");
			QOut(Term, "0You have been lynched!");
			sprintf(B1, "0### %s the %s was just lynched by an angry mob!",
					User->Name, CName[(int)User->Class]);
			AllMsg(B1);
			User->Dead = TRUE;
			User->Fatigue = User->Hits = 0;
		}
	}
}

/*	HitMonster strikes monster for damage.  Enough damage will kill them.
	Messages are sent to all involved in the kill, and treasure and experience
	are distributed. */

void	HitMonster(Damage)
int		Damage;
{
	MonsterPoint	Monster;
	RmCodeType		Rm;

	Rm = User->RmCode;
	Monster = User->DefMon;
	if (Monster->Hits <= Damage)
	{
		PrintMon(Monster, TRUE, FALSE, B2);
		sprintf(B1, "0You killed %s!", B2);
		QOut(Term, B1);
		PM(Monster, FALSE, B2);
		sprintf(B1, "0%s just killed %s!", User->Name, B2);
		LocalMsg(B1);
		if (Monster->Nice && !User->Evil)
		{
			QOut(Term, "0You suck!");
			User->Suck = TRUE;
			sprintf(B1, "0%s sucks!", User->Sex == Male ? "He" : "She");
			LocalMsg(B1);
		}
		ShareExperience(Monster, Rm);
		MakeTreasure(Monster, Rm);
		DeleteMonster(Monster, Rm);
		Destroy(Monster);
		User->DefMon = NULL;

		return;
	}

	if (!Monster->Permanent && (Monster->MParley == 1
			||	Monster->MParley == 3 || Monster->MParley == 4))
		Monster->MParley = 2;

	Monster->Hits -= Damage;
	if (User->Brief)
		strcpy(B2, "it");
	else
		PrintMon(Monster, TRUE, FALSE, B2);
	sprintf(B1, "0You hit %s for %d hit points!", B2, Damage);
	QOut(Term, B1);

	if ((Monster->Flee || User->Lvl >= Monster->Lvl + 5)
		&& (int)(((long)Monster->Hits * 100) / Monster->MaxHits) < Rnd(40))
	{
		int		GoodDir, Dir, ILoop;

		PrintMon(Monster, TRUE, TRUE, B2);
		sprintf(B1, "0%s flees from your attack!", B2);
		QOut(Term, B1);

		PM(Monster, TRUE, B2);
		sprintf(B1, "0%s flees from %s's attack!", B2, User->Name);
		LocalMsg(B1);

		for (GoodDir = ILoop = 0; !GoodDir && ILoop < 5; ILoop++)
		{
			Dir = Rnd(7);
			if (Dir == 7)
				GoodDir = Room[Rm].Out;
			else
				GoodDir = Room[Rm].Adjoin[Dir - 1];
		}

		DeleteMonster(Monster, Rm);
		if (GoodDir)
		{
			int			NewRm;
			UserPoint	Plyr;

			NewRm = S(GoodDir);
			Room[Rm].LastDir = Dir;
			Monster->DefPlayer = NULL;
			InsertMonster(Monster, NewRm);
			if (Room[NewRm].Dark)
				strcpy(B2, "Something");
			else
				PrintMon(Monster, FALSE, TRUE, B2);
			sprintf(B1, "0%s just arrived!", B2);
			for (Plyr = NextOkSend(Room[NewRm].RmPlayerTail);
				Plyr;
				Plyr = NextOkSend(Plyr->Next))
				QOut(Plyr->Trm, B1);
		}
		else
		{
			Monster->MaxHits /= 2;	/* since hits are used to calculate exp */
			if (User->Lvl < Monster->Lvl + 5)
				ShareExperience(Monster, Rm);
			Destroy(Monster);
		}
		User->DefMon = NULL;
	}
	else if (Monster->SummonHelp && Monster->Hits < Rnd(Monster->MaxHits))
	{
		UserPoint		Plyr;
		MonsterPoint	Mon;
		int				ILoop, NonDMs, NumMons;

		PM(Monster, TRUE, B2);
		sprintf(B1, "0%s cries out for help!", B2);
		NonDMs = 0;
		for (Plyr = Room[Rm].RmPlayerTail; Plyr; Plyr = Plyr->Next)
		{
			if (!Plyr->Invisible)
				NonDMs++;
			QOut(Plyr->Trm, B1);
		}

		Monster->SummonHelp = FALSE;
		NumMons = Rnd(Max(1, NonDMs));
		EncounterMsg(Monster, Rm, NumMons, NonDMs);
		for (ILoop = 0; ILoop < NumMons; ILoop++)
			if (Mon = (MonsterPoint)malloc(sizeof(MonsterType)))
			{
				*Mon = *Monster;
				Mon->ObjectTail = NULL;
				Mon->Hits = Min(Mon->MaxHits,
								Max(1, (12 - Rnd(3)) * Mon->MaxHits / 10));
				Mon->Lvl = Min(25, Max(1, Mon->Lvl + 2 - Rnd(3)));
				Mon->MReact = 0;
				InsertMonster(Mon, Rm);
				AttackMsg(Mon, Mon->DefPlayer);
			}
	}
}
