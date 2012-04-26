//Des.h

#ifndef _FRAMEWORK_COMPRESS_DES_H_
#define _FRAMEWORK_COMPRESS_DES_H_

#define EN0 0
#define DE1 1
#define maxsize 1024
#include<stdio.h>

namespace framework
{
    namespace security
    {
        namespace des
        {
            int Des3EnCrypt(unsigned char *key,int keyLen,unsigned char *sData,int iDataLen,unsigned char *sCipher,int *iCipherLen);
            int Des3DeCrypt(unsigned char *key,int keyLen,unsigned char *sCipher,int iCipherLen,unsigned char *sPlain,int *iPlainLen);

        }// namespace des
    }// namespace security
}// namespace framework

#endif //_FRAMEWORK_COMPRESS_DES_H_