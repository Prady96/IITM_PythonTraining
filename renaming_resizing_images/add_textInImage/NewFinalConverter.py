# Basic Outline of the program to be completed
# 1. pass arguments
# 2. Renaming all the folder files in python
# 3. check the file initilised
# 4. Conversion to jpg files
# 5 Resizing the JPG files

from PIL import Image
import os
import argparse

# 1. pass arguments
parser = argparse.ArgumentParser(description='Python file converter')
parser.add_argument('-f', '--folder', help="Folder path for all the image files", required=True)
args = parser.parse_args()
print(args)  # print dictionary


# 2. Renaming all the folder files in python

# 3. check the file initilised
print("folder for image files {}".format(args.folder))
cntJPG = 0
cntPNG = 0

# 4. Conversion to jpg files
# for files in named directory
for f in os.listdir(args.folder):
    f_name, f_ext = os.path.splitext(f)
    if f_ext == '.':  # for the jpg directory
        i = Image.open(f)
        fn, fext = os.path.splitext(f)  # split text for images file
        print(fn)
        # print(f)
        cntJPG += 1

    if f.endswith('.png'):  # for the png directory
        # print(f)
        cntPNG += 1

print("count for the PNG Image {}".format(cntPNG))
print("count for the JPG Image {}".format(cntJPG))

# 5 Resizing the JPG files
