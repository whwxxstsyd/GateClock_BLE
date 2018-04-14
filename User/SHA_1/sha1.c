#include "./SHA_1/sha1.h"
#include <stdio.h>
#include <stdlib.h>


#define SHA256_ROTL(a,b) (((a>>(32-b))&(0x7fffffff>>(31-b)))|(a<<b))
#define SHA256_SR(a,b) ((a>>b)&(0x7fffffff>>(b-1)))
#define SHA256_Ch(x,y,z) ((x&y)^((~x)&z))
#define SHA256_Maj(x,y,z) ((x&y)^(x&z)^(y&z))
#define SHA256_E0(x) (SHA256_ROTL(x,30)^SHA256_ROTL(x,19)^SHA256_ROTL(x,10))
#define SHA256_E1(x) (SHA256_ROTL(x,26)^SHA256_ROTL(x,21)^SHA256_ROTL(x,7))
#define SHA256_O0(x) (SHA256_ROTL(x,25)^SHA256_ROTL(x,14)^SHA256_SR(x,3))
#define SHA256_O1(x) (SHA256_ROTL(x,15)^SHA256_ROTL(x,13)^SHA256_SR(x,10))
const unsigned int A = 0x67452301;
const unsigned int B = 0xEFCDAB89;
const unsigned int C = 0x98BADCFE;
const unsigned int D = 0x10325476;
const unsigned int E = 0xC3D2E1F0; 
const unsigned int K1 = 0x5A827999;
const unsigned int K2 = 0x6ED9EBA1;
const unsigned int K3 = 0x8F1BBCDC;
const unsigned int K4 = 0xCA62C1D6;

// 循环左移num位
unsigned int ROTL(unsigned int a, int num) {
	unsigned int temp = a >> (32 - num);
	return (a << num) | temp;
}

// 4个阶段函数变化，同时这个函数可以输出四个阶段不同的K
unsigned int funt(unsigned int B, unsigned int C, unsigned int D, int t,unsigned int * K) {
	if (t <= 19) {
		*K = K1;
		return (B&C) | (~B&D);
	}
	else if (t <= 39) {
		*K = K2;
		return B^C^D;
	}
	else if (t <= 59) {
		*K = K3;
		return (B&C) | (B&D) | (C&D);
	}
	else {
		*K = K4;
		return  B^C^D;
	}
}

// SHA-1 转换
// in:			输入的密码
// in_length:	输入的字符个数，而不是比特数
void sha1(char * in, int in_length) {
	char ori[64];//512位明文
	int length = in_length * 8;
	int i;
	for (i = 0; i < in_length; i++)         ori[i] = in[i];
	ori[in_length] = 0x80;
	for (i = in_length + 1; i < 56; i++)    ori[i] = 0x00;
	for (i = 56; i < 60;i++)				ori[i] = 0x00;
	for (i = 60; i < 64; i++)				ori[i] = length >> ((63 -i) *8) & 0x000000ff;
	i = 0;
    // 分为80个组
	unsigned int W[80];
	int temp = 0;
	// 前16组
	for (i = 0; i < 16; i++) {
		temp = i * 4;
		W[i] = ori[temp];
		W[i] = (W[i] << 8) | (ori[temp + 1] & 0x000000ff);	// 这个&0x000000ff非常关键
		W[i] = (W[i] << 8) | (ori[temp + 2] & 0x000000ff);
		W[i] = (W[i] << 8) | (ori[temp + 3] & 0x000000ff);
	}
	for (i = 16; i < 80; i++) {
		W[i] = W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16];
		W[i] = ROTL(W[i], 1);
	}
	// 80步运算
	unsigned int Abefor = A;
	unsigned int Bbefor = B;
	unsigned int Cbefor = C;
	unsigned int Dbefor = D;
	unsigned int Ebefor = E;

	unsigned int Anow = 0x00;
	unsigned int Bnow = 0x00;
	unsigned int Cnow = 0x00;
	unsigned int Dnow = 0x00;
	unsigned int Enow = 0x00;
	unsigned int Ktemp;
	for (i=0; i<80; i++) {
		// A的计算分两步写，因为我不确定funt输出的Ktemp能不能立即用于同一行的Ktemp
		Anow = ROTL(Abefor, 5) + funt(Bbefor, Cbefor, Dbefor,i, &Ktemp) + Ebefor + W[i];
		Anow += Ktemp;
		Bnow = Abefor;
		Cnow = ROTL(Bbefor, 30);
		Dnow = Cbefor;
		Enow = Dbefor;

		Abefor = Anow;
		Bbefor = Bnow;
		Cbefor = Cnow;
		Dbefor = Dnow;
		Ebefor = Enow;
	}
	Anow += A;
	Bnow += B;
	Cnow += C;
	Dnow += D;
	Enow += E;

	// 最终输出
	// printf("%x,%x,%x,%x,%x\r\n", Anow, Bnow, Cnow, Dnow, Enow);
}


// int main(){
// 	char a[] = "1";
// 	sha1(a, 1);
// 	a[0] = 'a';
// 	sha1(a, 1);
// }
