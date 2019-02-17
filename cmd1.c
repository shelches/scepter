#include "scepter.h"

static void TrapCheck(int Trap);

static void TrapCheck(int Trap)
{
	int		Dmg;

	if (Trap <= 0 || User->Dex - Rnd(6) >= Trap)
		return;

	switch (Trap)
	{
		case  1:
		case  2:
		case  3: QOut(Term, "0Splinters on your hand!");		break;
		case  4:
		case  5: QOut(Term, "0Spring dart!");					break;
		case  6: QOut(Term, "0Small knife flies at you!");		break;
		case  7: User->Poisoned = TRUE;
				 QOut(Term, "0Poison dart!");					break;
		case  8: QOut(Term, "0Spear shoots out of the ground at you!");	break;
		case  9: QOut(Term, "0Dust sprays in your eyes!");		break;
		case 10: QOut(Term, "0Grubs bite you!");				break;
		case 11: QOut(Term, "0Steel wire cuts your hand!");		break;
		case 12: QOut(Term, "0Needles stab your toes!");		break;
		case 13: User->Poisoned = TRUE;
				 QOut(Term, "0Poison needles!");				break;
		case 14: User->Poisoned = TRUE;
				 QOut(Term, "0Cobra lunges at you!");			break;
		case 15: User->Poisoned = TRUE;
				 QOut(Term, "0Gas spores explode!");			break;
		case 16: QOut(Term, "0Rocks fall from the ceiling!");	break;
		case 17: QOut(Term, "0Blam!  Explosion in your face!");	break;
		case 18: QOut(Term, "0Acid splashes in your face!");	break;
		case 19: QOut(Term, "0Flames shoot out at you!");		break;
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25: User->Fatigue = 0;
				 QOut(Term, "0Spear impales your stomach!");	break;
		case 26:
		case 27:
		case 28:
		case 29:
		case 30: QOut(Term, "0Boooooom!");						break;
		case 31:
		case 32:
		case 33:
		case 34:
		case 35: QOut(Term, "0A rack of knives falls and crushes you!"); break;
		default: User->Fatigue = 0;
				 QOut(Term, "0Tons of rocks tumble down upon you!"); break;
	}

	Dmg = Max(0, Trap + User->AC - 10);
	Dmg = Dmg / 2 + Rnd(Dmg);
	PrintDmg(User, Dmg, User->Brief, B2);
	sprintf(B1, "0It hits you for %s", B2);
	QOut(Term, B1);

	if (User->Hits + User->Fatigue <= Dmg)
	{
		User->Dead = TRUE;
		User->Fatigue = User->Hits = 0;
		sprintf(B1, "0### %s was just killed by a deadly trap.", User->Name);
		AllMsg(B1);
	}
	else
	{
		if (Dmg > User->Fatigue)
			User->Hits -= Dmg - User->Fatigue;
		User->Fatigue = Max(0, User->Fatigue - Dmg);
	}
}

void StopUsing(UserPoint Usr, ObjectPoint Object)
{
	if (!Object)
		return;

	if (Object == Usr->USWeap)
		Usr->USWeap = NULL;
	else
	if (Object == Usr->USShield)
	{
		Usr->AC = Max(-50, Min(50, Usr->AC + Object->Object.Shield.ShPlus));
		Usr->USShield = NULL;
	}
	else
	if (Object == Usr->USArm)
	{
		Usr->AC = Max(-50, Min(50, Usr->AC + Object->Object.Armor.ArmPlus));
		Usr->USArm = NULL;
	}
	else
	if (Object == Usr->USRingL)
		Usr->USRingL = NULL;
	else
	if (Object == Usr->USRingR)
		Usr->USRingR = NULL;
}

