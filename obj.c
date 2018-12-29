#include "scepter.h"

#ifdef	PROTO
static void	AddSpell(int N, char *Nm, int Hash, int Len, int MP, int Lvl, int Int, int Ty);
#endif

static void	AddSpell(N, Nm, Hash, Len, MP, Lvl, Int, Ty)
int		N;
char	*Nm;
int		Hash, Len, MP, Lvl, Int, Ty;
{
	strcpy(SpellList[N], Nm);
	SpellClass[N].SplHash	= Hash;
	SpellClass[N].SplLen	= Len;
	SpellClass[N].SplMp		= MP;
	SpellClass[N].SplLvl	= Lvl;
	SpellClass[N].SplInt	= Int;
	SpellClass[N].SplType	= Ty;

	Spellen = Max(N, Spellen);
}

void	InitSpells()
{
	Spellen = 0;

	AddSpell(SPELL_NULL,       "NULL",  		  0,  0,  0,  0,  0, OnUser);
	AddSpell(SPELL_VIGOR,      "VIGOR",		131, 14,  3,  0, 10, OnUser | OnPlayer);
	AddSpell(SPELL_HEAL,       "HEAL",		190, 18,  6,  1, 10, OnUser | OnPlayer);
	AddSpell(SPELL_FIREBALL,   "FIREBALL",	135, 15, 10,  2, 11, OnMon | OnPlayer);
	AddSpell(SPELL_LIGHTNING,  "LIGHTNING",	340, 32, 15,  4, 13, OnMon | OnPlayer);
	AddSpell(SPELL_HURT,       "HURT",		 40,  3,  1, -3,  8, OnMon | OnPlayer);
	AddSpell(SPELL_CUREPOISON, "CUREPOISON",	317, 25,  6,  1,  9, OnUser | OnPlayer);
	AddSpell(SPELL_DISINTEGRA, "DISINTEGRA",	547, 50, 20,  5, 14, OnMon | OnPlayer);
	AddSpell(SPELL_BEFUDDLE,   "BEFUDDLE",	341, 27,  5,  1, 11, OnMon | OnPlayer);
	AddSpell(SPELL_TELEPORT,   "TELEPORT",	534, 46, 40,  8, 16, OnUser | OnPlayer);
	AddSpell(SPELL_WISH,       "WISH",		437, 43, 50, 10, 17, OnUser | OnPlayer);
	AddSpell(SPELL_PASSDOOR,   "PASSDOOR",	199, 17, 20,  5, 13, OnObject);
	AddSpell(SPELL_ENCHANT,    "ENCHANT",		 79, 12, 20,  5, 13, OnObject);
	AddSpell(SPELL_BLESS,      "BLESS",		296, 30, 16,  4, 11, OnPlayer);
	AddSpell(SPELL_PROTECTION, "PROTECTION",	340, 30, 10,  2, 10, OnUser);
	AddSpell(SPELL_CURSE,      "CURSE",		258, 26, 10,  5, 10, OnUser | OnPlayer);
	AddSpell(SPELL_POISON,     "POISON",		278, 27, 10,  4, 10, OnUser | OnPlayer);
	AddSpell(SPELL_INTOXICATE, "INTOXICATE",	268, 25,  8,  3,  9, OnUser | OnPlayer);
	AddSpell(SPELL_PSI_BLAST,  "PSI BLAST",	 66,  5, 20,  5, 14, OnMon | OnPlayer);
	AddSpell(SPELL_POLYMORPH,  "POLYMORPH",	 10,  5, 50, 10, 18, OnMon);
	AddSpell(SPELL_SUMMON,     "SUMMON",		 32,  4, 50, 10, 18, OnUser);
	AddSpell(SPELL_ANIMATE,    "ANIMATE",		 44,  5, 50, 10, 18, OnObject);
	AddSpell(SPELL_CHARM,      "CHARM",		 26,  4, 50, 10, 18, OnMon | OnPlayer);
	AddSpell(SPELL_ILLUMINATE, "ILLUMINATE",	 52,  5, 50, 10, 18, OnUser);
}

