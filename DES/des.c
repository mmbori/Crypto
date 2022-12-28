#include "des.h"

void main() {

	int i;
	int msg_len = 0, block_count = 0;
	BYTE p_text[64] = { 0, };
	BYTE c_text[64] = { 0, };
	BYTE d_text[64] = { 0, };
	BYTE key[9] = { 0, };


	printf("�� �Է� : ");
	scanf("%x",p_text);

	printf("���Ű �Է� : ");
	scanf("%x", key);


	printf("\n");
	printf("----------------------------------------------------------------------------\n");
	printf("Round	|		  Ki			|	Li	|	Ri	\n");
	printf("----------------------------------------------------------------------------\n");

	DES_Encryption(&p_text[0], &c_text[0], key);


	printf("\n��ȣ�� : ");
	for (i = 0; i < 16; i++) {
		printf("%x", c_text[i]);
	}
	printf("\n");

	
	DES_Decryption(&c_text[0], &d_text[0], key);

	printf("\n��ȣ�� : ");
	for (i = 0; i < 16; i++)
		printf("%c", d_text[i]);
	printf("\n");

}

// DES ��ȣȭ

void DES_Encryption(BYTE* p_text, BYTE* result, BYTE* key) {

	int i;
	BYTE data[BlOCK_SIZE] = { 0, };
	BYTE round_key[16][6] = { 0, };
	UINT L = 0, R = 0;

	key_expansion(key, round_key);
	IP(p_text, data);

	BtoW(data, &L, &R);

	printf("IP						  %08x	  %08x\n", L, R);

	for (i = 0; i < DES_ROUND; i++) {

		L = L ^ f(R, round_key[i]);

		if (i != DES_ROUND - 1)
			swap(&L, &R);

		printf("%d	    ", i + 1);
		for (int k = 0; k < 16; k++) {
			printf("%x", round_key[i][k]);
		}
		printf("	  %08x	  %08x\n", L, R);
		
		
	}
	WtoB(L, R, data);
	In_IP(data, result);
}

void DES_Decryption(BYTE* c_text, BYTE* result, BYTE* key) {

	int i;
	BYTE data[BlOCK_SIZE] = { 0, };
	BYTE round_key[16][6] = { 0, };
	UINT L = 0, R = 0;

	key_expansion(key, round_key);
	IP(c_text, data);

	BtoW(data, &L, &R);

	for (i = 0; i < DES_ROUND; i++) {

		L = L ^ f(R, round_key[DES_ROUND - 1 - i]);

		if (i != DES_ROUND - 1)
			swap(&L, &R);
	}
	WtoB(L, R, data);
	In_IP(data, result);

}

void swap(UINT* x, UINT* y) {

	UINT temp;

	temp = *x;
	*x = *y;
	*y = temp;
}

void BtoW(BYTE* in, UINT* x, UINT* y) {

	int i;

	for (i = 0; i < 8; i++) {

		if (i < 4)
			*x |= (UINT)in[i] << (24 - (i * 8));
		else
			*y |= (UINT)in[i] << (56 - (i * 8));
	}
}

void WtoB(UINT l, UINT r, BYTE* out) {

	int i;
	UINT mask = 0xFF000000;

	for (i = 0; i < 8; i++) {
		if (i < 4)
			out[i] = (l & (mask >> i * 8)) >> (24 - (i * 8));
		else
			out[i] = (r & (mask >> (i - 4) * 8)) >> (56 - (i * 8));
	}
}

// ----------------------------------------------------------------------------- �ʱ�, ���ʱ� ����
void IP(BYTE* in, BYTE* out) {

	int i;
	BYTE index, bit, mask = 0x80;

	for (i = 0; i < 64; i++) {
		index = (ip[i] - 1) / 8;
		bit = (ip[i] - 1) % 8;

		if (in[index] & (mask >> bit))
			out[i / 8] |= mask >> (i % 8);
	}
}