void GetObject(char *Word, int Num, RmCodeType Rm)
{
	ObjectPoint Object, ObTail;
	ObjectPoint Box = NULL;
	int Num2;
	int Held = FALSE;
	Alfa Word2;
	enum eWhere { Ground, Container, NoWhere } Where;

	GetWord(Word2, &Num2);

	if (*Word2)
	{
		if (!(Box = FindObject(Word2, Num, Room[Rm].RmObjectTail)))
			if ((Box = FindObject(Word2, Num, User->ObjectTail)) != NULL)
				Held = TRUE;

		if (!Box)
		{
			Where = NoWhere;
			QOut(Term, "0That container isn't here.");
		}
		else if (Box->ObClass != Chest)
		{
			Where = NoWhere;
			QOut(Term, "0That's not a container.");
		}
		else if (Box->Object.Chest.Closed)
		{
			Where = NoWhere;
			QOut(Term, "0You can't.  It's closed.");
		}
		else
		{
			Where = Container;
			ObTail = Box->Object.Chest.ObjectTail;
			Object = FindObject(Word, Num, ObTail);
		}
	}
	else
	{
		Where	= Ground;
		ObTail	= Room[Rm].RmObjectTail;
		Object	= FindObject(Word, Num, ObTail);
	}

	if (!Object || Where == NoWhere)
	{
		if (Where != NoWhere)
			QOut(Term, "0That object isn't here.");
	}
	else if (!Object->Carry)
		QOut(Term, "0You are not able to take that!");
	else
	{
		MonsterPoint	Mon;
		int				Watched = FALSE;

		for (Mon = Room[Rm].RmMonsterTail; Mon; Mon = Mon->Next)
		{
			if (Mon->Guard && !Held)
			{
				PM(Mon, TRUE, B2);
				sprintf(B1, "0%s stops you from getting it.", B2);
				QOut(Term, B1);
				return;
			}
			else if (Mon->Watch && !Held)
				Watched = TRUE;
		}

		if (User->Weight + Object->Weight > User->Str * 10 && !Held)
			QOut(Term, "0It's too much for you to carry!");
		else
		{
			DeleteObject(Object, &ObTail);
			QOut(Term, "0Ok.");
			Object->Invisible = FALSE;
			PrintObj(Object, TRUE, FALSE, B2);
			if (Where == Container)
			{
				PrintObj(Box, FALSE, FALSE, B3);
				sprintf(B1, "0%s picked up %s from inside %s.",
						User->Name, B2, B3);
			}
			else
				sprintf(B1, "0%s picked up %s.", User->Name, B2);
			LocalMsg(B1);

			if (Object->ObClass == Coins)
			{
				User->Money += Object->Price;
				sprintf(B1, "0You now have %ld shillings in cash.",
					User->Money);
				QOut(Term, B1);
				free(Object);
			}
			else
			{
				if (!Held)
					User->Weight = Min(2500, User->Weight + Object->Weight);
				if (!User->ObjectTail)
				{
					Object->Next = NULL;
					User->ObjectTail = Object;
				}
				else
				{
					ObjectPoint	Obj2;

					Obj2 = User->ObjectTail;
					while (Obj2->Next)
						Obj2 = Obj2->Next;
					Obj2->Next = Object;
					Object->Next = NULL;
				}
			}

			if (Where == Container)
			{
				Box->Weight = Max(0, Box->Weight - Object->Weight);
				Box->Object.Chest.NumInside =
					Max(0, Box->Object.Chest.NumInside - 1);
				Box->Object.Chest.ObjectTail = ObTail;
			}
			else
				Room[Rm].RmObjectTail = ObTail;

			if (Watched)
			{
				for (Mon = Room[Rm].RmMonsterTail; Mon; Mon = Mon->Next)
				{
					if (Mon->Watch)
					{
						Mon->DefPlayer = User;
						if (Mon->CharmPlayer == User)
							Mon->CharmPlayer = NULL;
						AttackMsg(Mon, User);
					}
				}
			}
		}
	}
}

