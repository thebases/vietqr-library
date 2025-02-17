#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_def.h"
#include "vietqr.h"

EmvQrDto g_emvQrDto = {0};

// Getters and Setters for EmvQrDto
EmvQrDto *GetEmvQrDto()
{
    return &g_emvQrDto;
}

/* The `crc16` function is calculating the CRC16-CCITT (XMODEM) checksum for a given
data array. It iterates over each byte in the data array, performs bitwise XOR
operations and bit shifting to calculate the checksum value, and finally returns the
calculated CRC value as an unsigned short integer. */

unsigned short crc16(const unsigned char *data, size_t len)
{
    unsigned short crc = 0xFFFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= (unsigned short)data[i] << 8;
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Function to create Field 38 for EMV QR Code
void create_field_38(char *output, size_t output_size, const char *bincode, const char *merchant_id, const char *merchant_name, const char *service_code)
{
    // TODO: Review this function, it's not render the right QR string
    // char payload1[50];  // Initialize buffer
    APP_TRACE("==== create_field_38 ====");
    char sub_field_01[128] = {0};
    char payload[128] = {0};                     // Increase output size
    snprintf(sub_field_01, sizeof(sub_field_01), // Use sizeof(payload1) instead of strlen(payload1)
             "00%02lu%s"
             "01%02lu%s",
             strlen(bincode), bincode,
             strlen(merchant_id), merchant_id);

    APP_TRACE("step 1: create sub field 38-01-payload: %s", sub_field_01);

    snprintf(payload, sizeof(payload), // Use payload_size instead of strlen(payload)
             "0010A000000727"
             "01%02lu%s"
             "02%02lu%s",
             strlen(sub_field_01), sub_field_01,
             strlen(service_code), service_code);
    APP_TRACE("step 2: create sub field 38-detail: %s", payload);
    snprintf(output, output_size, // Use output_size instead of strlen(buffer)
             "38%02lu%s",
             strlen(payload), payload);

    // Free payload
    // free(payload);
    APP_TRACE("create_field_38 output: %s", output);
}

/* The `generate_emv_qr` function is responsible for generating an EMV QR Code string based on the
provided input parameters.
 - It first creates Field 38 by calling the `create_field_38` function
with the BIN code, merchant ID, and service code.
 - Then, it constructs the payload string by formatting various data elements such as the merchant account information, transaction
currency, amount, merchant name, and other details.
 - At last, it input the final result to 'output' variable. */

// void GenerateEmvQr(char *output, size_t output_size, const char *bincode, const char *serviceCode, const char *merchantID,
//                    const char *merchantName, const char *currency, const char *amount)
// void GenerateEmvQr(EmvQrDto *emvQrDto)
void GenerateEmvQr(char *output, const char *bincode, const char *serviceCode, const char *merchantID,
                   const char *merchantName, const char *currency, const char *amount)
{
    char tmpServiceCode[10] = {0};
    char payload[512] = {0}; // Increase payload size
    char f38[128] = {0};

    if (serviceCode != "QRIBFTTC" || serviceCode != "QRPUSH" || serviceCode != "QRPAY")
    {
        strcpy(tmpServiceCode, "QRIBFTTA");
    }
    APP_TRACE("===== GenerateEmvQr =====");
    APP_TRACE("bincode: %s", bincode);
    APP_TRACE("serviceCode: %s", serviceCode);
    APP_TRACE("merchantId: %s", merchantID);
    APP_TRACE("merchantName: %s", merchantName);
    APP_TRACE("currency: %s", currency);
    APP_TRACE("amount: %s", amount);

    // Create Field 38
    create_field_38(f38, sizeof(f38), bincode, merchantID, merchantName, tmpServiceCode);
    APP_TRACE("create_field_38: %s", f38);
    // Generate Payload String
    snprintf(payload, sizeof(payload), // Use sizeof(payload) instead of strlen(payload)
             "000201"                  // Payload Format Indicator
             "010212"                  // Point of Initiation Method (Static QR)
             "%s"                      // Merchant Account Information (BIN for VietQR)
             "52049999"                // Merchant Category Code (Generic)
             "5303704"                 // Transaction Currency (ISO 4217)
             "54%02lu%s"               // Transaction Amount
             "5802VN"                  // Country Code (Vietnam)
             "59%02lu%s"               // Merchant Name
             "6003HCM"                 // Merchant City
             "6304",                   // CRC Placeholder
             f38,
             strlen(amount), amount,
             strlen(merchantName), merchantName);
    APP_TRACE("payload: %s", payload);
    // // Calculate CRC16 and append
    unsigned short crc = crc16((unsigned char *)payload, strlen(payload));
    APP_TRACE("CRC:%04X", crc);
    APP_TRACE("sizeof(output):", sizeof(output));
    snprintf(payload, sizeof(payload), "%s%04X", payload, crc);
    strcpy(output, payload);
    APP_TRACE("Generated EMV QR Code: %s", output);
    // Free variables
    // free(tmp);
    // free(payload);
}

/* ===========================================*/

void Split_string(const char *input, char result[MAX_WORDS][MAX_LENGTH], int *size) {
    char temp[256]; // Temporary buffer to avoid modifying the original string
    strcpy(temp, input);

    char *token = strtok(temp, " "); // First split
    *size = 0;

    while (token != NULL && *size < MAX_WORDS) {
        strcpy(result[*size], token);
        (*size)++;
        token = strtok(NULL, " "); // Get next token
    }
}   


// Vietnamese number names without accents
const char *vietnamese_numbers[] = {
    "", "mot", "hai", "ba", "bon", "nam", "sau", "bay", "tam", "chin"
};

// Function to convert a number to Vietnamese text 
// TODO: Need to review this function because it's not working correctly
void Convert_number_to_vietnamese(const char *number, char *result) {
    int length = strlen(number);
    int *num = (int *)malloc(length * sizeof(int));
    if (!num) {
        strcpy(result, "Memory allocation failed");
        return;
    }

    // Convert char digits to integer array
    for (int i = 0; i < length; i++) {
        num[i] = number[i] - '0';
    }

    char buffer[512] = "";
    int first_nonzero = -1;

    // Find first non-zero digit to avoid leading zeros
    for (int i = 0; i < length; i++) {
        if (num[i] != 0) {
            first_nonzero = i;
            break;
        }
    }

    if (first_nonzero == -1) { // If all are zeros
        strcpy(result, "khong");
        free(num);
        return;
    }

    int has_previous_nonzero = 0;
    int last_unit_position = -1;

    for (int i = first_nonzero; i < length; i++) {
        int digit = num[i];
        int position = length - i;
        int mod3 = position % 3;
        int group_index = position / 3;

        if (digit == 0) {
            // Handle "không trăm" when hundreds place is missing
            if (mod3 == 0 && (i + 2 < length) && (num[i + 1] != 0 || num[i + 2] != 0)) {
                strcat(buffer, " khong tram");
            }
            // Handle "linh" when zero appears in the tens place
            else if (mod3 == 2 && num[i - 1] != 0) {
                strcat(buffer, " linh");
            }
            continue;
        }

        // Handle "mười" case
        if (mod3 == 2 && digit == 1) {  
            strcat(buffer, has_previous_nonzero ? " muoi" : "muoi");
        } else if (mod3 == 1 && digit == 5 && has_previous_nonzero) {  
            strcat(buffer, " lam");
        } else { 
            strcat(buffer, has_previous_nonzero ? " " : "");  
            strcat(buffer, vietnamese_numbers[digit]);
        }

        has_previous_nonzero = 1;

        // Append unit names
        if (mod3 == 2 && digit != 1) {
            strcat(buffer, " muoi");
        } else if (mod3 == 0) {
            strcat(buffer, " tram");
        }

        // Append "nghin", "trieu", "ty" only if needed
        if (mod3 == 1 && group_index > 0) {
            int next_three_digits_zero = 1;

            // Check if the next three digits are all zero
            for (int j = i; j < i + 3 && j < length; j++) {
                if (num[j] != 0) {
                    next_three_digits_zero = 0;
                    break;
                }
            }

            // Add unit only if this is a nonzero group or if it's needed at the end
            if (!next_three_digits_zero || (length - i) > 3) {
                if (group_index == 1 && last_unit_position != 1) {
                    strcat(buffer, " nghin");
                    last_unit_position = 1;
                } else if (group_index == 2 && last_unit_position != 2) {
                    strcat(buffer, " trieu");
                    last_unit_position = 2;
                } else if (group_index == 3 && last_unit_position != 3) {
                    strcat(buffer, " ty");
                    last_unit_position = 3;
                }
            }
        }
    }

    // **Fix: Remove trailing "linh" if it's incorrectly placed**
    int buffer_length = strlen(buffer);
    if (buffer_length > 5 && strcmp(&buffer[buffer_length - 5], " linh") == 0) {
        buffer[buffer_length - 5] = '\0';  // Trim " linh"
    }

    // Copy final result
    strcpy(result, buffer);
    free(num);
} 