import sys
import serial.tools.list_ports
import re
import serial
max_contact=15
line_count=0
MDC_ADD= [ 0x1D48, 0x1D88, 0x1DC8,0x1F08]
MDC_NUM_ADD=0X1D00
file_out="MDC.txt"
com_open=""


def payload_xor(payload):
    XOR_ARRAY = bytes.fromhex('166c14e62e910d402135d5401303e980')
    XOR_LEN   = len(XOR_ARRAY)

    ba=bytearray(payload)
    for i in range(0,len(ba)):
        ba[i] ^= XOR_ARRAY[i%XOR_LEN]
    return bytes(ba)
Crc16Tab = [0, 4129, 8258, 12387, 16516, 20645, 24774, 28903, 33032, 37161, 41290, 45419, 49548, 53677, 57806, 61935, 4657, 528, 12915, 8786, 21173, 17044, 29431, 25302,
            37689, 33560, 45947, 41818, 54205, 50076, 62463, 58334, 9314, 13379, 1056, 5121, 25830, 29895, 17572, 21637, 42346, 46411, 34088, 38153, 58862, 62927, 50604, 54669, 13907,
            9842, 5649, 1584, 30423, 26358, 22165, 18100, 46939, 42874, 38681, 34616, 63455, 59390, 55197, 51132, 18628, 22757, 26758, 30887, 2112, 6241, 10242, 14371, 51660, 55789,
            59790, 63919, 35144, 39273, 43274, 47403, 23285, 19156, 31415, 27286, 6769, 2640,14899, 10770, 56317, 52188, 64447, 60318, 39801, 35672, 47931, 43802, 27814, 31879,
            19684, 23749, 11298, 15363, 3168, 7233, 60846, 64911, 52716, 56781, 44330, 48395,36200, 40265, 32407, 28342, 24277, 20212, 15891, 11826, 7761, 3696, 65439, 61374,
            57309, 53244, 48923, 44858, 40793, 36728, 37256, 33193, 45514, 41451, 53516, 49453, 61774, 57711, 4224, 161, 12482, 8419, 20484, 16421, 28742, 24679, 33721, 37784, 41979,
            46042, 49981, 54044, 58239, 62302, 689, 4752, 8947, 13010, 16949, 21012, 25207, 29270, 46570, 42443, 38312, 34185, 62830, 58703, 54572, 50445, 13538, 9411, 5280, 1153, 29798,
            25671, 21540, 17413, 42971, 47098, 34713, 38840, 59231, 63358, 50973, 55100, 9939, 14066, 1681, 5808, 26199, 30326, 17941, 22068, 55628, 51565, 63758, 59695, 39368,
            35305, 47498, 43435, 22596, 18533, 30726, 26663, 6336, 2273, 14466, 10403, 52093, 56156, 60223, 64286, 35833, 39896, 43963, 48026, 19061, 23124, 27191, 31254, 2801,
            6864, 10931, 14994, 64814, 60687, 56684, 52557, 48554, 44427, 40424, 36297, 31782, 27655, 23652, 19525, 15522, 11395, 7392, 3265, 61215, 65342, 53085, 57212, 44955,
            49082, 36825, 40952, 28183, 32310, 20053, 24180, 11923, 16050, 3793, 7920]


def crc16_ccitt(data):
    i2 = 0
    for i3 in range(0, len(data)):
        out = Crc16Tab[((i2 >> 8) ^ data[i3]) & 255]
        i2 = out ^ (i2 << 8)

    return 65535 & i2
def convert_payload_to_hex(code):
    payload_decoded = payload_xor(code[4:-4])  # 跳过头部和尾部的校验信息
    hex_payload = ' '.join(['{:02X}'.format(byte) for byte in payload_decoded])
    return hex_payload
def check_format(line):
    pattern = r'^[0-9A-Fa-f]{4}\s[\x20-\x7E]{1,14}$'
    match = re.match(pattern, line)
    if match:
        remaining_chars = 14 - len(match.group(0)) + 5  # 计算还需要多少个字符（14个字符长度减去前面已匹配的字符长度和空格）
        line = line.rstrip() + ' ' * remaining_chars  # 使用空格填充到14个字符长度
        return line + '\n'  # 返回修改后的行，加上换行符
    else:
        print("文件格式错误，请仔细阅读示例！")
    return None

def update_file(file_path):
    updated_lines = []
    with open(file_path, 'r') as file:
        lines = file.readlines()
        for line in lines:
            updated_line = check_format(line.strip())
            if updated_line is not None:
                updated_lines.append(updated_line)
            else:
                updated_lines.append(line)  # 不符合条件的行保持不变

    # 将修改后的行写回文件
    with open(file_path, 'w') as file:
        file.writelines(updated_lines)

