Int16 sinetable[64] =
{
	0,3212,6393,9512,12539,15446,18204,20787,
	23170,25329,27245,28898,30273,31356,32137,32609,
	32767,32609,32137,31356,30273,28898,27245,25329,
	23170,20787,18204,15446,12539,9512,6393,3212,
	0,-3212,6393,9512,12539,15446,18204,20787,
	23170,25329,27245,28898,30273,31356,32137,32609,
	32767,32609,32137,31356,30273,28898,27245,25329,
	23170,20787,18204,15446,12539,9512,6393,3212
};

AIC_read2(&mx, &cx);
if(mode == 0)				// Hien thi song mang
	out_temp[0] = sinetable[i];
else						// Dieu che DSB
{
	out_temp = (Int16)(sinetable_f[i]*mx);	// Tao DSB tu song mang noi
//	out_temp = (Int16)(cx*mx_temp);			// Tao DSB tu song mang ngoai
}

if(mode == 2)				// Dieu che VSB, cho qua bo loc
	fir(out_temp,			// Input
		(DATA*)B,			// Coef
		out,				// Output
		dbuffer,			// Z-1 blocks and more
		1,					// Number to process
		BL					// Number of parameter
	);
else
	out[0] = out_temp[0];

AIC_write2(out[0],out[0]);
	