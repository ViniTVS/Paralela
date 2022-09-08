import os
import numpy as np 

path = "../testes/"

arquivos = [
#    ["DNA10000A", "DNA10000B", "DNA 10000"],
    #["DNA50000A", "DNA100000B", "DNA 50000"],
    #["DNA100000A", "DNA100000B", "DNA 100000"],
    #["DNA500000A", "DNA500000B", "DNA 500000"],
    #["10000A", "10000B", 10000],
#    ["50000A", "50000B", 50000],
    ["50000A", "100000B", 50000],
    ["100000A", "100000B", 100000],
    ["10000A", "500000B", 500000],
    ["100000A", "500000B", 500000],
    ["500000A", "500000B", 500000],
]


f = open("saida.txt", "w")

for n_threads in reversed(range(1,5)):
    print("Padrão de saída: \nmédia; DP;")
    print("\tExecutando código com " + str(n_threads) + " thread(s):")
    f.write("Padrão de saída: \nmédia; DP;" + "\n")
    f.write("\tExecutando código com " + str(n_threads) + " thread(s):" + "\n")
    for arq in arquivos:
        print("Teste de arquivos de entrada " + arq[0] + " e " + arq[1])
        f.write("Teste de arquivos de entrada " + arq[0] + " e " + arq[1] + "\n")
        array_total = []
        array_paral = []
        for j in range(20):
            stream = os.popen("./lcs " + path + arq[0] + " " + path + arq[1] + " " + str(n_threads))
            output = stream.readlines()
            print(float(output[1]))
            array_total.append(float(output[1]))
            array_paral.append(float(output[2]))
        saida = str(np.average(array_total)) + "; " + str(np.std(array_total)) + "; "
        f.write(saida + "\n")
        print(saida)
        # exit(0)


f.close()
