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
print("Enumerate inside dict")
# Enumerate inside dict
[print(i, j) for i, j in enumerate(new_dict)]
