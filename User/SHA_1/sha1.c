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
int sha1(char * in, int in_length) {
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

	//************* 接下来开始选择使用 160 bit中的那几位作为暂时的密码 *************//
	// 1.先从第一步通过 SHA-1 算法加密得到的 20 字节长度的结果中选取最后一个字节的低字节位的 4 位（注意：动态密码算法中采用的大端(big-endian)存储）；
	// 2.将这 4 位的二进制值转换为无标点数的整数值，得到 0 到 15（包含 0 和 15）之间的一个数，这个数字作为 20 个字节中从 0 开始的偏移量；
	u8 move_val = (u8) (Enow & 0x0000000F);

	// 接着从指定偏移位开始，连续截取 4 个字节（32 位），最后返回 32 位中的后面 31 位；
	u8 string_20Bytes[20];
	for (i=0; i<4; i++) {
		string_20Bytes[i] = (u8)(Anow & 0x000000FF);
		Anow >>= 8;
	}
	for (i=0; i<4; i++) {
		string_20Bytes[i+4] = (u8)(Bnow & 0x000000FF);
		Bnow >>= 8;
	}
	for (i=0; i<4; i++) {
		string_20Bytes[i+8] = (u8)(Cnow & 0x000000FF);
		Cnow >>= 8;
	}
	for (i=0; i<4; i++) {
		string_20Bytes[i+12] = (u8)(Dnow & 0x000000FF);
		Dnow >>= 8;
	}
	for (i=0; i<4; i++) {
		string_20Bytes[i+16] = (u8)(Enow & 0x000000FF);
		Enow >>= 8;
	}
	int num31;
	num31 = string_20Bytes[move_val];
	num31 <<= 8;
	num31 = num31 | string_20Bytes[move_val+1];
	num31 <<= 8;
	num31 = num31 | string_20Bytes[move_val+2];
	num31 <<= 8;
	num31 = num31 | string_20Bytes[move_val+3];
	num31 = num31 & 0x7fffffff;

	// 回到算法本身，在获得 31 位的截断结果之后，我们将其又转换为无标点的大端表示的整数值，这个值的取值范围是 0 ~ 231，也即 0 ~ 2.147483648E9，最后我们将这个数对10的乘方（digit 指数范围 1-10）取模，得到一个余值，对其前面补0得到指定位数的字符串。
	num31 = num31%1000000;

	return num31;
}



// int main(){
// 	char a[] = "1";
// 	sha1(a, 1);
// 	a[0] = 'a';
// 	sha1(a, 1);
// }
