#ifdef PROTO
/* main.c */
void	main(void);

/* mon.c */
void	PM(MonsterPoint Mon, int C, char *Str);
void	PrintMon(MonsterPoint Mon, int S, int C, char *Str);
void	Destroy(MonsterPoint Mon);
void	FixMonCount(MonsterPoint MonsterTail, char *Name);
void	MonDisplay(MonsterPoint Mon);
MonsterPoint	FindMonster(char *Word, int Num, MonsterPoint MonsterTail);

/* db.c */
void	ReadUObj(RmCodeType Rm, ObjectPoint *Object, int Type);
void	WriteUObj(RmCodeType Rm, ObjectPoint Object, int Type);
void	DeleteUObj(RmCodeType Rm);
void	ReadUMon(RmCodeType Rm, MonsterPoint *Monster);
void	WriteUMon(RmCodeType Rm, MonsterPoint Monster);
void	DeleteMon(RmCodeType Rm);

/* log.c */
UserPoint	Login(void);
void	EnterSex(void);
void	NewPW(void);
void	EnterStats(void);
void	EnterSkill(void);
void	ChooseClass(void);
void	MatchPW(void);
void	GetName(void);

/* execute.c */
void	ExecuteCommand(void);

/* io.c */
void	InitCmds(void);
int		Min(int a, int b);
int		Max(int a, int b);
long	MinL(long a, long b);
long	MaxL(long a, long b);
void	Abort(char *Error);
void	SetRandom(int a, int b);
int		Rnd(int X);
int		SubSet(char *Part, char *Full);
char	*Pb(int X);
void	Pn(int N, char *Str);
void	PNth(int N, char *Str);
char	*Pa(char First);
int		Cap(int Ch);
void	CapAlfa(char *a);
void	GetWord(char *Word, int *Num);
void	DoInput(void);
int		BinaryMatch(char *Word, int Count, char **NameList);
int		WordMatch(char *Word, int Count, char (*NameList)[11]);
void	Punctuate(int Num, int Total, char *Str);
void	Pd(int Rec, int Line, int PhraseNum, int Brief, int *Cont, char *V);
void	PrintDesc(int Rec, int Line, int PhraseNum, int Brief, char *Str1, char *Str2);

/* obj.c */
void	InitSpells(void);
void	ODestroy(ObjectPoint Cont);
void	DeleteObject(ObjectPoint Pt, ObjectPoint *ObjectTail);
void	ExpandName(char *Name, char *Str);
void	PrintObj(ObjectPoint Obj, int Singular, int Capital, char *Str);
void	ObjDisplay(ObjectPoint Object);
void	Ld(char *Name, char *Entry);
ObjectPoint	FindObject(char *Word, int Num, ObjectPoint ObjectTail);

/* user.c */
int		PlayerOnFile(char *Name);
void	ReadPlayer(UserPoint Player, char *Name);
void	FreePlayer(UserPoint Player);
void	WritePlayer(UserPoint Player);
void	DeleteUsr(char *Nm);
void	ChangeUsrName(char *OldNm, char *NewNm);
void	Users(char *Word);
UserPoint	FindPlayer(char *Word, UserPoint PlayerTail);
UserPoint	FindUser(char *Word);
UserPoint	FindPid(int FPid);
int		Hash(char *Word);
long	Expr(int Lvl);
void	PrintExp(UserPoint Plyr);
void	Inventory(void);
void	PrintDmg(UserPoint Player, int Damage, int Brf, char *Str);
int		ReadyCheck(long LastAtk);
void	PromptUser(void);
void	Init(void);

/* spells.c */
void	EnterSpell(char *Word, int Num);
int		SpellMon(int SpellCode, int Level, int Intel, UserPoint Plyr, MonsterPoint Monster, ObjectPoint Obj, char *Target);
void	Spell(int SpellCode);

/* matt.c */
void	MonAttack(void);
void	MonStrike(RmCodeType Rm);
void	EncounterMsg(MonsterPoint Mon, RmCodeType Rm, int NumMons, int NonDMs);
void	AttackMsg(MonsterPoint Monster, UserPoint Plyr);
void	InitEvents(void);
void	TimeCheck(void);
void	SystemMsg(char *s);

