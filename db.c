#include "scepter.h"

static int	ReadObj(ObjRec *Rec, int Room, int Type);
static void	WriteObj(ObjRec *Rec, int Room, int Type);
static int	HashObj(int Room, int Type);
static int	ReadMon(MonRec *Rec, int Room, int Type);

void ReadUObj(RmCodeType Rm, ObjectPoint *Object, int Type)
{
	ObjRec		Rec;

	if (ReadObj(&Rec, Rm, Type))
	{
		ObjectPoint	Pt;
		int			N, ContCount;

		Pt = malloc(sizeof(ObjectType));
		memset(Pt, 0, sizeof(*Pt));

		*Object = Pt;

		*Pt = Rec.Obj[0];
		Pt->Next = NULL;
		ContCount = 1;
		if (Pt->ObClass == Chest)
		{
			ObjectPoint	Pt3;

			Pt3	= Pt->Object.Chest.ObjectTail;
			ReadUObj(Rm, &Pt3, ContCount);
			Pt->Object.Chest.ObjectTail = Pt3;
			ContCount++;
		}

		ContCount = 1;
		for (N = 2; N <= Rec.Num; N++)
		{
			ObjectPoint	Pt2;

			Pt2 = malloc(sizeof(ObjectType));
			Pt->Next = Pt2;
			Pt = Pt2;
			memset(Pt, 0, sizeof(*Pt));

			*Pt = Rec.Obj[N - 1];
			Pt->Next = NULL;
			if (Pt->ObClass == Chest && ContCount <= 7)
			{
				ObjectPoint	Pt3;

				Pt3	= Pt->Object.Chest.ObjectTail;
				ReadUObj(Rm, &Pt3, ContCount);
				Pt->Object.Chest.ObjectTail = Pt3;
				ContCount++;
			}
		}
	}
	else
		*Object = NULL;
}

void WriteUObj(RmCodeType Rm, ObjectPoint Object, int Type)
{
	ObjectPoint	Pt;
	ObjRec		Rec;
	int			N, ContCount;

	ContCount = 1;

	for (Pt = Object, N = 0; Pt && N < SavedItems; Pt = Pt->Next, N++)
	{
		Rec.Obj[N] = *Pt;
		if (Pt->ObClass == Chest && Pt->Object.Chest.ObjectTail && ContCount <= SavedItems)
		{
			WriteUObj(Rm, Pt->Object.Chest.ObjectTail, ContCount);
			ContCount++;
		}
	}

	Rec.Room = Rm;
	Rec.DataType = Type;
	Rec.Num = N;
	WriteObj(&Rec, Rm, Type);
}

void DeleteUObj(RmCodeType Rm)
{
	int		Type;

	for (Type = 0; Type <= 8; Type++)
	{
		ObjRec	Tmp;
		int		RecNum, First;

		First = RecNum = HashObj(Rm, Type);
		fseek(EDBObj, (long)RecNum * sizeof(ObjRec), 0);
		while (fread((char *)&Tmp, sizeof(ObjRec), 1, EDBObj))
		{
			if (Tmp.Room == Rm && Tmp.DataType == Type)
			{
				Tmp.Room = 0;
				fseek(EDBObj, (long) -sizeof(ObjRec), 1);
				fwrite((char *)&Tmp, sizeof(ObjRec), 1, EDBObj);
				fflush(EDBObj);
				break;
			}
			
			if (!Tmp.Room)
				break;

			if (++RecNum >= 1009)
			{
				fseek(EDBObj, 0L, 0);
				RecNum = 0;
			}

			if (RecNum == First)
				Abort(" Object DB full on delete!");
		}
	}
}

static int ReadObj(ObjRec *Rec, int Room, int Type)
{
	int		RecNum, First;

	First = RecNum = HashObj(Room, Type);
	fseek(EDBObj, (long)RecNum * sizeof(ObjRec), 0);
	while (fread((char *)Rec, sizeof(ObjRec), 1, EDBObj))
	{
		if (!Rec->Room)
			return 0;
		if (Rec->Room == Room && Rec->DataType == Type)
			return 1;

		if (++RecNum >= 1009)
		{
			fseek(EDBObj, 0L, 0);
			RecNum = 0;
		}

		if (RecNum == First)
			Abort(" Object DB full on read!");
	}

	return 0;
}

