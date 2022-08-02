import os


for i in range(1,5):
    print("Executando código com " + str(i) + " thread(s):")
    for j in range(20):
        stream = os.popen("./lcs ../testes/fileA.in ../testes/fileB.in " + str(i))
        output = stream.read()
        print()
        print(output)