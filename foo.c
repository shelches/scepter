#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct
{
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

} Flags;

/*
** main dementia
*/

int main(void)
{
	unsigned long oof;
	Flags foo;

	memset(&foo, 0, sizeof(foo));
	foo.Defend = 1;
	memcpy(&oof, &foo, sizeof(foo));

	printf("sizeof(Flags) == %d\n", sizeof(Flags));
	printf("oof == %lu\n", oof);

	return 0;
}
