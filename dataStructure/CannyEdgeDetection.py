import cv2

path = '/home/pradyum/The Complete Self-Driving Car Course - Applied Deep Learning/5. Computer Vision Finding Lane-Lines/2.1/Image/test_image.jpg'

## lanes
"""
two main functions
1. imread
2. imshow
3. waitKey
"""
image = cv2.imread(path)
cv2.imshow('result', image)
cv2.waitKey(0)

"""
image

"""
