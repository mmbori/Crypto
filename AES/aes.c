
#include "aes.h"

// Key Expansion -----------------------------------------------------------------------


//void KeyToState128(UCHAR originKey[17], UINT Key[4]) {
//
//	UINT mask = 0x000000ff;
//
//	for (i = 0; i < 16; i++) {
//		Key[i / 4] = Key[i / 4] | ((mask | originKey[i]) << (8 * (3 - i)));
//	}
//}


// subByte for Key Expansion
UINT SubByteKey(UINT keyBlock) {

	int front, back;
	UINT tmp, tmphex = 0x00000000;


	for (i = 0; i < 4; i++) {
		tmp = (keyBlock & mask4[i]) >> (8 * (3 - i));
		front = (tmp & FRONT) >> 4;
		back = (tmp & BACK);

		tmphex = tmphex | (sBox[front][back] << (8 * (3 - i)));

		tmp = 0;
	}
	return tmphex;
}

// subByte for decryption
UINT InvSubByteKey(UINT keyBlock) {

	int front, back;
	UINT tmp, tmphex = 0x00000000;


	for (i = 0; i < 4; i++) {
		tmp = (keyBlock & mask4[i]) >> (8 * (3 - i));
		front = (tmp & FRONT) >> 4;
		back = (tmp & BACK);

		tmphex = tmphex | (InvSBox[front][back] << (8 * (3 - i)));

		tmp = 0;
	}
	return tmphex;
}

void KeyExpansion(UINT Key[4], UINT expandedKey[][4]) {			// Key Expansion for 128 bits Key

	UINT hexG;

	// first word block and G for pre-transform key
	expandedKey[0][0] = Key[0];
	expandedKey[0][1] = Key[1];
	expandedKey[0][2] = Key[2];
	expandedKey[0][3] = Key[3];
	/*temp = (tempMask & Key[4]) >> 24;
	hexG = (Key[4] << 8) | temp;*/
	hexG = Key[3];
	hexG = (hexG << 8) | (hexG >> 24);		// shift left operation
	hexG = SubByteKey(hexG);				// s-box operation
	hexG = hexG ^ RC128[0];					// XOR with round constant number
	// hexG for foruth word's XOR operation

	for (k = 1; k < 10; k++) {
		expandedKey[k][0] = expandedKey[k - 1][0] ^ hexG;
		for (j = 1; j < 4; j++) {
			expandedKey[k][j] = expandedKey[k - 1][j] ^ expandedKey[k][j - 1];
		}
		// G operation
		hexG = expandedKey[k][3];
		hexG = (hexG << 8) | (hexG >> 24);		// shift left operation
		hexG = SubByteKey(hexG);				// s-box operation
		hexG = hexG ^ RC128[k];					// XOR with round constant number
	}
	expandedKey[10][0] = expandedKey[9][0] ^ hexG;
	for (r = 1; r < 4; r++)
		expandedKey[10][r] = expandedKey[9][r] ^ expandedKey[10][r - 1];
}

// Key Expansion finish ----------------------------------------------------------------

// Round function start ----------------------------------------------------------------

// SubBytes
void SubByte(UINT plain[4]) {

	for (rr = 0; rr < 4; rr++)
		plain[rr] = SubByteKey(plain[rr]);
}

// Inverse SubBytes
void InvSubByte(UINT plain[4]) {

	for (rr = 0; rr < 4; rr++)
		plain[rr] = InvSubByteKey(plain[rr]);
}


void shiftRow(UINT plain[4]) {

	UINT tempPlain = 0x00000000;

	for (i = 1; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			tempPlain = tempPlain | (((plain[j] & mask4[i]) >> (8 * (3 - i))) << (8 * (3 - j)));	// (x,i) to shift row
		}
		tempPlain = (tempPlain << (8 * i)) | (tempPlain >> (8 * (4 - i)));						// shift row operation
		for (j = 0; j < 4; j++) {
			plain[j] = (plain[j] & InvMask4[i]) | (((tempPlain & mask4[j]) >> (8 * (3 - j)) << (8 * (3 - i))));		// shifted byte return its own position
		}
		tempPlain = 0x00000000;		// temp initializing
	}
}

