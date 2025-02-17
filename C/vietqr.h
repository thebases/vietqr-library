#ifndef _vietqr_h_
#define _vietqr_h_

#define MAX_WORDS 20  // Maximum number of words
#define MAX_LENGTH 50 // Maximum length of each word

typedef struct EmvQrDto
{
    char bincode[20];
    char serviceCode[10];
    char merchantId[20];
    char merchantName[50];
    char currency[3];
    char amount[20];
    char qrCode[512];
} EmvQrDto;

EmvQrDto *GetEmvQrDto();
void GenerateEmvQr(char *output, const char *bincode, const char *serviceCode,
                   const char *merchantID, const char *merchantName, const char *currency, const char *amount);
void Convert_number_to_vietnamese(const char *number, char *result);
void Split_string(const char *input, char result[MAX_WORDS][MAX_LENGTH], int *size);

#endif