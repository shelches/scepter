#include "common.h"
#include "scepter.h"

static FILE			*EPerson;
static char			*EPERSON = WORKDIR "/ram/eperson.new";

static void	OpenPer(void);
static void	ClosePer(void);
static int	SearchPlFile(char *Name);

static void OpenPer(void)
{
	if (!(EPerson = fopen(EPERSON, FOPEN)))
		Abort(" Sce203 - Error opening person file.");
}

static void ClosePer(void)
{
	if (fclose(EPerson))
		Abort(" Sce204 - Error closing person file.");
}

static int SearchPlFile(char *Name)
{
	Alfa	TempName;

	strcpy(TempName, Name);
	CapAlfa(TempName);

	OpenPer();
	while (fread((char *)&UserFRec, sizeof(UserFRec), 1, EPerson))
		if (!strcmp(TempName, UserFRec.User.Name))
			return TRUE;
	return FALSE;
}

int PlayerOnFile(char *Name)
{
	int		Found;

	Found = SearchPlFile(Name);
	ClosePer();
	return Found;
}

void ReadPlayer(UserPoint Player, char *Name)
{
	if (SearchPlFile(Name))
	{
		int		ILoop;

		memcpy(Player, &UserFRec.User, sizeof UserFRec.User);
		strcpy(Player->Name, Name);
		Player->Weight		= 0;
		Player->AC			= 10;
		Player->Next		= NULL;
		Player->USWeap		= NULL;
		Player->USShield	= NULL;
		Player->USArm		= NULL;
		Player->USRingL		= NULL;
		Player->USRingR		= NULL;
		Player->Follow		= NULL;
		Player->DefPlayer	= NULL;
		Player->DefMon		= NULL;
		Player->ObjectTail	= NULL;
		Player->LastInput	= RealTime;
		Player->LastAtk		= RealTime - 50;
		Player->LastHeal	= RealTime - 50;
		Player->HitAtTime	= RealTime - 50;
		Player->Drunk		= RealTime;
		Player->Entry		= XName;

		for (ILoop = 0; ILoop < Min(MaxObjs, UserFRec.NObj); ILoop++)
		{
			ObjectPoint	Pt;

			Pt = malloc(sizeof(ObjectType));
			*Pt = UserFRec.Objs[ILoop];
			Player->Weight += Pt->Weight;

			/* since chests don't work correctly... */
			if (Pt->ObClass == Chest)
			{
				Pt->Object.Chest.NumInside = 0;
				Pt->Object.Chest.ObjectTail = NULL;
			}

			Pt->Next = Player->ObjectTail;
			Player->ObjectTail = Pt;

			if (ILoop == UserFRec.NWeap)
				Player->USWeap = Pt;
			else
			if (ILoop == UserFRec.NArm)
			{
				Player->USArm = Pt;
				Player->AC =
					Max(-50, Min(50, Player->AC - Pt->Object.Armor.ArmPlus));
			}
			else
			if (ILoop == UserFRec.NShield)
			{
				Player->USShield = Pt;
				Player->AC =
					Max(-50, Min(50, Player->AC - Pt->Object.Shield.ShPlus));
			}
			else
			if (ILoop == UserFRec.NRingL)
				Player->USRingL = Pt;
			else
			if (ILoop == UserFRec.NRingR)
				Player->USRingR = Pt;
		}
	}
	else
	{
		*Player = ProtoUser;
		*Player->Name = '\0';
	}
	ClosePer();
}

void FreePlayer(UserPoint Player)
{
	while (Player->ObjectTail)
	{
		ObjectPoint	Pt;

		Pt = Player->ObjectTail;
		Player->ObjectTail = Player->ObjectTail->Next;
		ODestroy(Pt);
	}
	free(Player);
}

void WritePlayer(UserPoint Player)
{
	ObjectPoint			Pt;
	struct tUserFRec	EBuffer;
	int					I;

	memcpy(&EBuffer.User, Player, sizeof EBuffer.User);
	CapAlfa(EBuffer.User.Name);
	EBuffer.NWeap = EBuffer.NArm = EBuffer.NShield =
	EBuffer.NRingL = EBuffer.NRingR = -1;

	for (Pt = Player->ObjectTail, I = 0; Pt && I < MaxObjs; Pt = Pt->Next, I++)
	{
		EBuffer.Objs[I] = *Pt;
		if (Pt == Player->USWeap)
			EBuffer.NWeap = I;
		else
		if (Pt == Player->USArm)
			EBuffer.NArm = I;
		else
		if (Pt == Player->USShield)
			EBuffer.NShield = I;
		else
		if (Pt == Player->USRingL)
			EBuffer.NRingL = I;
		else
		if (Pt == Player->USRingR)
			EBuffer.NRingR = I;
	}
	EBuffer.NObj = I;

	if (SearchPlFile(Player->Name))
		fseek(EPerson, -(long)sizeof(UserFRec), 1);
	else
	{
		fseek(EPerson, 0L, 0);
		while (fread((char *)&UserFRec, sizeof(UserFRec), 1, EPerson))
			if (!*UserFRec.User.Name)
			{
				fseek(EPerson, -(long)sizeof(UserFRec), 1);
				break;
			}
	}

	if (!fwrite((char *)&EBuffer, sizeof(EBuffer), 1, EPerson))
		Abort(" Sce206 - Error updating person.");
	ClosePer();
}

