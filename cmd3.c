#include "scepter.h"

static ObjectPoint	Fist(void);
static void	HitAtMonster(ObjectPoint Weapon, int Plus);
static void	HitAtPlayer(ObjectPoint Weapon, int Plus);
static ObjectPoint	SelectWeap(void);
static ToWhatType	AttackWhat(char *Word, int Num, RmCodeType Rm);

static ObjectPoint Fist(void)
{
	ObjectPoint	Weapon;

	Weapon = (ObjectPoint)malloc(sizeof(UserType));
	*Weapon = ProtoObject;
	Weapon->ObClass = Weap;
	Weapon->Object.Weap.MinHp = 1;
	Weapon->Object.Weap.MaxHp = 3;
	Weapon->Object.Weap.Strikes = 1;
	Weapon->Object.Weap.WeaPlus = -2;
	Weapon->Object.Weap.WeapType = Blunt;
	return Weapon;
}

static void HitAtMonster(ObjectPoint Weapon, int Plus)
{
	int		WithFist;

	if (Weapon)
		WithFist = FALSE;
	else
	{
		WithFist = TRUE;
		Weapon = Fist();
	}

	if (User->DefMon->Magic && !Weapon->Magic)
	{
		sprintf(B1, "0Your %s strikes with no effect!",
				WithFist ? "fist" : "weapon");
		QOut(Term, B1);
	}
	else
	{
		if (User->Class == Paladin
		&& (User->DefMon->SlowReact || User->DefMon->FastReact))
			Plus++;
		else
		if (User->Class == Fighter)
			Plus++;
		if (User->Lvl == 1)
			Plus += 2;
		if (User->DefMon->Invisible)
			Plus -= 4;

		if (User->DefMon->Defend && !User->DefMon->DefPlayer
		||	User->DefMon->AtkLastAggr && User->DefMon->DefPlayer != User)
		{
			User->DefMon->DefPlayer = User;
			if (User->DefMon->CharmPlayer == User)
				User->DefMon->CharmPlayer = NULL;
			if (User->DefMon->MReact >= 5)
				User->DefMon->MReact = 0;
		}

		if (!User->DefMon->Permanent
		&& (User->DefMon->MParley == 1 || User->DefMon->MParley == 3
		||	User->DefMon->MParley == 4))
			User->DefMon->MParley = 2;

		if (Rnd(20) != 20
		&&	Rnd(20) < 8 + User->DefMon->Lvl - User->Lvl + 10 - User->DefMon->AC
					  - Weapon->Object.Weap.WeaPlus - User->Str / 6 - Plus)
			QOut(Term, WithFist ? "0Your fist misses!" : "0You missed!");
		else
		{
			int		i, Damage;

			--Weapon->Object.Weap.Strikes;
			Damage = Max(0, Weapon->Object.Weap.MinHp
							+ Rnd(Weapon->Object.Weap.MaxHp
								- Weapon->Object.Weap.MinHp + 1)
							+ Weapon->Object.Weap.WeaPlus + Plus);

			if (WithFist)
				QOut(Term, "0Punch!");
			
			i = Rnd(100 - User->Lvl);
			if (i < 5)
			{
				QOut(Term, "0Double damage!");
				Damage <<= 1;
			}
			else
			if (i == 85)
			{
				QOut(Term, "0You fumbled!");
				Damage = 0;
				User->USWeap = NULL;
				if (!WithFist && Rnd(20) >= Weapon->Object.Weap.Strikes)
				{
					QOut(Term, "0Your weapon breaks!");
					Weapon->Object.Weap.Strikes = 0;
				}
			}

			if (!User->SkillNew && Damage >= User->DefMon->Hits
			&&	(User->DefMon->Lvl - User->Lvl) * 5 > Rnd(25))
			{
				User->SkillNew = TRUE;
				switch ((int)Weapon->Object.Weap.WeapType)
				{
					case Sharp:
						if (Rnd(User->SSharp) == 1)
							User->SSharp = Min(7, User->SSharp + 1);
						break;
					case Thrust:
						if (Rnd(User->SThrust) == 1)
							User->SThrust = Min(7, User->SThrust + 1);
						break;
					case Blunt:
						if (Rnd(User->SBlunt) == 1)
							User->SBlunt = Min(7, User->SBlunt + 1);
						break;
					case Long:
						if (Rnd(User->SLong) == 1)
							User->SLong = Min(7, User->SLong + 1);
						break;
				}
			}

			HitMonster(Damage);
		}
	}

	if (WithFist)
		free(Weapon);
}

