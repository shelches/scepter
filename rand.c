/* seed values */
static int	s1;		/* initialised in range [1,32362] */
static int	s2;		/* initialised in range [1,31726] */
static int	s3;		/* initialised in range [1,31656] */

void Srand(int x1, int x2, int x3)
{
	s1 = x1 % 32362 + 1;
	s2 = x2 % 31726 + 1;
	s3 = x3 % 31656 + 1;
}

int Rand(void)
{
	int		z, k;

	k = s1 / 206;
	s1 = 157 * (s1 - k * 206) - k * 21;
	if (s1 < 0)
		s1 += 32363;
	k = s2 / 217;
	s2 = 146 * (s2 - k * 217) - k * 45;
	if (s2 < 0)
		s2 += 31727;
	k = s3 / 222;
	s3 = 142 * (s3 - k * 222) - k * 133;
	if (s3 < 0)
		s3 += 31657;
	z = s1 - s2;
	if (z > 706)
		z -= 32362;
	z += s3;
	if (z < 1)
		z += 32362;
	return z;
}
