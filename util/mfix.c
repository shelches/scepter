#include "../scepter.h"
#include "oldmon.h"
#include "newmon.h"

OldMonsterType	OldMon;
NewMonsterType	NewMon;
FILE    *ifp, *ofp, *fopen();

void main(void)
{
	ifp = fopen("3:/etc/scepter/mlist", "r");
	ofp = fopen("3:/etc/scepter/newmlist", "w");
	while (fread((char *)&OldMon, sizeof OldMon, 1, ifp))
	{
		strcpy(NewMon.Name, OldMon.Name);
		strcat(NewMon.Name, ",*");
		NewMon.DescRec = OldMon.DescRec;
		NewMon.DescCode = OldMon.DescCode;
		NewMon.AtkSpeed = OldMon.AtkSpeed;
		NewMon.AC = OldMon.AC;
		NewMon.Hits = OldMon.Hits;
		NewMon.MaxHits = OldMon.MaxHits;
		NewMon.Lvl = OldMon.Lvl;
		NewMon.Num = OldMon.Num;
		NewMon.MReact = OldMon.MReact;
		NewMon.MParley = OldMon.MParley;
		NewMon.WhichObj = OldMon.WhichObj;

		NewMon.Defend = OldMon.Defend;
		NewMon.Block = OldMon.Block;
		NewMon.Follow = OldMon.Follow;
		NewMon.Guard = OldMon.Guard;
		NewMon.AtkLastAggr = OldMon.AtkLastAggr;
		NewMon.SlowReact = OldMon.SlowReact;
		NewMon.FastReact = OldMon.FastReact;
		NewMon.Invisible = OldMon.Invisible;
		NewMon.Regenerate = OldMon.Regenerate;
		NewMon.Drain = OldMon.Drain;
		NewMon.Poison = OldMon.Poison;
		NewMon.AntiMagic = OldMon.AntiMagic;
		NewMon.Undead = OldMon.Undead;
		NewMon.MoralReact = OldMon.MoralReact;
		NewMon.Flee = OldMon.Flee;
		NewMon.Assistance = OldMon.Assistance;
		NewMon.MonSpells = OldMon.MonSpells;
		NewMon.Top = OldMon.Top;
		NewMon.Permanent = OldMon.Permanent;
		NewMon.Magic = OldMon.Magic;
		NewMon.SummonHelp = OldMon.SummonHelp;
		NewMon.ImmoralReact = OldMon.ImmoralReact;

		fwrite((char *)&NewMon, sizeof NewMon, 1, ofp);
	}
	fclose(ofp);
	fclose(ifp);
}