void DeleteUsr(char *Nm)
{
	if (SearchPlFile(Nm))
	{
		*UserFRec.User.Name = '\0';
		fseek(EPerson, (long) -sizeof(UserFRec), 1);
		if (!fwrite((char *)&UserFRec, sizeof(UserFRec), 1, EPerson))
			Abort(" Sce208 - Error while deleting player.");
	}
	else
		QOut(Term, " Sce208 - Error while deleting player.");
	ClosePer();
}

void ChangeUsrName(char *OldNm, char *NewNm)
{
	if (SearchPlFile(OldNm))
	{
		CapAlfa(NewNm);
		strcpy(UserFRec.User.Name, NewNm);
		fseek(EPerson, (long) -sizeof(UserFRec), 1);
		if (!fwrite((char *)&UserFRec, sizeof(UserFRec), 1, EPerson))
			Abort(" Sce208 - Error while changing name.");
	}
	else
		QOut(Term, " Sce208 - Error while changing name.");
	ClosePer();
}

void Users(char *Word)
{
	UNUSED(Word);
	
	OpenPer();
	while (fread((char *)&UserFRec, sizeof(UserFRec), 1, EPerson))
		if (*UserFRec.User.Name)
		{
			if (User->SSJ)
				sprintf(B1, "0%c%c%-10s %4d %02d/%02d",
						UserFRec.User.SSJ ? '*' : ' ',
						UserFRec.User.It ? '+' : ' ', UserFRec.User.Name,
						UserFRec.User.Lvl,
						UserFRec.User.LastAccess >> 7,
						UserFRec.User.LastAccess & 0x3f);
			else
				sprintf(B1, "0%s", UserFRec.User.Name);
			QOut(Term, B1);
		}
	ClosePer();
}

UserPoint FindPlayer(char *Word, UserPoint PlayerTail)
{
	UserPoint	Usr;
	NameTList	UserNameList;
	int			Index, Count = 0;

	CapAlfa(Word);
	for (Usr = PlayerTail; Usr && Count < MaxNames; Usr = Usr->Next)
	{
		Count++;
		strcpy(UserNameList[Count], Usr->Name);
		CapAlfa(UserNameList[Count]);
	}
	if (Count >= MaxNames)
		QOut(Term, "0 Sce31 - Player table overflow!");

	if ((Index = WordMatch(Word, Count, UserNameList)) <= 0)
		return NULL;

	Usr = PlayerTail;
	for (Count = 1; Count < Index; Count++)
		Usr = Usr->Next;
	return Usr;
}

UserPoint FindUser(char *Word)
{
	UserPoint	Usr;
	NameTList	UserNameList;
	int			Index, Count = 0;

	CapAlfa(Word);
	for (Usr = UserTail; Usr && Count < MaxNames; Usr = Usr->NextUser)
	{
		Count++;
		strcpy(UserNameList[Count], Usr->Name);
		CapAlfa(UserNameList[Count]);
	}
	if (Count >= MaxNames)
		QOut(Term, "0 Sce31 - User table overflow!");

	if ((Index = WordMatch(Word, Count, UserNameList)) <= 0)
		return NULL;

	Usr = UserTail;
	for (Count = 1; Count < Index; Count++)
		Usr = Usr->NextUser;
	return Usr;
}

UserPoint FindPid(int FPid)
{
	UserPoint	Usr;

	for (Usr = UserTail; Usr; Usr = Usr->NextUser)
		if (Usr->XPid == FPid)
			return Usr;
	return NULL;
}

int Hash(char *Word)
{
	int		Total, ILoop;

	CapAlfa(Word);
	for (Total = ILoop = 0; Word[ILoop]; ILoop++)
		Total += Word[ILoop] * (ILoop + 1);
	return Total;
}

long Expr(int Lvl)
{
	if (Lvl <= 1)
		return 0L;
	if (Lvl > 24)
		return 99999999L;
	return 64L << Lvl;
}

