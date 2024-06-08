

from time import sleep  
import os  

try:
        print("=== Prueba de lectura de archivo ===")
        print("Abro archivo base")
        f = open("prueba/fisop", 'r') # abro el archivo en modo lectura
        print("Leo contenido")
        buffer = f.read() # leo el archivo completo\
        print("Imprimo contenido")
        print(buffer) # imprimo el contenido

        # string = "Probando file system\n"
        # f.write(string) # escribimos
        # string = "Segunda linea"
        # f.write(string)
        f.close() # cerramos archivo

        # f = open("test_funcional.txt", 'r') # lo abrimos en modo lectura
        # lines = f.readlines() # cargamos todas las líneas del archivo en una lista en memoria
        # f.close() # cerramos.
        # print("lineas guardadas en el archivo:")
        # for line in lines:
        #         print(line)

        # # sleep(5) #esperar 5 seg
        # os.remove("test_funcional.txt") #borrar el archivo
        # print("test_funcional.txt borrado. prueba exitosa.")
        
except Exception as e:  # si falla el file system espero agarrarlo acá.
        print(e)  