void	ODestroy(Cont)		/* var!! */
ObjectPoint	Cont;
{
	if (Cont->ObClass == Chest)
	{
		struct tChest	*Ch;

		Ch = &Cont->Object.Chest;
		while (Ch->ObjectTail)
		{
			ObjectPoint	Obj;

			Obj = Ch->ObjectTail;
			Ch->ObjectTail = Ch->ObjectTail->Next;
			ODestroy(Obj);
		}
	}
	free(Cont);
	Cont = NULL;
}

void	DeleteObject(Pt, ObjectTail)
ObjectPoint	Pt, *ObjectTail;
{
	if (!Pt)
		Abort(" Sce84 - Cannot delete NIL obj!");
	if (Pt == *ObjectTail)
		*ObjectTail = (*ObjectTail)->Next;
	else
	{
		ObjectPoint	Pt2;

		for (Pt2 = *ObjectTail; Pt2; Pt2 = Pt2->Next)
			if (Pt2->Next == Pt)
			{
				Pt2->Next = Pt->Next;	/* delete */
				return;
			}

		Abort(" Sce05 - Deleted obj not found");
	}
}

void	ExpandName(Name, Str)
char	*Name, *Str;
{
	int		ILoop;

	for (ILoop = 0; Name[ILoop]; ILoop++)
		if (Name[ILoop] == ',')
		{
			int		JLoop;
			char	*p;

			p = Str;
			for (JLoop = ILoop + 1; Name[JLoop]; JLoop++)
				if (Name[JLoop] == '*')
				{
					int		KLoop;

					for (KLoop = 0; KLoop < ILoop; KLoop++)
						*p++ = Name[KLoop];
				}
				else
					*p++ = Name[JLoop];
			*p = '\0';
			return;
		}

	strcpy(Str, Name);	/* Name has no commas */
}

void	PrintObj(Obj, Singular, Capital, Str)
ObjectPoint	Obj;
int			Singular, Capital;
char		*Str;
{
	char	Ar[20], Str1[256];

	if (Singular)
	{
		if (Obj->Article != None)
			strcpy(Ar, "the");
		else
			*Ar = '\0';
	}
	else if (Obj->ObClass != Door)
	{
		switch ((int)Obj->Article)
		{
			case A:		strcpy(Ar, "a");	break;
			case An:
				strcpy(Ar, Obj->Magic
						|| Obj->ObClass == Weap && !Obj->Object.Weap.Strikes ?
							"a" : "an");
				break;
			case The:	strcpy(Ar, "the");	break;
			case Some:	strcpy(Ar, "some");	break;
			default:	*Ar = '\0';			break;
		}
	}
	else
	{
		switch ((int)Obj->Article)
		{
			case A:
			case An:	strcpy(Ar, Obj->Object.Door.DClosed > 0 ? "a" : "an");
						break;
			case The:	strcpy(Ar, "the");	break;
			case Some:	strcpy(Ar, "some");	break;
			default:	*Ar = '\0';			break;
		}
		strcat(Ar, !Obj->Object.Door.DClosed ? " open"
				  : Obj->Object.Door.DLocked > 0 ? " locked" : " closed");
	}

	if (*Ar)
	{
		if (Capital)
		{
			*Ar = Cap(*Ar);
			Capital = FALSE;
		}
		sprintf(Str, "%s ", Ar);
	}
	else
		*Str = '\0';

	if (Obj->ObClass == Coins)
	{
		sprintf(Str1, "%d ", Obj->Price / Obj->Object.Coins.Multiplier);
		strcat(Str, Str1);
	}

	if (Obj->ObClass == Weap && !Obj->Object.Weap.Strikes)
	{
		strcat(Str, Capital ? "Broken " : "broken ");
		Capital = FALSE;
	}

	if (Obj->Magic)
		strcat(Str, Capital ? "Magic " : "magic ");

	ExpandName(Obj->Name, Str1);
	strcat(Str, Str1);

	if (!Singular)
	{
		if (Obj->Invisible)
			strcat(Str, " (INV)");
		if (Obj->Magic)
		{
			switch ((int)Obj->ObClass)
			{
				case Weap:
					sprintf(Str1, " (+%d)", abs(Obj->Object.Weap.WeaPlus));
					break;
				case Shield:
					sprintf(Str1, " (+%d)", abs(Obj->Object.Shield.ShPlus));
					break;
				case Armor:
					sprintf(Str1, " (+%d)", abs(Obj->Object.Armor.ArmPlus));
					break;
				default:
					return;
			}
			strcat(Str, Str1);
		}
	}
}

