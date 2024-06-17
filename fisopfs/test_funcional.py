from time import sleep  
import os
import re
import argparse

MOUNT_POINT = "prueba"

symbols = {
  "tick": "\033[92m✔\033[0m",
  "cross": "\033[91m✘\033[0m",
  "cloud": "☁️"
}

def print_msg(symbol, message):
  # Seleccionar el símbolo correspondiente
  symbol_to_use = symbols.get(symbol, "❓")  # Usa un signo de interrogación si el símbolo no es válido
  # Imprimir el mensaje con el símbolo
  print(f"{symbol_to_use} {message}")

def sanear_directorios():
  entries = os.listdir(MOUNT_POINT)
  for entry in entries:
    if os.path.isdir(MOUNT_POINT+"/"+entry):
      os.rmdir(MOUNT_POINT+"/"+entry)
    else:
      os.remove(MOUNT_POINT+"/"+entry)
  
def test_create_dir_in_root():
  try:
    print("=== Crear directorio en root vacío ===")
    sanear_directorios()
    route = MOUNT_POINT+"/directorio_prueba"
    os.mkdir(route)
    if "directorio_prueba" in os.listdir(MOUNT_POINT):
      print_msg("tick", "Directorio se crea exitosamente.")
    else:
      print_msg("cross", "No se encuentra el directorio en root.")
  except Exception as e:
    print_msg("cross",e)

def test_delete_dir_in_root():
  try:
    print("=== Prueba de creación de directorio ===")
    sanear_directorios()
    route = MOUNT_POINT+"/directorio_prueba"
    os.mkdir(route)
    assert "directorio_prueba" in os.listdir(MOUNT_POINT)
    os.rmdir(route)
    if "directorio_prueba" in os.listdir(MOUNT_POINT):
      print_msg("cross", "No se eliminó el directorio.")
    else:
      print_msg("tick", "Directorio se elimina exitosamente.")
  except Exception as e:
    print_msg("cross",e)

def test_create_file_in_root():
  try:
    print("=== Prueba de creación de archivo en root ===")
    sanear_directorios()
    route = MOUNT_POINT+"/archivo_prueba.txt"
    f = open(route, 'w')
    f.close()
    if "archivo_prueba.txt" in os.listdir(MOUNT_POINT):
      print_msg("tick", "Archivo se crea exitosamente.")
    else:
      print_msg("cross", "No se encuentra el archivo en root.")
  except Exception as e:
    print_msg("cross",e)

def test_write_file_in_root():
  try:
    print("=== Prueba de escritura en archivo en root ===")
    sanear_directorios()
    route = MOUNT_POINT+"/archivo_prueba.txt"
    f = open(route, 'w')
    string = "Probando file system\n"
    f.write(string)
    f.close()
    f = open(route, 'r')
    buffer = f.read()
    f.close()
    assert "archivo_prueba.txt" in os.listdir(MOUNT_POINT)
    if buffer == string:
      print_msg("tick", "Escritura en archivo exitosa.")
    else:
      print_msg("cross", "Escritura en archivo fallida.")
  except Exception as e:
    print_msg("cross",e)

def test_delete_file_in_root():
  try:
    print("=== Prueba de eliminación de archivo en root ===")
    sanear_directorios()
    route = MOUNT_POINT+"/archivo_prueba.txt"
    f = open(route, 'w')
    f.close()
    assert "archivo_prueba.txt" in os.listdir(MOUNT_POINT)
    os.remove(route)
    if "archivo_prueba.txt" in os.listdir(MOUNT_POINT):
      print_msg("cross", "No se eliminó el archivo.")
    else:
      print_msg("tick", "Archivo se elimina exitosamente.")
  except Exception as e:
    print_msg("cross",e)

def test_create_dir_in_dir():
  try:
    print("=== Prueba de creación de directorio en directorio ===")
    sanear_directorios()
    route = MOUNT_POINT+"/directorio_prueba"
    os.mkdir(route)
    assert "directorio_prueba" in os.listdir(MOUNT_POINT)
    route = MOUNT_POINT+"/directorio_prueba/directorio_prueba2"
    os.mkdir(route)
    if "directorio_prueba2" in os.listdir(MOUNT_POINT+"/directorio_prueba"):
      print_msg("tick", "Directorio se crea exitosamente.")
    else:
      print_msg("cross", "No se encuentra el directorio en el directorio padre.")
  except Exception as e:
    print_msg("cross",e)