void DropObject(char *Word, int Num, RmCodeType Rm)
{
	ObjectPoint	Object, ObTail, Box;
	int			Num2, Held;
	Alfa		Word2;
	enum eWhere { Ground, Container, NoWhere } Where;

	if (!(Object = FindObject(Word, Num, User->ObjectTail)))
	{
		QOut(Term, "0You aren't carrying that!");
		return;
	}

	GetWord(Word2, &Num2);
	if (*Word2)
	{
		Held = TRUE;
		Where = NoWhere;
		if (!(Box = FindObject(Word2, Num2, Room[Rm].RmObjectTail))
		&&	!(Box = FindObject(Word2, Num, User->ObjectTail)))
		{
			Held = FALSE;
			QOut(Term, "0That container isn't here!");
		}
		else
		if (Box->ObClass != Chest)
			QOut(Term, "0It won't fit in that!");
		else
		if (Box->Object.Chest.Closed)
			QOut(Term, "0You can't.  It's closed!");
		else
		if (Box->Object.Chest.NumInside >= 6 || Box->Weight > 200)
		{
			PrintObj(Box, TRUE, FALSE, B2);
			sprintf(B1,
					"0There is no more room inside %s for more objects!", B2);
			QOut(Term, B1);
		}
		else
		if (Box == Object)
			QOut(Term, "0You can't put an object inside itself!");
		else
		{
			Where = Container;
			ObTail = Box->Object.Chest.ObjectTail;
		}

		if (Where == NoWhere)
			return;
	}
	else
	{
		Box		= NULL;
		Held	= FALSE;
		Where	= Ground;
		ObTail	= Room[Rm].RmObjectTail;
	}

	DeleteObject(Object, &User->ObjectTail);
	Object->Next = ObTail;
	ObTail = Object;
	if (!Held)
		User->Weight = Max(0, User->Weight - Object->Weight);
	StopUsing(User, Object);
	if (Where == Container)
	{
		QOut(Term, "0Ok.  You drop it.");
		Box->Weight = Min(1000, Box->Weight + Object->Weight);
		Box->Object.Chest.NumInside = Min(10, Box->Object.Chest.NumInside + 1);
		Box->Object.Chest.ObjectTail = ObTail;
	}
	else
	{
		Room[Rm].RmObjectTail = ObTail;
		QOut(Term, "0Ok.");
	}

	PrintObj(Object, FALSE, FALSE, B2);
	if (Where == Container)
	{
		PrintObj(Box, TRUE, FALSE, B3);
		sprintf(B1, "0%s dropped %s into %s.", User->Name, B2, B3);
	}
	else
		sprintf(B1, "0%s dropped %s.", User->Name, B2);
	LocalMsg(B1);
}

int SpellCost(int Cost, int MinLvl, int MinInt)
{
	if (User->Int < MinInt)
		QOut(Term, "0You don't have the intelligence to cast the spell.");
	else
	{
		MinLvl += 2;
		switch ((int)User->Class)
		{
			case Fighter:	MinLvl += 2; break;
			case Thief:		MinLvl += 1; break;
			case MagicUser:	MinLvl -= 2; break;
			case Cleric:	MinLvl -= 1; break;
			case Barbarian:	MinLvl += 3; break;
		}

		if (MinLvl > (int)User->Lvl)
			QOut(Term, "0You aren't high enough level to cast the spell!");
		else
		if (Cost > User->Magic)
			QOut(Term, "0You're too weak to cast that spell.");
		else
		{
			User->Magic -= Cost;
			return TRUE;
		}
	}

	return FALSE;
}

