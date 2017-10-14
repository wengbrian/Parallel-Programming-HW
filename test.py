import os
import array
# test testcase
os.system("mpicxx test.cc")
for i in range(0,10):
    input_file = './samples/testcase{:02d}'.format(i+1)
    out_file = './ans/sorted{:02d}'.format(i+1)
    ans_file = './samples/sorted{:02d}'.format(i+1)
    with open(input_file, 'rb') as f:
        s = f.read()
        in_arr = array.array('f', s)
    num_input = len(in_arr)
    with open(ans_file, 'rb') as f:
        s = f.read()
        sorted_arr = array.array('f', s)
    for num_proc in range(1, 5):
        print("srun -n {} -p debug ./a.out {} {} {}".format(num_proc, num_input, input_file, out_file))
        os.system("srun -n {} -p debug ./a.out {} {} {}".format(num_proc, num_input, input_file, out_file))
        with open(out_file, 'rb') as f:
            s = f.read()
            out_arr = array.array('f', s).tolist()
            for a,b,c in zip(out_arr, sorted_arr, range(len(out_arr))):
                if a != b:
                    print(False)
                    print(['%.2f'% v for v in out_arr[c-5:c+5]])
                    print(['%.2f'% v for v in sorted_arr[c-5:c+5]])
                    print(['%.2f'% v for v in out_arr[c-5:c+5]])
                    print(['%.2f'% v for v in sorted_arr[c-5:c+5]])
                    exit()
            print(True)
# test self create test file
#for i in range(100):
#    out_file = 'testcase{:02d}ans'.format(i+1)
    
