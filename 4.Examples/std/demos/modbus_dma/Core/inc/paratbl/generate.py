# python3

import re
import os
import shutil

__block = r'''(EXPORT_PARA_GROUP typedef union \{
    __packed struct \{(.*?)\};
    u16 GROUP\[GROUP_SIZE\];.*?
\} (\w+_u);)'''

__line = r'''((\w+) \w+(\[(\d{1,})\])?;) {0,}///< ?(P\d{0,}\.\d{0,})? ?(.*?\n)'''


filesrc = "./tbl.h"
filedst = filesrc


def backup(filesrc):
    index = 1
    filedst = ""
    while True:
        filedst = filesrc + "." + str(index) + ".bak"
        if not os.path.exists(filedst):
            break
        index += 1
    shutil.copyfile(filesrc, filedst)


def add_prefix(number, prefix, length):
    result = str(number)
    delta = length - len(result)
    if delta > 0:
        result = prefix * delta + result
    return result


def generate_id(group, offset):
    return " ///< P" + add_prefix(group, "0", 2) + "." + add_prefix(offset, "0", 3) + " "


if os.path.exists(filesrc):

    file_ctx = ""

    if filedst == filesrc:  # backup
        backup(filesrc)

    with open(filesrc, "r", encoding="utf-8") as fsrc:
        file_ctx = ''.join(fsrc.readlines())

    with open(filedst, "w", encoding="utf-8") as fdst:

        # regex
        blocks = re.compile(__block, re.S)
        lines = re.compile(__line)

        # groups
        for grp_idx, block in enumerate(blocks.findall(file_ctx)):
            _grp_ctx = block[0]
            _grp_paras = block[1]
            _grp_name = block[2]

            # params
            para_elem_offset = 0
            para_elem_type = {
                "s16": 1, "s32": 2, "s64": 4,
                "u16": 1, "u32": 2, "u64": 4,
                "f32": 2, "f64": 4,
            }

            grp_paras = [""]

            for line in lines.findall(_grp_paras):

                _para_ctx_left = line[0]
                _para_ctx_right = line[5]

                _para_elem_type = line[1]
                _para_array_size = line[3]

                para_new_line = (_para_ctx_left + generate_id(grp_idx, para_elem_offset) + _para_ctx_right)
                grp_paras.append(para_new_line)
                # print(para_new_line)

                try:
                    para_array_size = 1 if _para_array_size == '' else int(_para_array_size)
                    para_elem_offset += para_elem_type[_para_elem_type] * para_array_size
                except:
                    raise f"error type: {_para_elem_type}"

            grp_paras_ctx = ""
            grp_paras_ctx += "EXPORT_PARA_GROUP typedef union {\n"
            grp_paras_ctx += "    __packed struct {\n"
            grp_paras_ctx += "        ".join(grp_paras)
            grp_paras_ctx += "    };\n"
            grp_paras_ctx += "    u16 GROUP[GROUP_SIZE];" + generate_id(grp_idx, para_elem_offset-1) + "\n"
            grp_paras_ctx += "} " + _grp_name + ";"

            file_ctx = file_ctx.replace(_grp_ctx, grp_paras_ctx)

        fdst.write(file_ctx)


# grp_idx = 0
# grp_elem_offset = 0
# grp_name = "arr_a"


# print(ctx)
