#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*#if defined(__HIGHC__) || defined(M_XENIX) || defined(mc68k)*/
# include <string.h>
# include <malloc.h>
/*# include <memory.h>*/
# define FOPEN	"r+"
/*#else*/	/* QNX? */
/*# define FOPEN	"rwo"*/
/*#endif*/

#define	FALSE	0
#define	TRUE	1

#define	UNUSED(x) x = x

#define	SavedItems	8
#define	MaxNames	40
#define	StrPerSeg	10

#define	CmdListLen	111		/* number of commands */

#define	LEncounter	200		/* encounters in EList */
#define	ObjListLen	200		/* treasures in TList */
#define	RanMonLen	200		/* monsters in MList */
#define	RanObjLen	200		/* objects in OList */
#define	MaxEvents	20		/* size of Event list */

#define	MaxUsers		12	/* Maximum number of users */
#define	MaxPlusOne		13	/* MaxUsers + 1 */
#define	RmLimit			130	/* MaxPlusOne * 10 */
#define	MaxSegs			199
#define	MaxObjs			10	/* max number of objects saved in user record */

#define	UpdatePeriod	3600	/* one hour */
#define	QuitWait		20
#define	HealWait		30

/* global types */
typedef unsigned char	uchar;
typedef signed char		schar;
typedef long			TimeType;
typedef short			RmCodeType;
typedef RmCodeType		AdjoinType[6];
typedef char			Alfa[11];
typedef unsigned short	TermIDType[20];
typedef char			*CmdListType[CmdListLen + 1];
typedef uchar			CmdNumType[CmdListLen + 1];
typedef Alfa			NameTList[MaxNames + 1];

/* spell types */
#define	OnUser		1
#define	OnPlayer	2
#define	OnMon		4
#define	OnObject	8

/* spells */
#define SPELL_NULL       0
#define SPELL_VIGOR      1
#define SPELL_HEAL       2
#define SPELL_FIREBALL   3
#define SPELL_LIGHTNING  4
#define SPELL_HURT       5
#define SPELL_CUREPOISON 6
#define SPELL_DISINTEGRA 7
#define SPELL_BEFUDDLE   8
#define SPELL_TELEPORT   9
#define SPELL_WISH       10
#define SPELL_PASSDOOR   11
#define SPELL_ENCHANT    12
#define SPELL_BLESS      13
#define SPELL_PROTECTION 14
#define SPELL_CURSE      15
#define SPELL_POISON     16
#define SPELL_INTOXICATE 17
#define SPELL_PSI_BLAST  18
#define SPELL_POLYMORPH  19
#define SPELL_SUMMON     20
#define SPELL_ANIMATE    21
#define SPELL_CHARM      22
#define SPELL_ILLUMINATE 23

typedef enum
	{Local, Yell, Others, All, BrOthers, BrAll, NoBlock, SysMsg} TalkHowType;
typedef enum
	{A, An, Some, The, None} ArticleType;
typedef enum
	{Male, Female} SexType;
typedef enum
	{DFlag, DNum, DWord, DOther} ParmType;
typedef enum
	{SInit, SLogin, SNormal} StatusType;
typedef enum
	{Sharp, Thrust, Blunt, Long} SkillType;
typedef enum
	{ToNone, ToMonster, ToPlayer} ToWhatType;
typedef enum
	{Portal, Treasure, Weap, Armor, Shield, Coins, Scroll, Chest, Door,
	 MagDevice, Teleport, Keys, Card, Ring, Misc, ObjDummy} ObClassType;
typedef enum
	{Fighter, Thief, MagicUser, DM, Cleric,
	 Paladin, Ranger, Barbarian} ChType;
typedef enum
	{XInit, XName, XCmd, XEdit, XPassword, XNewPW, XChangePW, XNewClass,
	 XSex, XStats, XNotice, XDead, XNews, XSell, XRepair, XWish, XSpell,
	 XSure, XParley, XSkill, XDesc, XRes1, XRes2, XRes3} EntryType;

typedef enum
{
	E_NULL = 0,
	E_MODIFY, E_DELETE, E_DISPLAY, E_CREATE, E_COPY, E_MOVE,
	E_QUIT, E_XQ, E_END
} EdFunc;

typedef struct
{
	char		*PName;
	ParmType	PType;
} PLType;

typedef struct
{
	int		SplHash;
	short	SplLen;
	short	SplMp;
	short	SplLvl;
	short	SplInt;
	int		SplType;
} SpellType;

union tObject
{
	struct tPortal
	{
		RmCodeType	ToWhere;
	} Portal;

	struct tWeap
	{
		short		MinHp, MaxHp;
		short		Strikes;
		short		WeaPlus;
		SkillType	WeapType;
	} Weap;

	struct tShield
	{
		short		ShPlus;
		short		ShHits;
	} Shield;

	struct tArmor
	{
		short		ArmPlus;
		short		ArmHits;
	} Armor;

