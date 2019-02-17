#include "../old/scepter.h"

struct tUserFRec	rec;

void main(void)
{
	FILE	*fp;
	int		i, j;

	if (!(fp = fopen("/etc/scepter/eperson", "r")))
		exit(1);

	for (j = 0; fread(&rec, sizeof rec, 1, fp); j++)
	{
		if (!*rec.User.Name)
			continue;
		tprintf("(%x) ", j * sizeof rec);
		tprintf("na=%s lvl=%d exp=%ld mo=%ld pw=%lx  ", rec.User.Name,
				rec.User.Lvl, rec.User.Experience, rec.User.Money, rec.User.PW);
		if (rec.User.AGuild)
			putchar('A');
		if (rec.User.TGuild)
			putchar('T');
		if (rec.User.Poisoned)
			putchar('P');
		if (rec.User.Evil)
			putchar('E');
		putchar('\n');
		tprintf("    st=%d in=%d dx=%d pt=%d co=%d\n", rec.User.Str, rec.User.Int, rec.User.Dex, rec.User.Pty, rec.User.Con);
		tprintf("    %d/%d %d/%d %d/%d  %d,%d,%d,%d  %c\n",
			rec.User.Hits, rec.User.MaxHits, rec.User.Fatigue, rec.User.MaxFatigue,
			rec.User.Magic, rec.User.MaxMagic, rec.User.SSharp, rec.User.SThrust,
			rec.User.SBlunt, rec.User.SLong, rec.User.SkillNew ? 'T' : 'F');
		for (i = 0; i < 8 && i < rec.NObj; i++)
			tprintf("\tna=%s we=%d va=%d\n", rec.Objs[i].Name,
					rec.Objs[i].Weight, rec.Objs[i].Price);
	}
	fclose(fp);
}
