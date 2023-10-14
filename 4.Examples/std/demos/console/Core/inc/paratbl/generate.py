import re
import os
import shutil

filesrc = "./tbl.h"
filedst = filesrc

# regex
blocks = re.compile(r"typedef __packed struct {\n(.*?)} (\w+_t);", re.S)
lines = re.compile(r"((([u|s|f])(\d{1,2}) .*?(\[(\d{1,})\])?;\s+///<) ?(\d{1,})? ?(.*?))\n")

if os.path.exists(filesrc):

    ctx = ""

    if filedst == filesrc:  # backup
        backup_index = 1
        backup_filename = ""
        while True:
            backup_filename = filesrc + "." + str(backup_index) + ".bak"
            if not os.path.exists(backup_filename):
                break
            backup_index += 1
        shutil.copyfile(filesrc, backup_filename)

    with open(filesrc, "r") as fsrc:
        ctx = fsrc.read()

    with open(filedst, "w+") as fdst:

        # group
        for block in blocks.findall(ctx):

            grp_paras = block[0]
            grp_name = block[1]
            print(grp_name)

            id = 0

            # para
            for line in lines.findall(grp_paras):

                para_line = line[0]
                para_left = line[1]
                para_type = line[2]
                para_elem_size = line[3]
                para_array_size = line[5]
                para_right = line[7]

                if para_type == "s":
                    pass
                elif para_type == "u":
                    pass
                elif para_type == "f":
                    pass
                else:
                    print(para_type)
                    raise "error type"

                para_index = " " + ("0" * (3 - len(str(id)))) + str(id) + " "
                para_newline = para_left + para_index + para_right
                ctx = ctx.replace(para_line, para_newline)
                # print(para_newline)

                offset = 0

                if para_elem_size == "16":
                    offset = 1
                elif para_elem_size == "32":
                    offset = 2
                elif para_elem_size == "64":
                    offset = 4
                else:
                    print(para_elem_size)
                    raise "error size"

                if para_array_size != '':
                    offset *= int(para_array_size)

                id += offset

        fdst.write(ctx)
