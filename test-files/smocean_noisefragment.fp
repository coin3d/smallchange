!!ARBfp1.0

TEMP r0;
TEMP r1;
TEMP r2;
PARAM c0 = {1,0,0,1 };

TXB r0, fragment.texcoord[0], texture[0], 2D;
TXB r1, fragment.texcoord[1], texture[1], 2D;
ADD r2, r0, r1;
MOV result.color, c0;
END