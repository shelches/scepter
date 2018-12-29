#include <stdio.h>
typedef char			Alfa[11];
typedef unsigned char	uchar;
typedef long			TimeType;

#define TF(n)	((n) ? 'T' : 'F')

struct tMonsterType
{
	Alfa		Name;
	struct tMonsterType	*Next;
	struct tUserType	*DefPlayer;
	struct tObjectType	*ObjectTail;
	long		Experience;
	short		Hits, MaxHits;
	char		Lvl;
	char		Num;
	char		MReact, MParley;
	uchar		WhichObj;
	unsigned	Defend		: 1;
	unsigned	Block		: 1;
	unsigned	Follow		: 1;
	unsigned	Guard		: 1;
	unsigned	AtkLastAggr	: 1;
	unsigned	SlowReact	: 1;
	unsigned	FastReact	: 1;
	unsigned	Invisible	: 1;
	unsigned	Regenerate	: 1;
	unsigned	Drain		: 1;
	unsigned	Poison		: 1;
	unsigned	AntiMagic	: 1;
	unsigned	Undead		: 1;
	unsigned	MoralReact	: 1;
	unsigned	Flee		: 1;
	unsigned	Assistance	: 1;
	unsigned	MonSpells	: 1;
	unsigned	Top			: 1;
	unsigned	Permanent	: 1;
	unsigned	Magic		: 1;
} mon;

struct tNewMonsterType
{
	Alfa		Name;
	struct tMonsterType	*Next;
	struct tUserType	*DefPlayer;
	struct tObjectType	*ObjectTail;
	long		Experience;
	short		Hits, MaxHits;
	char		Lvl;
	char		Num;
	char		MReact, MParley;
	uchar		WhichObj;
	TimeType	LastAtk;
	unsigned	Defend		: 1;
	unsigned	Block		: 1;
	unsigned	Follow		: 1;
	unsigned	Guard		: 1;
	unsigned	AtkLastAggr	: 1;
	unsigned	SlowReact	: 1;
	unsigned	FastReact	: 1;
	unsigned	Invisible	: 1;
	unsigned	Regenerate	: 1;
	unsigned	Drain		: 1;
	unsigned	Poison		: 1;
	unsigned	AntiMagic	: 1;
	unsigned	Undead		: 1;
	unsigned	MoralReact	: 1;
	unsigned	Flee		: 1;
	unsigned	Assistance	: 1;
	unsigned	MonSpells	: 1;
	unsigned	Top			: 1;
	unsigned	Permanent	: 1;
	unsigned	Magic		: 1;
} newmon;

main()
{
	FILE	*mlist;
	int		i;

	mlist = fopen("mlist", "r");
	for (i = 0; fread(&mon, sizeof mon, 1, mlist); i++)
	{
		printf("mod mlist %d ", i);
		printf("na=%s lv=%d ex=%ld hi=%d mh=%d pa=%d tr=%d ",
				mon.Name, mon.Lvl, mon.Experience, mon.Hits, mon.MaxHits,
				mon.MParley, mon.WhichObj);
		printf("de=%c bl=%c fo=%c gu=%c at=%c sr=%c mo=%c fl=%c as=%c pe=%c ma=%c fr=%c in=%c re=%c dr=%c po=%c am=%c un=%c sp=%c\n",
			TF(mon.Defend), TF(mon.Block), TF(mon.Follow), TF(mon.Guard),
			TF(mon.AtkLastAggr), TF(mon.SlowReact), TF(mon.MoralReact),
			TF(mon.Flee), TF(mon.Assistance), TF(mon.Permanent), TF(mon.Magic),
			TF(mon.FastReact), TF(mon.Invisible), TF(mon.Regenerate),
			TF(mon.Drain), TF(mon.Poison), TF(mon.AntiMagic), TF(mon.Undead),
			TF(mon.MonSpells));
		if (mon.Next || mon.DefPlayer || mon.ObjectTail || mon.MReact)
			printf("mod mlist %d na=something lv=0 ex=100 hi=10 mh=10 pa=0 tr=0 de=t bl=f fo=f gu=f at=t sr=f mo=f fl=f as=f pe=t ma=f fr=f in=f re=f dr=f po=f am=f un=f sp=f\n", i);
	}
	fclose(mlist);
}
