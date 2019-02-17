#include "../scepter.h"

void main(void)
{
	FILE	*fp;
	ObjRec	rec;
	int		i, j;

	fp = fopen("/etc/scepter/dbobj", "r");
	for (i = 0; fread((char *)&rec, sizeof rec, 1, fp); i++)
		if (rec.Room && ((long)rec.Room * 73 + rec.DataType) % 1009 != i)
		{
			tprintf("(%ld) %d:%d\n", (long)i * sizeof rec, rec.Room, rec.DataType);
			for (j = 0; j < SavedItems && j < rec.Num; j++)
				tprintf("\t%s\n", rec.Obj[j].Name);
		}
	fclose(fp);
}
