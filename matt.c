#include "scepter.h"

void	MonAttack()
{
	UserPoint		Plyr;
	static TimeType	LastAtk, LastBuzz;
	char	*Str;

	Str = (char *)ctime(&RealTime) + 11;
	Str[8] = '\0';

#ifdef TIMECHECK
	if (!strcmp(NextEvent, "INITIALIZE") || strcmp(Str, NextEvent) >= 0)
		if (NUsers)
			TimeCheck();
#endif

	if (RealTime < LastAtk + 1)
		return;

	LastAtk = RealTime;

	for (Plyr = UserTail; Plyr; Plyr = Plyr->NextUser)
	{
		if (Plyr->Status != SLogin)
		{
			if (Plyr->It && RealTime > LastBuzz + Rnd(30) + 30)
			{
				LastBuzz = RealTime;

				switch (Rnd(3))
				{
					case 1:
						strcpy(B1,
							"0A cloud of gnats buzzes irritatingly around your head.");
						break;
					case 2:
						strcpy(B1, "0Gremlins pick your pocket!");
						Plyr->Money -= Rnd(5 * Plyr->Lvl);
						if (Plyr->Money < 0)
							Plyr->Money = 0;
						break;
					case 3:
						strcpy(B1,
							"0You hear muffled laughter behind your back.");
						break;
				}

				QOut(Plyr->Trm, B1);
			}

			if (Room[Plyr->RmCode].RmPlayerTail == Plyr)
				MonStrike(Plyr->RmCode);
		}
	}
}