/* room.c */
void	OpenCave(void);
void	CloseCave(void);
int		W(int Rm);
void	Condition(UserPoint Plyr);
void	PlayerDisplay(UserPoint Plyer);
void	WriteSeg(int SlotNum, int SegNum, int Erase);
int		S(int Rm);
void	AddSeg(void);
void	DeleteMonster(MonsterPoint Pt, RmCodeType Rm);
void	InsertMonster(MonsterPoint Monster, RmCodeType Rm);
void	PlacePlayer(UserPoint Plyr, RmCodeType Rm);
void	DeletePlayer(UserPoint Pt, RmCodeType Rm);
void	DeleteUser(UserPoint Pt);
void	StopFollow(UserPoint Plyr);
void	RoomDisplay(int Rm, int Brief);
UserPoint	NextOkSend(UserPoint Usr);
int		MsgTerm(TalkHowType TalkHow);
void	RoomMsg(RoomType *Rm, int *Count);

/* ipc.c */
void	QAss(char *Mbx, int *Chn);
void	QCre(void);
void	QDea(int Chn);
void	QRea(char *Str, int *Chn);
void	QOut(int Chn, char *Str);
int	GetETIndex(int table, int row, int col);
void	GetNextMsg(void);
int   IsGuest(int term);
void	PutETIndex(int table, int row, int col, int val);
void	Who(void);
void	CheckHup(void);

/* edit1.c */
void	Edit(void);
int		GetEditParm(int LPList, char *Word, int *Num, int *Flag);

/* edit2.c */
void	ModName(char *Str);
void	EditObject(EdFunc EditCmd, char *Which, int ONum);
void	EditOList(EdFunc EditCmd, int OIndex);
void	EditDscrpt(EdFunc EditCmd, int Rec);
void	EnterDesc(void);
void	EditETList(EdFunc EditCmd, int WhatCode, int Index);

/* cmd1.c */
void	StopUsing(UserPoint Usr, ObjectPoint Object);
void	GetObject(char *Word, int Num, RmCodeType Rm);
void	DropObject(char *Word, int Num, RmCodeType Rm);
int		SpellCost(int Cost, int MinLvl, int MinInt);
int		Look(char *Word, int Num);
void	Train(void);
void	OpenClose(char *Word, int Num);
void	HitShArmor(UserPoint Plyr, int *Damage);
void	Hide(char *Word, int Num);
void	Search(void);
UserPoint	FindDm(UserPoint Usr);
void	LocalMsg(char *s);
void	AllMsg(char *s);

/* cmd2.c */
void	GoDirection(char *Word, int Num);
void	ReturnObj(char *Word, int Num);
void	Track(void);
int		UseObject(char *Word, int Num);
void	Logoff(UserPoint Usr);
void	KillPlayer(void);
void	SaveChar(void);
void	Off(void);
void	Dead(void);
int		MonExperience(MonsterPoint Mon);
void	ProtectNPC(UserPoint Player, MonsterPoint Monster);
void	Turn(char *Word, int Num);
void	HitPlayer(int Damage);
void	HitMonster(int Damage);

/* cmd3.c */
void	Attack(char *Word, int Num, RmCodeType Rm);
void	UpdateF(void);
void	Repair(char *Word, int Num);
void	ChangeName(char *Word, int Num);
void	ChangePW(char *Word, int Num);
void	Offer(char *Word, int Num);
void	Accept(void);
void	NumUsers(void);
void	Help(void);
void	Parley(char *Word, int Num);
void	YesNo(void);
void	EnterNotice(void);
void	EnterNews(void);
void	StealObj(char *Word, int Num);

/* cmd4.c */
void	Appeal(void);
void	Run(void);
void	Pawn(char *Word, int Num);
void	Buy(int Num, int Rm);
void	Catalog(void);
void	TagPlayer(char *Word);
void	FollowPlayer(char *Word);
void	Lose(char *Word);
void	Quit(int QuitHow);
void	GetStatus(char *Word);
void	TextLimits(int *MesStart, int *MesEnd);
void	SendMsg(int Tm, char *Verb, int MStart, int MEnd, int NoID, int NoQuote);
void	Talk(char *Word);
void	DoCmd(void);