static void HitAtPlayer(ObjectPoint Weapon, int Plus)
{
	int				WithFist;
	UserPoint		Plyr;
	struct tWeap	*p;

	if (!(Plyr = User->DefPlayer))	/* is this possible? */
		return;

	if (Weapon)
		WithFist = FALSE;
	else
	{
		WithFist = TRUE;
		Weapon = Fist();
	}
	p = &Weapon->Object.Weap;

	if (Rnd(20) != 20
	&& (!Plyr->Hits
	||	Rnd(20) < 24 + Plyr->Lvl - User->Lvl - p->WeaPlus - User->Str / 6
					 - Plyr->AC + Plyr->Dex / 6 - Plus))
	{
		QOut(Term, WithFist ? "0Whooosh!  Your fist misses!" : "0You missed!");
		sprintf(B1, "0%s %s you!", User->Name, WithFist ? "takes a swing"
														: "misses");
		QOut(Plyr->Trm, B1);
	}
	else
	{
		int		Damage;

		--p->Strikes;
		Damage = Max(0, p->MinHp + Rnd(p->MaxHp - p->MinHp + 1)
								 + p->WeaPlus + Plus);
		if (WithFist)
			QOut(Term, "0Punch!");
		HitPlayer(Damage);
	}

	Plyr->HitAtTime = RealTime;

	if (WithFist)
		free(Weapon);
}

static ObjectPoint SelectWeap(void)
{
	if (!User->USWeap)
		return NULL;
	if (!User->USWeap->Object.Weap.Strikes)
	{
		QOut(Term, "0Your weapon is broken!");
		User->USWeap->Permanent = FALSE;
		return NULL;
	}
	if (User->USWeap->Object.Weap.Strikes == 3)
		QOut(Term, "0Your weapon is about to break!");
	return User->USWeap;
}

static ToWhatType AttackWhat(char *Word, int Num, RmCodeType Rm)
{
	UserPoint	OldDefPlayer;

	OldDefPlayer = User->DefPlayer;
	User->DefPlayer = FindPlayer(Word, Room[Rm].RmPlayerTail);
	if (User->DefPlayer == User)
		User->DefPlayer = NULL;
	if (!User->DefPlayer)
	{
		MonsterPoint	OldDefMon;

		OldDefMon = User->DefMon;
		if (!(User->DefMon = FindMonster(Word, Num, Room[Rm].RmMonsterTail)))
		{
			if (FindObject(Word, Num, Room[Rm].RmObjectTail))
				QOut(Term, "0Use the command \"SMASH\" instead.");
			else
			{
				sprintf(B1, "0%s isn't here.", Word);
				QOut(Term, B1);
			}
			return ToNone;
		}
		if (OldDefMon != User->DefMon)
		{
			PM(User->DefMon, FALSE, B2);
			sprintf(B1, "0%s attacks %s!", User->Name, B2);
			LocalMsg(B1);
		}
		return ToMonster;
	}
	if (OldDefPlayer != User->DefPlayer)
	{
		int		ILoop, Count;

		sprintf(B1, "0%s is attacking you!", User->Name);
		QOut(User->DefPlayer->Trm, B1);

		sprintf(B1, "0%s attacks %s!", User->Name, User->DefPlayer->Name);
		Count = MsgTerm(Local);
		for (ILoop = 0; ILoop < Count; ILoop++)
			if (TermList[ILoop] != User->DefPlayer->Trm)
				QOut(TermList[ILoop], B1);
	}
	return ToPlayer;
}