void	MonStrike(Rm)
RmCodeType	Rm;
{
	MonsterPoint	Monster;
	UserPoint		Plyr;
	int				ILoop, Count;

	Count = 1;

	if (Room[Rm].RmMonsterTail && !Room[Rm].RmMonsterTail->Permanent)
		Count *= 2;

	if (Room[Rm].WhichEncounter && EncIndex(Room[Rm].WhichEncounter, 0)
		&&	Room[Rm].EncounterTime && Rnd((int)Room[Rm].EncounterTime * Count) == 1)
	{
		int		NumMons, MonNum, NonDMs;

		for (ILoop = 0; ILoop < 8; ILoop++)
			if (!EncIndex(Room[Rm].WhichEncounter, ILoop))
				break;

		NonDMs = 0;
		for (Plyr = Room[Rm].RmPlayerTail; Plyr; Plyr = Plyr->Next)
			if (!Plyr->Invisible)
				NonDMs++;

		MonNum = EncIndex(Room[Rm].WhichEncounter, Rnd(ILoop) - 1);
		NumMons = Rnd(Max(1, NonDMs - 1));
		if (Rnd(4) == 1)
			NumMons *= 2;

		for (ILoop = 0; ILoop < NumMons; ILoop++)
		{
			if (Monster = (MonsterPoint)malloc(sizeof(MonsterType)))
			{
				fseek(MList, (long)MonNum * sizeof(MonsterType), 0);
				fread(Monster, sizeof(MonsterType), 1, MList);

				if (Monster->Unique)
				{
					MonsterPoint	M;
	
					for (M = Room[Rm].RmMonsterTail; M; M = M->Next)
					{
						if (M->Unique && !strcmp(M->Name, Monster->Name))
						{
							NumMons = 0;
							free(Monster);
							break;
						}
					}

					if (NumMons == 0)
						break;
					NumMons = 1;
				}

				Monster->Hits = Min(Monster->MaxHits,
									Max(1, (12 - Rnd(3)) * Monster->Hits / 10));
				Monster->Lvl = Min(25, Max(1, Monster->Lvl + 2 - Rnd(3)));
				InsertMonster(Monster, Rm);
			}
			else	/* malloc failed */
			{
				NumMons = ILoop;
				break;
			}
		}

		if (NumMons)
			EncounterMsg(Monster, Rm, NumMons, NonDMs);
	}

	if (!Room[Rm].Safe)
	{
		UserPoint		TUser = User;

		for (Monster = Room[Rm].RmMonsterTail; Monster; Monster = Monster->Next)
		{
			int		AtkFlag = FALSE;

			if (!Monster->DefPlayer && (Monster->Top || Monster->Num < 9))
			{
				if (Monster->CharmPlayer && Monster->CharmPlayer->DefPlayer)
				{
					AtkFlag = TRUE;
					Monster->DefPlayer = Monster->CharmPlayer->DefPlayer;
				}
				else if (Monster->MoralReact || Monster->ImmoralReact)
				{
					Monster->DefPlayer = Room[Rm].RmPlayerTail;

					for (Plyr = Monster->DefPlayer; Plyr; Plyr = Plyr->Next)
						if (Rnd(3) < 3 && !Plyr->Invisible
								&&	Monster->CharmPlayer != Plyr
								&& (Monster->MoralReact
								&&	Plyr->Pty < Monster->DefPlayer->Pty
								||	Monster->ImmoralReact
								&&	Plyr->Pty > Monster->DefPlayer->Pty
								||	Plyr->It && Plyr->Lvl > 5))
							Monster->DefPlayer = Plyr;

					if ((Monster->MoralReact && Monster->DefPlayer->Pty > 7
							||	Monster->ImmoralReact && Monster->DefPlayer->Pty < 8
							||	Monster->DefPlayer->Invisible)
							&&	!Monster->DefPlayer->It)
						Monster->DefPlayer = NULL;
					else
						AtkFlag = TRUE;
				}

				if (Monster->Assistance && !Monster->DefPlayer)
				{
					Monster->DefPlayer = &ProtoUser;
					for (Plyr = Room[Rm].RmPlayerTail; Plyr; Plyr = Plyr->Next)
						if (Rnd(3) < 3 && !Plyr->Invisible
								&&	Plyr->DefPlayer && !Plyr->DefPlayer->DefPlayer
								&&	Plyr != Monster->CharmPlayer
								&&	Plyr->Lvl >= Monster->DefPlayer->Lvl)
							Monster->DefPlayer = Plyr;

					if (Monster->DefPlayer == &ProtoUser
							||	!Monster->DefPlayer->DefPlayer
							||	Monster->DefPlayer->DefPlayer->DefPlayer)
						Monster->DefPlayer = NULL;
					else
						AtkFlag = TRUE;
				}

				if (!Monster->DefPlayer
					&& (Monster->SlowReact && Rnd(MSpeed * 2) == 1
					||	Monster->FastReact))
				{
					int		NumTargets = 0;

					for (Plyr = Room[Rm].RmPlayerTail; Plyr; Plyr = Plyr->Next)
					{
						if (!Plyr->Invisible)
						{
							if (Plyr->It)
							{
								NumTargets = 0;
								AtkFlag = TRUE;
								if (Monster->CharmPlayer == Plyr)
									Monster->CharmPlayer = NULL;
								Monster->DefPlayer = Plyr;
								break;
							}
							else if (!Plyr->Evil && !Plyr->Hidden
									&&	Monster->CharmPlayer != Plyr)
								NumTargets++;
						}
					}

					if (NumTargets)
					{
						Plyr = Room[Rm].RmPlayerTail;
						Count = Rnd(NumTargets);

						do
						{
							while (Plyr
									&& (Plyr->Evil || Plyr->Invisible || Plyr->Hidden
									||	Monster->CharmPlayer == Plyr))
								Plyr = Plyr->Next;

						} while (--Count > 0 && (Plyr = Plyr->Next));

						if (Plyr)
						{
							AtkFlag = TRUE;
							Monster->DefPlayer = Plyr;
						}
					}
				}
			}

			User = Monster->DefPlayer;

			if (User)
				Term = User->Trm;

			Plyr = User;

			if (AtkFlag)
			{
				Monster->MReact = 0;
				AttackMsg(Monster, Plyr);
			}

			if (Monster->DefPlayer && Rnd(MSpeed - Monster->AtkSpeed) == 1
				&& (Monster->MReact < 1 || Monster->MReact > 3))
			{
				if (Monster->Regenerate)
					Monster->Hits = Min(Monster->MaxHits,
										Monster->Hits + Monster->Lvl / 2 + 1);

				Plyr = Monster->DefPlayer;
				Plyr->HitAtTime = RealTime;

				if (Rnd(20) != 20 && Monster->MReact != 6
					&&	Rnd(20) < 18 - Monster->Lvl + Plyr->Lvl
								- Plyr->AC + Plyr->Dex / 6
					||	Plyr->Hits == 0 || Monster->MReact == 4)
				{
					if (!Plyr->Brief)
					{
						PM(Monster, TRUE, B2);
						sprintf(B1, "0%s misses you!", B2);
						QOut(Plyr->Trm, B1);
					}
				}
				else
				{
					int		Damage, TempFat = 0;

					if (Monster->MonSpells && !Room[Rm].AntiMagic
						&&	Rnd(3) == 2)
					{
						int		NumSp;

						switch (Rnd(11))
						{
							case 1:
							case 2:
							case 3:
								NumSp = 3;	/* fireball */
								break;
							case 4:
							case 5:
							case 6:
								NumSp = 4;	/* lightning */
								break;
							case 7:
							case 8:
								NumSp = 8;	/* befuddle */
								break;
							case 9:
							case 10:
								NumSp = 16;	/* poison */
								break;
							case 11:
								NumSp = 18;	/* psi blast */
								break;
						}

						User = Plyr;
						Term = Plyr->Trm;
						PrintMon(Monster, TRUE, TRUE, B2);
						sprintf(B1, "0%s casts %s %s spell on %s!",
								B2, Pa(*SpellList[NumSp]), SpellList[NumSp], Plyr->Name);
						LocalMsg(B1);
						sprintf(B1, "0%s casts %s %s spell on you!",
								B2, Pa(*SpellList[NumSp]), SpellList[NumSp]);
						QOut(Term, B1);
						Damage = SpellMon(NumSp, Monster->Lvl, 15, Plyr,
											NULL, NULL, Plyr->Name);
						if (!Damage && (NumSp == 3 || NumSp == 4))
							QOut(Term, "0It missed!");
					}
					else
						Damage = Max(1, Monster->Lvl * 4 + 3 - Rnd(5));

					if (Damage)
					{
						if (Monster->MReact == 5)
							Damage = Damage / 2 + 1;
						else if (Monster->MReact == 6)
							Damage *= 2;

						switch (Rnd(100))
						{
							case 1:
							case 2:
							case 3:
							case 4:
								QOut(Term, "0Double damage!");
								Damage *= 2;
								break;

							case 12:
							case 13:
							case 14:
								QOut(Term, "0Vital wound!");
								TempFat = Plyr->Fatigue;
								Plyr->Fatigue = 0;
								break;

							case 18:
							case 19:
							case 20:
							case 21:
								Monster->MReact = 3;
								Damage = 0;
								PrintMon(Monster, TRUE, TRUE, B2);
								sprintf(B1, "0%s fumbles!", B2);
								QOut(Term, B1);
								Monster->DefPlayer = NULL;
								break;
						}

						HitShArmor(Plyr, &Damage);
						PM(Monster, TRUE, B2);
						PrintDmg(Plyr, Damage, Plyr->Brief, B3);
						sprintf(B1, "0%s hits you for %s", B2, B3);
						QOut(Term, B1);

						if (Monster->Drain && Rnd(25) > Plyr->Con)
						{
							if (Plyr->Lvl <= 1)
							{
								Plyr->Dead = TRUE;
								Plyr->Hits = Plyr->Fatigue = Plyr->Con = 0;
							}
							else
							{
								QOut(Term, "0You feel weak and drained...");

								if (Rnd(4) == 4)
								{
									Plyr->MaxHits -= Plyr->MaxHits / Plyr->Lvl;
									Plyr->MaxFatigue -= Plyr->MaxFatigue / Plyr->Lvl;
									Plyr->MaxMagic -= Plyr->MaxMagic / Plyr->Lvl;
									--Plyr->Lvl;
									ILoop = 2;
								}
								else
									ILoop = 1;

								while (ILoop-- > 0)
								{
									switch (Rnd(4))
									{
										case 1:
											Plyr->Str = Max(3, Plyr->Str - 1);
											break;
										case 2:
											Plyr->Int = Max(3, Plyr->Int - 1);
											break;
										case 3:
											Plyr->Dex = Max(3, Plyr->Dex - 1);
											break;
										case 4:
											Plyr->Con = Max(3, Plyr->Con - 1);
											break;
									}
								}
							}
						}
					}

					if (Plyr->Hits + Plyr->Fatigue <= Damage)
					{
						Plyr->Dead = TRUE;
						Plyr->Hits = Plyr->Fatigue = 0;
						User = Plyr;
						Term = Plyr->Trm;
						PrintMon(Monster, FALSE, FALSE, B2);

						sprintf(B1, "0### %s the %s was just %s by %s.",
							Plyr->Name, CName[(int)Plyr->Class],
							Rnd(2) == 1 ? "slain" : "killed", B2);

						if (Plyr->Lvl > 4)
							sprintf(B3, "0### We shall all grieve for %s.",
									Pro[(int)Plyr->Sex]);

						Count = MsgTerm(All);

						for (ILoop = 0; ILoop < Count; ILoop++)
						{
							QOut(TermList[ILoop], B1);

							if (Plyr->Lvl > 4)
								QOut(TermList[ILoop], B3);
						}
					}
					else
					{
						if (Monster->Poison && Damage > 0 && Rnd(3) == 3
							&&	Rnd(Monster->Lvl * 2) > Plyr->Lvl)
						{
							QOut(Term, "0You've been poisoned!");
							Plyr->Poisoned = TRUE;
						}

						if (Damage > Plyr->Fatigue)
							Plyr->Hits = Max(0,
								Plyr->Hits - Damage + Plyr->Fatigue);

						Plyr->Fatigue = Max(0, Plyr->Fatigue - Damage);

						if (TempFat > 0)
							Plyr->Fatigue = TempFat;
					}

					Monster->MReact = 0;
				}

				if (Monster->MReact == 4)
					Monster->MReact = 0;
			}
			else if (Monster->MReact >= 1 && Monster->MReact <= 3)
				--Monster->MReact;
		}

		User = TUser;
	}
}

