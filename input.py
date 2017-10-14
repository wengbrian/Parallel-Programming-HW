import array
import argparse

parser = argparse.ArgumentParser(description='Proce')
parser.add_argument('--path', help='python input.py --path=path')

args = parser.parse_args()
path = args.path
with open(path, 'rb') as f:
    s = f.read()
    print(array.array('f', s))
    print(len(array.array('f', s)))