int Look(char *Word, int Num)
{
	UserPoint	Player;
	int			RetVal;

	RetVal = 0;
	if (CmdCode == 31)
	{
		int		Legal;

		if (Room[User->RmCode].AntiMagic)
		{
			QOut(Term, "0Such magic does not work here.");
			Legal = FALSE;
		}
		else
			Legal = SpellCost(10, 5, 10);

		if (!Legal)
			return RetVal;
	}

	if (Player = FindPlayer(Word, Room[User->RmCode].RmPlayerTail))
	{
		if (Player->USArm && !Player->USArm->Invisible)
		{
			PrintObj(Player->USArm, FALSE, FALSE, B2);
			sprintf(B1, "0%s the %s is wearing %s.",
					Player->Name, CName[(int) Player->Class], B2);
		}
		else
			sprintf(B1, "0You see %s the %s.",
					Player->Name, CName[(int) Player->Class]);
		QOut(Term, B1);

		if (Player->USWeap && !Player->USWeap->Invisible
		||	Player->USShield && !Player->USShield->Invisible)
		{
			sprintf(B1, "0%s is holding ", Player->Sex == Male ? "He" : "She");
			if (Player->USWeap && !Player->USWeap->Invisible)
			{
				PrintObj(Player->USWeap, FALSE, FALSE, B2);
				strcat(B1, B2);
				if (Player->USShield && !Player->USShield->Invisible)
					strcat(B1, " and ");
			}

			if (Player->USShield && !Player->USShield->Invisible)
			{
				PrintObj(Player->USShield, FALSE, FALSE, B2);
				strcat(B1, B2);
			}

			strcat(B1, ".");
			QOut(Term, B1);
		}
		if (Player->USRingL && !Player->USRingL->Invisible)
		{
			PrintObj(Player->USRingL, FALSE, FALSE, B2);
			sprintf(B1, "0On %s left hand is %s.", Player->Sex == Male ? "his" : "her", B2);
			QOut(Term, B1);
		}
		if (Player->USRingR && !Player->USRingR->Invisible)
		{
			PrintObj(Player->USRingR, FALSE, FALSE, B2);
			sprintf(B1, "0On %s right hand is %s.", Player->Sex == Male ? "his" : "her", B2);
			QOut(Term, B1);
		}

		if (CmdCode == 31)
			PlayerDisplay(Player);
	}
	else
	{
		ObjectPoint	Object;
		enum eWhere { Ground, Person } Where;

		if (Object = FindObject(Word, Num, User->ObjectTail))
			Where = Person;
		else
		{
			Where = Ground;
			Object = FindObject(Word, Num, Room[User->RmCode].RmObjectTail);
		}

		if (Object)
		{
			if (Object->ObClass == Scroll && Where == Ground)
				QOut(Term, "0Pick it up first, Eagle Eyes!");
			else
			if (CmdCode == 31)
				ObjDisplay(Object);
			else
			{
				if (CmdCode == 64 && Object->ObClass == Scroll)
				{
					if (ReadyCheck(User->LastAtk))
					{
						int		Temp2;
						Alfa	Temp1;

						strcpy(Temp1, User->WData);
						Temp2 = User->Data;
						GetWord(Temp1, &Temp2);
						strcpy(User->WData, Temp1);
						User->Data = Temp2;
						RetVal = Object->Object.Scroll.Spell;
					}
					else
						CmdCode = 31;	/* inhibit scroll destruction */
				}
				else
				{
					if (Object->DescRec)
					{
						strcpy(B2, "0");
						PrintDesc(Object->DescRec, Object->DescCode,
									0, FALSE, B2, B1);
					}
					else
					{
						PrintObj(Object, FALSE, FALSE, B2);
						sprintf(B1, "0%s %s.",
							Where == Ground ? "It's" : "You are holding", B2);
					}
					QOut(Term, B1);

					if (Object->ObClass == Weap)
					{
						static char	*wtxt[4] =
						{
							"sharp, bladed", "thrusting", "blunt", "pole"
						};

						sprintf(B1, "0This is a %s weapon.",
								wtxt[(int)Object->Object.Weap.WeapType]);
						QOut(Term, B1);
					}
					else
					if (Object->ObClass == Chest)
					{
						if (Object->Object.Chest.Closed)
							QOut(Term, "0It's closed.");
						else
						if (!Object->Object.Chest.ObjectTail)
							QOut(Term, "0It's empty.");
						else
						{
							ObjectPoint	TObj;

							PrintObj(Object, TRUE, FALSE, B2);
							sprintf(B1,
									"0Inside %s you see the following:", B2);
							QOut(Term, B1);

							for (TObj = Object->Object.Chest.ObjectTail;
								TObj;
								TObj = TObj->Next)
							{
								PrintObj(TObj, FALSE, FALSE, B2);
								sprintf(B1, "0  %s", B2);
								QOut(Term, B1);
							}
						}
					}
				}

				if (Object->ObClass == Scroll && CmdCode != 31)
				{
					PrintObj(Object, TRUE, TRUE, B2);
					sprintf(B1, "0%s disintegrates!", B2);
					QOut(Term, B1);

					DeleteObject(Object, &User->ObjectTail);
					User->Weight -= Object->Weight;
					free(Object);
				}
			}
		}
		else	/* Monster? */
		{
			MonsterPoint	Mon;

			Mon = FindMonster(Word, Num, Room[User->RmCode].RmMonsterTail);
			if (!Mon)
				QOut(Term, "0I don't see that here.");
			else
			if (CmdCode == 31)
				MonDisplay(Mon);
			else
			if (Mon->DescRec && Mon->DescCode)
			{
				strcpy(B2, "0");
				PrintDesc(Mon->DescRec, Mon->DescCode, 0, FALSE, B2, B1);
				QOut(Term, B1);
			}
			else
			{
				int		Lev;
				char	LevBuf[16];

				PrintMon(Mon, FALSE, FALSE, B2);
				Lev = ((Mon->Lvl - 1) / 5) * 5 + 1;
				PNth(Lev, LevBuf);
				PNth(Lev + 4, B3);
				sprintf(B1, "0It's %s, %s to %s level.", B2, LevBuf, B3);
				QOut(Term, B1);

				if (Mon->Magic)
					QOut(Term, "0It looks magical!");
				if (Mon->SlowReact || Mon->FastReact)
					QOut(Term, "0It looks hostile!");
			}
		}
	}

	return RetVal;
}