void InvshiftRow(UINT plain[4]) {

	UINT tempPlain = 0x00000000;

	for (i = 1; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			tempPlain = tempPlain | (((plain[j] & mask4[i]) >> (8 * (3 - i))) << (8 * (3 - j)));	// (x,i) to shift row
		}
		tempPlain = (tempPlain >> (8 * i)) | (tempPlain << (8 * (4 - i)));						// shift row operation
		for (j = 0; j < 4; j++) {
			plain[j] = (plain[j] & InvMask4[i]) | (((tempPlain & mask4[j]) >> (8 * (3 - j)) << (8 * (3 - i))));		// shifted byte return its own position
		}
		tempPlain = 0x00000000;		// temp initializing
	}
}


UCHAR x_time(UCHAR plain, UCHAR count) {

	UCHAR temp = 0;
	UINT mask = 0x80;

	if (count == 0x01)
		;
	else if (count == 0x02) {
		if (plain & mask)
			plain = (plain << 1) ^ 0x1B;
		else
			plain <<= 1;
	}
	else if (count == 0x03) {
		for (i = 0; i < 2; i++) {
			if (plain & mask)
				plain ^= (plain << 1) ^ 0x1B;
			else
				plain ^= (plain << 1);
		}
	}
	else if (count == 0x09) {
		for (i = 0; i < 8; i++) {
			if (plain & mask)
				plain ^= (plain << 1) ^ 0x1B;
			else
				plain ^= (plain << 1);
		}
	}
	else if (count == 0x0B) {
		for (i = 0; i < 10; i++) {
			if (plain & mask)
				plain ^= (plain << 1) ^ 0x1B;
			else
				plain ^= (plain << 1);
		}
	}
	else if (count == 0x0D) {
		for (i = 0; i < 12; i++) {
			if (plain & mask)
				plain ^= (plain << 1) ^ 0x1B;
			else
				plain ^= (plain << 1);
		}
	}
	else if (count == 0x0E) {
		for (i = 0; i < 13; i++) {
			if (plain & mask)
				plain ^= (plain << 1) ^ 0x1B;
			else
				plain ^= (plain << 1);
		}
	}
	return plain;
}

UCHAR xx_time(UINT plain, UCHAR count) {
	UCHAR temp = 0;
	UINT mask = 0x01;
	UINT mod = 0x80;

	for (i = 0; i < 8; i++) {
		if (plain & mask)
			temp ^= count;
		if (count & 0x80)
			count = (count << 1) ^ 0x1B;
		else
			count <<= 1;
		mask <<= 1;
	}
	return temp;
}

//UCHAR InvX_time(UCHAR plain, UCHAR count) {
//
//	UINT mask = 0x80;
//	UCHAR temp = plain;
//
//	if (count == 0x09) {
//		for (i = 0; i < 3; i++) {
//			if (plain & mask)
//				plain = (plain << 1) ^ 0x1B;
//			else
//				plain <<= 1;
//		}
//		plain ^= temp;
//	}
//	else if (count == 0x0B) {
//		for (i = 0; i < 3; i++) {
//			if (plain & mask)
//				plain = (plain << 1) ^ 0x1B;
//			else
//				plain <<= 1;
//		}
//		if (temp & mask)
//			plain ^= (temp << 1) ^ 0x1B;
//		else
//			plain ^= (temp << 1);
//		plain ^= temp;
//	}
//	else if (count == 0x0D) {
//		UCHAR plain2 = plain;
//		for (i = 0; i < 3; i++) {
//			if (plain & mask)
//				plain = (plain << 1) ^ 0x1B;
//			else
//				plain <<= 1;
//		}
//		for (i = 0; i < 2; i++) {
//			if (plain2 & mask)
//				plain2 = (plain2 << 1) ^ 0x1B;
//			else
//				plain2 <<= 1;
//		}
//		plain ^= plain2;
//		plain ^= temp;
//	}
//	else if (count == 0x0E) {
//		UCHAR plain2 = plain, plain3 = plain;
//		for (i = 0; i < 3; i++) {
//			if (plain & mask)
//				plain = (plain << 1) ^ 0x1B;
//			else
//				plain <<= 1;
//		}
//		for (i = 0; i < 2; i++) {
//			if (plain2 & mask)
//				plain2 = (plain2 << 1) ^ 0x1B;
//			else
//				plain2 <<= 1;
//		}
//		if (plain3 & mask)
//			plain3 = (plain3 << 1) ^ 0x1B;
//		else
//			plain3 <<= 1;
//		plain ^= plain2 ^ plain3;
//	}
//	return plain;
//}