void PrintExp(UserPoint Plyr)
{
	long	Exp;

	Exp = Expr(Plyr->Lvl + 1) - Expr(Plyr->Lvl) - Plyr->Experience;
	sprintf(B1, "0You need %ld more exp points to train for the next level.",
			Exp > 0 ? Exp : 0L);
	QOut(Term, B1);
	sprintf(B1, "0You have %ld shillings in cash.", Plyr->Money);
	QOut(Term, B1);
}

void Inventory(void)
{
	if (!User->ObjectTail)
		QOut(Term, "0You are carrying nothing at all.");
	else
	{
		ObjectPoint	Object;
		int			Count, ILoop;

		Count = 0;
		for (Object = User->ObjectTail; Object; Object = Object->Next)
			Count++;

		QOut(Term, "0You are carrying the following objects:");
		strcpy(B1, "0");
		ILoop = 0;
		for (Object = User->ObjectTail;
			Object && ILoop < Count;
			Object = Object->Next)
		{
			ILoop++;
			PrintObj(Object, FALSE, ILoop == 1, B2);
			strcat(B1, B2);
			Punctuate(ILoop, Count, B1);
		}
		strcat(B1, ".");
		QOut(Term, B1);
		QOut(Term, "0");
	}
}

void PrintDmg(UserPoint Player, int Damage, int Brf, char *Str)
{
	if (Brf)
	{
		if (!Player->Fatigue)
			sprintf(Str, "%d vit!", Damage);
		else
		if (Damage > Player->Fatigue)
			sprintf(Str, "%d fat, and %d vit!",
					Player->Fatigue, Damage - Player->Fatigue);
		else
			sprintf(Str, "%d fat!", Damage);
	}
	else
	{
		if (!Player->Fatigue)
			sprintf(Str, "%d vitality points!", Damage);
		else
		if (Damage > Player->Fatigue)
			sprintf(Str, "%d fatigue points, and %d vitality pts!",
					Player->Fatigue, Damage - Player->Fatigue);
		else
			sprintf(Str, "%d fatigue points!", Damage);
	}
}

int ReadyCheck(long LastAtk)
{
	if (RealTime + 1 < LastAtk)
	{
		sprintf(B1, "0Not ready!  Wait %ld more seconds.", LastAtk - RealTime);
		QOut(Term, B1);
	}
	else
	if (User->Drunk - RealTime >= 60)
		QOut(Term, "0You're too hammered to do anything!");
	else
		return TRUE;

	return FALSE;
}

void PromptUser(void)
{
	char	*p = NULL;

	switch ((int)User->Entry)
	{
		case XCmd:		QOut(Term, "1");							break;
		case XName:		p = "Enter your name";						break;
		case XEdit:		p = User->Brief ? "Ed" : "Edit command";	break;
		case XPassword:	p = "Enter your password";					break;
		case XNewPW:	p = "Enter access password";				break;
		case XChangePW:	p = "Enter new password";					break;
		case XNewClass:	p = "Class";								break;
		case XStats:	p = "Stats";								break;
		case XNotice:	p = "Enter notice";							break;
		case XSex:	p = "Finally, is your character a male or female"; break;
		case XDead:
			QOut(Term, "0Congratulations!  Your death has made the newspapers!");
			QOut(Term, "0In one line, try to explain what killed you and how.");
			QOut(Term, "0Use words like \"he\" and \"she\" instead of \"I\".");
			QOut(Term, "0Example:  \"His weapon broke and before he knew it, the");
			QOut(Term, "0vampire drained his life away...\"");
			p = "";
			break;
		case XNews:		p = "Enter news";							break;
		case XSell:
		case XRepair:	p = "Yes/No";								break;
		case XWish:		p = "Enter wish";							break;
		case XSpell:	p = "Say chant";							break;
		case XSure:		p = "Are you sure";							break;
		case XParley:	p = "Would you like to have it, Effendi";	break;
		case XSkill:	p = "Skill";								break;
		case XDesc:		p = "Enter description";					break;
	}
	if (p)
	{
		sprintf(B1, "1%s? ", p);
		QOut(Term, B1);
	}
}