void Attack(char *Word, int Num, RmCodeType Rm)
{
	int			Plus;
	ObjectPoint	Weapon;
	ToWhatType	ToWhat = ToNone;

	Weapon = SelectWeap();
	if (ReadyCheck(User->LastAtk) && (Weapon || !User->USWeap))
	{
		if (Room[User->RmCode].Safe)
			QOut(Term, "0This is a safe haven.");
		else
		if (*Word)
			ToWhat = AttackWhat(Word, Num, Rm);
		else
		{
			if (User->DefMon)
				ToWhat = ToMonster;
			else
			if (User->DefPlayer)
				ToWhat = ToPlayer;
			else
				QOut(Term, "0Specify target, please!");
		}
	}
	if (ToWhat == ToNone)
		return;

	if (CmdCode != 42 && User->Fatigue + User->Con < 10)
	{
		QOut(Term, "0You're too exhausted to strike!");
		return;
	}

	switch (CmdCode)
	{
		case 11:	/* attack */
			Plus = 0;
			User->Fatigue = Max(0, User->Fatigue - Rnd(2) + 1);
			break;

		case 42:	/* parry */
			Plus = -(User->Lvl / 2 + 1);
			if (ToWhat == ToMonster)
				User->DefMon->MReact = 5;	/* half damage */
			break;

		case 43:	/* thrust */
			Plus = User->Lvl;
			User->Fatigue = Max(0, User->Fatigue - User->Lvl);
			if (ToWhat == ToMonster && Rnd(4) == 4)
			{
				User->DefMon->MReact = 6;	/* double damage */
				QOut(Term, "0You are vulnerable!");
			}
			break;

		case 85:	/* feint */
		case 86:	/* circle */
			QOut(Term, "0Ok.");
			if (User->Dex <= Rnd(CmdCode == 85 ? 30 : 20))
				QOut(Term, "0Didn't work!");
			else
			if (ToWhat == ToMonster)
				User->DefMon->MReact = CmdCode == 85 ? 3 : 2;
			else
			{
				User->DefPlayer->LastAtk = RealTime + CmdCode == 85 ? 10 : 8;
				sprintf(B1, "0%s %s you!", User->Name,
						CmdCode == 85 ? "feints at" : "circles");
				QOut(User->DefPlayer->Trm, B1);
				if (CmdCode == 85)
				{
					sprintf(B1, "0%s falls back!",
							User->DefPlayer->Sex == Male ? "He" : "She");
					B1[1] = Cap(B1[1]);
				}
				else
					sprintf(B1, "0You circle %s!",
							Pro[(int)User->DefPlayer->Sex]);
				QOut(Term, B1);
			}
			ToWhat = ToNone;
			break;

		case 72:	/* backstab */
			if (User->Hidden)
				Plus = 5;
			else
			{
				Plus = -5;
				QOut(Term, "0You are discovered!");
				if (ToWhat == ToMonster)
					User->DefMon->MReact = 6;	/* double damage */
			}
			break;
	}

	if (Weapon)
		switch ((int)Weapon->Object.Weap.WeapType)
		{
			case Sharp:	 Plus += User->SSharp;	break;
			case Thrust: Plus += User->SThrust;	break;
			case Blunt:  Plus += User->SBlunt;	break;
			case Long:   Plus += User->SLong;	break;
		}

	User->Hidden = FALSE;
	if (ToWhat == ToMonster)
	{
	/*	if (User->DefMon->Assistance)
			ProtectNPC(User, User->DefMon);
		else	*/
			HitAtMonster(Weapon, Plus);
	}
	else
	if (ToWhat == ToPlayer)
		HitAtPlayer(Weapon, Plus);

	/* calculate time until next allowed attack */
	Plus = Weapon ? Weapon->Weight : 2;
	Plus = Plus - User->Str + 12;
	if (CmdCode == 43)	/* thrust */
		Plus++;
	else
	if (CmdCode == 42)	/* parry */
		Plus--;
	if (ToWhat == ToPlayer)
		Plus += 3;
	User->LastAtk = RealTime + Max(3, Plus);
}

void UpdateF(void)
{
	int		ILoop;

	if (NUsers && User)
		for (ILoop = MsgTerm(SysMsg); ILoop > 0;)
			QOut(TermList[--ILoop], "0Files updating.  Please wait.");

	for (ILoop = 0; ILoop <= MaxUsers; ILoop++)
		if (SlotTbl[ILoop] > -1)
			WriteSeg(ILoop, SlotTbl[ILoop], FALSE);

	EFile = fopen(EFILE, "w");

	fprintf(EFile, "%s\n", Notice);

	for (ILoop = 0; ILoop < 5; ILoop++)
		fprintf(EFile, "%s\n", NewsBuf[ILoop]);
	for (ILoop = 0; ILoop < 3; ILoop++)
	{
		fprintf(EFile, "%d %s %s\n", TLvl[ILoop], TName[ILoop], TClass[ILoop]);
		fprintf(EFile, "%s\n", TBuf[ILoop]);
	}

	fprintf(EFile, "%c %d %d\n", TaskClosed, NumRun + 1, MSpeed);

	for (ILoop = 1; ILoop <= LEncounter; ILoop++)
		fprintf(EFile, "%d %d %d %d %d %d %d %d\n",
				EncIndex(ILoop, 0), EncIndex(ILoop, 1), EncIndex(ILoop, 2),
				EncIndex(ILoop, 3), EncIndex(ILoop, 4), EncIndex(ILoop, 5),
				EncIndex(ILoop, 6), EncIndex(ILoop, 7));
	for (ILoop = 1; ILoop <= ObjListLen; ILoop++)
		fprintf(EFile, "%d %d %d %d %d %d %d %d\n",
				ObjIndex(ILoop, 0), ObjIndex(ILoop, 1), ObjIndex(ILoop, 2),
				ObjIndex(ILoop, 3), ObjIndex(ILoop, 4), ObjIndex(ILoop, 5),
				ObjIndex(ILoop, 6), ObjIndex(ILoop, 7));

	fclose(EFile);
}