// done
void mixColumns(UINT plain[4]) {

	UINT temp = 0x00000000;
	UCHAR tmp[4], ttmp[4];
	UINT mixedPlain[4] = { 0, };
	UCHAR sum = 0x00;

	for (i = 0; i < 4; i++) {				// plain�� i��° -> ���ι������� ���
		tmp[0] = (plain[i] & mask4[0]) >> 24;
		tmp[1] = (plain[i] & mask4[1]) >> 16;
		tmp[2] = (plain[i] & mask4[2]) >> 8;
		tmp[3] = (plain[i] & mask4[3]);
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++) {
				ttmp[k] = x_time(tmp[k], mixMatrix[j][k]);
			}
			sum = ttmp[0] ^ ttmp[1] ^ ttmp[2] ^ ttmp[3];
			temp |= sum << (8 * (3 - j));
		}
		mixedPlain[i] = temp;
		temp = 0;
	}

	for (i = 0; i < 4; i++)
		plain[i] = mixedPlain[i];
}

// done
void InvMixColumns(UINT plain[4]) {

	UINT temp = 0x00000000;
	UCHAR tmp[4], ttmp[4];
	UINT mixedPlain[4] = { 0, };
	UCHAR sum = 0x00;
	
	for (i = 0; i < 4; i++) {				// plain�� i��° -> ���ι������� ���
		tmp[0] = (plain[i] & mask4[0]) >> 24;
		tmp[1] = (plain[i] & mask4[1]) >> 16;
		tmp[2] = (plain[i] & mask4[2]) >> 8;
		tmp[3] = (plain[i] & mask4[3]);
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 4; k++) {
				ttmp[k] = x_time(tmp[k], InvMixMatrix[j][k]);
			}
			sum = ttmp[0] ^ ttmp[1] ^ ttmp[2] ^ ttmp[3];
			temp |= sum << (8 * (3 - j));
		}
		mixedPlain[i] = temp;
		temp = 0;
	}

	for (i = 0; i < 4; i++)
		plain[i] = mixedPlain[i];
}


void addRoundKey(UINT plain[4], UINT expandedKey[11][4], int count) {

	UINT temp = 0x0;
	for (i = 0; i < 4; i++) {
		temp |= plain[i] ^ expandedKey[count][i];
		plain[i] = temp;
		temp = 0x0;
	}
}

void encRoundFunc(UINT plain[4], UINT expandedKey[11][4], UINT cipher[4]) {

	addRoundKey(plain, expandedKey, 0);
	for (iii = 1; iii < 10; iii++) {
		SubByte(plain);
		shiftRow(plain);
		mixColumns(plain);
		addRoundKey(plain, expandedKey, iii);
	}
	SubByte(plain);
	shiftRow(plain);
	addRoundKey(plain, expandedKey, 10);
	
	for (i = 0; i < 4; i++) {
		cipher[i] = plain[i];
		printf("%x\n", cipher[i]);
	}
}

void decRoundFunc(UINT cipher[4], UINT expandedKey[11][4], UINT decryption[4]) {

	addRoundKey(cipher, expandedKey, 10);
	for (iii = 9; iii > 0; iii--) {
		InvshiftRow(cipher);
		InvSubByte(cipher);
		addRoundKey(cipher, expandedKey, iii);
		InvMixColumns(cipher);
	}
	InvshiftRow(cipher);
	InvSubByte(cipher);
	addRoundKey(cipher, expandedKey, 0);

	for (i = 0; i < 4; i++) {
		decryption[i] = cipher[i];
		printf("%x\n", decryption[i]);
	}
}


int main() {

	//** Key Expansion, Substitute bytes, Shift rows, Mix columns, Add round key function is done
	//** InvSubstitute bytes, InvShiftRow is done
	//*** cannot certificate InvMixColumn

	UINT plain[4] = { 0x01234567, 0x89abcdef, 0xfedcba98, 0x76543210 };
	UINT key[4] = { 0x0f1571c9, 0x47d9e859, 0x0cb7add6, 0xaf7f6798 };

	UINT ExpandedKey[11][4];
	UINT cipher[4];
	UINT decryption[4];

	KeyExpansion(key, ExpandedKey);

	printf("Plain text : \n");
	for (i = 0; i < 4; i++)
		printf("%x\n", plain[i]);

	KeyExpansion(key, ExpandedKey);
	printf("\nCipher : \n");
	encRoundFunc(plain, ExpandedKey, cipher);

	printf("\nPlain text : \n");
	decRoundFunc(cipher, ExpandedKey, decryption);


	return 0;
}
