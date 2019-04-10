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

def make_cordinates(image, line_parameters):
    """
    y = mx + c
    y,x --> cordinates
    m --> slope
    c --> intercept

    here we have slope and intercept

    Now for the cordinates formula is
    y = mx + c
    x = (-c + y)/m
    x = (y - intercept)/slope
    ## final cordinates (x,y) from slope and intercept
    """
    slope, intercept = line_parameters
    y1 = image.shape[0]
    y2 = int(y1*(3/5))
    x1 = int((y1 - intercept)/slope)
    x2 = int((y2 - intercept)/slope)
    return np.array([x1, y1, x2, y2])

def averaged_slope_intercept(image, lines):
    """
    We are using np.polyfit it returns two parematers
    1. Slope
    2. Intercept

    From which -ve slope added to left_fit
                +ve slope added to right_fit
    """
    left_fit = []
    right_fit = []
    for line in lines:
        x1, y1, x2, y2 = line.reshape(4)  ## convert 2d into 1d
        paramters = np.polyfit((x1,x2), (y1, y2), 1)
        slope = paramters[0]
        intercept = paramters[1]
        if slope < 0:
            left_fit.append((slope, intercept))
        else:
            right_fit.append((slope, intercept))
    #average left & right
    left_fit_average = np.average(left_fit, axis=0)
    right_fit_average = np.average(right_fit, axis=0)
    #cordinates
    left_line = make_cordinates(image, left_fit_average)
    right_line = make_cordinates(image, right_fit_average)
    return np.array([left_line, right_line])

def display_lines(image, lines):
    line_image = np.zeros_like(image)
    if lines is not None:
        for line in lines:
            x1, y1, x2, y2 = line.reshape(4) #convert 2d array into 1d array
            cv2.line(line_image, (x1,y1), (x2,y2), (255,0,0), 10)
    return line_image

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
    gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
    blur = cv2.GaussianBlur(gray, (5,5),0)
    canny = cv2.Canny(blur, 50, 150)
    return canny

# image = cv2.imread(path)
# cv2.imshow('original Image', image)
# lane_image = np.copy(image)


######### Video Capture ################

videoPath = '/home/pradyum/The Complete Self-Driving Car Course - Applied Deep Learning/5. Computer Vision Finding Lane-Lines/13.1 test2.mp4.mp4'
cap = cv2.VideoCapture(videoPath)
while(cap.isOpened()):
    _,frame = cap.read()
    canny_image = Canny(frame)
    cropped_image = region_of_interest(canny_image)

    ## 2px with 1 degree precision
    lines = cv2.HoughLinesP(cropped_image, 2, np.pi/180,
                            100, np.array([]), minLineLength=40,
                            maxLineGap=5)
    averaged_lines = averaged_slope_intercept(frame, lines)
    line_image = display_lines(frame, averaged_lines)
    ## blending both the image
    combo_image = cv2.addWeighted(frame, 0.8, line_image, 1, 1)
    # cv2.imshow('result of region of interest', line_image)
    cv2.imshow("Combo image", combo_image)
    if cv2.waitKey(1) & 0xff == ord('q'):
        break
cap.release()
cv2.distroyAllWindows()













































#### ref

## printing the array of the image
    # np.set_printoptions(threshold=sys.maxsize)
    # print(np.array(newIMage[0]))
    # print(x[:1])

# cv2.imshow('original_image', lane_image)
# cv2.imshow('cannyImage', canny)

# plt.imshow(canny)
# plt.show()
    # print(x[:1])