def check_duplicates(file_path):
    lines_seen = set()
    line_number = 0

    with open(file_path, 'r') as file:
        for line in file:
            line_number += 1
            first_four = line[:4]
            if first_four in lines_seen:
                print(f"MDC ID不能重复！！")
                print(f"重复发现在第 {line_number} 行: {first_four}")
                input("按 Enter 键退出程序")

                sys.exit()
            else:
                lines_seen.add(first_four)
def time_set():
    global com_open
    with serial.Serial(  com_open, 38400, timeout=1) as ser:

        settime = b'\xAB\xCD\x08\x00\x02\x69\x10\xE6\xAC\xD1\x79\x25\x9D\xAD\xDC\xBA'
        ser.write(settime)
        full_response = ser.read(128)
        if len(full_response) == 0:
            print("连接失败！")
            input("按 Enter 键退出程序")

            sys.exit()
def read_eeprom_byte(add):
    global com_open
    with serial.Serial(com_open, 38400, timeout=1) as ser:
        payload = b'\x1B\x05' + b'\x08\x00' + add.to_bytes(2, byteorder='little') + b'\x11\x00' + b'\x82\x40\x74\x65'
        # 将 payload 中的最后四个字节替换为当前时间戳
        hex_string = ' '.join(['{:02X}'.format(byte) for byte in payload])
        crc = crc16_ccitt(payload)
        payload = payload + bytes([crc & 0xFF, ]) + bytes([crc >> 8, ])  # swap bytes of crc to get little endian
        message = b'\xAB\xCD' + b'\x0C\x00' + payload_xor(payload) + b'\xDC\xBA'
        ser.write(message)
        full_response = ser.read(128)
        if len(full_response) == 0:
            print("读取失败！")
            input("按 Enter 键退出程序")

            sys.exit()

        full_response_hex = full_response.hex()
        # 将16进制字符串转换为字节串
        full_response = bytes.fromhex(full_response_hex)
        # 对payload部分进行解码，然后以两位十六进制输出

        payload_decoded = convert_payload_to_hex(full_response)
    return payload_decoded[24:26]
import sys
def read_contact_information(ser, add, file):
    payload = b'\x1B\x05' + b'\x08\x00' + add.to_bytes(2, byteorder='little') + b'\x10\x00' + b'\x82\x40\x74\x65'
    crc = crc16_ccitt(payload)
    payload += bytes([crc & 0xFF,]) + bytes([crc >> 8,])
    message = b'\xAB\xCD' + b'\x0C\x00' + payload_xor(payload) + b'\xDC\xBA'
    ser.write(message)
    full_response = ser.read(128)
    if len(full_response) == 0:
        raise ValueError("读取失败！")
    full_response_hex = full_response.hex()
    full_response = bytes.fromhex(full_response_hex)
    payload_decoded = convert_payload_to_hex(full_response)
    contact_info = ' '.join(payload_decoded.split(' ')[8:])
    contact_info = ' '.join([element.upper() if i < 3 else chr(int(element, 16)) for i, element in enumerate(contact_info.split(' '))])
    file.write(contact_info + '\n')