	struct tCoins
	{
		short		Multiplier;
	} Coins;

	struct tScroll
	{
		short		Spell;
	} Scroll;

	struct tChest
	{
		char		Closed;
		char		Trap;
		short		Locked;
		char		NumInside;
		struct tObjectType	*ObjectTail;
	} Chest;

	struct tDoor
	{
		RmCodeType	DToWhere;
		unsigned	DClosed	: 1;
		unsigned	DSpring	: 1;
		unsigned	DToll	: 1;
		short		DTrap;
		short		DLocked;
	} Door;

	struct tKeys
	{
		short		UnLock;
		short		NumUses;
	} Keys;

	struct tMagDevice
	{
		short		MSpell;
		short		NumCharges;
		char		MLevel;
	} MagDevice;

	struct tTeleport
	{
		RmCodeType	TToWhere;
		short		TActiveRm;
	} Teleport;

	struct tRing
	{
		short		RSpell;
		char		RLevel;
	} Ring;
};

typedef struct tObjectType
{
	char		Name[21];
	struct tObjectType	*Next;
	ArticleType	Article;
	short		Weight;
	short		Price;
	uchar		DescCode, DescRec;
	unsigned	Carry		: 1;
	unsigned	Magic		: 1;
	unsigned	Permanent	: 1;
	unsigned	Invisible	: 1;
	unsigned	Cursed		: 1;
	ObClassType	ObClass;
	union tObject	Object;
} ObjectType, *ObjectPoint;

typedef struct tMonsterType
{
	char		Name[21];
	struct tMonsterType	*Next;
	struct tMonsterType	*DefMon;
	struct tUserType	*DefPlayer;
	struct tUserType	*CharmPlayer;
	ObjectPoint	ObjectTail;
	ArticleType	Article;
	uchar		DescRec, DescCode;
	char		AtkSpeed;
	schar		AC;
	short		Hits, MaxHits;
	char		Lvl;
	char		Num;
	char		MReact, MParley;
	uchar		WhichObj;
	TimeType	Time;
	long		Filler;
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
	unsigned	SummonHelp	: 1;
	unsigned	ImmoralReact	: 1;
	unsigned	Unique		: 1;
	unsigned	Watch		: 1;
	unsigned	Nice		: 1;
} MonsterType, *MonsterPoint;

typedef struct tUserType	UserType, *UserPoint;

struct tUserType
{
	/* the first part is the data which gets saved */
	Alfa		Name;
	short		Lvl;
	ChType		Class;
	short		Hits, MaxHits;
	short		Fatigue, MaxFatigue;
	short		Magic, MaxMagic;
	long		Experience;
	RmCodeType	RmCode;
	short		Str, Int, Dex, Pty, Con;
	short		LastAccess;		/* mmmmdddddd */
	long		PW;
	long		Money;
	SexType		Sex;
	short		SSharp, SThrust, SBlunt, SLong;
	short		Assoc;			/* if non-zero, restricts assoc-dm editing */
	long		Spells;			/* known spells */
	unsigned	SSJ			: 1;
	unsigned	Master		: 1;
	unsigned	PlayTester	: 1;
	unsigned	AGuild		: 1;
	unsigned	TGuild		: 1;
	unsigned	Evil		: 1;
	unsigned	Invisible	: 1;
	unsigned	NonExistant	: 1;
	unsigned	SkillNew	: 1;
	unsigned	SpellNew	: 1;
	unsigned	Poisoned	: 1;
	unsigned	Hidden		: 1;
	unsigned	It			: 1;
	unsigned	Suck		: 1;
	unsigned	Brief		: 1;
	unsigned	Echo		: 1;
	unsigned	MesBlock	: 1;
	unsigned	Dead		: 1;
	unsigned	Plague		: 1;

	/* The rest is data which is only applicable during play. */
	/* All these fields should be initialised at load time. */
	StatusType	Status;
	short		Trm;
	long		XPid;
	short		Weight;
	short		AC;
	ObjectPoint	ObjectTail;
	UserPoint	Next;			/* next in room */
	UserPoint	DefPlayer;
	UserPoint	Follow;
	UserPoint	NextUser;		/* next in game */
	ObjectPoint	USWeap, USArm, USShield, USRingL, USRingR;
	MonsterPoint	DefMon;
	EntryType	Entry;
	TimeType	LastInput;
	TimeType	LastAtk;
	TimeType	LastHeal;
	TimeType	HitAtTime;
	TimeType	Drunk;
	Alfa		LastCmd;
	Alfa		WData;
	long		Data;
};

/* NOTE:  tUserSave must be EXACTLY the same as the first part of tUserType */

