import os
import argparse
import glob

parser = argparse.ArgumentParser(prog='binToHeader', description='Converts a binary to a C array')
parser.add_argument('inputFiles', nargs='+', help="List of input binaries to convert to headers (supports wildcards)")
parser.add_argument('-o', '--output', required=True, help="Output file name")

args = parser.parse_args()

expandedInputFiles = []
for pattern in args.inputFiles:
    expandedInputFiles.extend(glob.glob(pattern))

if (expandedInputFiles):
    print(f"Converting {len(expandedInputFiles)} file(s):")
    for file in expandedInputFiles:
         print(os.path.abspath(file))

arrayRowSize = 16

with open(args.output, 'w') as outFile:
    contents = ''
    for file in expandedInputFiles:
        arrayName = os.path.basename(file).partition(".")[0] + "_binary"
        contents += f'const unsigned char {arrayName}[] = {{'
        with open(file, 'rb') as inFile:
            for byte in range(os.path.getsize(file)):
                    if (byte % arrayRowSize == 0):
                         contents += '\n    '
                    contents += '0x' + inFile.read(1).hex() + ', '
        contents += f'\n}};\n\n'

        contents += f'const int {arrayName}_size = sizeof({arrayName});\n\n'

    outFile.write(contents)
