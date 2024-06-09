import os

list1 = [
    'ENABLE_CHINESE_FULL=4',
    'ENABLE_CHINESE_FULL=0'
]

list2 = [
    'ENABLE_ENGLISH=1'
]

list3 = [
    'ENABLE_MESSENGER=1',
    'ENABLE_MESSENGER_DELIVERY_NOTIFICATION=1',
    'ENABLE_MESSENGER_NOTIFICATION=1'
]

list4 = [
    'ENABLE_DOPPLER=1',
]

list5 = [
    'ENABLE_MDC1200=1',
    'ENABLE_MDC1200_EDIT=1',
    'ENABLE_MDC1200_CONTACT=1'
]

list6 = [
    'ENABLE_4732=1',
    'ENABLE_FMRADIO=1',
]

list7 = [
    'ENABLE_PINYIN=1',
]

list8 = [
    'ENABLE_SPECTRUM=1'
]

list9 = [
    'ENABLE_4732SSB=1'
]

all_lists = [list1, [[], list3], [[], list4], [[], list5], [[], list6[0], list6[1]], [[], list7], [[], list8],
             [[], list9],[[],list2]]

strx = []
stry = []

indexes = [0] * len(all_lists)


def find_first_non_zero_index(s):
    """
    从后往前查找字符串中第一个不为 '0' 的字符的索引
    :param s: 输入字符串
    :return: 第一个不为 '0' 的字符的索引，如果没有返回 -1
    """
    # 从后往前遍历字符串
    for i in range(len(s) - 1, -1, -1):
        if s[i] != '0':
            return i - 6
    return -1


while indexes[0] < len(all_lists[0]):
    chinese = all_lists[0][indexes[0]]

    indexes[1] = 0
    while indexes[1] < len(all_lists[1]):
        messenger = all_lists[1][indexes[1]]

        indexes[2] = 0
        while indexes[2] < len(all_lists[2]):
            doppler = all_lists[2][indexes[2]]

            indexes[3] = 0
            while indexes[3] < len(all_lists[3]):
                mdc1200 = all_lists[3][indexes[3]]

                indexes[4] = 0
                while indexes[4] < len(all_lists[4]):
                    fm = all_lists[4][indexes[4]]

                    indexes[5] = 0
                    while indexes[5] < len(all_lists[5]):

                        if chinese[-1] == '0' and indexes[5] == 1:
                            indexes[5] += 1
                            continue
                        pinyin = all_lists[5][indexes[5]]

                        indexes[6] = 0
                        while indexes[6] < len(all_lists[6]):
                            spectrum = all_lists[6][indexes[6]]
                            indexes[7] = 0
                            while indexes[7] < len(all_lists[7]):
                                ssb = all_lists[7][indexes[7]]
                                if ssb and fm != list6[0]:
                                    indexes[7] += 1
                                    continue
                                indexes[8] = 0
                                while indexes[8] < len(all_lists[8]):
                                    english = all_lists[8][indexes[8]]
                                    strm = ''
                                    strn = ''
                                    strm += chinese + ' '
                                    strn += chinese[-1]
                                    if messenger:
                                        strm += " ".join(list3) + ' '
                                        strn += '1'
                                    else:
                                        strn += '0'

                                    if doppler:
                                        strm += " ".join(list4) + ' '
                                        strn += '1'
                                    else:
                                        strn += '0'

                                    if mdc1200:
                                        strm += " ".join(list5) + ' '
                                        strn += '1'
                                    else:
                                        strn += '0'

                                    if fm:
                                        strm += fm + ' '
                                        strn += fm[7]
                                    else:
                                        strn += '0'
                                    if pinyin:
                                        strm += " ".join(list7) + ' '
                                        strn += '1'
                                    else:
                                        strn += '0'

                                    if spectrum:
                                        strm += " ".join(list8) + ' '
                                        strn += '1'
                                    else:
                                        strn += '0'
                                    if ssb:
                                        strm += " ".join(list9) + ' '
                                        strn += '1'
                                    else:
                                        strn += '0'

                                    if english:
                                        strm += " ".join(list2) + ' '
                                        strn += '1'
                                    else:
                                        strn += '0'
                                    cuscanhshu_value = strm
                                    customname_value = 'LOSEHU' + strn
                                    result = 0
                                    result = os.system(
                                        'make full_all CUSCANSHU="{}" CUSTOMNAME="{}"'.format(cuscanhshu_value.rstrip()
                                                                                              ,
                                                                                              customname_value))

                                    # print('make full_all CUSCANSHU=' + cuscanhshu_value + ' CUSTOMNAME=' + customname_value)
                                    # if customname_value=='LOSEHU40104111':
                                    #     result=1

                                    # 仅在文件夹1中的文件: {'LOSEHU4010F110.bin', 'LOSEHU0010F000.bin', 'LOSEHU4010F000.bin', 'LOSEHU0010F100.bin', 'LOSEHU4010F010.bin', 'LOSEHU4010F100.bin', 'LOSEHU0010F010.bin', 'LOSEHU0010F110.bin'}
                                    if result == 0:
                                        pass
                                    else:
                                        index = find_first_non_zero_index(customname_value)
                                        if index != -1:
                                            indexes[len(all_lists)-1] = 2
                                            for i in range(index + 1, len(all_lists)-1):
                                                indexes[i] = len(all_lists[i]) - 1
                                        continue
                                    indexes[8] += 1
                                indexes[7] += 1
                            indexes[6] += 1
                        indexes[5] += 1
                    indexes[4] += 1
                indexes[3] += 1
            indexes[2] += 1
        indexes[1] += 1
    indexes[0] += 1
