#include "./SHA_1/sha1.h"



// 将原始的【6位数】密码 转化为【512bit】密码
// input:   【6位数】密码   (0x313233343536)格式    
// output:  【512bit】密码  (0x313233343536 8000 0000 0000 ... 0000)格式  16个int
u8* Generate_512bit_Input(u8* input) {
    // 总长度为 512 bit
    u8 result[64];

    // 复制前面的6个数字
    for (u8 i=0; i<6; i++) {
        result[i] = input[i];
    }
    // 添加一个 0x80
    result[6] = 0x80;
    // 后面补零至第 448 bit
    for (u8 i=7; i<56; i++) {
        result[i] = 0;
    }

    // 后面加上原始密码的长度，相当于直接加上一个定值，因为我们的密码只有6位
    for (u8 i=56; i<62; i++) {
        result[i] = 0;
    }
    result[63] = 0x30;  // 0x30=48bit   6*8=48

    return result;
}
