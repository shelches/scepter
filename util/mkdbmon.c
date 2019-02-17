#include "../scepter.h"

MonRec	rec;
int		i;

void main(void)
{
	FILE	*fp;

	memset(&rec, 0, sizeof(rec));

	fp = fopen("dbmon", "w");
	for (i = 0; i < 1009; i++)
		fwrite((char *)&rec, sizeof rec, 1, fp);
	fclose(fp);
}