void Repair(char *Word, int Num)
{
	ObjectPoint	Obj;

	if (W(User->RmCode) == 10)
		QOut(Term, "0\"I don't have such skills.  Sorry!\"");
	else
	if (W(User->RmCode) != 18)
		QOut(Term, "0Find a smith first!");
	else
	if (!(Obj = FindObject(Word, Num, User->ObjectTail)))
		QOut(Term, "0Object not found.");
	else
	if (Obj->ObClass != Weap && Obj->ObClass != Armor && Obj->ObClass != Shield)
		QOut(Term, "0\"I don't know how to repair that!\"");
	else
	{
		User->Data = (Obj->Price * 6) / 10 + Rnd(5);
		if (Obj->Magic)
		{
			User->Data <<= 1;
			QOut(Term, "0\"Very well, but it may lose its magical dweomer!\"");
		}

		if (Obj->ObClass == Weap
		&&	Obj->Object.Weap.MaxHp + Obj->Object.Weap.MinHp > 30)
			QOut(Term, "0\"I'm not sure I can repair such a great weapon!\"");

		PrintObj(Obj, FALSE, FALSE, B2);
		sprintf(B1,
			"0Hmm.  Repairing %s will cost you %ld shillings.  Do you accept?",
			B2, User->Data);
		QOut(Term, B1);
		User->Entry = XRepair;
		User->USWeap = Obj;
	}
}

void ChangeName(char *Word, int Num)
{
	for (Num = 0; Num < sizeof(Alfa); Num++)
		if (Cap(Word[Num]) < 'A' || Cap(Word[Num]) > 'Z')
		{
			Word[Num] = '\0';
			break;
		}

	if (!*Word)
		QOut(Term, "0Missing or bad characters in name.");
	else
	if (FindUser(Word) || PlayerOnFile(Word))
		QOut(Term, "0Name already in use elsewhere.");
	else
	{
		sprintf(B1, "0### %s just changed name to %s.", User->Name, Word);
		AllMsg(B1);
		ChangeUsrName(User->Name, Word);
		strcpy(User->Name, Word);
		QOut(Term, "0Name change complete.");
	}
}

void ChangePW(char *Word, int Num)
{
	Alfa	Word3;

	GetWord(Word3, &Num);
	if (!*Word3 || Num)
		QOut(Term, "0Missing or bad characters in new password.");
	else
	if (Hash(Word) != User->PW)
		QOut(Term, "0Wrong password, sorry.");
	else
	{
		User->PW = Hash(Word3);
		QOut(Term, "0PW change complete.");
		QOut(Term, "0Be sure to \"SAVE\" your character to record the change.");
	}
}

void Offer(char *Word, int Num)
{
	UserPoint	TPlyr;
	int			Num2;
	Alfa		Word3, Word4;
	ObjectPoint	FObj, TObj;

	if (TradeTime >= RealTime)
	{
		sprintf(B1, "0Trade already in progess.  Try again in %ld seconds.",
				TradeTime - RealTime);
		QOut(Term, B1);
		return;
	}
	if (Loc >= LenBuf)
	{
		QOut(Term,
			"0Format is: OFFER <who> <your item/cash> FOR <their item/cash>");
		return;
	}
	if (!(TPlyr = FindPlayer(Word, Room[User->RmCode].RmPlayerTail)))
	{
		QOut(Term, "0Player isn't here!");
		return;
	}

	GetWord(Word3, &Num);
	GetWord(Word4, &Num2);
	if (!(FObj = FindObject(Word3, Num, User->ObjectTail)) && *Word3)
	{
		QOut(Term, "0You don't have that!");
		return;
	}
	if ((TObj = FindObject(Word4, Num2, TPlyr->ObjectTail)) && *Word4)
	{
		QOut(Term, "0Transaction cancelled.");
		return;
	}

	sprintf(B1, "0Offer sent to %s.", TPlyr->Name);
	QOut(Term, B1);

	FCash = Max(0, Num);
	TCash = Max(0, Num2);
	TradeTime = RealTime + 20;
	strcpy(NFPlyr, User->Name);
	strcpy(NTPlyr, TPlyr->Name);
	if (FObj)
		strcpy(SFObj, FObj->Name);
	strcpy(NFObj, Word3);
	if (TObj)
		strcpy(STObj, TObj->Name);
	strcpy(NTObj, Word4);

	if (FObj)
	{
		PrintObj(FObj, FALSE, FALSE, B3);
		sprintf(B2, "0%s offers you %s", User->Name, B3);
	}
	else
		sprintf(B2, "0%s offers you %d shillings", User->Name, FCash);

	if (TObj)
	{
		PrintObj(TObj, TRUE, FALSE, B3);
		sprintf(B1, "%s %s.", B2, B3);
	}
	else
	if (TCash)
		sprintf(B1, "%s for %d shillings.", B2, TCash);
	else
		sprintf(B1, "%s.", B2);
	QOut(TPlyr->Trm, B1);
	QOut(TPlyr->Trm, "0Enter \"ACCEPT\" to accept the offer.");
}

