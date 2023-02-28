#include <stdio.h>
#include <stdlib.h>

// For information on the variable names, check out the first lines of Deoxys_boomerang_truncated_all.c
int i,j,r;

int rl,ru;
int unext; /* next unused state variable index */
int udummy; /* next unused dummy variable index */
int udummy_e;
int ucancel;
int ucolumn_deg;


int lnext; /* next unused state variable index */
int ldummy; /* next unused dummy variable index */
int ldummy_e;
int lcancel;
int lcolumn_deg;

int snext;


const int KEYROTATE[16] = {1,6,11,12, 5,10,15,0, 9,14,3,4, 13,2,7,8};
FILE *fi;
char filename[100];

void AddTweakeyL(int a[4][4], int tk[4][4]){
	//In1 In2 Out Cancel
	// 0   0   0    0  ok
	// 0   0   0    1  invalid

	// 0   0   1    *  invalid

	// 0   1   0    *  invalid

	// 0   1   1    0  ok
	// 0   1   1    1  invalid

	// 1   0   0    *  invalid

	// 1   0   1    0  ok
	// 1   0   1    1  invalid

	// 1   1   0    0  invalid
	// 1   1   0    1  ok

	// 1   1   1    0  ok
	// 1   1   1    1  invalid
	for(j=0; j < 4; j++){
		for(i = 0; i < 4; i++){
			fprintf(fi,"  lx%i + ltk%i - ly%i >= 0\n",a[i][j],tk[i][j],a[i][j]);
			fprintf(fi,"  lx%i - ltk%i + ly%i >= 0\n",a[i][j],tk[i][j],a[i][j]);
			fprintf(fi,"- lx%i + ltk%i + ly%i >= 0\n",a[i][j],tk[i][j],a[i][j]);
			
			fprintf(fi,"  lx%i + ltk%i + ly%i - lcancel%i >= 0\n" ,a[i][j],tk[i][j],a[i][j],lcancel);
			fprintf(fi,"  lx%i - ltk%i - ly%i - lcancel%i >= -2\n",a[i][j],tk[i][j],a[i][j],lcancel);
			fprintf(fi,"- lx%i + ltk%i - ly%i - lcancel%i >= -2\n",a[i][j],tk[i][j],a[i][j],lcancel);
			fprintf(fi,"- lx%i - ltk%i + ly%i + lcancel%i >= -1\n",a[i][j],tk[i][j],a[i][j],lcancel);
			fprintf(fi,"- lx%i - ltk%i - ly%i - lcancel%i >= -3\n",a[i][j],tk[i][j],a[i][j],lcancel);
			lcancel++;

			// Truncated constraints
			// 2. xt <=> yt 
			// xe = ye

			fprintf(fi,"lxt%i - lyt%i = 0\n",a[i][j],a[i][j]);
			fprintf(fi,"lxe%i - lye%i = 0\n",a[i][j],a[i][j]);

		}
	}
}

void AddTweakeyU(int a[4][4], int tk[4][4]){
	//In1 In2 Out Cancel
	// 0   0   0    0  ok
	// 0   0   0    1  invalid

	// 0   0   1    *  invalid

	// 0   1   0    *  invalid

	// 0   1   1    0  ok
	// 0   1   1    1  invalid

	// 1   0   0    *  invalid

	// 1   0   1    0  ok
	// 1   0   1    1  invalid

	// 1   1   0    0  invalid
	// 1   1   0    1  ok

	// 1   1   1    0  ok
	// 1   1   1    1  invalid
	for(j=0; j < 4; j++){
		for(i = 0; i < 4; i++){
			fprintf(fi,"  ux%i + utk%i - uy%i >= 0\n",a[i][j],tk[i][j],a[i][j]);
			fprintf(fi,"  ux%i - utk%i + uy%i >= 0\n",a[i][j],tk[i][j],a[i][j]);
			fprintf(fi,"- ux%i + utk%i + uy%i >= 0\n",a[i][j],tk[i][j],a[i][j]);
			
			fprintf(fi,"  ux%i + utk%i + uy%i - ucancel%i >= 0\n" ,a[i][j],tk[i][j],a[i][j],ucancel);
			fprintf(fi,"  ux%i - utk%i - uy%i - ucancel%i >= -2\n",a[i][j],tk[i][j],a[i][j],ucancel);
			fprintf(fi,"- ux%i + utk%i - uy%i - ucancel%i >= -2\n",a[i][j],tk[i][j],a[i][j],ucancel);
			fprintf(fi,"- ux%i - utk%i + uy%i + ucancel%i >= -1\n",a[i][j],tk[i][j],a[i][j],ucancel);
			fprintf(fi,"- ux%i - utk%i - uy%i - ucancel%i >= -3\n",a[i][j],tk[i][j],a[i][j],ucancel);
			ucancel++;

			// Truncated constraints
			// 2. xt <=> yt 
			// xe = ye

			fprintf(fi,"uxt%i - uyt%i = 0\n",a[i][j],a[i][j]);
			fprintf(fi,"uxe%i - uye%i = 0\n",a[i][j],a[i][j]);

		}
	}
}

void ShuffleKey(int tk[4][4]) {
	int tmp[16];
	for(i = 0; i < 4; i++)
		for(j=0; j < 4; j++)
			tk[i][j]+=16;
}

int NextPosition(int targetindex) {
	return targetindex;
}

void ShiftRows(int a[4][4]) {
	int tmp[4];
	for(i=1; i < 4; i++) {
		for(j = 0; j < 4; j++) tmp[j] = a[i][(j + i) % 4];
		for(j = 0; j < 4; j++) a[i][j] = tmp[j];
	}
}