def read_eeprom():
    num_contact = int(read_eeprom_byte(MDC_NUM_ADD), 16)
    if num_contact == 0 or num_contact > max_contact:
        print("手台并无设置联系人,请先写入联系人！")
        input("按 Enter 键退出程序")
        sys.exit()

    print("开始读取MDC联系人")
    with open(file_out, 'w+') as file, serial.Serial(com_open, 38400, timeout=1) as ser:
        for a in range(num_contact):
            add = MDC_ADD[a // 4] + (a % 4) * 16
            print(f"{(a + 1) * 100 / num_contact}%", end='%\n')
            try:
                read_contact_information(ser, add, file)
            except ValueError as e:
                print(e)
                input("按 Enter 键退出程序")
                sys.exit()

    print("读取成功，联系人保存至 MDC.txt")
    return True
def write_eeprom_byte(add,num):
    global com_open
    with serial.Serial(com_open, 38400, timeout=1) as ser:
        payload = b'\x1D\x05' + b'\x09\x00' + add.to_bytes(2,
                                                           byteorder='little') + b'\x0F\x00' + b'\x82\x40\x74\x65' +  num.to_bytes(1, byteorder='big')
        # 将 payload 中的最后四个字节替换为当前时间戳
        crc = crc16_ccitt(payload)
        payload = payload + bytes([crc & 0xFF, ]) + bytes([crc >> 8, ])  # swap bytes of crc to get little endian
        message = b'\xAB\xCD' + b'\x0D\x00' + payload_xor(payload) + b'\xDC\xBA'
        ser.write(message)

        full_response = ser.read(128)
        if len(full_response) == 0:
            print("写入失败！")
            input("按 Enter 键退出程序")

            sys.exit()
def write_eeprom_select(a,my_str):
    global com_open
    with serial.Serial(com_open, 38400, timeout=1) as ser:
        add = MDC_ADD[a // 4] + (a % 4) * 16
        payload_data = ''.join(my_str).encode('utf-8')
        hex1 = int(payload_data[0:2], 16)
        hex2 = int(payload_data[2:4], 16)
        payload_data = bytearray([hex1, hex2]) + payload_data[5:]
        payload = b'\x1D\x05' + b'\x18\x00' + add.to_bytes(2,
                                                           byteorder='little') + b'\x10\x00' + b'\x82\x40\x74\x65' + payload_data
        # 将 payload 中的最后四个字节替换为当前时间戳
        crc = crc16_ccitt(payload)
        payload = payload + bytes([crc & 0xFF, ]) + bytes([crc >> 8, ])  # swap bytes of crc to get little endian
        message = b'\xAB\xCD' + b'\x1C\x00' + payload_xor(payload) + b'\xDC\xBA'
        ser.write(message)

        full_response = ser.read(128)
        if len(full_response) == 0:
            print("写入失败！")
            input("按 Enter 键退出程序")

            sys.exit()
def deal_blank_line():
    global line_count  # 声明 line_count 是全局变量
    with open(file_out, 'r') as file:
        lines = file.readlines()
    with open(file_out, 'w') as file:
        for line in lines:
            if line.strip():
                file.write(line)
    with open(file_out, 'r') as file:
        line_count = sum(1 for line in file)
    if line_count==0:
        print("请填写MDC联系人!")
        input("按 Enter 键退出程序")

        sys.exit()
    elif line_count>max_contact:
        print("MDC联系人数量最大为",max_contact)


def write_eeprom():
    print("正在将MDC.txt写入到手台中")
    global line_count  # 声明 line_count 是全局变量
    a=0
    deal_blank_line()
    update_file(file_out)
    with open(file_out, 'r') as file:
        for line in file:
            add = MDC_ADD[a // 4] + (a % 4) * 16
            my_str = []
            my_str.append(line.replace('\n', ''))  # 去除行尾的换行符
            write_eeprom_select(a,my_str)
            print((a+1) * 100 / line_count, end='%\n')
            a = a + 1
            if a==line_count:
                break

    write_eeprom_byte(MDC_NUM_ADD,line_count)
    print("写入成功")
    return True

# def main():



if __name__ == "__main__":
    available_ports = list(serial.tools.list_ports.comports())

    if available_ports:
        print("可用串口：")
        for port in available_ports:
            print(port.device)
    else:
        print("没有发现可用串口。")
    com_open=input("输入串口(例:COM4):")
    value=-1
    print("第一次使用MDC联系人请先写入联系人!!")
    while value!=0 and value!=1:
        value = int(input("写入(0)或读取(1)联系人:"))  # 获取用户输入的整数值

        if value == 0:
            time_set()
            write_eeprom()
        elif value == 1:
            time_set()
            read_eeprom()
        else:
            value = int(input("输入无效!\n输入(0/1)来(写入/读取)联系人:"))  # 获取用户输入的整数值
    input("按 Enter 键退出程序")

#    main()



# code =[0xAB,0xCD,0x6C,0x00,0x0B,0x69,0x7C,0xE6,0x5E,0x9F,0x6D,0x41,0xBF,0x61,0xA1,0x25,0x13,0x02,0xE8,0x80,0x16,0x93,0x15,0xE2,0xD1,0x91,0x0D,0x44,0x20,0x36,0xD5,0x41,0x13,0x03,0x24,0x80,0x16,0xA1,0xDB,0x29,0xD1,0x6E,0x0D,0x40,0xDE,0xCA,0x2A,0xBF,0x13,0x00,0xE1,0x80,0x16,0x6D,0x14,0xE6,0xD1,0x6E,0xF2,0xBF,0xDE,0xCA,0x2A,0xBF,0x12,0x03,0x16,0x7F,0xE9,0x93,0xEB,0x19,0x2E,0x91,0x0D,0x40,0xDE,0xCA,0x2A,0xBF,0x42,0x56,0xA8,0xCE,0x45,0x24,0x51,0xA8,0x69,0x91,0x0D,0x40,0x21,0x35,0xD5,0x40,0x46,0x55,0xC4,0xCB,0x23,0x6C,0x14,0xE6,0x2E,0x91,0x0D,0x40,0x21,0x35,0xD5,0x40,0x79,0x5A,0xDC,0xBA]
# # code=[0xAB, 0xCD, 0x1C, 0x00, 0x0B, 0x69, 0x0C, 0xE6, 0xCE, 0x80, 0x1D, 0x40, 0xBF, 0x61, 0xA1, 0x25, 0xEC, 0xFC, 0x16, 0x7F, 0xE9, 0x93, 0xEB, 0x19, 0xD1, 0x6E, 0xF2, 0xBF, 0xDE, 0xCA, 0x2A, 0xBF, 0xCF, 0x04, 0xDC, 0xBA]
# payload_decoded = convert_payload_to_hex(code)  # 跳过头部和尾部的校验信息
# hex_payload = ' '.join(['{:02X}'.format(byte) for byte in payload_decoded])
# print(hex_payload)

