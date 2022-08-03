import os

path = "/home/vinicius/Desktop/UFPR/Paralela/T1/testes/"

arquivos = [
    # ["DNA10000A", "DNA10000B", 10000],
    # ["DNA50000A", "DNA50000B", 50000],
    ["DNA100000A", "DNA100000B", 100000],
    ["DNA500000A", "DNA500000B", 500000],
]

for n_threads in range(1,5):
    print("Executando código com " + str(n_threads) + " thread(s):")
    for arq in arquivos:
        print("Teste de arquivos de entrada " + str(arq[2]))
        for j in range(20):
            stream = os.popen("./lcs " + path + arq[0] + " " + path + arq[1] + " " + str(n_threads))
            output = stream.read()
            print()
            print(output)