def test_delete_dir_in_dir():
  try:
    print("=== Prueba de eliminación de directorio en directorio ===")
    sanear_directorios()
    route = MOUNT_POINT+"/directorio_prueba"
    os.mkdir(route)
    assert "directorio_prueba" in os.listdir(MOUNT_POINT)
    route = MOUNT_POINT+"/directorio_prueba/directorio_prueba2"
    os.mkdir(route)
    assert "directorio_prueba2" in os.listdir(MOUNT_POINT+"/directorio_prueba")
    os.rmdir(route)
    if "directorio_prueba2" in os.listdir(MOUNT_POINT+"/directorio_prueba"):
      print_msg("cross", "No se eliminó el directorio.")
    else:
      print_msg("tick", "Directorio se elimina exitosamente.")
  except Exception as e:
    print_msg("cross",e)

def test_create_file_in_dir():
  try:
    print("=== Prueba de creación de archivo en directorio ===")
    sanear_directorios()
    route = MOUNT_POINT+"/directorio_prueba"
    os.mkdir(route)
    assert "directorio_prueba" in os.listdir(MOUNT_POINT)
    route = MOUNT_POINT+"/directorio_prueba/archivo_prueba.txt"
    f = open(route, 'w')
    f.close()
    if "archivo_prueba.txt" in os.listdir(MOUNT_POINT+"/directorio_prueba"):
      print_msg("tick", "Archivo se crea exitosamente.")
    else:
      print_msg("cross", "No se encuentra el archivo en el directorio.")
  except Exception as e:
    print_msg("cross",e)

def test_delete_file_in_dir():
  try:
    print("=== Prueba de eliminación de archivo en directorio ===")
    sanear_directorios()
    route = MOUNT_POINT+"/directorio_prueba"
    os.mkdir(route)
    assert "directorio_prueba" in os.listdir(MOUNT_POINT)
    route = MOUNT_POINT+"/directorio_prueba/archivo_prueba.txt"
    f = open(route, 'w')
    f.close()
    assert "archivo_prueba.txt" in os.listdir(MOUNT_POINT+"/directorio_prueba")
    os.remove(route)
    if "archivo_prueba.txt" in os.listdir(MOUNT_POINT+"/directorio_prueba"):
      print_msg("cross", "No se eliminó el archivo.")
    else:
      print_msg("tick", "Archivo se elimina exitosamente.")
  except Exception as e:
    print_msg("cross",e)

def test_delete_dir_deletes_childrens():
  try:
    print("=== Prueba de eliminación de directorio con hijos ===")
    sanear_directorios()
    route = MOUNT_POINT+"/directorio_prueba"
    os.mkdir(route)
    assert "directorio_prueba" in os.listdir(MOUNT_POINT)
    route = MOUNT_POINT+"/directorio_prueba/archivo_prueba.txt"
    f = open(route, 'w')
    f.close()
    assert "archivo_prueba.txt" in os.listdir(MOUNT_POINT+"/directorio_prueba")
    os.rmdir(MOUNT_POINT+"/directorio_prueba")
    assert "directorio_prueba" not in os.listdir(MOUNT_POINT)
    try:
      open(MOUNT_POINT+"/directorio_prueba/archivo_prueba.txt")
      assert False
    except IOError:
      assert True
    if "directorio_prueba" in os.listdir(MOUNT_POINT):
      print_msg("cross", "No se eliminó el directorio.")
    else:
      print_msg("tick", "Directorio y sus archivos se eliminan exitosamente.")
  except Exception as e:
    print_msg("cross",e)

if __name__ == "__main__":
  print("[[[[[ Pruebas de directorios ]]]]]")
  test_create_dir_in_root()
  test_delete_dir_in_root()
  test_create_dir_in_dir()
  test_delete_dir_in_dir()

  print("\n\n[[[[[ Pruebas de archivos ]]]]]")
  test_create_file_in_root()
  test_write_file_in_root()
  test_delete_file_in_root()
  test_create_file_in_dir()
  test_delete_file_in_dir()

  print("\n\n[[[[[ Pruebas de archivos y directorios ]]]]]")
  test_delete_dir_deletes_childrens()