void Train(void)
{
	int		TRoom;
	long	ReqExp;

	switch ((int)User->Class)
	{
		case Paladin:	TRoom = 21;	break;
		case Cleric:	TRoom = 22;	break;
		case Thief:		TRoom = 23;	break;
		case MagicUser:	TRoom = 24;	break;
		case Ranger:	TRoom = 25;	break;
		default:		TRoom = 20;	break;	/* fighter, barbarian, DM... */
	}

	if (W(User->RmCode) != TRoom)
	{
		QOut(Term, "0This is not the proper place for training!");
		return;
	}
	ReqExp = Expr(User->Lvl + 1) - Expr(User->Lvl);
	if (User->Money * 2 < ReqExp)
	{
		QOut(Term, "0You haven't enough funds to spend on training!");
		return;
	}
	if (User->Experience < ReqExp)
	{
		QOut(Term, "0You are not experienced enough for further training.");
		return;
	}
	if (User->Lvl >= 20)
	{
		QOut(Term, "0You have reached the peak of excellence.");
		return;
	}

	sprintf(B1, "0After many weeks of training %syou find...",
			User->Class != Barbarian && User->Class != Thief
		&&	User->Class != Fighter ? "and meditation " : "");
	QOut(Term, B1);

	User->Money		-= ReqExp / 2;
	User->Experience = 0;
	User->MaxHits	 = Min(2500, User->MaxHits + User->MaxHits / User->Lvl);
	User->MaxFatigue = Min(2500,
							User->MaxFatigue + User->MaxFatigue / User->Lvl);
	User->MaxMagic	 = Min(2500, User->MaxMagic + User->MaxMagic / User->Lvl);
	User->Lvl++;

	switch (Rnd(5))
	{
		case 1: User->Str = Min(25, User->Str + 1); break;
		case 2: User->Dex = Min(25, User->Dex + 1); break;
		case 3: User->Int = Min(25, User->Int + 1); break;
		case 4: User->Pty = Min(25, User->Pty + 1); break;
		case 5: User->Con = Min(25, User->Con + 1); break;
	}

	User->SkillNew = FALSE;
	if (User->Lvl <= 10)
		User->Con = Min(25, User->Con + 1);
	PlayerDisplay(User);
	SaveChar();
}