void Accept(void)
{
	UserPoint	FPlyr;
	ObjectPoint	FObj, TObj;
	int			OkTrade;

	if (TradeTime < RealTime || strcmp(NTPlyr, User->Name))
	{
		QOut(Term, "0Nothing to accept.");
		return;
	}
	if (!(FPlyr = FindPlayer(NFPlyr, Room[User->RmCode].RmPlayerTail)))
	{
		QOut(Term, "0Transaction cancelled.");
		return;
	}
	FObj = FindObject(NFObj, FCash,	FPlyr->ObjectTail);
	TObj = FindObject(NTObj, TCash,	User->ObjectTail);
	if (!FObj && NFObj[0] || !TObj && NTObj[0])
	{
		QOut(Term, "0Transaction cancelled.");
		return;
	}

	OkTrade = TRUE;
	if (FObj)
		OkTrade = !strcmp(FObj->Name, SFObj)
				&& FObj->Weight + User->Weight <= User->Str * 10;
	if (TObj && OkTrade)
		OkTrade = !strcmp(TObj->Name, STObj)
				&& TObj->Weight + FPlyr->Weight <= FPlyr->Str * 10;
	if (OkTrade)
		OkTrade = FCash <= FPlyr->Money && TCash <= User->Money;

	if (!OkTrade)
	{
		QOut(Term, "0Transaction cancelled.");
		return;
	}

	if (FObj)
	{
		StopUsing(FPlyr, FObj);
		DeleteObject(FObj, &FPlyr->ObjectTail);
		FPlyr->Weight = Max(0, FPlyr->Weight - FObj->Weight);
		FObj->Next = User->ObjectTail;
		User->ObjectTail = FObj;
		User->Weight = Min(5000, User->Weight + FObj->Weight);
	}
	else
	{
		FPlyr->Money -= FCash;
		User->Money += FCash;
	}

	if (TObj)
	{
		StopUsing(User, TObj);
		DeleteObject(TObj, &User->ObjectTail);
		User->Weight = Max(0, User->Weight - TObj->Weight);
		TObj->Next = FPlyr->ObjectTail;
		FPlyr->ObjectTail = TObj;
		FPlyr->Weight = Min(5000, FPlyr->Weight + TObj->Weight);
	}
	else
	{
		User->Money -= TCash;
		FPlyr->Money += TCash;
	}

	TradeTime = RealTime - 50;
	sprintf(B1, "0Ok, you trade with %s.", Pro[(int)FPlyr->Sex]);
	QOut(Term, B1);
	sprintf(B1, "0%s accepts your offer and trades with you.", User->Name);
	QOut(FPlyr->Trm, B1);
}

void NumUsers(void)
{
	UserPoint	Plyr;
	int			ILoop;

	for (ILoop = 0, Plyr = UserTail; Plyr; Plyr = Plyr->NextUser)
		if (Plyr->Invisible)
			ILoop++;
	sprintf(B1, "0%2d users (Max Online)", MaxUsers);
	QOut(Term, B1);
	sprintf(B1, "0%2d users (Active)", User->SSJ ? NUsers : NUsers - ILoop);
	QOut(Term, B1);
	if (User->SSJ)
	{
		sprintf(B1, "0%2d users (Invisible)", ILoop);
		QOut(Term, B1);
	}
}

void Help(void)
{
	int		ILoop;

	QOut(Term, "0List of all commands:");
	strcpy(B1, "0");
	for (ILoop = 1; ILoop <= CmdListLen; ILoop++)
	{
		if (ILoop % 6 == 1)
		{
			QOut(Term, B1);
			strcpy(B1, "0");
		}
		if (User->SSJ || (*CmdList[ILoop] != '*'))
		{
			sprintf(B2, "  %-10s", CmdList[ILoop]);
			strcat(B1, B2);
		}
	}
	if (B1[2])
		QOut(Term, B1);
	QOut(Term, "0");
}