void	ObjDisplay(Object)
ObjectPoint	Object;
{
	PrintObj(Object, FALSE, TRUE, B2);
	sprintf(B1, "0%s, ", B2);
	if (Object->Carry)
		strcat(B1, "carryable, ");
	sprintf(B2, "%d lbs, %d shillings.", Object->Weight, Object->Price);
	strcat(B1, B2);
	QOut(Term, B1);

	if (Object->DescRec && Object->ObClass != Scroll)
	{
		sprintf(B1, "0DR=%d, DI=%d.", Object->DescRec, Object->DescCode);
		QOut(Term, B1);
		strcpy(B2, "0Descrp:  ");
		PrintDesc(Object->DescRec, Object->DescCode, 0, FALSE, B2, B1);
		QOut(Term, B1);
	}

	sprintf(B1, "0Magic=%s, Permanent=%s, Invisible=%s.",
			Pb(Object->Magic), Pb(Object->Permanent), Pb(Object->Invisible));
	QOut(Term, B1);

	switch ((int)Object->ObClass)
	{
		case Portal:
			sprintf(B1, "0Portal to room %d.", Object->Object.Portal.ToWhere);
			QOut(Term, B1);
			break;

		case Weap:
		{
			char	*p;
			struct tWeap	*Wp;

			Wp = &Object->Object.Weap;
			sprintf(B1, "0Offensive weapon:  %d-%d HP, %d strikes left.",
					Wp->MinHp, Wp->MaxHp, Wp->Strikes);
			QOut(Term, B1);

			switch ((int)Wp->WeapType)
			{
				case Sharp:		p = "Sharp";	break;
				case Thrust:	p = "Thrust";	break;
				case Blunt:		p = "Blunt";	break;
				case Long:		p = "Long";		break;
			}
			sprintf(B1, "0%s class weapon, + %d to hit.", p, Wp->WeaPlus);
			QOut(Term, B1);
			break;
		}
		case Shield:
			sprintf(B1, "0Shield: + %d protection, %d hits left.",
					Object->Object.Shield.ShPlus, Object->Object.Shield.ShHits);
			QOut(Term, B1);
			break;

		case Armor:
			sprintf(B1, "0Armor: + %d protection, %d hits left.",
					Object->Object.Armor.ArmPlus, Object->Object.Armor.ArmHits);
			QOut(Term, B1);
			break;

		case Coins:
			sprintf(B1, "0Money, value multiplier:  X%d.",
					Object->Object.Coins.Multiplier);
			QOut(Term, B1);
			break;

		case Scroll:
			sprintf(B1, "0Scroll, spell = %s.",
					SpellList[Object->Object.Scroll.Spell]);
			QOut(Term, B1);
			break;

		case Chest:
		{
			ObjectPoint		TObj;
			struct tChest	*Ch;

			Ch = &Object->Object.Chest;
			sprintf(B1, "0Container, objects=%d, trap=%d, lock type=%d.",
					Ch->NumInside, Ch->Trap, Ch->Locked);
			QOut(Term, B1);

			if (TObj = Ch->ObjectTail)
			{
				QOut(Term, "0Items inside:");
				while (TObj)
				{
					PrintObj(TObj, FALSE, FALSE, B2);
					sprintf(B1, "0  %s", B2);
					QOut(Term, B1);
					TObj = TObj->Next;
				}
			}
			break;
		}
		case Door:
		{
			struct tDoor	*Dr;

			Dr = &Object->Object.Door;
			sprintf(B1, "0Door, portal to room %d.  Trap=%d, lock type=%d.",
					Dr->DToWhere, Dr->DTrap, Dr->DLocked);
			QOut(Term, B1);
			if (Dr->DToll)
			{
				sprintf(B1, "0A %d-shilling toll will be charged for passage.",
						Object->Price);
				QOut(Term, B1);
			}
			if (Dr->DSpring)
				QOut(Term, "0Door will close automatically.");
			break;
		}
		case MagDevice:
			PNth(Object->Object.MagDevice.MLevel, B2);
			sprintf(B1, "0%s level magical device, spell = %s.  Number of charges = %d.",
					B2, SpellList[Object->Object.MagDevice.MSpell],
					Object->Object.MagDevice.NumCharges);
			QOut(Term, B1);
			break;

		case Teleport:
			sprintf(B1, "0Teleport device, from room %d to room %d.",
					Object->Object.Teleport.TActiveRm,
					Object->Object.Teleport.TToWhere);
			QOut(Term, B1);
			break;

		case Keys:
			sprintf(B1, "0Keys, unlock type=%d.", Object->Object.Keys.UnLock);
			if (Object->Object.Keys.NumUses > 0)
			{
				sprintf(B2, "  (%d uses remaining)",
						Object->Object.Keys.NumUses);
				strcat(B1, B2);
			}
			QOut(Term, B1);
			break;

		case Treasure:
			QOut(Term, "0Treasure.");
			break;

		case Card:
			QOut(Term, "0Cards, teleport to player.");
			break;

		case Ring:
			PNth(Object->Object.Ring.RLevel, B2);
			sprintf(B1, "0%s level ring, spell = %s.",
					B2, SpellList[Object->Object.Ring.RSpell]);
			QOut(Term, B1);
			break;

		case Misc:
			QOut(Term, "0Miscellaneous object.");
			break;
	}
}

