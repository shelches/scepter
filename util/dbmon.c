#include "../scepter.h"

MonRec	Rec;

main()
{
	FILE	*EDBMon;
	int		i;

	EDBMon = fopen("3:/etc/scepter/dbmon", FOPEN);

	while (fread((char *)&Rec, sizeof(Rec), 1, EDBMon))
		if (Rec.Room)
		{
			long	ftell();

			printf("(%ld) %d:%d\n", ftell(EDBMon) - sizeof Rec, Rec.Room, Rec.DataType);
			for (i = 0; i < Rec.Num; i++)
			{
				MonsterPoint	p;

				p = &Rec.Mon[i];
				printf("\t%s lvl:%d %d/%d o:%d  %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\n",
					p->Name, p->Lvl, p->Hits, p->MaxHits, p->WhichObj,
					p->Defend, p->Block, p->Follow, p->Guard, p->AtkLastAggr,
					p->SlowReact, p->FastReact, p->Invisible, p->Regenerate,
					p->Drain, p->Poison, p->AntiMagic, p->Undead, p->MoralReact,
					p->Flee, p->Assistance, p->MonSpells, p->Permanent, p->Magic,
					p->ImmoralReact);
			}
		}

	fclose(EDBMon);
}