void Parley(char *Word, int Num)
{
	MonsterPoint	Mon;
	int				Opt, N1;

	if (!(Mon = FindMonster(Word, Num, Room[User->RmCode].RmMonsterTail)))
	{
		QOut(Term, "0That non-player character isn't here.");
		return;
	}

	Opt = Mon->MParley;
	if (Opt == 9)
		Opt = Rnd(8);
	if (Mon->DescRec == 0 && (Opt >= 2 && Opt <= 4 || Opt >= 11))
		Opt = 0;

	PM(Mon, FALSE, B2);
	sprintf(B1, "0%s talks with %s.", User->Name, B2);
	LocalMsg(B1);

	switch (Opt)
	{
		case 1:
			N1 = 0;
			Mon->MParley = 3;
			if (Mon->WhichObj)
				N1 = ObjIndex(Mon->WhichObj, Rnd(8) - 1);
			if (!N1)
				QOut(Term, "0I have nothing to sell you, good sir!");
			else
			{
				ObjectType	ObjTemp;

				fseek(OList, (long)N1 * sizeof(ObjectType), 0);
				fread((char *)&ObjTemp, sizeof(ObjectType), 1, OList);
				User->Data = ObjTemp.Price / 2 + Rnd(ObjTemp.Price);
				User->LastAtk = N1;
				PM(Mon, TRUE, B2);
				PrintObj(&ObjTemp, FALSE, FALSE, B3);
				sprintf(B1,
					"0%s says, \"I will sell you %s for a mere %ld shillings.\"",
					B2, B3, User->Data);
				QOut(Term, B1);
				User->Entry = XParley;
			}
			break;

		case 2:
		case 3:
		case 4:
			if (Opt == 2)
				N1 = Rnd(5);
			else
			if (Opt == 3)
				N1 = Rnd(5) + 5;
			else
				N1 = Rnd(10);
			PM(Mon, TRUE, B1);
			sprintf(B2, "0%s says, \"", B1);
			PrintDesc(Mon->DescRec, N1, 0, FALSE, B2, B1);
			strcat(B1, "\"");
			QOut(Term, B1);
			break;

		case 5:
		case 8:
		{
			ObjectPoint	Obj;
			int			N2;

			if (User->Lvl < Mon->Lvl + 5)
			{
				PM(Mon, TRUE, B2);
				sprintf(B1, "0%s stands its ground!", B2);
				QOut(Term, B1);
				break;
			}

			N1 = 0;
			if (Opt == 8)
			{
				Mon->MParley = 5;
				if (Mon->WhichObj)
					N1 = ObjIndex(Mon->WhichObj, Rnd(8) - 1);
				if (N1)
				{
					Obj = (ObjectPoint)malloc(sizeof(ObjectType));
					fseek(OList, (long)N1 * sizeof(ObjectType), 0);
					fread((char *)Obj, sizeof(ObjectType), 1, OList);
					Obj->Next = Room[User->RmCode].RmObjectTail;
					Room[User->RmCode].RmObjectTail = Obj;
					N1 = 1;
				}
			}

			N2 = Rnd(2);
			PM(Mon, TRUE, B2);
			if (N1)
			{
				PrintObj(Obj, FALSE, FALSE, B3);
				sprintf(B1, "0%s throws down %s and ", B2, B3);
			}
			else
				sprintf(B1, "0%s ", B2);
			strcat(B1, N2 == 1 ? "begs for mercy!" : "flees from your sight!");
			QOut(Term, B1);

			PM(Mon, TRUE, B2);
			if (N1)
			{
				PrintObj(Obj, FALSE, FALSE, B3);
				sprintf(B1, "0%s throws down %s and ", B2, B3);
			}
			else
				sprintf(B1, "0%s ", B2);
			if (N2 == 1)
				strcat(B1, "begs for mercy!");
			else
			{
				sprintf(B2, "flees from %s's sight!", User->Name);
				strcat(B1, B2);
			}
			LocalMsg(B1);

			if (N2 == 2)
			{
				DeleteMonster(Mon, User->RmCode);
				Destroy(Mon);
			}
			break;
		}
		case 6:
			PM(Mon, TRUE, B2);
			if (Mon->DefPlayer == User && Rnd(4) == 2)
			{
				Mon->MReact = 2;
				Mon->DefPlayer = NULL;
				sprintf(B1, "0%s pauses for a moment.", B2);
			}
			else
				sprintf(B1, "0%s makes no apparent response.", B2);
			QOut(Term, B1);
			break;

		case 7:
			Mon->DefPlayer = User;
			if (Mon->CharmPlayer == User)
				Mon->CharmPlayer = NULL;
			PM(Mon, TRUE, B2);
			sprintf(B1, "0%s attacks you!", B2);
			QOut(Term, B1);

			sprintf(B1, "0%s attacks %s!", B2, User->Name);
			LocalMsg(B1);
			break;

		case 10:
		{
			int		T, NewRm, ILoop, Exit;

			PM(Mon, TRUE, B2);
			sprintf(B1, "0** ZAP **  %s teleports you elsewhere!", B2);
			QOut(Term, B1);

			sprintf(B1, "0%s makes %s vanish in a cloud of smoke!",
					B2, User->Name);
			LocalMsg(B1);

			do
			{
				if (Mon->Lvl < 10)
					NewRm = (User->RmCode / 100) * 100 + Rnd(99);
				else
					NewRm = Rnd(669) + 30;
				T = S(NewRm);
				for (Exit = ILoop = 0; !Exit && ILoop < 6; ILoop++)
					Exit += Room[T].Adjoin[ILoop];
			} while (!Exit && !Room[T].Out);

			StopFollow(User);
			DeletePlayer(User, User->RmCode);
			PlacePlayer(User, NewRm);
			break;
		}

		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			PM(Mon, TRUE, B3);
			sprintf(B2, "0%s says, \"", B3);
			PrintDesc(Mon->DescRec, Opt - 10, 0, FALSE, B2, B1);
			strcat(B1, "\"");
			QOut(Term, B1);
			break;

		default:
			PM(Mon, TRUE, B2);
			sprintf(B1, "0%s makes no apparent response.", B2);
			QOut(Term, B1);
			break;
	}
}

