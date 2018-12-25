## Installation of PIL file
## Run this script in the . dir
## installation from source
#wget https://github.com/python-pillow/Pillow/archive/master.zip
#unzip master.zip
#cd Pillow-master/
#python setup.py install
#python3
#>>>import PIL "check PIL"
#ctrl + d

from PIL import Image
import os

## os is mainly used for the change/Move in directory
## create dirs for the images
def create_project_dirs(directory):
    if not os.path.exists(directory):
        print('create project ' + directory)
        os.makedirs(directory)


# create function for the directory
# create_project_dirs('300')
create_project_dirs('700')
create_project_dirs('compressed')

## resize images in which you want
# size_300 = (300,300) ## 300 pixel file sizes
size_700 = (700,700) ## 700 pixel file sizes

##
for f in os.listdir('.'):
    if f.endswith('.jpg'):
        i = Image.open(f)               ## creating an image object of each image
        fn, fext = os.path.splitext(f)  ## split the fist name for each image
        # print(fn)
        # print(fext)
        i.save('compressed/{}{}'.format(fn,fext)) ##Now save them in .png image with same as fist name
        # i.thumbnail(size_300)
        # i.save('300/{}{}'.format(fn,fext))

        i.thumbnail(size_700)
        i.save('700/{}{}'.format(fn,fext))
    else:
        i = Image.open(f)               ## creating an image object of each image
        fn, fext = os.path.splitext(f)  ## split the fist name for each image
        # print(fn)
        # print(fext)
        i.save('compressed/{}{}'.format(fn,fext)) ##Now save them in .png image with same as fist name
        # i.thumbnail(size_300)
        # i.save('300/{}{}'.format(fn,fext))

        i.thumbnail(size_700)
        i.save('700/{}{}'.format(fn,fext))


