import sys

head = '''
#ifndef __CUB_MACRO_REPEAT_CALL_2_H_
#define __CUB_MACRO_REPEAT_CALL_2_H_

'''

def gen_repeat_call_macro(n):
    all_macros = ""
    for i in range(n):
        all_macros = all_macros + "#define __CUB_repeat_call_2_{}(call, i, x, ...)".format(i)
        if i > 0:
            all_macros = all_macros + " call(i, x) __CUB_repeat_call_2_{}(call, i+1, __VA_ARGS__)".format(i - 1)

        all_macros = all_macros + "\n"

    return all_macros

def main():
    # print command line arguments
    if len(sys.argv) != 3:
        print("specify the num and file")
        exit(-1)

    n = int(sys.argv[1]) + 1
    lines = head +  gen_repeat_call_macro(n) + "\n\n#endif"

    file = open(sys.argv[2], "w")
    file.writelines(lines)
    file.close()

if __name__ == "__main__":
    main()