void	EncounterMsg(Mon, Rm, NumMons, NonDMs)
MonsterPoint	Mon;
RmCodeType		Rm;
{
	if (!Mon->Invisible && !Room[Rm].Dark)
	{
		UserPoint		Plyr;

		strcpy(B1, NonDMs > 1 ? "0Your party encounters " : "0You encounter ");
		if (NumMons == 1)
		{
			PrintMon(Mon, FALSE, FALSE, B2);
			strcat(B1, B2);
			strcat(B1, "!");
		}
		else
		{
			Pn(NumMons, B3);
			strcat(B1, B3);
			ExpandName(Mon->Name, B2);
			strcat(B1, B2);
			strcat(B1, "s!");
		}

		Plyr = Room[Rm].RmPlayerTail;
		while (Plyr)
		{
			QOut(Plyr->Trm, B1);
			Plyr = Plyr->Next;
		}
	}
}

void	AttackMsg(Monster, Plyr)
MonsterPoint	Monster;
UserPoint		Plyr;
{
	UserPoint	TUser;

	TUser = User;
	User = Plyr;
	PM(Monster, TRUE, B2);
	sprintf(B1, "0%s attacks %s!", B2, User->Name);
	LocalMsg(B1);
	PM(Monster, TRUE, B2);
	sprintf(B1, "0%s attacks you!", B2);
	QOut(User->Trm, B1);
	User = TUser;
}

