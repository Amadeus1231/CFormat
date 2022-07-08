import subprocess
import os
from sys import argv
puk = os.path.exists(argv[1])
if puk:
    with open("result.txt", "w") as file:
        subprocess.run(f'clang-format {argv[1]} -n', stdout=None, stderr=file)
    with open("result.txt", "r") as file:
        text = file.read()
        if text:
            print("файл не соотв. критериям форматирования")
        else:
            print("файл отформатирован")
    os.remove("result.txt")
else:
    print("файл не существует")