void OpenClose(char *Word, int Num)
{
	ObjectPoint	Obj;
	int			ObTrap, ObLocked, ObClosed;
	int			Succeeded = FALSE;
	enum eObType { Portl, Box, Neither } ObType;

	ObType = Neither;
	if (!(Obj = FindObject(Word, Num, User->ObjectTail))
	&&	!(Obj = FindObject(Word, Num, Room[User->RmCode].RmObjectTail)))
		QOut(Term, "0That object isn't here!");
	else
	if (Obj->ObClass == Chest)
	{
		ObType		= Box;
		ObTrap		= Obj->Object.Chest.Trap;
		ObLocked	= Obj->Object.Chest.Locked;
		ObClosed	= Obj->Object.Chest.Closed;
	}
	else
	if (Obj->ObClass == Door)
	{
		ObType		= Portl;
		ObTrap		= Obj->Object.Door.DTrap;
		ObLocked	= Obj->Object.Door.DLocked;
		ObClosed	= Obj->Object.Door.DClosed;
	}
	else
		QOut(Term, "0I don't know how to do such a thing.");

	if (ObType == Neither)
		return;

	if (CmdCode == 66)	/* close */
	{
		if (ObClosed)
			QOut(Term, "0It's already closed!");
		else
		{
			ObClosed = TRUE;
			PrintObj(Obj, TRUE, FALSE, B2);
			sprintf(B1, "0%s just closed %s.", User->Name, B2);
			LocalMsg(B1);
			QOut(Term, "0Ok.");
			Succeeded = TRUE;
		}
	}
	else
	if (CmdCode == 67 || CmdCode == 68)		/* lock/unlock */
	{
		ObjectPoint	Key;
		int			Num2;

		GetWord(Word, &Num2);
		Key = FindObject(Word, Num2, User->ObjectTail);
		if (!*Word)
			QOut(Term, "0With what?");
		else
		if (!Key)
			QOut(Term, "0You don't have the right key.");
		else
		if (Key->ObClass != Keys)
			QOut(Term, "0That won't unlock anything!");
		else
		{
			int		UnLock = Key->Object.Keys.UnLock;

			if ((UnLock % 10 == 0 && UnLock != ObLocked - ObLocked % 10
			||	 UnLock % 10 != 0 && UnLock != ObLocked)
			&&	UnLock != 1000)
			{
				PrintObj(Key, TRUE, FALSE, B2);
				sprintf(B1, "0%s doesn't work!", B2);
				QOut(Term, B1);
			}
			else
			{
				ObClosed = CmdCode == 67;
				PrintObj(Obj, TRUE, FALSE, B2);
				PrintObj(Key, FALSE, FALSE, B3);
				sprintf(B1, "0%s just %s %s with %s.", User->Name,
						ObClosed ? "locked" : "unlocked", B2, B3);
				LocalMsg(B1);
				QOut(Term, "0Ok.");
				Succeeded = TRUE;

				if (Key->Object.Keys.NumUses > 0
				&&	--Key->Object.Keys.NumUses == 0)
				{
					PrintObj(Key, TRUE, TRUE, B2);
					sprintf(B1, "0%s falls apart!", B2);
					QOut(Term, B1);
					DeleteObject(Key, &User->ObjectTail);
					User->Weight -= Key->Weight;
					free(Key);
				}
			}
		}
	}
	else
	if (CmdCode == 65)	/* open */
	{
		if (!ObClosed)
			QOut(Term, "0It's already open!");
		else
		if (ObLocked)
			QOut(Term, "0You can't.  It's locked!");
		else
		{
			ObClosed = FALSE;
			PrintObj(Obj, TRUE, FALSE, B2);
			sprintf(B1, "0%s just opened %s.", User->Name, B2);
			LocalMsg(B1);
			QOut(Term, "0Ok.");
			Succeeded = TRUE;
			TrapCheck(ObTrap);
		}
	}
	else
	if (CmdCode == 69 && ReadyCheck(User->LastAtk))		/* picklock */
	{
		if (ObLocked % 10 == 0
		||	(User->Lvl + User->Dex / 2 + (User->Class == Thief ? 3 : -10)) / 3
				< ObLocked % 10)
		{
			User->LastAtk = RealTime + 15;
			QOut(Term, "0You fail to pick the lock!");
		}
		else
		{
			ObClosed = FALSE;
			if (User->Class != Thief)
			{
				PrintObj(Obj, TRUE, FALSE, B2);
				sprintf(B1, "0%s picks the lock on %s open!", User->Name, B2);
				LocalMsg(B1);
				Succeeded = TRUE;
			}
			QOut(Term, "0You picked the lock open!");
			TrapCheck(ObTrap);
		}
	}
	else
	if (CmdCode == 70 && ReadyCheck(User->LastAtk))		/* smash */
	{
		if (!ObClosed)
			QOut(Term, "0It's already open!");
		else
		if (Rnd(3) != 1 || User->Str * 10 < Obj->Weight)
		{
			User->LastAtk = RealTime + 15;
			PrintObj(Obj, TRUE, FALSE, B2);
			sprintf(B1, "0%s fails to smash %s open.", User->Name, B2);
			LocalMsg(B1);
			QOut(Term, "0Bang!  You fail to smash it open!");

			if (Obj->ObClass == Door)
			{
				int ILoop, Count;

				Count = 0;
				RoomMsg(&Room[S(Obj->Object.Door.DToWhere)], &Count);
				sprintf(B1, "0You hear a noise on the other side of %s.", B2);
				for (ILoop = 0; ILoop < Count; ILoop++)
					QOut(TermList[ILoop], B1);
			}
		}
		else
		{
			PrintObj(Obj, TRUE, FALSE, B2);
			sprintf(B1, "0%s smashes %s open.", User->Name, B2);
			LocalMsg(B1);
			ObClosed = FALSE;
			QOut(Term, "0You smash it open!");
			Succeeded = TRUE;
			TrapCheck(ObTrap);
		}
	}

	if (ObType == Box)
	{
		Obj->Object.Chest.Closed = ObClosed;
		Obj->Object.Chest.Trap	 = ObTrap;
		Obj->Object.Chest.Locked = ObLocked;
	}
	else
	if (ObType == Portl)
	{
		ObjectPoint	OtherDoor;
		struct tDoor	*Dr;

		Dr = &Obj->Object.Door;
		Dr->DClosed = ObClosed;
		Dr->DTrap	= ObTrap;
		Dr->DLocked = ObLocked;

		for (OtherDoor = Room[S(Dr->DToWhere)].RmObjectTail;
			OtherDoor;
			OtherDoor = OtherDoor->Next)
			if (!strcmp(OtherDoor->Name, Obj->Name))
			{
				OtherDoor->Object.Door.DClosed	= ObClosed;
				OtherDoor->Object.Door.DTrap	= ObTrap;
				OtherDoor->Object.Door.DLocked	= ObLocked;

				if (Succeeded)
				{
					UserPoint	OPly;

					PrintObj(OtherDoor, TRUE, FALSE, B2);
					sprintf(B1, "0Someone just %sed %s.",
							ObClosed ? "clos" : "open", B2);
					for (OPly = NextOkSend(Room[S(Dr->DToWhere)].RmPlayerTail);
						OPly;
						OPly = NextOkSend(OPly->Next))
						QOut(OPly->Trm, B1);
				}
				break;
			}
	}
}