void	Ld(Name, Entry)
char	*Name, *Entry;
{
	int		ILoop;

	for (ILoop = 0;
		ILoop < sizeof(Alfa) - 1 && Name[ILoop] && Name[ILoop] != ',';
		ILoop++)
		Entry[ILoop] = Name[ILoop];

	Entry[ILoop] = '\0';
}

/* FindObject returns the matching object to *Word* in the list */
ObjectPoint	FindObject(Word, Num, ObjectTail)
char		*Word;
int			Num;
ObjectPoint	ObjectTail;
{
	ObjectPoint	Object;
	NameTList	ObjNameList;
	int			I, Count, Index;
	Alfa		Name;

	CapAlfa(Word);
	Count = 0;
	for (Object = ObjectTail; Object && Count < MaxNames; Object = Object->Next)
	{
		Count++;
		*ObjNameList[Count] = '\0';
		Ld(Object->Name, ObjNameList[Count]);
		CapAlfa(ObjNameList[Count]);
	}
	if (Count >= MaxNames)
		QOut(Term, "0 Sce30 - Object table overflow");

	if ((Index = WordMatch(Word, Count, ObjNameList)) <= 0)
		return NULL;

	Object = ObjectTail;
	for (I = 1; I < Index; I++)
		Object = Object->Next;

	if (Num > 1)
	{
		int		NumMatch = 1;

		strcpy(Name, ObjNameList[Index]);
		for (I = Index + 1; NumMatch < Num && I <= Count; I++)
		{
			Object = Object->Next;
			if (!strcmp(ObjNameList[I], Name))
				NumMatch++;
		}
		if (I > Count && NumMatch < Num)
			Object = NULL;
	}

	return Object;
}
