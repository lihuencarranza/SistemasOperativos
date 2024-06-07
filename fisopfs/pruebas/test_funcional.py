

from time import sleep  
import os  

try:
        f = open("test_funcional.txt", 'w') # creamos el archivo
        string = "Probando file system\n"
        f.write(string) # escribimos
        string = "Segunda linea"
        f.write(string)
        f.close() # cerramos archivo

        f = open("test_funcional.txt", 'r') # lo abrimos en modo lectura
        lines = f.readlines() # cargamos todas las líneas del archivo en una lista en memoria
        f.close() # cerramos.
        print("lineas guardadas en el archivo:")
        for line in lines:
                print(line)

        sleep(5) #esperar 5 seg
        os.remove("test_funcional.txt") #borrar el archivo
        print("test_funcional.txt borrado. prueba exitosa.")
        
except Exeption as e:  # si falla el file system espero agarrarlo acá.
        print(e)  