void HitShArmor(UserPoint Plyr, int *Damage)
{
	ObjectPoint	Temp;

	if (Plyr->USArm)
	{
		Temp = Plyr->USArm;
		*Damage = Max(0, *Damage - Temp->Object.Armor.ArmPlus);
		Temp->Object.Armor.ArmHits = Max(0, Temp->Object.Armor.ArmHits - 1);
		if (!Temp->Object.Armor.ArmHits)
		{
			QOut(Term, "0Your armor falls apart!");
			DeleteObject(Temp, &Plyr->ObjectTail);
			Plyr->Weight -= Temp->Weight;
			StopUsing(Plyr, Temp);
			free(Temp);
		}
	}

	if (Plyr->USShield)
	{
		Temp = Plyr->USShield;
		Temp->Object.Shield.ShHits = Max(0, Temp->Object.Shield.ShHits - 1);
		if (!Temp->Object.Shield.ShHits)
		{
			QOut(Term, "0Your shield falls apart!");
			DeleteObject(Temp, &Plyr->ObjectTail);
			Plyr->Weight -= Temp->Weight;
			StopUsing(Plyr, Temp);
			free(Temp);
		}
	}
}

void Hide(char *Word, int Num)
{
	ObjectPoint	Obj;

	if (!*Word)
	{
		int		Chance;

		Chance = User->Lvl * 20 + User->Dex;
		if (User->AGuild || User->Class == Thief)
			Chance *= 2;
		if (Room[User->RmCode].RmMonsterTail)
			Chance /= 2;
		User->Hidden = Rnd(100) <= Min(66, Chance);
		QOut(Term, "0You hide in the shadows.");
	}
	else
	if (Obj = FindObject(Word, Num, User->ObjectTail))
	{
		StopUsing(User, Obj);
		DeleteObject(Obj, &User->ObjectTail);
		User->Weight -= Obj->Weight;
		Obj->Next = Room[User->RmCode].RmObjectTail;
		Room[User->RmCode].RmObjectTail = Obj;
		Obj->Invisible = TRUE;
		PrintObj(Obj, TRUE, FALSE, B2);
		sprintf(B1, "0You carefully hide %s in the room.", B2);
		QOut(Term,B1);
		if (Rnd(4) == 1)
		{
			PrintObj(Obj, FALSE, FALSE, B2);
			sprintf(B1, "0You see %s trying to hide %s in the room.",
					User->Name, B2);
			LocalMsg(B1);
			Obj->Invisible = FALSE;
		}
	}
	else
		QOut(Term, "0You aren't holding that.");
}