static void WriteObj(ObjRec *Rec, int Room, int Type)
{
	ObjRec	Tmp;
	int		RecNum, First;

	First = RecNum = HashObj(Room, Type);
	fseek(EDBObj, (long)RecNum * sizeof(ObjRec), 0);
	while (fread((char *)&Tmp, sizeof(ObjRec), 1, EDBObj))
	{
		if (Tmp.Room == Room && Tmp.DataType == Type || !Tmp.Room)
		{
			fseek(EDBObj, (long) -sizeof(ObjRec), 1);
			fwrite((char *)Rec, sizeof(ObjRec), 1, EDBObj);
			fflush(EDBObj);
			break;
		}
		
		if (++RecNum >= 1009)
		{
			fseek(EDBObj, 0L, 0);
			RecNum = 0;
		}

		if (RecNum == First)
			Abort(" Object DB full on write!");
	}
}

static int HashObj(int Room, int Type)
{
	return (int)(((long)Room * 73 + Type) % 1009);
}

void ReadUMon(RmCodeType Rm, MonsterPoint *Monster)
{
	MonRec	MRec;

	if (ReadMon(&MRec, Rm, 0))
	{
		MonsterPoint	Pt;
		int				N;

		Pt = malloc(sizeof(MonsterType));
		memset(Pt, 0, sizeof(*Pt));

		*Monster = Pt;

		*Pt = MRec.Mon[0];
		Pt->Next = NULL;
		Pt->DefPlayer = NULL;
		if (Pt->ObjectTail)
			Pt->ObjectTail = NULL;

		for (N = 1; N < MRec.Num; N++)
		{
			Pt->Next = malloc(sizeof(MonsterType));
			Pt = Pt->Next;
			memset(Pt, 0, sizeof(*Pt));

			*Pt = MRec.Mon[N];
			Pt->Next = NULL;
			Pt->DefPlayer = NULL;
			if (Pt->ObjectTail)
				Pt->ObjectTail = NULL;
		}
	}
	else
		*Monster = NULL;
}

static int ReadMon(MonRec *Rec, int Room, int Type)
{
	int		RecNum, First;

	First = RecNum = HashObj(Room, Type);
	fseek(EDBMon, (long)RecNum * sizeof(MonRec), 0);
	while (fread((char *)Rec, sizeof(MonRec), 1, EDBMon))
	{
		if (!Rec->Room)
			return 0;
		if (Rec->Room == Room && Rec->DataType == Type)
			return 1;

		if (++RecNum >= 1009)
		{
			fseek(EDBMon, 0L, 0);
			RecNum = 0;
		}

		if (RecNum == First)
			Abort(" Monster DB full on read!");
	}

	return 0;
}

void WriteUMon(RmCodeType Rm, MonsterPoint Monster)
{
	MonsterPoint	Pt;
	MonRec			MRec, Tmp;
	int				N, RecNum, First;

	for (Pt = Monster, N = 0; Pt && N < SavedItems; Pt = Pt->Next, N++)
		MRec.Mon[N] = *Pt;

	MRec.Room = Rm;
	MRec.DataType = 0;
	MRec.Num = N;

	First = RecNum = HashObj(Rm, 0);
	fseek(EDBMon, (long)RecNum * sizeof(MonRec), 0);
	while (fread((char *)&Tmp, sizeof(MonRec), 1, EDBMon))
	{
		if (Tmp.Room == 0 || Tmp.Room == Rm && Tmp.DataType == 0)
		{
			fseek(EDBMon, (long) -sizeof(MonRec), 1);
			fwrite((char *)&MRec, sizeof(MonRec), 1, EDBMon);
			fflush(EDBMon);
			return;
		}
		
		if (++RecNum >= 1009)
		{
			fseek(EDBMon, 0L, 0);
			RecNum = 0;
		}

		if (RecNum == First)
			Abort(" Monster DB full on write!");
	}
}

void DeleteMon(RmCodeType Rm)
{
	MonRec	MRec;
	int		RecNum, First;

	First = RecNum = HashObj(Rm, 0);
	fseek(EDBMon, (long)RecNum * sizeof(MonRec), 0);
	while (fread((char *)&MRec, sizeof(MonRec), 1, EDBMon))
	{
		if (MRec.Room == Rm && MRec.DataType == 0)
		{
			MRec.Room = 0;
			fseek(EDBMon, (long) -sizeof(MonRec), 1);
			fwrite((char *)&MRec, sizeof(MonRec), 1, EDBMon);
			fflush(EDBMon);
			return;
		}
		
		if (!MRec.Room)
			return;

		if (++RecNum >= 1009)
		{
			fseek(EDBMon, 0L, 0);
			RecNum = 0;
		}

		if (RecNum == First)
		 	Abort(" Monster DB full on delete!");
	}
}