void YesNo(void)
{
	ObjectPoint	Obj;
	int			Num, Yes;
	Alfa		Word;

	GetWord(Word, &Num);
	Yes = Cap(Word[0]) == 'Y';

	if (User->Entry == XParley)
	{
		char	*p;

		if (!Yes)
			p = "0Oh please reconsider, Effendi!  It is of such high quality!";
		else
		if (User->Data > User->Money)
			p = "0You have not enough money, Effendi!";
		else
		{
			User->Money -= User->Data;
			Obj = (ObjectPoint)malloc(sizeof(ObjectType));
			fseek(OList, (long)User->LastAtk * sizeof(ObjectType), 0);
			fread((char *)Obj, sizeof(ObjectType), 1, OList);
			Obj->Next = Room[User->RmCode].RmObjectTail;
			Room[User->RmCode].RmObjectTail = Obj;
			p = "0Here you are, my friend.  You are indeed a shrewd buyer, Effendi!";
		}
		QOut(Term, p);
	}
	else
	if (User->Entry == XSell)
	{
		Obj = User->USWeap;
		User->USWeap = NULL;
		if (Yes && Obj)
		{
			User->Money += User->Data;
			User->Weight = Max(0, User->Weight - Obj->Weight);
			DeleteObject(Obj, &User->ObjectTail);
			ODestroy(Obj);
			QOut(Term, "0\"Thank you!  Come again!\"");
		}
	}
	else	/* XRepair */
	{
		Obj = User->USWeap;
		User->USWeap = NULL;
		if (Yes && Obj)
		{
			if (User->Data > User->Money)
				QOut(Term, "0\"You don't have enough money!\"");
			else
			{
				if (Rnd(2) == 1)
					Obj->Magic = FALSE;
				switch ((int)Obj->ObClass)
				{
					case Weap:
					{
						struct tWeap	*p = &Obj->Object.Weap;

						if (p->MaxHp + p->MinHp > 30 && Rnd(2) == 1)
						{
							QOut(Term, "0<<CRACK>> \"Oops! Sorry!\" the repairman says meekly.");
							p->MinHp = 1;
							p->MaxHp = p->MaxHp / 2 + 1;
						}
						else
						{
							if (!Obj->Magic)
								p->WeaPlus = Min(0, p->WeaPlus);
							p->Strikes = Min(100, p->Strikes * 2 + 50);
						}
						break;
					}
					case Shield:
						Obj->Object.Shield.ShHits =
							Min(50, Obj->Object.Shield.ShHits * 2 + 25);
						break;

					case Armor:
						Obj->Object.Armor.ArmHits =
							Min(50, Obj->Object.Armor.ArmHits * 2 + 25);
						break;
				}

				QOut(Term, "0\"Here it is sir, repaired as well it can be.\"");
				User->Money -= User->Data;
			}
		}
	}

	User->Entry = XCmd;
	Loc = LenBuf + 1;
}

