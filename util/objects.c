#include "../scepter.h"

ObjRec	rec;

void main(void)
{
	FILE	*fp;
	int		i, j;

	if (!(fp = fopen("/etc/scepter/dbobj", "r")))
		exit(1);

	for (j = 0; fread(&rec, sizeof rec, 1, fp); j++)
		if (rec.Room)
		{
			tprintf("(%ld) ", (long)j * sizeof rec);
			tprintf("%d:%d\n", rec.Room, rec.DataType);
			for (i = 0; i < rec.Num; i++)
				tprintf("\tna=%s we=%d va=%d\n", rec.Obj[i].Name,
						rec.Obj[i].Weight, rec.Obj[i].Price);
		}
	fclose(fp);
}
