import random
import string
import sys

def get_random_string(length):
    # choose from all lowercase letter
    letters = string.ascii_lowercase
    result_str = ''.join(random.choice(letters) for i in range(length))
    # print("Random string of length", length, "is:", result_str)
    print(result_str)

if (len(sys.argv) < 2):
    print("Falta passar o número de chars!")
    sys.exit(0)
get_random_string(int(sys.argv[1]))