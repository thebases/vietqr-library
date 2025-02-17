import 'dart:convert';
import 'package:qr_flutter/qr_flutter.dart';

class EmvQrDto {
  String bincode;
  String serviceCode;
  String merchantId;
  String merchantName;
  String currency;
  String amount;
  String qrCode;

  EmvQrDto({
    required this.bincode,
    required this.serviceCode,
    required this.merchantId,
    required this.merchantName,
    required this.currency,
    required this.amount,
    this.qrCode = '',
  });
}

int crc16(List<int> data) {
  int crc = 0xFFFF;
  for (var byte in data) {
    crc ^= (byte << 8);
    for (int i = 0; i < 8; i++) {
      if ((crc & 0x8000) != 0) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc & 0xFFFF;
}

String createField38(String bincode, String merchantId, String merchantName, String serviceCode) {
  String subField01 = "00${bincode.length.toString().padLeft(2, '0')}$bincode"
      "01${merchantId.length.toString().padLeft(2, '0')}$merchantId";

  String payload = "0010A000000727"
      "01${subField01.length.toString().padLeft(2, '0')}$subField01"
      "02${serviceCode.length.toString().padLeft(2, '0')}$serviceCode";

  return "38${payload.length.toString().padLeft(2, '0')}$payload";
}

String generateEmvQr(EmvQrDto dto) {
  String serviceCode = (dto.serviceCode != "QRIBFTTC" &&
          dto.serviceCode != "QRPUSH" &&
          dto.serviceCode != "QRPAY")
      ? "QRIBFTTA"
      : dto.serviceCode;

  String f38 = createField38(dto.bincode, dto.merchantId, dto.merchantName, serviceCode);

  String payload = "000201"
      "010212"
      "$f38"
      "52049999"
      "5303704"
      "54${dto.amount.length.toString().padLeft(2, '0')}${dto.amount}"
      "5802VN"
      "59${dto.merchantName.length.toString().padLeft(2, '0')}${dto.merchantName}"
      "6003HCM"
      "6304";

  int crc = crc16(utf8.encode(payload));
  return "$payload${crc.toRadixString(16).toUpperCase().padLeft(4, '0')}";
}