typedef struct tUserSave
{
	Alfa		Name;
	short		Lvl;
	ChType		Class;
	short		Hits, MaxHits;
	short		Fatigue, MaxFatigue;
	short		Magic, MaxMagic;
	long		Experience;
	RmCodeType	RmCode;
	short		Str, Int, Dex, Pty, Con;
	short		LastAccess;		/* mmmmdddddd */
	long		PW;
	long		Money;
	SexType		Sex;
	short		SSharp, SThrust, SBlunt, SLong;
	short		Assoc;			/* if non-zero, restricts assoc-dm editing */
	long		Spells;			/* known spells */
	unsigned	SSJ			: 1;
	unsigned	Master		: 1;
	unsigned	PlayTester	: 1;
	unsigned	AGuild		: 1;
	unsigned	TGuild		: 1;
	unsigned	Evil		: 1;
	unsigned	Invisible	: 1;
	unsigned	NonExistant	: 1;
	unsigned	SkillNew	: 1;
	unsigned	SpellNew	: 1;
	unsigned	Poisoned	: 1;
	unsigned	Hidden		: 1;
	unsigned	It			: 1;
	unsigned	Suck		: 1;
	unsigned	Brief		: 1;
	unsigned	Echo		: 1;
	unsigned	MesBlock	: 1;
	unsigned	Dead		: 1;
	unsigned	Plague		: 1;
} UserSave;

extern struct tUserFRec
{
	UserSave	User;
	short		NObj;
	short		NWeap, NArm, NShield, NRingL, NRingR;	/* index into Objs[] */
	ObjectType	Objs[MaxObjs];
} UserFRec;

typedef struct
{
	AdjoinType		Adjoin;
	RmCodeType		Out;
	uchar			DescRec, DescCode;
	uchar			WhichEncounter, EncounterTime;
	uchar			LastDir;
	MonsterPoint	RmMonsterTail;
	UserPoint		RmPlayerTail;
	ObjectPoint		RmObjectTail;
	unsigned		NotifyDM	: 1;
	unsigned		Safe		: 1;
	unsigned		AntiMagic	: 1;
	unsigned		Dark		: 1;
} RoomType;

typedef struct
{
	RmCodeType	Room;
	char		DataType;
	char		Num;
	ObjectType	Obj[SavedItems];
} ObjRec;

typedef struct
{
	RmCodeType	Room;
	char		DataType;
	char		Num;
	MonsterType	Mon[SavedItems];
} MonRec;


/* global variables */
extern FILE		*EDBMon;
extern FILE		*EDBObj;
extern FILE		*ECaves;
extern FILE		*EDesc;
extern FILE		*EFile;
extern FILE		*MList;
extern FILE		*OList;

extern RoomType	Room[RmLimit];
extern int		SlotTbl[MaxPlusOne];
extern int		Active[MaxPlusOne];
extern RoomType	ProtoRoom;

extern int		NUsers;
extern TimeType	LastUpdate;

extern int		NumSegs;
extern short	Today;		/* mmmmdddddd */
extern int		NumRooms;

extern char		*DirList[7];
extern char		*CName[8];
extern char		*Pro[2];

extern UserPoint	User, UserTail;
extern UserType	ProtoUser;

extern CmdListType	CmdList;
extern CmdNumType	CmdNum;
extern PLType		Parms[43];

extern int		Term;

extern char		Buf[256];
extern int		Loc, LenBuf;
extern int		CmdCode;

extern TermIDType	TermList;
extern int			Pid;

extern char		B1[256], B2[256], B3[256], C1[256];

extern char		*Event[MaxEvents], *NextEvent;
extern int			LenEvent;

extern MonsterType	ProtoMonster;

extern int			FolCount;
extern ObjectType	ProtoObject;

extern NameTList	SpellList;
extern SpellType	SpellClass[MaxNames];
extern int			Spellen;

extern char		Notice[256];
extern char		NewsBuf[5][256];
extern int		TLvl[3];
extern Alfa		TName[3];
extern char		TClass[3][13];
extern char		TBuf[3][256];
extern char		TaskClosed;
extern int		MSpeed;
extern int		NumRun;

extern char		SFObj[21], STObj[21];
extern Alfa		NFPlyr, NTPlyr, NFObj, NTObj;
extern int		FCash, TCash;
extern TimeType	TradeTime;

extern TimeType	RealTime;

extern char				DescBuf[StrPerSeg][80];
extern int				CurrentRec;

/* function declarations */
#include "proto.h"

/* data files */
extern char	DBMON[];
extern char	DBOBJ[];
extern char	DSCRPT[];
extern char	HDDSCRPT[];
extern char	EFILE[];
extern char	EDUNGEON[];
extern char	MLIST[];
extern char	OLIST[];
extern char	HDMLIST[];
extern char	HDOLIST[];
extern char	ERRLOG[];
extern char	PASSWD[];

#define	EncIndex(r, c)	GetETIndex(0, r, c)
#define	ObjIndex(r, c)	GetETIndex(1, r, c)
#define	PutEncIndex(r, c, v)	PutETIndex(0, r, c, v)
#define	PutObjIndex(r, c, v)	PutETIndex(1, r, c, v)
