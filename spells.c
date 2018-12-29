#include "scepter.h"

void	EnterSpell(Word, Num)
char	*Word;
int		Num;
{
	if (ReadyCheck(User->LastAtk))
	{
		CapAlfa(Word);
		if (WordMatch(Word, Spellen, SpellList))
			GetWord(Word, &Num);
		strcpy(User->WData, Word);
		User->Data = Num;
		User->Entry = XSpell;
	}
	LenBuf = 0;
}

int		SpellMon(SpellCode, Level, Intel, Plyr, Monster, Obj, Target)
int				SpellCode, Level, Intel;
UserPoint		Plyr;
MonsterPoint	Monster;
ObjectPoint		Obj;
char			*Target;
{
	int		Damage = 0;
	static char	*NothingText = "0Nothing happens.";

	switch (SpellCode)
	{
		case SPELL_VIGOR:		/* vigor, OnUsPlayer */
			Plyr->Fatigue = Min(Plyr->MaxFatigue, Plyr->Fatigue + Level * 4);
			sprintf(B1, "0You now have %d fatigue points.", Plyr->Fatigue);
			QOut(Plyr->Trm, B1);
			break;

		case SPELL_HEAL:		/* heal, OnUsPlayer */
			Plyr->Hits = Min(Plyr->MaxHits, Plyr->Hits + Level * 3);
			sprintf(B1, "0You now have %d vitality points.", Plyr->Hits);
			QOut(Plyr->Trm, B1);
			break;

		case SPELL_FIREBALL:		/* fireball, OnMonPlayer */
			if (Level + 10 > Rnd(20))
				Damage = Min(Intel * 2,	3 * (Level + 1));
			break;

		case SPELL_LIGHTNING:		/* lightning, OnMonPlayer */
			if (Level + 8 > Rnd(20))
				Damage = Min(20, 2 * (Level + 1)) + Rnd(2 * Intel);
			break;

		case SPELL_HURT:		/* hurt, OnMonPlayer */
			Damage = Rnd(3);
			break;

		case SPELL_CUREPOISON:		/* curepoison, OnUsPlayer */
			Plyr->Poisoned = FALSE;
			QOut(Plyr->Trm, "0The venom has disappeared!");
			break;

		case SPELL_DISINTEGRA:		/* disintegrate, OnMonPlayer */
			Damage = Rnd(5 * Intel)	+ Level * 2;
			break;

		case SPELL_BEFUDDLE:		/* befuddle, OnMonPlayer */
			if (Plyr)
				Plyr->LastAtk = RealTime + Intel + Rnd(Level);
			else
			{
				Monster->MReact = 4;
				Monster->DefPlayer = NULL;
				Monster->CharmPlayer = NULL;
				QOut(Term, "0It pauses for a moment...");
			}
			break;

		case SPELL_TELEPORT:		/* teleport, OnUsPlayer */
		{
			int		NewRm, Exit;

			do
			{
				int		T;

				if (Level < 10)
					NewRm = (Plyr->RmCode / 100) * 100 + Rnd(99);
				else
					NewRm = Rnd(669) + 30;
				T = S(NewRm);
				Exit = 0;
				if (!Room[T].AntiMagic)
				{
					int		ILoop;

					for (ILoop = 0; ILoop < 6; ILoop++)
						Exit += Room[T].Adjoin[ILoop];
					Exit += Room[T].Out;
				}
			} while (!Exit);
			StopFollow(Plyr);
			DeletePlayer(Plyr, Plyr->RmCode);
			PlacePlayer(Plyr, NewRm);
			QOut(Plyr->Trm, "0**Poof** You find that you are elsewhere..");
			break;
		}
		case SPELL_WISH:	/* wish, OnUsPlayer */
			strcpy(User->WData, Target);
			User->Entry = XWish;
			break;

		case SPELL_PASSDOOR:	/* passdoor, OnObject */
			if (Obj->ObClass != Door && Obj->ObClass != Portal)
				QOut(Term, "0Can't be done!");
			else
			if (Obj->Magic)
				QOut(Term, "0A magic barrier stops you from passing!");
			else
			{
				StopFollow(User);
				DeletePlayer(User, User->RmCode);
				if (Obj->ObClass == Door)
					PlacePlayer(User, Obj->Object.Door.DToWhere);
				else
					PlacePlayer(User, Obj->Object.Portal.ToWhere);
				QOut(Term, "0You pass through the portal!");
			}
			break;

		case SPELL_ENCHANT:	/* enchant, OnObject */
			if (Obj->Carry)
			{
				Obj->Magic = TRUE;
				QOut(Term, "0Ok.  It begins to glow!");
			}
			else
				QOut(Term, "0Enchantment fails!");
			break;

		case SPELL_BLESS:	/* bless, OnPlayer */
			if (User->Lvl < Plyr->Lvl && User->Lvl < 10)
				QOut(Term,
					 "0You are not worthy of blessing this great person!");
			else
			{
				Plyr->Pty = Min(25, Plyr->Pty + 1);
				QOut(Plyr->Trm, "0You feel purified and whole.");
			}
			break;

		case SPELL_PROTECTION:	/* protection, OnUser */
			StopUsing(User, User->USArm);
			StopUsing(User, User->USShield);
			User->AC = 10 - Level / 5 - 1;
			QOut(Term, "0You feel limber and strong..");
			break;

		case SPELL_CURSE:	/* curse, OnPlayer */
			Plyr->Pty = Max(-10, Plyr->Pty - 1);
			QOut(Plyr->Trm, "0You've been cursed!");
			break;

		case SPELL_POISON:	/* poison, OnPlayer */
			Plyr->Poisoned = TRUE;
			QOut(Plyr->Trm, "0You've been poisoned!");
			break;

		case SPELL_INTOXICATE:	/* intoxicate, OnUsPlayer */
			Plyr->Drunk = Max(RealTime, Plyr->Drunk) + 60;
			QOut(Plyr->Trm, "0(HIC)  You feel tipsy.");
			break;

		case SPELL_PSI_BLAST:	/* psi blast, OnMonPlayer */
			if (Plyr)
				Plyr->Magic = Max(0, Plyr->Magic - Rnd(3 * Intel) - 2 * Level);
			else if (Monster)
				Monster->MonSpells = FALSE;
			break;

		case SPELL_POLYMORPH:	/* polymorph, OnMon */
		{
			MonsterType	TmpMon;
			char		TmpName[21];

			do
			{
				fseek(MList, (long)(Rnd(73) + 29) * sizeof(MonsterType), 0);
				fread((char *)&TmpMon, sizeof(MonsterType), 1, MList);
			} while (!*TmpMon.Name || !strcmp(TmpMon.Name, ProtoMonster.Name));
			TmpMon.Lvl = Min(25, Max(1, TmpMon.Lvl + 2 - Rnd(3)));
			TmpMon.Hits = Min(TmpMon.MaxHits,
						Max(1, TmpMon.Hits * Monster->Hits / Monster->MaxHits));
			TmpMon.Next = Monster->Next;
			TmpMon.DefPlayer = Monster->DefPlayer;
			TmpMon.DefMon = Monster->DefMon;
			TmpMon.CharmPlayer = Monster->CharmPlayer;
			TmpMon.ObjectTail = Monster->ObjectTail;
			strcpy(TmpName, Monster->Name);
			PM(Monster, TRUE, B2);
			*Monster = TmpMon;
			FixMonCount(Room[User->RmCode].RmMonsterTail, TmpName);
			FixMonCount(Room[User->RmCode].RmMonsterTail, Monster->Name);
			PrintMon(Monster, FALSE, FALSE, B3);
			sprintf(B1, "0%s transforms into %s!", B2, B3);
			LocalMsg(B1);
			QOut(Term, B1);
			break;
		}
		case SPELL_SUMMON:	/* summon, OnUser (on nothing, really) */
		{
			RmCodeType	Rm;

			Rm = User->RmCode;
			if (Room[Rm].WhichEncounter && EncIndex(Room[Rm].WhichEncounter, 0))
			{
				MonsterPoint	Mon;
				UserPoint		Pl;
				int				ILoop, MonNum, NonDMs;

				for (ILoop = 0; ILoop < 8; ILoop++)
					if (!EncIndex(Room[Rm].WhichEncounter, ILoop))
						break;
				MonNum = EncIndex(Room[Rm].WhichEncounter, Rnd(ILoop) - 1);
				Mon = (MonsterPoint)malloc(sizeof(MonsterType));
				fseek(MList, (long)MonNum * sizeof(MonsterType), 0);
				fread((char *)Mon, sizeof(MonsterType), 1, MList);
				if (Mon->Unique)
				{
					MonsterPoint	M;

					for (M = Room[Rm].RmMonsterTail; M; M = M->Next)
						if (M->Unique && !strcmp(M->Name, Mon->Name))
						{
							free(Mon);
							Mon = NULL;
							QOut(Term, NothingText);
							break;
						}
				}
				if (Mon)
				{
					Mon->Hits = Min(Mon->MaxHits,
									Max(1, (12 - Rnd(3)) * Mon->MaxHits));
					Mon->Lvl = Min(25, Max(1, Mon->Lvl + 2 - Rnd(3)));
					InsertMonster(Mon, Rm);
					NonDMs = 0;
					for (Pl = Room[Rm].RmPlayerTail; Pl; Pl = Pl->Next)
						if (!Pl->Invisible)
							NonDMs++;
					EncounterMsg(Mon, Rm, 1, NonDMs);
				}
			}
			else
				QOut(Term, NothingText);
			break;
		}
		case SPELL_ANIMATE:	/* animate, OnObject */
			if ((Obj->ObClass == Weap || Obj->ObClass == MagDevice)
			&&	Obj->Carry)
			{
				MonsterPoint	Mon;

				Mon = (MonsterPoint)malloc(sizeof(MonsterType));
				*Mon = ProtoMonster;
				strcpy(Mon->Name, Obj->Name);
				Mon->Article = Obj->Article;
				if (Obj->ObClass == Weap)
				{
					struct tWeap	*pW = &Obj->Object.Weap;

					Mon->Lvl = Max(0, Min(25, (pW->MaxHp + pW->MinHp) / 8));
					Mon->Hits = pW->Strikes;
					Mon->AC = -pW->WeaPlus;
				}
				else
				{
					struct tMagDevice	*pM = &Obj->Object.MagDevice;

					Mon->Lvl = Max(1, pM->MLevel);
					Mon->Hits = Mon->Lvl * Min(100, Max(10, pM->NumCharges));
					Mon->AC = 0;
					if (SpellClass[pM->MSpell].SplType & OnMon)
						Mon->MonSpells = TRUE;
					if (pM->MSpell == SPELL_POISON)
						Mon->Poison = TRUE;
					Mon->AntiMagic = Obj->Magic;
					Mon->Regenerate = TRUE;
				}
				Mon->MaxHits = Mon->Hits;
				Mon->AtkSpeed = Max(-4, Min(4, 3 - Obj->Weight / 2));
				Mon->DescRec = Obj->DescRec;
				Mon->DescCode = Obj->DescCode;
				Mon->Magic = Obj->Magic;
				Mon->Permanent = Obj->Permanent;
				Mon->Invisible = Obj->Invisible;
				if (Level > 9)
					Mon->FastReact = TRUE;
				else
				if (Level > 4)
					Mon->SlowReact = TRUE;
				DeleteObject(Obj, &Room[User->RmCode].RmObjectTail);
				free(Obj);
				InsertMonster(Mon, User->RmCode);
			}
			else
				QOut(Term, "0Spell fails.");
			break;

		case SPELL_CHARM:	/* charm, OnMonPlayer */
			if (Plyr)
				if (Rnd(100) < 50 + Level + Intel - Plyr->Lvl - Plyr->Int)
				{
					Plyr->LastAtk = RealTime + Rnd(Intel + Level);
					Plyr->Follow = User;
				}
				else
					QOut(Plyr->Trm, "0It failed!");
			else
			{
				Monster->MReact = 4;
				Monster->DefPlayer = NULL;
				Monster->CharmPlayer = User;
			}
			break;

		case SPELL_ILLUMINATE:	/* illuminate, OnUser (on nothing, really) */
			if (Room[User->RmCode].Dark)
			{
				UserPoint	Temp;
				RoomType	*Rm = &Room[User->RmCode];

				Rm->Dark = FALSE;	/* temporarily */
				Temp = User;
				for (User = Rm->RmPlayerTail; User; User = User->Next)
				{
					Term = User->Trm;
					RoomDisplay(User->RmCode, User->Brief);
				}
				User = Temp;
				Term = User->Trm;
				Rm->Dark = TRUE;
			}
			else
				QOut(Term, NothingText);
			break;
	}

	return Damage;
}