void	InitEvents()
{
	Event[0]  = "00:01:00";
	Event[1]  = "00:05:00";
	Event[2]  = "00:10:00";
	Event[3]  = "04:30:00";
	Event[4]  = "04:45:00";
	Event[5]  = "04:55:00";
	Event[6]  = "06:20:59";
	Event[7]  = "06:21:00";
	Event[8]  = "07:10:00";
	Event[9]  = "12:00:00";
	Event[10] = "14:30:00";
	Event[11] = "14:45:00";
	Event[12] = "14:55:00";
	Event[13] = "14:59:59";
	Event[14] = "15:00:00";
	Event[15] = "15:04:00";
	Event[16] = "19:00:00";
	Event[17] = "20:59:00";
	Event[18] = "22:40:00";
	Event[19] = "23:55:00";
	NextEvent = "INITIALIZE";
	LenEvent = 20;
}

void	TimeCheck()
{
	int		EventNum, Found;

	User = UserTail;
	Term = User->Trm;	/* select dummy term so MsgTerm doesn't blow up */

	if (!strcmp(NextEvent, "INITIALIZE"))
	{
		char	*Str;
		int		ILoop;

		Str = (char *)ctime(&RealTime) + 11;
		Str[8] = '\0';

		for (ILoop = LenEvent - 1; ILoop >= 0; --ILoop)
			if (strcmp(Str, Event[ILoop]) <= 0)
				NextEvent = Event[ILoop];
		if (!strcmp(NextEvent, "INITIALIZE"))
			NextEvent = Event[1];
		return;
	}

	Found = FALSE;
	for (EventNum = LenEvent - 1; !Found && EventNum >= 0; --EventNum)
		Found = (strcmp(NextEvent, Event[EventNum]) >= 0);
	if (!Found)
		Abort(" Sce943 - Timed event not found!");

	switch (EventNum)
	{
		case 0:		/* 12:01 AM */
			NextEvent = Event[2];
			break;

		case 1:		/* 12:05 AM */
			break;

		case 2:		/* 12:10 AM */
			SystemMsg("0You hear an owl hooting in the distance.");
			break;

		case 3:		/* 4:30	AM */
			SystemMsg("0>>> Program will abort at 5:00 AM.");
			break;

		case 4:		/* 4:45	AM */
			SystemMsg("0>>> Program aborting in 15 minutes.");
			break;

		case 5:		/* 4:55	AM */
			SystemMsg("0>>> Program aborting in 5 minutes.");
			break;

		case 6:		/* 4:59	AM */
			SystemMsg("0Wait until program aborts.  A hangup will cost you a con point.");
			LastUpdate = RealTime - UpdatePeriod - 10;	/* force update */
			break;

		case 7:		/* 4:59	AM */
			SystemMsg("0*** Warning ***  Program aborting.");
			Off();
			Abort(" Mil001 - Program took itself down.");
			break;

		case 8:		/* 7:10 AM */
			SystemMsg("0Good morning and welcome to Scepter!");
			break;

		case 9:		/* 12:00 PM */
			SystemMsg("0You hear the town clock strike the noon hour.");
			break;

		case 10:	/* 2:30 PM */
			break;

		case 11:	/* 2:45 PM */
			break;

		case 12:	/* 2:55 PM */
			break;

		case 13:	/* 2:59 PM */
			break;

		case 14:	/* 2:59 PM */
			break;

		case 15:	/* 3:04 PM */
			break;

		case 16:	/* 7:00	PM */
			SystemMsg("0The sun sets in the west.");
			break;

		case 17:	/* 8:59 PM */
			SystemMsg("0The town watch sets out on its patrol.");
			break;

		case 18:	/* 10:40 PM */
			SystemMsg("0You hear crickets chirping in the grass.");
			break;

		case 19:	/* 11:55 PM */
			SystemMsg("0The town clock strikes the midnight hour.");
			break;
	}

	if (EventNum > 2)
		NextEvent = Event[(EventNum + 1) % LenEvent];
}

void	SystemMsg(s)
char	*s;
{
	int		Count, ILoop;

	Count = MsgTerm(SysMsg);
	for (ILoop = 0; ILoop < Count; ILoop++)
		QOut(TermList[ILoop], s);
}
