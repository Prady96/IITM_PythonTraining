"""
Edge Detection : identifying sharp changes in intensity
"""
"""
GaussiaBlur : "5x5 fixed kernel size" -- blur image
"""
path = '/home/pradyum/The Complete Self-Driving Car Course - Applied Deep Learning/5. Computer Vision Finding Lane-Lines/2.1/Image/test_image.jpg'
path1 = '/home/pradyum/Github/IITM_PythonTraining/dataStructure/images/clm_t_1.jpg'
path2 = '/home/pradyum/Github/IITM_PythonTraining/dataStructure/images/clm_t_2.jpg'
path3 = '/home/pradyum/Github/IITM_PythonTraining/dataStructure/images/clm_t_3.jpg'
path4 = '/home/pradyum/Github/IITM_PythonTraining/dataStructure/images/clm_t_4.jpg'
path5 = '/home/pradyum/Github/IITM_PythonTraining/dataStructure/images/clm_t_5.jpg'
path6 = '/home/pradyum/Github/IITM_PythonTraining/dataStructure/images/clm_t_6.jpg'
path7 = '/home/pradyum/Github/IITM_PythonTraining/dataStructure/images/clm_t_7.jpg'


import cv2
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
import sys

def region_of_interest(image):
    """
    This will depict the masked area from the image
    """
    height = image.shape[0]
    polygons = np.array([
    [(200, height), (1100, height), (550, 250)]
    ])
    mask = np.zeros_like(image) #black image
    newIMage = cv2.fillPoly(mask, polygons, 255) #polygon embed masked image
    masked_image = cv2.bitwise_and(image, mask) #bitwise and operation performed
    return masked_image


def Canny(image):
    gray = cv2.cvtColor(lane_image, cv2.COLOR_RGB2GRAY)
    blur = cv2.GaussianBlur(gray, (5,5),0)
    canny = cv2.Canny(blur, 50, 150)
    return canny
    # print(x[:1])
image = cv2.imread(path)
cv2.imshow('original Image', image)
lane_image = np.copy(image)
canny = Canny(lane_image)
cropped_image = region_of_interest(canny)
lines = cv2.HoughLinesP(cropped_image, )
cv2.imshow('result of region of interest', cropped_image)
cv2.waitKey(0)
















#### ref

## printing the array of the image
    # np.set_printoptions(threshold=sys.maxsize)
    # print(np.array(newIMage[0]))
    # print(x[:1])

# cv2.imshow('original_image', lane_image)
# cv2.imshow('cannyImage', canny)

# plt.imshow(canny)
# plt.show()