void EnterNotice(void)
{
	int		ILoop, JLoop;

	for (JLoop = 0, ILoop = Loc; ILoop < Min(80, LenBuf);)
		Notice[JLoop++] = Buf[ILoop++];
	Notice[JLoop] = '\0';
	Loc = LenBuf + 1;
	User->Entry = XCmd;
}

void EnterNews(void)
{
	int		ILoop, Found;

	Found = FALSE;
	for (ILoop = 0; !Found && ILoop < 5; ILoop++)
		Found = !*NewsBuf[ILoop];

	if (!Found)
		QOut(Term, "0*DELETENEWS a line first.");
	else
	{
		int		JLoop, KLoop;

		--ILoop;
		KLoop = 0;
		for (JLoop = Loc; JLoop < Min(79, LenBuf); JLoop++)
			NewsBuf[ILoop][KLoop++] = Buf[JLoop];
		NewsBuf[ILoop][KLoop] = '\0';

		sprintf(B1, "0Entered as line %d.", ILoop + 1);
		QOut(Term, B1);
	}

	Loc = LenBuf + 1;
	User->Entry = XCmd;
}

void StealObj(Alfa Word, int Num)
{
	UserPoint	Plyr;
	Alfa		Word2;
	int			Lvl, Num2;
	char		Notice, Steal;
	int			StealChance;
	ObjectPoint	Obj;

	if (!ReadyCheck(User->LastAtk))
		return;

	Lvl = User->Lvl;
	if (User->TGuild)
		Lvl++;
	if (User->Class != Thief)
		Lvl -= 6;

	GetWord(Word2, &Num2);
	Plyr = FindPlayer(Word2, Room[User->RmCode].RmPlayerTail);
	if (Plyr == User)
		Plyr = NULL;
	if (!Plyr)
	{
		QOut(Term, "0That player is not here!");
		return;
	}
	if (Room[User->RmCode].Safe)
	{
		QOut(Term, "0This is a safe haven!");
		return;
	}

	User->LastAtk = RealTime + 15;
	Obj = FindObject(Word, Num, Plyr->ObjectTail);
	if (!Plyr->SSJ && (Obj || !*Word && Num > 0 && Num <= Plyr->Money))
	{
		StealChance = 4 * (Lvl - Plyr->Lvl + User->Dex / 2 - 6)
					- (Obj ? Obj->Weight : (int)(Num * 100 / Plyr->Money));
		if (Plyr->Class == Thief || Plyr->Class == Ranger)
			StealChance -= 16;
		if (User->Hidden)
			StealChance += 16;
	}
	else
		StealChance = -1;

	Steal = Rnd(100) < StealChance;
	if (Obj == Plyr->USWeap || Obj == Plyr->USArm || Obj == Plyr->USShield
	||	Obj == Plyr->USRingL || Obj == Plyr->USRingR)
		Notice = TRUE;
	else
		Notice = Rnd(100) >= StealChance;

	User->DefPlayer = Plyr;
	User->DefMon = NULL;
	if (Notice)
	{
		sprintf(B1, "0%s %s ", User->Name, Steal ? "steals" : "fails to steal");
		if (Obj)
		{
			PrintObj(Obj, TRUE, FALSE, B2);
			strcat(B1, B2);
			strcat(B1, " ");
		}
		else
		if (!*Word)
		{
			sprintf(B2, "%d shillings ", Num);
			strcat(B1, B2);
		}
		strcat(B1, "from you!");
		QOut(Plyr->Trm, B1);
	}
	if (Notice && Rnd(2) == 1)
		QOut(Term, "0You were discovered!");
	if (!Steal)
		QOut(Term, "0You fail to steal it!");
	else
	{
		QOut(Term, "0You steal it!");
		if (Obj)
		{
			StopUsing(Plyr, Obj);
			DeleteObject(Obj, &Plyr->ObjectTail);
			Plyr->Weight = Max(0, Plyr->Weight - Obj->Weight);
			if (User->Weight + Obj->Weight > User->Str * 10)
			{
				QOut(Term, "0It is too heavy to carry!  You drop it.");
				Obj->Next = Room[User->RmCode].RmObjectTail;
				Room[User->RmCode].RmObjectTail = Obj;
			}
			else
			{
				Obj->Invisible = FALSE;
				User->Weight = Min(2500, User->Weight + Obj->Weight);
				Obj->Next = User->ObjectTail;
				User->ObjectTail = Obj;
			}
		}
		else
		{
			User->Money += Num;
			Plyr->Money -= Num;
		}
	}
}
