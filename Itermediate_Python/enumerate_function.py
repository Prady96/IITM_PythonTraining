example = ['left', 'right', 'down', 'up']

print()
print("Normal Function")
# Normal Function
"""
i -- index
example[i] -- value at index position
"""
for i in range(len(example)):
    print(i, example[i])

print()
print("Enumerate Function")
# Enumerate Function
"""
i -- index
j -- value for the index pos
"""
# we dont require to put example index
for i, j in enumerate(example):
    print(i, j)

print()
print("Enuemrate with dictionary")
# Enumerate to form dictionary from lists
# dictionry function
new_dict = dict(enumerate(example))
print(new_dict)

print()
# print("Enumerate inside dict")
# Enumerate inside dict
# [print(i, j) for i, j in enumerate(new_dict)]


# OUTPUT
"""

root@pradyum-Lenovo-B40-80:~/django-apps/freshInstall/Intermediate_python/IITM_PythonTraining/Itermediate_Python# python enumerate_function.py
()
Normal Function
(0, 'left')
(1, 'right')
(2, 'down')
(3, 'up')
()
Enumerate Function
(0, 'left')
(1, 'right')
(2, 'down')
(3, 'up')
()
Enuemrate with dictionary
{0: 'left', 1: 'right', 2: 'down', 3: 'up'}
()

"""
