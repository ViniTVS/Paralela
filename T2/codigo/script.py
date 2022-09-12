import os
import numpy as np 

path = "../../"

arquivos = [
#    ["DNA10000A", "DNA10000B", "DNA 10000"],
    #["DNA50000A", "DNA100000B", "DNA 50000"],
    #["DNA100000A", "DNA100000B", "DNA 100000"],
    ["100000.in", "200000.in2", 100000],
    ["200000.in", "200000.in2", "2 x 4"],
    ["200000.in", "400000.in2", "4 x 4"]
]


f = open("saida.txt", "w")

for n_threads in ["8 --use-hwthread-cpus", 4, 2, 1]:
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
            stream = os.popen("mpirun lcs " + str(n_threads) + " " + path + arq[0] + " " + path + arq[1])
            output = stream.readlines()
            print(float(output[1]))
            array_total.append(float(output[1]))
            array_paral.append(float(output[2]))
        saida = str(np.average(array_total)) + "; " + str(np.std(array_total)) + "; "
        f.write(saida + "\n")
        print(saida)
        # exit(0)


f.close()