void	Spell(SpellCode)
int		SpellCode;
{
	MonsterPoint	Mon = NULL;
	ObjectPoint		Obj = NULL;
	UserPoint		Player = NULL;
	Alfa			Target, Dummy;
	int				Damage, LvlNeeded, MPNeeded, InNeeded;
	int				Num, ILoop, Count, GoodSpl;
	int				SplLevel, SplInt;
	SpellType		*Sp;
	enum {Chant, Scrl, Device}	MagType;

	User->Hidden = FALSE;
	SplLevel = User->Lvl;
	SplInt = User->Int;
	MagType = Chant;

	if (!SpellCode)
	{
		int		Sum = 0, Length = 0;

		User->Entry = XCmd;
		for (ILoop = 0; ILoop < LenBuf; ILoop++)
			if (Cap(Buf[ILoop]) >= 'A' && Cap(Buf[ILoop]) <= 'Z')
			{
				Sum += Cap(Buf[ILoop]) - 'A';
				Length++;
			}
		LenBuf = 0;

		for (SpellCode = 1; SpellCode <= Spellen; SpellCode++)
			if (Sum == SpellClass[SpellCode].SplHash
			&&	Length == SpellClass[SpellCode].SplLen)
				break;

		if (SpellCode > Spellen)
		{
			QOut(Term, "0You mispronounced the spell!");
			return;
		}
	}
	else
	if (SpellCode < 0)
	{
		SpellCode = -SpellCode;
		MagType = Device;
		SplLevel = Max(1, Min(25, SpellCode >> 8));
		SplInt = SplLevel / 4 + 12;
		SpellCode &= 0xff;
	}
	else
		MagType = Scrl;

	GoodSpl = TRUE;
	strcpy(Target, User->WData);
	CapAlfa(Target);
	Num = User->Data;
	Sp = &SpellClass[SpellCode];
	switch ((int)Sp->SplType)
	{
		case OnUser:
		case OnPlayer:
		case OnUser | OnPlayer:
			if (*Target)
				Player = FindPlayer(Target, Room[User->RmCode].RmPlayerTail);
			else
				Player = User;

			if (Sp->SplType == OnPlayer && (!*Target || Player == User)
			|| (*Target && (!Player || Sp->SplType == OnUser)))
			{
				GoodSpl = FALSE;
				QOut(Term, *Target ? "0That spell target is illegal!"
								   : "0The spell target was not specified!");
			}
			break;

		case OnMon:
		case OnMon | OnPlayer:
			Player = FindPlayer(Target, Room[User->RmCode].RmPlayerTail);
			Mon = FindMonster(Target, Num, Room[User->RmCode].RmMonsterTail);
			if (!Mon && (!Player || Sp->SplType == OnMon) || Player == User)
			{
				GoodSpl = FALSE;
				QOut(Term, *Target ? "0That spell target is illegal!"
								   : "0The spell target was not specified!");
			}
			break;

		case OnObject:
			Obj = FindObject(Target, Num, User->ObjectTail);
			if (!Obj || SpellCode == SPELL_ANIMATE)	/* animate */
				Obj = FindObject(Target, Num, Room[User->RmCode].RmObjectTail);
			if (!Obj)
			{
				GoodSpl = FALSE;
				QOut(Term, "0Object not here!");
			}
			break;
	}

	if (Room[User->RmCode].AntiMagic || Room[User->RmCode].Safe
		&&	SpellCode != SPELL_VIGOR
		&& SpellCode != SPELL_HEAL
		&& SpellCode != SPELL_CUREPOISON
		&& SpellCode != SPELL_BLESS)
	{
		GoodSpl = FALSE;
		QOut(Term, "0Such magic does not work here.  Spell fails.");
	}

	if (Mon && Player)
		Player = NULL;
	if (Mon && Mon->AntiMagic)
	{
		GoodSpl = FALSE;
		QOut(Term, "0Your spell has no effect on it!");
	}

	GoodSpl = GoodSpl && ReadyCheck(User->LastAtk);
	if (MagType == Device)
	{
		MPNeeded	= 0;
		LvlNeeded	= -10;
		InNeeded	= 0;
	}
	else
	{
		MPNeeded	= Sp->SplMp;
		LvlNeeded	= Sp->SplLvl;
		InNeeded	= Sp->SplInt;
		if (MagType == Scrl)
		{
			MPNeeded = 0;
			LvlNeeded -= 3 + Rnd(4);
			InNeeded -= 3;
		}
	}

	if (MagType == Chant)
	{
		if (User->Class != MagicUser
			&& (SpellCode == SPELL_DISINTEGRA
				|| SpellCode == SPELL_WISH
				|| SpellCode == SPELL_ENCHANT
				||	SpellCode == SPELL_PROTECTION
				|| SpellCode == SPELL_PSI_BLAST
				|| SpellCode == SPELL_POLYMORPH))
		{
			GoodSpl = FALSE;
			QOut(Term, "0Only magic-users can cast that spell!");
		}
		else
		if (User->Class != Cleric
			&& (SpellCode == SPELL_HEAL || SpellCode == SPELL_BLESS))
		{
			GoodSpl = FALSE;
			QOut(Term, "0Only clerics can cast that spell!");
		}
		else
		if (SpellCode == SPELL_VIGOR
			&& User->Class != Cleric && User->Class != MagicUser)
		{
			GoodSpl = FALSE;
			QOut(Term, "0Only magic users and clerics may cast vigor spells!");
		}
	}

	if (!GoodSpl || !SpellCost(MPNeeded, LvlNeeded, InNeeded))
		return;

	strcpy(Dummy, SpellList[SpellCode]);
	for (ILoop = 0; ILoop < sizeof(Alfa); ILoop++)
		if (Dummy[ILoop] >= 'A' && Dummy[ILoop] <= 'Z')
			Dummy[ILoop] += ' ';

	if (MagType != Device)
	{
		sprintf(B1, "0You cast %s %s spell!", Pa(*Dummy), Dummy);
		QOut(Term, B1);
	}
	else
		QOut(Term, "0Ok.");

	Count = MsgTerm(Local);
	for (ILoop = 0; ILoop < Count; ILoop++)
	{
		if (Mon)
		{
			PM(Mon, FALSE, B2);
			sprintf(B1, "0%s casts %s %s spell on %s!",
					User->Name, Pa(*Dummy), Dummy, B2);
		}
		else
		if (Obj)
		{
			PrintObj(Obj, TRUE, FALSE, B2);
			sprintf(B1, "0%s casts %s %s spell on %s!",
					User->Name, Pa(*Dummy), Dummy, B2);
		}
		else if (Player->Trm != TermList[ILoop])
		{
			if (User == Player)
				sprintf(B1, "0%s casts %s %s spell!",
						User->Name, Pa(*Dummy), Dummy);
			else
				sprintf(B1, "0%s casts %s %s spell on %s!",
						User->Name, Pa(*Dummy), Dummy, Player->Name);
		}
		else
			sprintf(B1, "0%s casts %s %s spell on you!",
					User->Name, Pa(*Dummy), Dummy);

		QOut(TermList[ILoop], B1);
	}

	Damage = SpellMon(SpellCode, SplLevel, SplInt, Player, Mon, Obj, Target);

	if (SpellCode == SPELL_FIREBALL
		|| SpellCode == SPELL_LIGHTNING
		|| SpellCode == SPELL_HURT
		||	SpellCode == SPELL_DISINTEGRA
		|| SpellCode == SPELL_PSI_BLAST)
	{
		User->LastAtk = RealTime + Max(5, Sp->SplMp / 2 - User->Lvl);
		User->DefPlayer = NULL;
		User->DefMon = NULL;
		if (Player)
		{
			User->DefPlayer = Player;
			Player->HitAtTime = RealTime;
		}
		else if (Mon)
			User->DefMon = Mon;
		else
			Abort(" Sce106 - no spell target!");

		if (!Damage && SpellCode != SPELL_PSI_BLAST)
		{
			QOut(Term, "0It missed!");
			if (Player)
				QOut(Player->Trm, "0It missed!");
		}
		else if (Player && SpellCode != SPELL_PSI_BLAST)
			HitPlayer(Damage);
#ifdef USEJAIL
		else if (Mon->Assistance)
			ProtectNPC(User, Mon);
#endif
		else
		{
			if (Mon->Defend && !Mon->DefPlayer
			||	Mon->AtkLastAggr && Mon->DefPlayer != User)
			{
				Mon->DefPlayer = User;
				if (Mon->CharmPlayer == User)
					Mon->CharmPlayer = NULL;
				if (Mon->MReact >= 5)
					Mon->MReact = 0;
			}
			if (SpellCode != SPELL_PSI_BLAST)
				HitMonster(Damage);
		}
	}
}