void SubBytesL(int a[4][4]) {
	for(i=0; i < 4; i++) {
		for(j = 0; j < 4; j++) {

			// 
			// Recall that 1 >= ly >= lyt >= lye >= 0
			//             1 >= ly >= lzt >= lze >= 0
			// Either ly = 1 and
			//lyt lzt lye  lze Variable    P_fromPlaintext  P_fromCiphertext    Set as impossible
			// 0   0   0    0  laa         2^{-12}          2^{-12} 
			// 0   1   0    0  lat         2^{-16}          2^{-8}     
			// 0   1   0    1  lae         2^{-14}          2^{-15}                 
			// 1   1   0    1  lte         1                2^{-8}                 
			// 1   1   1    1  lee         2{-7}            2{-7}                  
           
			
			// 1   0   0    0  lta         1                2^{-8}
			// 1   0   1    0  lea         2^{-8}           2^{-8}
			// 1   1   0    0  ltt         1                1
			// 1   1   1    0  let         2^{-8}           1
			
			// Or ly = 0

			// Following condition, exactly one of the above cases is true
			fprintf(fi,"laa%i + lat%i + lae%i + lte%i + lee%i + lta%i + lea%i + ltt%i + let%i - ly%i = 0\n",a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j]);

			// Now if a variable is forced to take the value 1 in one condition, all other variables are set to 0 automatically 

			// laa = 1 if ly = 1 and all 4 variables lyt lzt lye lze are 0 (checking that ly=1 and lyt=lzt=0 is enough)
			fprintf(fi,"laa%i - ly%i + lyt%i + lzt%i >= 0\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// lat = 1 if lzt = 1 and lyt = lye = lze = 0 (checking that lzt = 1 and lze=lyt=0 is enough)
			fprintf(fi,"lat%i - lzt%i + lyt%i + lze%i >= 0\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// lea = 1 if lye = lyt = 1 and lzt = lze = 0 (checking that lye = 1 and lzt = 0 is enough)
			fprintf(fi,"lea%i - lye%i + lzt%i >= 0\n",a[i][j],a[i][j],a[i][j]);

			// lta = 1 if lyt = 1 and lzt = lze = lye = 0 (checking that lyt = 1 and lye = lzt = 0 is enough)
			fprintf(fi,"lta%i - lyt%i + lye%i + lzt%i >= 0\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// ltt = 1 if lyt = lzt = 1 and lze = lye = 0 
			fprintf(fi,"ltt%i - lyt%i - lzt%i + lye%i + lze%i >= -1\n",a[i][j],a[i][j],a[i][j],a[i][j],a[i][j]);

			// let = 1 if lzt = lyt = lye = 1 and lze = 0 (checking that lye = lzt = 1 and lze = 0 is enough)
			fprintf(fi,"let%i - lzt%i - lye%i + lze%i >= -1\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// lae = 1 if lze = lzt = 1 and lyt = lye = 0 (checking that lze = 1 and lyt = 0 is enough)
			fprintf(fi,"lae%i - lze%i + lyt%i >= 0\n",a[i][j],a[i][j],a[i][j]);

			// lte = 1 if lze = lzt = lyt = 1 and lye = 0 (checking that lze = lyt = 1 and lye = 0 is enough)
			fprintf(fi,"lte%i - lze%i - lyt%i + lye%i >= -1\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// lte = 1 if lze = lzt = lyt = lye = 1 and lye = 0 (checking that lze = lye = 1 is enough)
			fprintf(fi,"lee%i - lze%i - lye%i >= -1\n",a[i][j],a[i][j],a[i][j]);

		}
	}
}

void SubBytesU(int a[4][4]) {
	for(i=0; i < 4; i++) {
		for(j = 0; j < 4; j++) {

			// 
			// Recall that 1 >= uy >= uyt >= uye >= 0
			//             1 >= uy >= uzt >= uze >= 0
			// Either uy = 1 and
			//uyt uzt uye  uze Variable    P_fromPlaintext  P_fromCiphertext    Set as impossible
			// 0   0   0    0  uaa         2^{-12}          2^{-12} 
			// 0   1   0    0  uat         2^{-8}           1                
			// 0   1   0    1  uae         2^{-8}           2^{-8}
			// 1   0   0    0  uta         2^{-8}           2^{-16}
			// 1   1   0    0  utt         1                1
			// 1   1   0    1  ute         1                2^{-8}
			// 1   1   1    1  uee         2^{-7}           2^{-7}                 
			// 1   0   1    0  uea         2^{-15}          2^{-14}                 
			// 1   1   1    0  uet         2^{-8}           1                   

			// Or uy = 0


			// Following condition, exactly one of the above cases is true
			fprintf(fi,"uaa%i + uat%i + uae%i + uta%i + utt%i + ute%i + uee%i + uet%i + uea%i - uy%i = 0\n",a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j],a[i][j]);

			// Now if a variable is forced to take the value 1 in one condition, all other variables are set to 0 automatically 

			// uaa = 1 if uy = 1 and all 4 variables uyt uzt uye uze are 0 (checking that uy=1 and uyt=uzt=0 is enough)
			fprintf(fi,"uaa%i - uy%i + uyt%i + uzt%i >= 0\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// uat = 1 if uzt = 1 and uyt = uye = uze = 0 (checking that uzt = 1 and uze=uyt=0 is enough)
			fprintf(fi,"uat%i - uzt%i + uyt%i + uze%i >= 0\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// uae = 1 if uze = uzt = 1 and uyt = uye = 0 (checking that uze = 1 and uyt = 0 is enough)
			fprintf(fi,"uae%i - uze%i + uyt%i >= 0\n",a[i][j],a[i][j],a[i][j]);

			// uta = 1 if uyt = 1 and uzt = uze = uye = 0 (checking that uyt = 1 and uye = uzt = 0 is enough)
			fprintf(fi,"uta%i - uyt%i + uye%i + uzt%i >= 0\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// utt = 1 if uyt = uzt = 1 and uze = uye = 0 
			fprintf(fi,"utt%i - uyt%i - uzt%i + uye%i + uze%i >= -1\n",a[i][j],a[i][j],a[i][j],a[i][j],a[i][j]);

			// ute = 1 if uyt = uzt = uze = 1 and uye = 0 (checking that uze = uyt = 1 and uye = 0 is enough)
			fprintf(fi,"ute%i - uyt%i - uze%i + uye%i >= -1\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			// uee = 1 if uye = uze = uyt = uzt = 1 (checking that uze = uye = 1 is enough)
			fprintf(fi,"uee%i - uze%i - uye%i >= -1\n",a[i][j],a[i][j],a[i][j]);

			// uea = 1 if uye = uyt 1 and uze = uzt = 0 (checking that uye = 1 and uzt = 0 is enough)
			fprintf(fi,"uea%i - uye%i + uzt%i >= 0\n",a[i][j],a[i][j],a[i][j]);

			// uet = 1 if uye = uyt = uzt = 1 and uze = 0 (checking that uye = uzt = 1 and uze = 0 is enough)
			fprintf(fi,"uet%i - uye%i - uzt%i + uze%i >= -1\n",a[i][j],a[i][j],a[i][j],a[i][j]);

			

		}
	}
}

void MixColumnL(int a[4][4]) {
	for(j = 0; j < 4; j++) {
		for (i = 0; i < 4; i++) fprintf(fi,"ly%i + ",a[i][j]);
		for (i = 0; i < 3; i++) fprintf(fi,"lx%i + ",lnext+i);
		fprintf(fi,"lx%i - 5 ld%i >= 0\n",lnext+3,ldummy);

		for(i = 0; i < 4; i++)
			fprintf(fi,"ld%i - ly%i >= 0\n",ldummy,a[i][j]);
		for(i = 0; i < 4; i++)
			fprintf(fi,"ld%i - lx%i >= 0\n",ldummy,lnext+i);
		///////////////////////////////////////////
		// Caclutating consumed degrees          //
		fprintf(fi,"4 ld%i - lx%i - lx%i - lx%i - lx%i - le%i = 0\n",ldummy,lnext,lnext+1,lnext+2,lnext+3,ldummy_e);
		fprintf(fi,"lcolumn_deg%i + ly%i + ly%i + ly%i + ly%i - le%i - lcancel%i - lcancel%i - lcancel%i - lcancel%i >= 0\n", lcolumn_deg,a[0][j],a[1][j],a[2][j],a[3][j],ldummy_e,lnext,lnext+1,lnext+2,lnext+3);
		fprintf(fi,"lcolumn_deg%i >= 0\n",lcolumn_deg);
		///////////////////////////////////////////


		// Truncated constraints
		// 1. 0 or at least 5 bytes of input and output columns should be truncated


		for(i = 0; i < 4; i++)
			fprintf(fi,"ldt%i - lxt%i + lxe%i >= 0\n",ldummy,lnext+i,lnext+i);
		for(i = 0; i < 4; i++)
			fprintf(fi,"ldt%i - lzt%i + lze%i >= 0\n",ldummy,a[i][j],a[i][j]);

		fprintf(fi,"8 ldt%i - lxt%i - lxt%i - lxt%i - lxt%i - lzt%i - lzt%i - lzt%i - lzt%i + lxe%i + lxe%i + lxe%i + lxe%i + lze%i + lze%i + lze%i + lze%i  <= 3\n",ldummy,lnext,lnext+1,lnext+2,lnext+3,a[0][j],a[1][j],a[2][j],a[3][j],lnext,lnext+1,lnext+2,lnext+3,a[0][j],a[1][j],a[2][j],a[3][j]);

		fprintf(fi,"8 ldt%i - lxt%i - lxt%i - lxt%i - lxt%i - lzt%i - lzt%i - lzt%i - lzt%i + lxe%i + lxe%i + lxe%i + lxe%i + lze%i + lze%i + lze%i + lze%i - lmct%i = 0\n",ldummy,lnext,lnext+1,lnext+2,lnext+3,a[0][j],a[1][j],a[2][j],a[3][j],lnext,lnext+1,lnext+2,lnext+3,a[0][j],a[1][j],a[2][j],a[3][j], ldummy);


		fprintf(fi,"4 ldt%i - lzt%i - lzt%i - lzt%i - lzt%i + lze%i + lze%i + lze%i + lze%i - lmcat%i = 0\n",ldummy,a[0][j],a[1][j],a[2][j],a[3][j],a[0][j],a[1][j],a[2][j],a[3][j], ldummy);

		// Equal constraints
		// 1. 0 or at least 5 bytes of input and ouput columns should be 
		for(i = 0; i < 4; i++)
			fprintf(fi,"lde%i - lxe%i >= 0\n",ldummy,lnext+i);
		for(i = 0; i < 4; i++)
			fprintf(fi,"lde%i - lze%i >= 0\n",ldummy,a[i][j]);

		fprintf(fi,"8 lde%i - lxe%i - lxe%i - lxe%i - lxe%i - lze%i - lze%i - lze%i - lze%i  <= 3\n",ldummy,lnext,lnext+1,lnext+2,lnext+3,a[0][j],a[1][j],a[2][j],a[3][j]);

		fprintf(fi,"8 lde%i - lxe%i - lxe%i - lxe%i - lxe%i - lze%i - lze%i - lze%i - lze%i  - lmce%i = 0\n",ldummy,lnext,lnext+1,lnext+2,lnext+3,a[0][j],a[1][j],a[2][j],a[3][j], ldummy);

		fprintf(fi,"4 lde%i - lze%i - lze%i - lze%i - lze%i  - lmcae%i = 0\n",ldummy,a[0][j],a[1][j],a[2][j],a[3][j], ldummy);


		// Little help to the MILP Solver

		fprintf(fi,"ldt%i + lde%i <= 1\n",ldummy,ldummy);
		

		for(i = 0; i < 4; i++) a[i][j]=lnext++;
		lcolumn_deg++;
		ldummy++;
		ldummy_e++;
	}
}

void MixColumnU(int a[4][4]) {
	for(j = 0; j < 4; j++) {
		for (i = 0; i < 4; i++) fprintf(fi,"uy%i + ",a[i][j]);
		for (i = 0; i < 3; i++) fprintf(fi,"ux%i + ",unext+i);
		fprintf(fi,"ux%i - 5 ud%i >= 0\n",unext+3,udummy);

		for(i = 0; i < 4; i++)
			fprintf(fi,"ud%i - uy%i >= 0\n",udummy,a[i][j]);

		for(i = 0; i < 4; i++)
			fprintf(fi,"ud%i - ux%i >= 0\n",udummy,unext+i);
		///////////////////////////////////////////
		// Caclutating consumed degrees          //
		fprintf(fi,"4 ud%i - ux%i - ux%i - ux%i - ux%i - ue%i = 0\n",udummy,unext,unext+1,unext+2,unext+3,udummy_e);
		fprintf(fi,"ucolumn_deg%i + uy%i + uy%i + uy%i + uy%i - ue%i - ucancel%i - ucancel%i - ucancel%i - ucancel%i >= 0\n", ucolumn_deg,a[0][j],a[1][j],a[2][j],a[3][j],udummy_e,unext,unext+1,unext+2,unext+3);
		fprintf(fi,"ucolumn_deg%i >= 0\n",ucolumn_deg);
		///////////////////////////////////////////


		// Truncated constraints
		// 1. 0 or at least 5 bytes of input and output columns should be truncated


		for(i = 0; i < 4; i++)
			fprintf(fi,"udt%i - uxt%i + uxe%i >= 0\n",udummy,unext+i,unext+i);
		for(i = 0; i < 4; i++)
			fprintf(fi,"udt%i - uzt%i + uze%i >= 0\n",udummy,a[i][j],a[i][j]);

		fprintf(fi,"8 udt%i - uxt%i - uxt%i - uxt%i - uxt%i - uzt%i - uzt%i - uzt%i - uzt%i + uxe%i + uxe%i + uxe%i + uxe%i + uze%i + uze%i + uze%i + uze%i <= 3\n",udummy,unext,unext+1,unext+2,unext+3,a[0][j],a[1][j],a[2][j],a[3][j],unext,unext+1,unext+2,unext+3,a[0][j],a[1][j],a[2][j],a[3][j]);

		fprintf(fi,"8 udt%i - uxt%i - uxt%i - uxt%i - uxt%i - uzt%i - uzt%i - uzt%i - uzt%i + uxe%i + uxe%i + uxe%i + uxe%i + uze%i + uze%i + uze%i + uze%i  - umct%i = 0\n",udummy,unext,unext+1,unext+2,unext+3,a[0][j],a[1][j],a[2][j],a[3][j],unext,unext+1,unext+2,unext+3,a[0][j],a[1][j],a[2][j],a[3][j],udummy);

		fprintf(fi,"4 udt%i - uxt%i - uxt%i - uxt%i - uxt%i + uxe%i + uxe%i + uxe%i + uxe%i - umcat%i = 0\n",udummy,unext,unext+1,unext+2,unext+3,unext,unext+1,unext+2,unext+3,udummy);


		// Equal constraints
		// 1. 0 or at least 5 bytes of input and ouput columns should be in the equal state

		for(i = 0; i < 4; i++)
			fprintf(fi,"ude%i - uxe%i >= 0\n",udummy,unext+i);
		for(i = 0; i < 4; i++)
			fprintf(fi,"ude%i - uze%i >= 0\n",udummy,a[i][j]);

		fprintf(fi,"8 ude%i - uxe%i - uxe%i - uxe%i - uxe%i - uze%i - uze%i - uze%i - uze%i <= 3\n",udummy,unext,unext+1,unext+2,unext+3,a[0][j],a[1][j],a[2][j],a[3][j]);

		fprintf(fi,"8 ude%i - uxe%i - uxe%i - uxe%i - uxe%i - uze%i - uze%i - uze%i - uze%i - umce%i = 0\n",udummy,unext,unext+1,unext+2,unext+3,a[0][j],a[1][j],a[2][j],a[3][j], udummy);

		fprintf(fi,"4 ude%i - uxe%i - uxe%i - uxe%i - uxe%i - umcae%i = 0\n",udummy,unext,unext+1,unext+2,unext+3, udummy);


		for(i = 0; i < 4; i++) a[i][j]=unext++;
		ucolumn_deg++;
		udummy++;
		udummy_e++;
	}
}

void AddTweakeyS(int a[4][4]){
	for(j=0; j < 4; j++){
		for(i = 0; i < 4; i++){
			// ATK: doesn't change anything
			fprintf(fi,"  uxswitched%i - uyswitched%i = 0\n",a[i][j],a[i][j]);
			fprintf(fi,"  lxswitched%i - lyswitched%i = 0\n",a[i][j],a[i][j]);	
		}
	}
}

void SubBytesS(int a[4][4]){
	// SubBytes: anything can happen, it is only a matter of probability computation now

	for(j=0; j < 4; j++){
		for(i = 0; i < 4; i++){
			int l = a[i][j];
			int u = a[i][j] + 16*(ru-1);

			//lyt lzt lye lze Variable    P_fromPlaintext  P_fromCiphertext    Set as impossible
			// 0   0   0   0  laa         2^{-12}          2^{-12} 
			// 0   1   0   0  lat         2^{-16}          2^{-8}     
			// *   1   *   1  lze                                              yes
			// 1   0   0   0  lta         1                2^{-8}
			// 1   0   1   0  lea         2^{-8}           2^{-8}
			// 1   1   0   0  ltt         1                1
			// 1   1   1   0  let         2^{-8}           1

			// probaswitch = 0 in everycase except:
			// If the transition does not happen and uswitched = 0 and u** = true, then u**noswitch = true (u** being any variable (uat, uta ute ...))
			// If the transition does not happen and lswitched = 0 and l** = true, then l**noswitch = true (l** being any variable (lat, lta lte ...))
			// If the transition happens : summed up in the table beneath
			//
			// Therefore, we will create variables which equal 1 in each of those cases only.
			//
			// uyswitched uzswitched uy uyt uzt uye uze ly lyt lzt lye lze  variable     P_fromPlaintext  P_fromCiphertext  Impossible
			// *          *           0  0   0   0   0   0  0   0   0   0   nanaswitch   1                1
			// 1          0           *  *   *   *   *   *  *   *   *   *   ---          ---              ---                yes
			// 0          0           1  0   0   0   0   *  *   *   *   *   uaanoswitch  2^{-12}          2^{-12} 
			// 0          0           1  0   1   0   0   *  *   *   *   *   uatnoswitch  2^{-8}           1
			// 0          0           1  0   1   0   1   *  *   *   *   *   uaenoswitch  2^{-7}           2^{-7}
			// 0          0           1  1   0   0   0   *  *   *   *   *   utanoswitch  2^{-8}           2^{-16}
			// 0          0           1  1   1   0   0   *  *   *   *   *   uttnoswitch  1                1 
			// 0          0           1  1   1   0   1   *  *   *   *   *   utenoswitch  1                2^{-8}
			// 0          0           1  1   0   1   0   *  *   *   *   *   ueanoswitch  2^{-15}          2^{-14}      NEW
			// 0          0           1  1   0   1   1   *  *   *   *   *   uetnoswitch  2^{-8}           1            NEW
			// 0          0           1  1   1   1   1   *  *   *   *   *   ueenoswitch  2^{-7}           2^{-7}       NEW
			// 0          0           1  1   *   1   *   *  *   *   *   *   uyenoswitch  ---              ---          OLD   yes 
			// 1          1           *  *   *   *   *   1  0   0   0   0   laanoswitch  2^{-12}          2^{-12} 
			// 1          1           *  *   *   *   *   1  0   1   0   0   latnoswitch  2^{-16}          2^{-8}  
			// 1          1           *  *   *   *   *   1  1   1   1   1   leenoswitch  2^{-7}           2^{-7}       NEW  
			// 1          1           *  *   *   *   *   1  1   1   0   1   ltenoswitch  1                2^{-8}       NEW
			// 1          1           *  *   *   *   *   1  0   1   0   1   laenoswitch  2^{-14}          2^{-15}      NEW
			// 1          1           *  *   *   *   *   1  *   1   *   1   lzenoswitch  ---              ---          OLD   yes
			// 1          1           *  *   *   *   *   1  1   0   0   0   ltanoswitch  1                2^{-8}
			// 1          1           *  *   *   *   *   1  1   0   1   0   leanoswitch  2^{-7}           2^{-7}
			// 1          1           *  *   *   *   *   1  1   1   0   0   lttnoswitch  1                1
			// 1          1           *  *   *   *   *   1  1   1   1   0   letnoswitch  2^{-8}           1		
			// 0          1           1  0   *   0   *   0  *   0   *   0   anaswitch    1                1
			// 0          1           1  1   *   0   *   0  *   0   *   0   tnaswitch    1                2^{-8}
			// 0          1           1  1   *   1   *   0  *   0   *   0   enaswitch    1                1
			// 0          1           0  0   *   0   *   1  *   0   *   0   naaswitch    1                1
			// 0          1           0  0   *   0   *   1  *   1   *   0   natswitch    2^{-8}           1 
			// 0          1           0  0   *   0   *   1  *   1   *   1   naeswitch    1                1
			// 0          1           1  0   *   0   *   1  *   0   *   0   aaswitch     2^{-6}           2^{-6}
			// 0          1           1  0   *   0   *   1  *   1   *   0   atswitch     2^{-8}           1
			// 0          1           1  0   *   0   *   1  *   1   *   1   aeswitch     2^{-8}           2^{-8}
			// 0          1           1  1   *   0   *   1  *   0   *   0   taswitch     1                2^{-8}
			// 0          1           1  1   *   0   *   1  *   1   *   0   ttswitch     1                1
			// 0          1           1  1   *   0   *   1  *   1   *   1   teswitch     1                2^{-8}
			// 0          1           1  1   *   1   *   1  *   0   *   0   easwitch     2^{-8}           2^{-8}
			// 0          1           1  1   *   1   *   1  *   1   *   0   etswitch     2^{-8}           1
			// 0          1           1  1   *   1   *   1  *   1   *   1   eeswitch     2^{-8}           2^{-8}
  


			// The switch can not be done in the wrong way
			fprintf(fi,"uyswitched%i - uzswitched%i <= 0\n",l,l);
			       
			fprintf(fi,"nanaswitch%i + uaanoswitch%i + uatnoswitch%i + uaenoswitch%i + utanoswitch%i + uttnoswitch%i + utenoswitch%i + uyenoswitch%i + laanoswitch%i + latnoswitch%i + lzenoswitch%i + ltanoswitch%i + leanoswitch%i + lttnoswitch%i + letnoswitch%i + anaswitch%i + tnaswitch%i + enaswitch%i + naaswitch%i + natswitch%i + naeswitch%i + aaswitch%i + atswitch%i + aeswitch%i + taswitch%i + ttswitch%i + teswitch%i + easwitch%i + etswitch%i + eeswitch%i = 1\n",l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l,l);

			//nanaswitch
			fprintf(fi,"nanaswitch%i + uy%i + ly%i >= 1\n",l,u,l);

			//upper noswitch

			//uaanoswitch
			fprintf(fi,"uaanoswitch%i + uyswitched%i + uzswitched%i - uaa%i >= 0\n",l,l,l,u);
			//uatnoswitch
			fprintf(fi,"uatnoswitch%i + uyswitched%i + uzswitched%i - uat%i >= 0\n",l,l,l,u);
            //uaenoswitch
			fprintf(fi,"uaenoswitch%i + uyswitched%i + uzswitched%i - uae%i >= 0\n",l,l,l,u);
            //utanoswitch
			fprintf(fi,"utanoswitch%i + uyswitched%i + uzswitched%i - uta%i >= 0\n",l,l,l,u);
            //uttnoswitch
			fprintf(fi,"uttnoswitch%i + uyswitched%i + uzswitched%i - utt%i >= 0\n",l,l,l,u);
            //utenoswitch
			fprintf(fi,"utenoswitch%i + uyswitched%i + uzswitched%i - ute%i >= 0\n",l,l,l,u);
            //ueenoswitch
			fprintf(fi,"ueenoswitch%i + uyswitched%i + uzswitched%i - uee%i >= 0\n",l,l,l,u);
			//uetnoswitch
			fprintf(fi,"uetnoswitch%i + uyswitched%i + uzswitched%i - uet%i >= 0\n",l,l,l,u);
			//ueanoswitch
			fprintf(fi,"ueanoswitch%i + uyswitched%i + uzswitched%i - uea%i >= 0\n",l,l,l,u);

			//lower noswitch

			//laanoswitch
			fprintf(fi,"laanoswitch%i + lyswitched%i + lzswitched%i - laa%i >= 0\n",l,l,l,l);
            //latnoswitch
			fprintf(fi,"latnoswitch%i + lyswitched%i + lzswitched%i - lat%i >= 0\n",l,l,l,l);
            //laenoswitch
			fprintf(fi,"laenoswitch%i + lyswitched%i + lzswitched%i - lae%i >= 0\n",l,l,l,l);
			//leenoswitch
			fprintf(fi,"leenoswitch%i + lyswitched%i + lzswitched%i - lee%i >= 0\n",l,l,l,l);
			//ltenoswitch
			fprintf(fi,"ltenoswitch%i + lyswitched%i + lzswitched%i - lte%i >= 0\n",l,l,l,l);
            //ltanoswitch
			fprintf(fi,"ltanoswitch%i + lyswitched%i + lzswitched%i - lta%i >= 0\n",l,l,l,l);
            //leanoswitch
			fprintf(fi,"leanoswitch%i + lyswitched%i + lzswitched%i - lea%i >= 0\n",l,l,l,l);
            //lttnoswitch
			fprintf(fi,"lttnoswitch%i + lyswitched%i + lzswitched%i - ltt%i >= 0\n",l,l,l,l);
            //letnoswitch
			fprintf(fi,"letnoswitch%i + lyswitched%i + lzswitched%i - let%i >= 0\n",l,l,l,l);

			// switch active: add uyswitched + lzswitched to the equation >= 0

			//anaswitch
			fprintf(fi,"anaswitch%i + uyswitched%i + lzswitched%i + ly%i - uy%i + uyt%i >= 0\n",l,l,l,l,u,u);
			//tnaswitch
			fprintf(fi,"tnaswitch%i + uyswitched%i + lzswitched%i + ly%i - uyt%i + uye%i >= 0\n",l,l,l,l,u,u);
			//enaswitch
			fprintf(fi,"enaswitch%i + uyswitched%i + lzswitched%i + ly%i - uye%i >= 0\n",l,l,l,l,u);
			//naaswitch
			fprintf(fi,"naaswitch%i + uyswitched%i + lzswitched%i - ly%i + lzt%i + uy%i >= 0\n",l,l,l,l,l,u);
			//natswitch
			fprintf(fi,"natswitch%i + uyswitched%i + lzswitched%i - lzt%i + lze%i + uy%i >= 0\n",l,l,l,l,l,u);
			//naeswitch
			fprintf(fi,"naeswitch%i + uyswitched%i + lzswitched%i - lze%i + uy%i >= 0\n",l,l,l,l,u);
			//aaswitch 
			fprintf(fi,"aaswitch%i + uyswitched%i + lzswitched%i - ly%i + lzt%i - uy%i + uyt%i >= -1\n",l,l,l,l,l,u,u);
			//atswitch 
			fprintf(fi,"atswitch%i + uyswitched%i + lzswitched%i - lzt%i + lze%i - uy%i + uyt%i >= -1\n",l,l,l,l,l,u,u);
			//aeswitch 
			fprintf(fi,"aeswitch%i + uyswitched%i + lzswitched%i - lze%i - uy%i + uyt%i >= -1\n",l,l,l,l,u,u);
			//taswitch 
			fprintf(fi,"taswitch%i + uyswitched%i + lzswitched%i - ly%i + lzt%i - uyt%i + uye%i >= -1\n",l,l,l,l,l,u,u);
			//ttswitch 
			fprintf(fi,"ttswitch%i + uyswitched%i + lzswitched%i - lzt%i + lze%i - uyt%i + uye%i >= -1\n",l,l,l,l,l,u,u);
			//teswitch 
			fprintf(fi,"teswitch%i + uyswitched%i + lzswitched%i - lze%i - uyt%i + uye%i >= -1\n",l,l,l,l,u,u);
			//easwitch 
			fprintf(fi,"easwitch%i + uyswitched%i + lzswitched%i - ly%i + lzt%i - uye%i >= -1\n",l,l,l,l,l,u);
			//etswitch 
			fprintf(fi,"etswitch%i + uyswitched%i + lzswitched%i - lzt%i + lze%i - uye%i >= -1\n",l,l,l,l,l,u);
			//eeswitch 
			fprintf(fi,"eeswitch%i + uyswitched%i + lzswitched%i - lze%i - uye%i >= -1\n",l,l,l,l,u);
			
		}
	}
}

void MixColumnS(int a[4][4]){
	for(j = 0; j < 4; j++) {
		// Either the column is fully switched or not switched at all.
		// uzswitched%i = 1 - lzswitched%i therefore no need to do it with lzswitched
		for(i = 0; i < 4; i++)
			fprintf(fi,"switch%i - uzswitched%i = 0\n",j,a[i][j]);
		for(i = 0; i < 4; i++)
			fprintf(fi,"switch%i - uxswitched%i = 0\n",j,snext+i);

		// Depending on the value switch, we need to apply the probability of the upper or the lower trail
		// umctswitch etc... are the equivalent of mct etc... when switch is false on the size.
		//

		fprintf(fi,"umctswitch%i - umct%i + 3 switch%i >= 0\n",j,j+4*(ru-1),j);
		fprintf(fi,"umcatswitch%i - umcat%i + 3 switch%i >= 0\n",j,j+4*(ru-1),j);
		fprintf(fi,"umcaeswitch%i - umcae%i + 3 switch%i >= 0\n",j,j+4*(ru-1),j);
		fprintf(fi,"umceswitch%i - umce%i + 3 switch%i >= 0\n",j,j+4*(ru-1),j);

		fprintf(fi,"lmctswitch%i - lmct%i - 3 switch%i >= -3\n",j,j,j);
		fprintf(fi,"lmcatswitch%i - lmcat%i - 3 switch%i >= -3\n",j,j,j);
		fprintf(fi,"lmcaeswitch%i - lmcae%i - 3 switch%i >= -3\n",j,j,j);
		fprintf(fi,"lmceswitch%i - lmce%i - 3 switch%i >= -3\n",j,j,j);
		
		

		for(i = 0; i < 4; i++) a[i][j]=snext++;
	}
}



int main(int argc, char** argv){
	int tk[4][4]; /* the bytes of the TWEAKEY state */
	int a[4][4]; /* the bytes of the AES state */
	int tmpindex;	
	ru = 5;
	rl = 5;
	// fromPlaintext = 1 if the trail starts from the plaintext and 0 if it starts from the ciphertext
	char fromPlaintext = 0;


	// allowEqualStates enables the optimisation with equal states.
	char allowEqualStates=1;

	//upper trails and lower trails of respectively ru and rl rounds.
	//In order to come up with such trails, we create differential trails of length rl+1 and ru+1
	if(argc>2){
		ru = atoi(argv[1]);
		rl = atoi(argv[2]);
	}else{
		printf("Usage: ./executable U L fP opt \nU is the number of upper rounds, L of lower rounds\nfP is a boolean equal to 1 iff the trail starts from plaintext\nopt enable the equal states optimisation\n");	
		return 0;
		}
	if(argc>3){
		fromPlaintext = atoi(argv[3]);
	}
	if(argc>4){
		allowEqualStates = atoi(argv[4]);
	}

	char* s = "_optimized";
	if(!allowEqualStates){
		s = "";
	}
	
	if(fromPlaintext) sprintf(filename,"LPFiles/Kiasu%s_boomerang_fromPlaintext_%dR_%dR.lp",s,ru,rl);
  	if(!fromPlaintext) sprintf(filename,"LPFiles/Kiasu%s_boomerang_fromCiphertext_%dR_%dR.lp",s,ru,rl);

	
	
	fi=fopen(filename,"wt");

	fprintf(fi,"Minimize\n"); /* print objective function */


	fprintf(fi,"complexity\n");



	fprintf(fi,"Subject To\n"); /* proba def */
	// proba = proba of lower trail * proba of upper trail * proba of the boomerang switch
	// Now, the boomerang switch is done in the first turn of the lower trail (ie r1 +1 th turn)
	// 2^Proba = (2^6)^2 for active but non truncated active Sboxes
	// * 2^8 foreach truncated to non truncated Sbox (or the other way around) in the upper trail
	// * (2^8)^2 foreach active non truncated to truncated Sbox in the lower trail
	// * 2^8 foreach inactive byte after and before a MixColumn operation which is truncatedly active in the upper trail
	// * (2^8)^2 foreach inactive byte before the MixColumn in the lower trail
	
	if(fromPlaintext){
		for (i = 0; i < (ru-1)*16; i++) fprintf(fi,"12 uaa%i + 8 uat%i + 7 uae%i + 8 uta%i + 256 uye%i + ",i,i,i,i,i);
		for (i = 0; i < 4*(ru-1); i++) fprintf(fi,"8 umct%i + 8 umcae%i + ", i,i);

		// In the boomerang switch, see the section Sbox for more detail about the following lines
		
		fprintf(fi,"uswitchproba + ");
		for (i = 32; i < (rl+1)*16; i++) fprintf(fi,"12 laa%i + 16 lat%i + 256 lze%i + 7 lea%i + 8 let%i + ",i,i,i,i,i);
		for (i = 4; i < 4*rl; i++) fprintf(fi,"16 lmcat%i + 8 lmcae%i + ", i, i);
		fprintf(fi,"-1 uproba = 0\n\n");
	}
	


	// The symmetrically opposite for the best truncated trails starting from the ciphertext
	
	if (!fromPlaintext){
		for (i = 0; i < (ru-1)*16; i++) fprintf(fi,"12 uaa%i + 7 uae%i + 16 uta%i + 8 ute%i + 256 uye%i + ",i,i,i,i,i);
		for (i = 0; i < 4*(ru-1); i++) fprintf(fi,"16 umcat%i + 8 umcae%i + ", i,i);
		// In the boomerang switch, see the section Sbox for more detail about the following lines
		fprintf(fi,"lswitchproba + ");

		for (i = 32; i < (rl+1)*16; i++) fprintf(fi,"12 laa%i + 8 lat%i + 256 lze%i + 8 lta%i + 7 lea%i + ",i,i,i,i,i);
		for (i = 4; i < 4*rl; i++) fprintf(fi,"8 lmct%i + 8 lmcae%i + ",i,i);
		
		fprintf(fi,"-1 lproba = 0\n\n");
	}
	

	// Define the structure max size
	for (i = 16*rl; i < 16*rl + 16; i++) fprintf(fi,"8 lzt%i + ",i);
	for (i = 0; i < 16-1; i++) fprintf(fi,"8 uxt%i + ",i);
	fprintf(fi, "8 uxt%i - structsize >= 0\n", 15);

	// The distinguisher should be valid => the proba should be higher than the proba of a random pair
	
	// For the boomerang trail starting 
	//
	if (fromPlaintext){
			// uprand = number of non truncated bits in input = proba of a random quartet collision
	
		for (i = 0; i < 16-1; i++) fprintf(fi,"8 uxt%i + ",i);
		fprintf(fi, "8 uxt%i + uprand = 128\n", 15);
	}

	if (!fromPlaintext){
		// lprand = number of non truncated bits in input = proba of a random quartet collision
		for (i = rl*16; i < rl*16 + 16-1; i++) fprintf(fi,"8 lzt%i + ",i);
		fprintf(fi, "8 lzt%i + lprand = 128\n", rl*16 + 15);
	}

	

	// The filter condition corresponds to the fact that the correct key counter should in average be more than a random key counter during the key recovery
	// Compute the transition probability from the last round to the second to last round and the transition probability from the first to the second round
	// 
	// 2^ll possible states at round ru+rl-1
	// 2^{lk} = 2^{128-prand} possible states at round ru+rl
	// laa/6 active <-> active transitions in the last Sbox
	//
	// 2^{ul} possible states at round 1
	// 2^{uk} possible states at round 0
	// uaa/6 active <-> active transitions in the first Sbox
	//
	// Probability (last round) is computed : p = 2^{ - (2 * laa + (lkt -ll)) }
	// A random key candidate counter is incremented only if both pairs pass the first round with the counter's key candidate.
	// It happens with probability p^2
	//
	// => A random counter is incremented with N = (prand) * 2^{2 ll - 2 lk - 2 laa} * 2^{2 ul - 2 uk - 2 uaa} while a correct counter is incremented proba time
	// The signal-to-noise ratio = N/proba = sigma
	// Complexity = 1/proba * min(1,sigma) 

	//ll
	for (i = 16*(rl-1); i < 16*rl-1; i++) fprintf(fi,"8 lzt%i + ",i);
	fprintf(fi, "8 lzt%i - ll = 0\n", 16*rl-1);
	//lk
	for (i = rl*16; i < rl*16 + 16-1; i++) fprintf(fi,"8 lzt%i + ",i);
	fprintf(fi, "8 lzt%i - lk = 0\n", rl*16 + 15);
	//laa
	for (i = rl*16; i < rl*16 + 16-1; i++) fprintf(fi,"6 laa%i + ",i);
	fprintf(fi, "6 laa%i - laa = 0\n", rl*16 + 15);

	fprintf(fi, "lfilter >= 0\n");
	fprintf(fi, "lfilter - 2 lk + 2 ll - 2 laa >= 0\n");
	fprintf(fi, "lfilter - 255 lfilterbinary <= 0\n");
	fprintf(fi, "lfilter + 255 lfilterbinary - 2 lk + 2 ll - 2 laa <= 255\n");
	// Misses a condition so that lfilter <= max(0, 2 lk - 2 ll)

	//ul
	for (i = 16; i < 31; i++) fprintf(fi,"8 uxt%i + ",i);
	fprintf(fi, "8 uxt%i - ul = 0\n", 31);

	//uaa
	for (i = 0; i < 15; i++) fprintf(fi,"6 uaa%i + ",i);
	fprintf(fi, "6 uaa%i - uaa = 0\n", 15);

	//uk
	for (i = 0; i < 16-1; i++) fprintf(fi,"8 uxt%i + ",i);
	fprintf(fi, "8 uxt%i - uk = 0\n", 15);

	fprintf(fi, "ufilter >= 0\n");
	fprintf(fi, "ufilter - 2 uk + 2 ul - 2 uaa >= 0\n");
	fprintf(fi, "ufilter - 255 ufilterbinary <= 0\n");
	fprintf(fi, "ufilter + 255 ufilterbinary - 2 uk + 2 ul - 2 uaa <= 255\n");

	if(fromPlaintext){
		// sigma = log(min(1,signal-to-noise)) 
		fprintf(fi, "lfilter + ufilter - uproba + uprand - sigma >= 0 \n");
		fprintf(fi, "sigma <= 0 \n");
		// Il faut calculer cxté en data = N = proba - sigma
		// sigma <= 0 fyi
		fprintf(fi, "uproba - sigma - 2 structsize >= 0\n");
		fprintf(fi, "uproba - sigma - structsize - complexity = 0\n");
	}

	if(!fromPlaintext){
		// filter condition
		fprintf(fi, "lfilter + ufilter - lproba + lprand - sigma >= 0 \n");
		fprintf(fi, "sigma <= 0 \n");
		// Il faut calculer cxté en data = N = proba * max(prand/proba,1) 
		fprintf(fi, "lproba - sigma - 2 structsize >= 0\n");
		fprintf(fi, "lproba - sigma - structsize - complexity = 0\n");
	}

	// Constraint to avoid dumb trails: 
	// No MixColumn transition should have 4 active truncated columns.
	// In the middle rounds, if a column is already switch, it costs nothing to put active differences into it.
	// In the middle rounds, we won't allow 4 active truncated columns if the state is not switched.
	// However, we allow the MixColumn at the first/last round to be full truncated, since it can be compen
	for(i = 1; i < (ru + 1) ; i++){
		fprintf(fi, "udt%i + udt%i + udt%i + udt%i <= 3\n", 4*i, 4*i + 1, 4*i + 2, 4*i + 3);
	}
	for(i = 0; i < rl ; i++){
		fprintf(fi, "ldt%i + ldt%i + ldt%i + ldt%i <= 3\n", 4*i, 4*i + 1, 4*i + 2, 4*i + 3);
	}
	
	/* round function constraints */
	/* Constraints for subtweakeys*/
	
	//TK1 model Kiasu
	//upper trail
	for(i=0;i<16;i++){
		if(i%4< 2){
			for (r = 0; r<(ru+1); r++) {
				fprintf(fi,"utk%i - uLANE%i = 0\n",(16*r)+i,i);
			}
		}else{
			for (r = 0; r<(ru+1); r++) {
				fprintf(fi,"utk%i = 0\n",(16*r)+i);
			}
			fprintf(fi,"uLANE%i = 0\n",i);
		}
	}

	// lower trail
	for(i=0;i<16;i++){
		if(i%4< 2){
			for (r = 0; r<(rl+1); r++) {
				fprintf(fi,"ltk%i - lLANE%i = 0\n",(16*r)+i,i);
			}
		}else{
			for (r = 0; r<(rl+1); r++) {
				fprintf(fi,"ltk%i = 0\n",(16*r)+i);
			}
			fprintf(fi,"lLANE%i = 0\n",i);
		}
	}
	
	
	unext=0;
 	udummy=0;
 	udummy_e=0;
 	ucancel=0;
 	ucolumn_deg=0;
 	for (j = 0; j < 4; j++){
 		for (i = 0; i < 4; i++){
 			a[i][j] = unext++; /* initialize variable indices */
 			tk[i][j] = a[i][j];
 		}
 	}
	for (r = 0; r<ru; r++) {
		AddTweakeyU(a,tk);
		ShuffleKey(tk);
		SubBytesU(a);
		ShiftRows(a);
		MixColumnU(a);
	}
	AddTweakeyU(a,tk);
	SubBytesU(a);

	//fprintf(fi,"LANE%i = %d\n",16-1,active_num);
	//fprintf(fi,"LANE%i = 1\n\n",16-1);
	
	//x-xt>=0
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"ux%i - uxt%i >= 0\n",i,i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uy%i - uyt%i >= 0\n",i,i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uy%i - uzt%i >= 0\n",i,i);

	//xt-xe >=0
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uxt%i - uxe%i >= 0\n",i,i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uyt%i - uye%i >= 0\n",i,i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uzt%i - uze%i >= 0\n",i,i);

	if(!allowEqualStates){
		// Disable the equal states optimisation: set the values u*e to 0 and let the solver remove all the useless equations.
		for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uxe%i = 0\n",i);
		for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uye%i = 0\n",i);
		for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uze%i = 0\n",i);
	}

	lnext=0;
	ldummy=0;
	ldummy_e=0;
	lcancel=0;
	lcolumn_deg=0;
	for (j = 0; j < 4; j++){
		for (i = 0; i < 4; i++){
			a[i][j] = lnext++; /* initialize variable indices */
			tk[i][j] = a[i][j];
		}
	}

	for (r = 0; r<rl; r++) {
		AddTweakeyL(a,tk);
		ShuffleKey(tk);
		SubBytesL(a);
		ShiftRows(a);
		MixColumnL(a);
	}
	AddTweakeyL(a,tk);
	SubBytesL(a);

	//fprintf(fi,"LANE%i = %d\n",16-1,active_num);
	//fprintf(fi,"LANE%i = 1\n\n",16-1);
	
	// constraints on freedom degrees for canceling difference
	// available freedom degrees: model*(Sum of LANE) 
	// consuming degrees for state diff cancelation: Sum of column_deg
	// consuming degrees for key schedule: ROUNDS*(Sum of LANE) - (Sum of tk)
	// Constraints:
	//      model*(Sum of LANE) >= (Sum of cancel) + ROUNDS*(Sum of LANE) - (Sum of tk)
	// ==>  (model-ROUNDS)*(Sum of LANE) - (Sum of canceli) + (Sum of tk) >= 0

	

	//x-xt>=0
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lx%i - lxt%i >= 0\n",i,i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"ly%i - lyt%i >= 0\n",i,i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"ly%i - lzt%i >= 0\n",i,i);

	//x-xe>=0
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lxt%i - lxe%i >= 0\n",i,i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lyt%i - lye%i >= 0\n",i,i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lzt%i - lze%i >= 0\n",i,i);

	if(!allowEqualStates){
		// Disable the equal states optimisation: set the values l*e to 0 and let the solver remove all the useless equations.
		for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lxe%i = 0\n",i);
		for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lye%i = 0\n",i);
		for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lze%i = 0\n",i);
	}

	
	// Either the lower or the upper trail is switched.
	for(i=0; i < 32; i++){
		fprintf(fi,"uzswitched%i + lzswitched%i = 1\n",i,i);
		fprintf(fi,"uyswitched%i + lyswitched%i = 1\n",i,i);
		fprintf(fi,"uxswitched%i + lxswitched%i = 1\n",i,i);
	}

	// The transition starts at round (ru-1) and finishes at round 2 of the lower trail (= ru+1 of the upper trail) 

	
	for(i=0; i < 16; i++){

		// It starts at x of round (ru-1)
		fprintf(fi,"uxswitched%i = 0\n",i);
		// It finishes at z of round (ru+1)
		fprintf(fi,"lzswitched%i = 0\n",i + 16);
		
	}

	// Transition laws of the switched values 
	snext = 0;
	for(j=0; j < 4; j++){
		for(i = 0; i < 4; i++){
			a[i][j] = snext++;
		}
	}

	

	AddTweakeyS(a);
	SubBytesS(a);
	ShiftRows(a);
	MixColumnS(a);
	AddTweakeyS(a);
	SubBytesS(a);

	if(fromPlaintext) {
		// Boomerang switch probability when starting from the plaintext
		for (i = 0; i < 32; i++) fprintf(fi,"12 uaanoswitch%i + 8 uatnoswitch%i + 7 uaenoswitch%i + 8 utanoswitch%i + 15 ueanoswitch%i + 7 ueenoswitch%i + 8 uetnoswitch%i + 12 laanoswitch%i + 16 latnoswitch%i + 7 leenoswitch%i + 14 laenoswitch%i + 7 leanoswitch%i + 8 letnoswitch%i + 8 natswitch%i + 6 aaswitch%i + 8 atswitch%i + 8 aeswitch%i + 8 easwitch%i + 8 etswitch%i + 8 eeswitch%i + ",i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i);
		// MixColumn Probabilities
		for(i = 0; i < 4; i++) fprintf(fi,"16 lmcatswitch%i + 8 umctswitch%i + 8 umcaeswitch%i + 8 lmcaeswitch%i + ",i,i,i,i);
		fprintf(fi,"-1 uswitchproba = 0\n");
	}
	

	if(!fromPlaintext) {
		// Boomerang switch probability when starting from the ciphertext
		for (i = 0; i < 32; i++) fprintf(fi,"12 uaanoswitch%i + 7 uaenoswitch%i + 16 utanoswitch%i + 8 utenoswitch%i + 14 ueanoswitch%i + 7 ueenoswitch%i + 12 laanoswitch%i + 8 latnoswitch%i + 15 laenoswitch%i + 7 leenoswitch%i + 8 ltenoswitch%i + 8 ltanoswitch%i + 7 leanoswitch%i + 8 tnaswitch%i + 6 aaswitch%i + 8 aeswitch%i + 8 taswitch%i + 8 teswitch%i + 8 easwitch%i + 8 eeswitch%i + ",i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i);
		// MixColumn Probabilities
		for(i = 0; i < 4; i++) fprintf(fi,"8 lmctswitch%i + 16 umcatswitch%i + 8 lmcaeswitch%i + 8 umcaeswitch%i + ",i,i,i,i);
		fprintf(fi,"-1 lswitchproba = 0\n");
	}

	

	// TKi model
	/* The tweakey must be active */
	for (i = 0; i < 16-1; i++) fprintf(fi,"uLANE%i + ",i);
	fprintf(fi,"uLANE%i >= 1\n\n",16-1);

	for (i = 0; i < 16-1; i++) fprintf(fi,"lLANE%i + ",i);
	fprintf(fi,"lLANE%i >= 1\n\n",16-1);

	



	// Help the MILP:
	
	for (i = 0; i<16*(ru-1); i++) {
		fprintf(fi,"uxe%i = 0\n",i);
		fprintf(fi,"uye%i = 0\n",i);
		if (i<16*(ru-2)) fprintf(fi,"uze%i = 0\n",i);
	}

	for (i = 32; i<16*(rl+1); i++) {
		if (i>=48) fprintf(fi,"lxe%i = 0\n",i);
		if (i>=48) fprintf(fi,"lye%i = 0\n",i);
		fprintf(fi,"lze%i = 0\n",i);
	}

	
	fprintf(fi,"Bounds\n");
	for (i = 0; i < lcolumn_deg; i++) fprintf(fi,"0 <= lcolumn_deg%i <= 7\n",i);
	for (i = 0; i < ldummy_e; i++) fprintf(fi,"0 <= le%i <= 4\n",i);
	for (i = 0; i < ldummy; i++) fprintf(fi,"0 <= lmct%i <= 3\n",i);
	for (i = 0; i < ldummy; i++) fprintf(fi,"0 <= lmcat%i <= 3\n",i);
	fprintf(fi,"0 <= lfreedom <= 48\n\n");

	for (i = 0; i < ucolumn_deg; i++) fprintf(fi,"0 <= ucolumn_deg%i <= 7\n",i);
	for (i = 0; i < udummy_e; i++) fprintf(fi,"0 <= ue%i <= 4\n",i);
	for (i = 0; i < udummy; i++) fprintf(fi,"0 <= umct%i <= 3\n",i);
	for (i = 0; i < udummy; i++) fprintf(fi,"0 <= umcat%i <= 3\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"0 <= lmcatswitch%i <= 3\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"0 <= umcatswitch%i <= 3\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"0 <= lmctswitch%i <= 3\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"0 <= umctswitch%i <= 3\n\n",i);

	fprintf(fi,"0 <= structsize <= 128\n\n");

	fprintf(fi,"0 <= ufreedom <= 48\n\n");
	
	fprintf(fi,"0 <= complexity <= 256\n\n");

	fprintf(fi,"0 <= lfilter <= 255\n\n");
	fprintf(fi,"0 <= ufilter <= 255\n\n");

	if(fromPlaintext){
		fprintf(fi,"0 <= uprand <= 128\n\n");
		fprintf(fi,"0 <= uproba <= 384\n\n");

	}


	if(!fromPlaintext){
		fprintf(fi,"0 <= lprand <= 128\n\n");
		fprintf(fi,"0 <= lproba <= 384\n\n");
	}
	

	fprintf(fi,"\nGeneral\n"); /* integer constraints */
	for (i = 0; i < lcolumn_deg; i++) fprintf(fi,"lcolumn_deg%i\n",i);
	for (i = 0; i < lcolumn_deg; i++) fprintf(fi,"le%i\n",i);
	for (i = 0; i < ldummy; i++) fprintf(fi,"lmct%i\n",i);
	for (i = 0; i < ldummy; i++) fprintf(fi,"lmcat%i\n",i);
	for (i = 0; i < ldummy; i++) fprintf(fi,"lmce%i\n",i);
	for (i = 0; i < ldummy; i++) fprintf(fi,"lmcae%i\n",i);
	for (i = 0; i < ucolumn_deg; i++) fprintf(fi,"ucolumn_deg%i\n",i);
	for (i = 0; i < ucolumn_deg; i++) fprintf(fi,"ue%i\n",i);
	for (i = 0; i < udummy; i++) fprintf(fi,"umct%i\n",i);
	for (i = 0; i < udummy; i++) fprintf(fi,"umcat%i\n",i);
	for (i = 0; i < udummy; i++) fprintf(fi,"umce%i\n",i);
	for (i = 0; i < udummy; i++) fprintf(fi,"umcae%i\n",i);
	
	fprintf(fi,"structsize\n\n");
	fprintf(fi,"ufilter\n\n");
	fprintf(fi,"lfilter\n\n");
	
	if(fromPlaintext){
		fprintf(fi,"uprand\n\n");
		fprintf(fi,"uproba\n\n");
		fprintf(fi,"uswitchproba\n\n");
	
	}
	if(!fromPlaintext){
		fprintf(fi,"lproba\n\n");
		fprintf(fi,"lprand\n\n");
		fprintf(fi,"lswitchproba\n\n");
	}

	fprintf(fi,"laa\n\n");
	fprintf(fi,"lk\n\n");
	fprintf(fi,"ll\n\n");
	fprintf(fi,"uaa\n\n");
	fprintf(fi,"uk\n\n");
	fprintf(fi,"ul\n\n");

	fprintf(fi,"sigma\n\n");

	for (i = 0; i < 4; i++) fprintf(fi,"lmcatswitch%i\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"umcatswitch%i\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"lmctswitch%i\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"umctswitch%i\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"lmcaeswitch%i\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"umcaeswitch%i\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"lmceswitch%i\n\n",i);
	for (i = 0; i < 4; i++) fprintf(fi,"umceswitch%i\n\n",i);
	
	fprintf(fi,"complexity\n\n");
	
	fprintf(fi,"\nBinary\n"); /* binary constraints */

	fprintf(fi,"ufilterbinary\n\n");
	fprintf(fi,"lfilterbinary\n\n");

	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lx%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"ly%i\n",i);

	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"ux%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uy%i\n",i);
	//Truncated 0 <= xt <= x, xt = 1 <=> xt is truncated
	// 0 <= yt <= y; yt = 1 <=> yt is truncated 
	// 0 <= zt <= y; zt = 1 <=> the value after the Sbox is truncated
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lxt%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lyt%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lzt%i\n",i);

	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uxt%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uyt%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uzt%i\n",i);
	
	// All the SubBytes cases
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uaa%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uat%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uae%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uta%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"utt%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"ute%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uee%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uea%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uet%i\n",i);

	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"laa%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lat%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lta%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lea%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"ltt%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"let%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lae%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lee%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lte%i\n",i);
	
	
	
	
	
	

	//Equal : 0 <= xe <= xt. xe = 1 <=> The difference in xe is any difference BUT the same between both pairs of the quartet.
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lxe%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lye%i\n",i);
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"lze%i\n",i);

	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uxe%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uye%i\n",i);
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"uze%i\n",i);


	for (i = 0; i < ldummy; i++) fprintf(fi,"ldt%i\n",i);
	for (i = 0; i < ldummy; i++) fprintf(fi,"ld%i\n",i);
	for (i = 0; i < ldummy; i++) fprintf(fi,"lde%i\n",i);
	for (i = 0; i < lcancel; i++) fprintf(fi,"lcancel%i\n",i);
	for (i = 0; i < 16; i++) fprintf(fi,"lLANE%i\n",i);

	
	for (i = 0; i < (rl+1)*16; i++) fprintf(fi,"ltk%i\n",i);

	for (i = 0; i < udummy; i++) fprintf(fi,"udt%i\n",i);
	for (i = 0; i < udummy; i++) fprintf(fi,"ud%i\n",i);
	for (i = 0; i < udummy; i++) fprintf(fi,"ude%i\n",i);
	for (i = 0; i < ucancel; i++) fprintf(fi,"ucancel%i\n",i);

	for (i = 0; i < 16; i++) fprintf(fi,"uLANE%i\n",i);

	
	for (i = 0; i < (ru+1)*16; i++) fprintf(fi,"utk%i\n",i);

	for (i = 0; i < 32; i++) fprintf(fi,"uxswitched%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"lxswitched%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"uzswitched%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"lzswitched%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"uyswitched%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"lyswitched%i\n",i);

	for (i = 0; i < 32; i++) fprintf(fi,"nanaswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"uaanoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"uatnoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"uaenoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"utanoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"uttnoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"utenoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"ueanoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"uetnoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"ueenoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"laanoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"latnoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"laenoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"leenoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"ltenoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"ltanoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"leanoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"lttnoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"letnoswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"anaswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"tnaswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"enaswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"naaswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"natswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"naeswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"aaswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"atswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"aeswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"taswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"ttswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"teswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"easwitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"etswitch%i\n",i);
	for (i = 0; i < 32; i++) fprintf(fi,"eeswitch%i\n",i);

	for (i = 0; i < 4; i++) fprintf(fi,"switch%i\n",i);
	fprintf(fi,"End\n");

	fclose(fi);
	
	printf("LP file generated in %s\n", filename);
	return 0;
}