void In_IP(BYTE* in, BYTE* out) {

	int i;
	BYTE index, bit, mask = 0x80;

	for (i = 0; i < 64; i++) {
		index = (ip_1[i] - 1) / 8;
		bit = (ip_1[i] - 1) % 8;

		if (in[index] & (mask >> bit))
			out[i / 8] |= mask >> (i % 8);
	}
}

//------------------------------------------------------ round function

UINT f(UINT r, BYTE* rkey) {

	int i;
	BYTE data[6] = { 0, };
	UINT out;

	EP(r, data);

	for (i = 0; i < 6; i++)
		data[i] = data[i] ^ rkey[i];

	out = Permutation(S_box_Transfer(data));

	return out;
}

void EP(UINT r, BYTE* out) {

	int i;
	UINT mask = 0x80000000;

	for (i = 0; i < 48; i++) {
		if (r & (mask >> (E[i] - 1)))
			out[i / 8] |= (BYTE)(0x80 >> (i % 8));
	}
}

UINT S_box_Transfer(BYTE* in) {

	int i, row, column, shift = 28;
	UINT temp = 0, result = 0, mask = 0x00000080;

	for (i = 0; i < 48; i++) {

		if (in[i / 8] & (BYTE)(mask >> (i % 8)))
			temp |= 0x20 >> (i % 6);

		if ((i + 1) % 6 == 0) {
			row = ((temp & 0x20) >> 4) + (temp & 0x01);
			column = (temp & 0x1E) >> 1;
			result += ((UINT)s_box[i / 6][row][column] << shift);
			shift -= 4;
			temp = 0;
		}
	}
	return result;
}

UINT Permutation(UINT in) {

	int i;
	UINT out = 0, mask = 0x80000000;

	for (i = 0; i < 32; i++) {
		if (in & (mask >> (P[i] - 1)))
			out |= (mask >> i);
	}
	return out;
}

//--------------------------------------------------- key generator

void key_expansion(BYTE* key, BYTE round_key[16][6]) {

	int i;
	BYTE pc1_result[7] = { 0, };
	UINT c = 0, d = 0;

	PC1(key, pc1_result);
	makeBit28(&c, &d, pc1_result);

	for (i = 0; i < 16; i++) {

		c = cir_shift(c, i);
		d = cir_shift(d, i);
		PC2(c, d, round_key[i]);
	}
}

void makeBit28(UINT* c, UINT* d, BYTE* data) {

	int i;
	BYTE mask = 0x80;

	for (i = 0; i < 56; i++) {
		if (i < 28) {
			if (data[i / 8] & (mask >> (i % 8)))
				*c |= 0x08000000 >> i;
		}
		else {
			if (data[i / 8] & (mask >> (i % 8)))
				*d |= 0x08000000 >> (i - 28);
		}
	}
}

void PC1(BYTE* in, BYTE* out) {

	int i, index, bit;
	UINT mask = 0x00000080;

	for (i = 0; i < 56; i++) {
		index = (PC_1[i] - 1) / 8;
		bit = (PC_1[i] - 1) % 8;

		if (in[index] & (BYTE)(mask >> bit))
			out[i / 8] |= (BYTE)(mask >> (i % 8));
	}
}

void PC2(UINT c, UINT d, BYTE* out) {

	int i;
	UINT mask = 0x08000000;

	for (i = 0; i < 48; i++) {
		if ((PC_2[i] - 1) < 28) {
			if (c & (mask >> (PC_2[i] - 1)))
				out[i / 8] |= 0x80 >> (i % 8);
		}
		else {
			if (d & (mask >> (PC_2[i] - 1 - 28)))
				out[i / 8] |= 0x80 >> (i % 8);
		}
	}
}

UINT cir_shift(UINT n, int r) {

	int n_shift[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };

	if (n_shift[r] == 1)
		n = ((n << 1) + (n >> 27)) & 0xFFFFFFFF;

	else
		n = ((n << 2) + (n >> 26)) & 0xFFFFFFFF;

	return n;
}