void Init(void)
{
	int		ILoop;

	for (ILoop = 0; ILoop < 6; ILoop++)
		ProtoRoom.Adjoin[ILoop] = 0;
	ProtoRoom.Out			= 0;
	ProtoRoom.DescCode		= 0;
	ProtoRoom.DescRec		= 0;
	ProtoRoom.LastDir		= 0;
	ProtoRoom.WhichEncounter= 0;
	ProtoRoom.EncounterTime	= 0;
	ProtoRoom.NotifyDM		= FALSE;
	ProtoRoom.Safe			= FALSE;
	ProtoRoom.AntiMagic		= FALSE;
	ProtoRoom.Dark			= FALSE;
	ProtoRoom.RmPlayerTail	= NULL;
	ProtoRoom.RmMonsterTail	= NULL;
	ProtoRoom.RmObjectTail	= NULL;

	strcpy(ProtoObject.Name, "thing,*");
	ProtoObject.Next		= NULL;
	ProtoObject.Article		= A;
	ProtoObject.Carry		= TRUE;
	ProtoObject.Weight		= 1;
	ProtoObject.Price		= 0;
	ProtoObject.Magic		= FALSE;
	ProtoObject.Permanent	= FALSE;
	ProtoObject.Invisible	= FALSE;
	ProtoObject.DescCode	= 0;
	ProtoObject.DescRec		= 0;
	ProtoObject.ObClass		= Misc;

	strcpy(ProtoMonster.Name, "something,*");
	ProtoMonster.Next		= NULL;
	ProtoMonster.DefPlayer	= NULL;
	ProtoMonster.Lvl		= 0;
	ProtoMonster.Hits		= 10;
	ProtoMonster.MaxHits	= 10;
	ProtoMonster.ObjectTail	= NULL;
	ProtoMonster.Top		= TRUE;
	ProtoMonster.Num		= 1;
	ProtoMonster.Permanent	= TRUE;
	ProtoMonster.Magic		= FALSE;
	ProtoMonster.Defend		= TRUE;
	ProtoMonster.AtkLastAggr= TRUE;
	ProtoMonster.WhichObj	= 0;
	ProtoMonster.MReact		= 0;
	ProtoMonster.MParley	= 0;

	ProtoUser.Name[0]		= '\0';
	ProtoUser.Weight		= 0;
	ProtoUser.ObjectTail	= NULL;
	ProtoUser.Lvl			= 1;
	ProtoUser.Class			= Fighter;
	ProtoUser.Hits			= 10;
	ProtoUser.MaxHits		= 10;
	ProtoUser.Magic			= 10;
	ProtoUser.MaxMagic		= 10;
	ProtoUser.Experience	= 0;
	ProtoUser.AGuild		= FALSE;
	ProtoUser.AC			= 10;
	ProtoUser.RmCode		= 1;
	ProtoUser.Str			= 11;
	ProtoUser.Int			= 11;
	ProtoUser.Dex			= 11;
	ProtoUser.Pty			= 11;
	ProtoUser.Con			= 11;
	ProtoUser.Evil			= FALSE;
	ProtoUser.LastAccess	= Today;
	ProtoUser.PW			= Hash("DUMMY");
	ProtoUser.Money			= 200;
	ProtoUser.Invisible		= FALSE;
	ProtoUser.Fatigue		= 10;
	ProtoUser.NonExistant	= FALSE;
	ProtoUser.SkillNew		= FALSE;
	ProtoUser.Sex			= Male;
	ProtoUser.Dead			= FALSE;
	ProtoUser.MaxFatigue	= 10;
	ProtoUser.Poisoned		= FALSE;
	ProtoUser.PlayTester	= FALSE;
	ProtoUser.Echo			= FALSE;
	ProtoUser.SSharp		= 0;
	ProtoUser.SThrust		= 0;
	ProtoUser.SBlunt		= 0;
	ProtoUser.SLong			= 0;
	ProtoUser.Hidden		= FALSE;
	ProtoUser.SSJ			= FALSE;
	ProtoUser.Assoc			= 0;
	ProtoUser.Master		= FALSE;
	ProtoUser.TGuild		= FALSE;
	ProtoUser.LastInput		= 0;
	ProtoUser.Next			= NULL;
	ProtoUser.USWeap		= NULL;
	ProtoUser.USArm			= NULL;
	ProtoUser.USShield		= NULL;
	ProtoUser.USRingL		= NULL;
	ProtoUser.USRingR		= NULL;
	ProtoUser.DefPlayer		= NULL;
	ProtoUser.DefMon		= NULL;
	ProtoUser.Entry			= XInit;
	ProtoUser.Follow		= NULL;
	ProtoUser.NextUser		= NULL;
	ProtoUser.LastAtk		= 0;
	ProtoUser.LastHeal		= 0;
	ProtoUser.HitAtTime		= 0;
	ProtoUser.Drunk			= 0;
	ProtoUser.It			= FALSE;
	ProtoUser.Suck			= FALSE;
	ProtoUser.Plague		= FALSE;
	ProtoUser.Brief			= FALSE;
	ProtoUser.MesBlock		= FALSE;
	ProtoUser.Spells		= 0L;
	ProtoUser.LastCmd[0]	= '\0';
	ProtoUser.WData[0]		= '\0';
	ProtoUser.Data			= 0;
}
