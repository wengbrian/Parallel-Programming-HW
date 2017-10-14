import os
import array
# test testcase
for i in range(10):
    input_file = './samples/testcase{:02d}'.format(i+1)
    out_file = './ans/testcase{:02d}ans'.format(i+1)
    with open(input_file, 'rb') as f:
        s = f.read()
        in_arr = array.array('f', s)
    num_input = len(in_arr)
    sorted_arr = sorted(in_arr)
    for num_proc in range(1, 4):
        print("srun -n {} -p batch ./a.out {} {} {}".format(num_proc, num_input, input_file, out_file))
        os.system("srun -n {} -p batch ./a.out {} {} {}".format(num_proc, num_input, input_file, out_file))
        with open(out_file, 'rb') as f:
            s = f.read()
            out_arr = array.array('f', s).tolist()
            if out_arr == sorted_arr:
                print(True)
            else:
                print(False)
                print(out_arr)
                print(sorted_arr)
# test self create test file
#for i in range(100):
#    out_file = 'testcase{:02d}ans'.format(i+1)
    
