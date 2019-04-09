"""
Edge Detection : identifying sharp changes in intensity
"""
"""
GaussiaBlur : "5x5 fixed kernel size" -- blur image
"""


path = '/home/pradyum/The Complete Self-Driving Car Course - Applied Deep Learning/5. Computer Vision Finding Lane-Lines/2.1/Image/test_image.jpg'
path2 = '/home/pradyum/Github/IITM_PythonTraining/dataStructure/images/classroom_test.jpg'


import cv2
import numpy as np

image = cv2.imread(path2)
lane_image = np.copy(image)
gray = cv2.cvtColor(lane_image, cv2.COLOR_RGB2GRAY)
blur = cv2.GaussianBlur(gray, (5,5),0)
canny = cv2.Canny(blur, 50, 150)
cv2.imshow('result', canny)
cv2.waitKey()
