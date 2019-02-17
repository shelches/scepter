#include <dev.h>
#include "../scepter.h"

FILE	*fp;
struct tUserFRec	rec;

void main(void)
{
	int			c, i;
	unsigned	opt;

	if (!(fp = fopen("/etc/scepter/eperson", "r")))
		exit(1);

	opt = get_option(stdin);
	set_option(stdin, opt & ~(ECHO | EDIT));
	term_load(stdout);
	term_clear(0);

	i = 0;
	do
	{
		readrec(i);
		disprec(i);
		term_cur(15, 0);
		c = getchar();
		if (c == '+')
			i++;
		else
		if (c == '-' && i > 0)
			i--;
	} while (c != EOF && c != '\033' && c != '\003');
	set_option(stdin, opt);
	fclose(fp);
}

size_t readrec(int n)
{
	fseek(fp, (long)n * sizeof rec, 0);
	return fread(&rec, sizeof rec, 1, fp);
}

char	*class[] =
{"fighter", "thief", "MU", "DM", "cleric", "paladin", "ranger", "barbarian"};

void disprec(int n)
{
	int		i;
	static int	prev = -1;

	if (n == prev)
		return;
	prev = n;

	term_tprintf(0, 8, 0, "%03d (%04x)", n, n * sizeof rec);
	if (!*rec.User.Name)
	{
		term_type(0, 20, "--undefined--", 13, 0);
		return;
	}
	term_tprintf(0, 20, 0, "%s the %s (%c)", rec.User.Name,
		class[rec.User.Class], rec.User.Sex == Male ? 'M' : 'F');
	term_clear(1);
	term_tprintf(1, 4, 0, "lvl=%-5d exp=%-10ld mo=%-10ld we=%-5d",
			rec.User.Lvl, rec.User.Experience, rec.User.Money, rec.User.Weight);
	term_type(2, 15, rec.User.AGuild ? "X" : "_", 1, 0);
	term_type(2, 29, rec.User.TGuild ? "X" : "_", 1, 0);
	term_type(2, 44, rec.User.Poisoned ? "X" : "_", 1, 0);
	term_type(2, 55, rec.User.Evil ? "X" : "_", 1, 0);

	term_type(3, 15, rec.User.SSJ ? "X" : "_", 1, 0);
	term_type(3, 29, rec.User.Invisible ? "X" : "_", 1, 0);
	term_type(3, 44, rec.User.NonExistant ? "X" : "_", 1, 0);
	term_type(3, 55, rec.User.Hidden ? "X" : "_", 1, 0);

	term_tprintf(4, 4, 0, "str=%-5d int=%-5d dex=%-5d pty=%-5d con=%-5d",
			rec.User.Str, rec.User.Int, rec.User.Dex, rec.User.Pty,
			rec.User.Con);
	term_tprintf(5, 4, 0, "vit=%d/%d fat=%d/%d ma=%d/%d",
			rec.User.Hits, rec.User.MaxHits, rec.User.Fatigue,
			rec.User.MaxFatigue, rec.User.Magic, rec.User.MaxMagic);
	term_tprintf(6, 4, 0, "sh=%5d0%%  th=%5d0%%  bl=%5d0%%  po=%5d0%%  (%c)",
			rec.User.SSharp, rec.User.SThrust, rec.User.SBlunt, rec.User.SLong,
			rec.User.SkillNew ? 'T' : 'F');
/*	for (i = 0; i < 8 && i < rec.NObj; i++)
		term_tprintf(5 + i, 8, 0, "na=%s we=%d va=%d", rec.Objs[i].Name,
				rec.Objs[i].Weight, rec.Objs[i].Price); */
}

/*
Record #000 (xxxx)  AAAAAAAAAA the aaaaaaaaa (M)
	lvl=00000 exp=0000000000 mo=0000000000 we=00000 room=00000
    Assassins ___   Thieves ___   Poisoned ___   Evil ___
	       DM ___       Inv ___        Non ___    Hid ___
    str=00000 int=00000 dex=00000 pty=00000 con=00000
    vit=00000/00000 fat=00000/00000 mp=00000/00000
	sharp=00000  thrust=00000  blunt=00000  pole=00000  (F)
*/

#ifdef MOO

Alfa		Name;
StatusType	Status;	/* SInit, SLogin, SNormal */
short		Weight;
short		Lvl;
ChType		Class;	/* Fighter/Thief/MU/DM/Cleric/Paladin/Ranger/Barbarian */
short		Hits;
short		MaxHits;
short		Fatigue;
short		MaxFatigue;
short		Magic;
short		MaxMagic;
long		Experience;
RmCodeType	RmCode;
short		Str, Int, Dex, Pty, Con;
long		Money;
SexType		Sex;	/* Male, Female */
short		SSharp, SThrust, SBlunt, SLong;
unsigned	SSJ			: 1;
unsigned	AGuild		: 1;
unsigned	Evil		: 1;
unsigned	Invisible	: 1;
unsigned	NonExistant	: 1;
unsigned	SkillNew	: 1;
unsigned	Poisoned	: 1;
unsigned	PlayTester	: 1;
unsigned	Hidden		: 1;
unsigned	Assoc		: 1;
unsigned	Master		: 1;
unsigned	TGuild		: 1;

#endif
