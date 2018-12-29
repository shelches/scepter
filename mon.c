#include "scepter.h"

void	PM(Mon, C, Str)
MonsterPoint	Mon;
int				C;
char			*Str;
{
	char	Str1[40];

	ExpandName(Mon->Name, Str1);
	if (!Mon->Top || Mon->Num > 1)
		sprintf(Str, "%s #%d", Str1, Mon->Num);
	else
	if (Mon->Article != None)
		sprintf(Str, "the %s", Str1);
	else
		strcpy(Str, Str1);

	if (C)
		*Str = Cap(*Str);
}

void	PrintMon(Mon, S, C, Str)
MonsterPoint	Mon;
int				S, C;
char			*Str;
{
	char	Str1[256];

	if (Mon->Article == None)
		*Str = '\0';
	else
	{
		static char	*art[4] = {"a ", "an ", "some ", "the "};

		strcpy(Str, art[S ? 3 : (int)Mon->Article]);	/* singular == "the" */
		if (C)
			*Str = Cap(*Str);
	}

	ExpandName(Mon->Name, Str1);
	strcat(Str, Str1);

	if (!S && Mon->Invisible)
		strcat(Str, " (INV)");
}

void	Destroy(Mon)
MonsterPoint	Mon;
{
	while (Mon->ObjectTail)
	{
		ObjectPoint	Obj;
		
		Obj = Mon->ObjectTail;
		Mon->ObjectTail = Mon->ObjectTail->Next;
		ODestroy(Obj);
	}
	free(Mon);
}

void	FixMonCount(MonsterTail, Name)
MonsterPoint	MonsterTail;
char			*Name;
{
	MonsterPoint	Pt;
	int				NumMon;

	NumMon = 0;
	for (Pt = MonsterTail; Pt; Pt = Pt->Next)
		if (!strcmp(Pt->Name, Name))
			NumMon++;

	if (NumMon)
	{
		int		I;

		Pt = MonsterTail;
		for (I = NumMon; I; --I)
		{
			while (strcmp(Pt->Name, Name))
				Pt = Pt->Next;
			Pt->Num = Min(9, I);
			Pt->Top = (I == NumMon);
			Pt = Pt->Next;
		}
	}
}

void	MonDisplay(Mon)
MonsterPoint	Mon;
{
	ObjectPoint	Object;

	PM(Mon, TRUE, B2);
	PNth(Mon->Lvl, B3);
	sprintf(B1, "0%s is a %s level monster with %d/%d H.P.",
			B2, B3, Mon->Hits, Mon->MaxHits);
	QOut(Term, B1);

	sprintf(B1, "0Defend=%s, Block=%s, Follow=%s, Watch=%s,",
			Pb(Mon->Defend), Pb(Mon->Block), Pb(Mon->Follow), Pb(Mon->Watch));
	QOut(Term, B1);

	sprintf(B1, "0Guard=%s, AtkLastAggr=%s, DR=%d, DI=%d.",
			Pb(Mon->Guard), Pb(Mon->AtkLastAggr), Mon->DescRec, Mon->DescCode);
	QOut(Term, B1);

	sprintf(B1, "0%8d E.P., Permanent=%s, AtkRate=%d, AC=%d.",
			MonExperience(Mon), Pb(Mon->Permanent), Mon->AtkSpeed, Mon->AC);
	QOut(Term, B1);

	if (Mon->DescRec && Mon->DescCode)
	{
		strcpy(B2, "0Descrp:  ");
		PrintDesc(Mon->DescRec, Mon->DescCode, 0, FALSE, B2, B1);
		QOut(Term, B1);
	}

	if (Mon->MoralReact)
		QOut(Term, "0Monster will hit players with piety < 8.");
	if (Mon->ImmoralReact)
		QOut(Term, "0Monster will hit players with piety > 7.");
	if (Mon->Invisible)
		QOut(Term, "0Monster is invisible.");
	if (Mon->Flee)
		QOut(Term, "0Monster may flee.");
	if (Mon->SummonHelp)
		QOut(Term, "0Monster may summon help if attacked.");
	if (Mon->Assistance)
		QOut(Term, "0Monster will assist attacked players.");
	if (Mon->Magic)
		QOut(Term, "0Monster is affected only by magic weapons.");
	if (Mon->Poison)
		QOut(Term, "0Monster is poisonous.");
	if (Mon->AntiMagic)
		QOut(Term, "0Monster is immune to spells.");
	if (Mon->Undead)
		QOut(Term, "0Monster is undead.");
	if (Mon->SlowReact)
		QOut(Term, "0Monster will react slowly.");
	if (Mon->FastReact)
		QOut(Term, "0Monster will react quickly.");
	if (Mon->Regenerate)
		QOut(Term, "0Monster can regenerate hit points.");
	if (Mon->Drain)
		QOut(Term, "0Monster has energy-drain.");
	if (Mon->MonSpells)
		QOut(Term, "0Monster can cast spells.");
	if (Mon->Unique)
		QOut(Term, "0Monster is unique.");
	if (Mon->Nice)
		QOut(Term, "0Anyone who'd kill such a nice monster must suck.");
	if (Mon->MParley)
	{
		sprintf(B1, "0Monster will do type %d parley.", Mon->MParley);
		QOut(Term, B1);
	}
	if (Mon->CharmPlayer)
		QOut(Term, "0Monster is charmed.");
	sprintf(B1, "0Treasure type:  %d.", Mon->WhichObj);
	QOut(Term, B1);

	if (Object = Mon->ObjectTail)
	{
		QOut(Term, "0Objects carried:");
		do
		{
			PrintObj(Object, FALSE, TRUE, B2);
			sprintf(B1, "0%s", B2);
			QOut(Term, B1);
		} while (Object = Object->Next);
	}
}

/* returns the matching	monster	to *Word* in the list */
MonsterPoint FindMonster(Word, Num,	MonsterTail)
char			*Word;
int				Num;
MonsterPoint	MonsterTail;
{
	int				Count, Index;
	MonsterPoint	Mon;
	NameTList		MonNameList;

	CapAlfa(Word);
	Count = 0;
	for (Mon = MonsterTail; Mon && Count < MaxNames; Mon = Mon->Next)
	{
		Count++;
		Ld(Mon->Name, MonNameList[Count]);
		CapAlfa(MonNameList[Count]);
	}

	if (Count >= MaxNames)
		QOut(Term, "0 Sce30 - Monster table overflow");

	if ((Index = WordMatch(Word, Count, MonNameList)) <= 0)
		return NULL;

	for (Count = 1, Mon = MonsterTail; Count < Index; Count++, Mon = Mon->Next)
		;
	if (Num > 0)
	{
		char	*MonName;

		for (MonName = Mon->Name; Mon; Mon = Mon->Next)
			if (!strcmp(Mon->Name, MonName) && Mon->Num == Num)
				break;
	}
	return Mon;
}