void Search(void)
{
	UserPoint	HidPlyr;
	ObjectPoint	HidObj;
	int			Factor, Rm, Find;

	if (!ReadyCheck(User->LastAtk))
		return;

	Find = FALSE;
	Rm = User->RmCode;
	Factor = 50;
	if (User->Class == Thief || User->Class == Ranger)
		Factor += 25;

	User->LastAtk = RealTime + 10;
	for (HidPlyr = Room[Rm].RmPlayerTail; HidPlyr; HidPlyr = HidPlyr->Next)
		if ((HidPlyr->Hidden || Room[Rm].Dark) && HidPlyr != User
		&&	Rnd(100) <= Factor)
		{
			HidPlyr->Hidden = FALSE;
			Find = TRUE;
			sprintf(B1, "0You spot %s hiding in the shadows.", HidPlyr->Name);
			QOut(Term, B1);
		}

	for (HidObj = Room[Rm].RmObjectTail; HidObj; HidObj = HidObj->Next)
		if ((HidObj->Invisible || Room[Rm].Dark) && Rnd(100) <= Factor)
		{
			Find = TRUE;
			PrintObj(HidObj, FALSE, FALSE, B2);
			sprintf(B1, "0While searching, you discover %s!", B2);
			QOut(Term, B1);
		}

	if (!Find)
		QOut(Term, "0You didn't find anything.");
}

UserPoint FindDm(UserPoint Usr)
{
	UserPoint	Temp;

	for (Temp = Usr; Temp; Temp = Temp->NextUser)
		if (Temp->SSJ)
			return Temp;

	return NULL;
}

void LocalMsg(char *s)
{
	int		Count, ILoop;

	Count = MsgTerm(Local);
	for (ILoop = 0; ILoop < Count; ILoop++)
		QOut(TermList[ILoop], s);
}

void AllMsg(char *s)
{
	int		Count, ILoop;

	Count = MsgTerm(All);
	for (ILoop = 0; ILoop < Count; ILoop++)
		QOut(TermList[ILoop], s);
}
