import os
import numpy as np 

path = "../testes/"

arquivos = [
    ["DNA10000A", "DNA10000B", "DNA 10000"],
    ["DNA50000A", "DNA50000B", "DNA 50000"],
    ["DNA100000A", "DNA100000B", "DNA 100000"],
    ["DNA500000A", "DNA500000B", "DNA 500000"],
    ["10000A", "10000B", 10000],
    ["50000A", "50000B", 50000],
    ["100000A", "100000B", 100000],
    ["500000A", "500000B", 500000],
]

for n_threads in range(1,5):
    print("Padrão de saída: \nmédia; DP;")
    print("Executando código com " + str(n_threads) + " thread(s):")
    for arq in arquivos:
        print("Teste de arquivos de entrada " + str(arq[2]))
        array_total = []
        array_paral = []
        for j in range(20):
            stream = os.popen("./lcs " + path + arq[0] + " " + path + arq[1] + " " + str(n_threads))
            output = stream.readlines()
            # print(int(output[0]))
            array_total.append(float(output[1]))
            array_paral.append(float(output[2]))
        print(str(np.average(array_total)) + "; " + str(np.std(array_total)) + "; ")
        # exit(0)