/* rand.c */
int	Rand(void);
void	Srand(int x1, int x2, int x3);

/* library */
/*
void	*malloc(int size), free(void *p);
void	exit(int errno), get_date(unsigned date[2]), srand(int seed);
int		rand(void), abs(int n), getpid(void);
*/

#else

/* main.c */
void	main();

/* mon.c */
void	PM(), PrintMon(), Destroy(), FixMonCount(), MonDisplay();
MonsterPoint	FindMonster();

/* db.c */
void	ReadUObj(), WriteUObj(), DeleteUObj(), ReadUMon(), WriteUMon();
void	DeleteMon();

/* log.c */
UserPoint	Login();
void	EnterSex(), NewPW(), EnterStats(), EnterSkill(), ChooseClass();
void	MatchPW(), GetName();

/* execute.c */
void	ExecuteCommand();

/* io.c */
int		Min(), Max(), Rnd(), SubSet(), Cap(), BinaryMatch(), WordMatch();
char	*Pb(), *Pa();
long	MinL(), MaxL();
void	InitCmds(), Abort(), SetRandom(), Pn(), PNth(), CapAlfa(), GetWord();
void	DoInput(), Punctuate(), Pd(), PrintDesc();

/* obj.c */
void	InitSpells(), ODestroy(), DeleteObject(), ExpandName(), PrintObj();
void	ObjDisplay(), Ld();
ObjectPoint	FindObject();

/* user.c */
int		PlayerOnFile(), Hash(), ReadyCheck();
long	Expr();
void	ReadPlayer(), FreePlayer(), WritePlayer(), DeleteUsr(), ChangeUsrName();
void	Users(), PrintExp(), Inventory(), PrintDmg(), PromptUser(), Init();
UserPoint	FindPlayer();
UserPoint	FindUser();
UserPoint	FindPid();

/* spells.c */
int		SpellMon();
void	EnterSpell(), Spell();

/* matt.c */
void	MonAttack(), MonStrike(), EncounterMsg(), AttackMsg(), InitEvents();
void	TimeCheck(), SystemMsg();

/* room.c */
int		S(), W(), MsgTerm();
void	OpenCave(), CloseCave(), Condition(), PlayerDisplay(), WriteSeg();
void	AddSeg(), DeleteMonster(), InsertMonster(), PlacePlayer();
void	DeletePlayer(), DeleteUser(), StopFollow(), RoomDisplay(), RoomMsg();
UserPoint	NextOkSend();

/* ipc.c */
void	QAss(), QCre(), QDea(), QRea(), QOut(), GetNextMsg(), Who(), CheckHup();

/* edit1.c */
void	Edit();
int		GetEditParm();

/* edit2.c */
void	ModName(), EditObject(), EditOList(), EditDscrpt(), EnterDesc();
void	EditETList();

/* cmd1.c */
int		SpellCost(), Look();
void	StopUsing(), GetObject(), DropObject(), Train(), OpenClose();
void	HitShArmor(), Hide(), Search(), LocalMsg(), AllMsg();
UserPoint	FindDm();

/* cmd2.c */
void	GoDirection(), ReturnObj(), Track(), Logoff(), Kill(), SaveChar();
void	Off(), Dead(), ProtectNPC(), Turn(), HitPlayer(), HitMonster();
int		UseObject(), MonExperience();

/* cmd3.c */
void	Attack(), UpdateF(), Repair(), ChangeName(), ChangePW(), Offer();
void	Accept(), NumUsers(), Help(), Parley(), YesNo(), EnterNotice();
void	EnterNews(), StealObj();

/* cmd4.c */
void	Appeal(), Run(), Pawn(), Buy(), Catalog(), TagPlayer(), FollowPlayer();
void	Lose(), Quit(), GetStatus(), TextLimits(), SendMsg(), Talk(), DoCmd();

/* library */
void	*malloc();
#endif
