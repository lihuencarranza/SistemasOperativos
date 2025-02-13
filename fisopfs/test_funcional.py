from time import sleep  
import os
import re
import argparse
import shutil

MOUNT_POINT = "prueba"

symbols = {
  "tick": "\033[92m✔\033[0m",
  "cross": "\033[91m✘\033[0m",
  "cloud": "☁️"
}

def print_msg(symbol, message):
  symbol_to_use = symbols.get(symbol, "❓") 
  print(f"{symbol_to_use} {message}")

def sanear_directorios():
    for root, dirs, files in os.walk('prueba', topdown=False):
        for name in files:
            os.remove(os.path.join(root, name))
        for name in dirs:
            os.rmdir(os.path.join(root, name))

  
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


# CHALLENGE

def test_chmod_file():
  try:
    print("=== Prueba de cambio de permisos de archivo ===")
    sanear_directorios()
    route = MOUNT_POINT+"/archivo_prueba.txt"
    f = open(route, 'w')
    f.close()
    os.chmod(route, 0o777)
    if re.match(r"^-rwxrwxrwx", os.popen(f"ls -l {route}").read().split()[0]):
      print_msg("tick", "Permisos de archivo se cambian exitosamente.")
    else:
      print_msg("cross", "No se cambiaron los permisos de archivo.")
  except Exception as e:
    print_msg("cross",e) 

def test_chown_file():
    try:
        print("=== Prueba de cambio de propietario de archivo ===")
        sanear_directorios()
        route = os.path.join(MOUNT_POINT, "archivo_prueba_1.txt")
        f = open(route, 'w')
        f.close()
        
        new_uid = 1001  
        new_gid = 1001  
        
        os.chown(route, new_uid, new_gid)
        output = os.popen(f"ls -l {route}").read().split()
        if int(output[2]) == new_uid and int(output[3]) == new_gid:
            print_msg("tick", "Propietario de archivo se cambia exitosamente.")
        else:
            print_msg("cross", "No se cambió el propietario de archivo.")
    except Exception as e:
        print_msg("cross", str(e))
    os.remove(route)

def test_list_files_in_dir():
  try:
    print("=== Prueba de listado de archivos en directorio ===")
    sanear_directorios()
    route = os.path.join(MOUNT_POINT, "directorio_prueba")
    os.mkdir(route)
    file1 = os.path.join(route, "archivo1.txt")
    file2 = os.path.join(route, "archivo2.txt")
    with open(file1, 'w') as f:
        f.write("Contenido de archivo 1")
    with open(file2, 'w') as f:
        f.write("Contenido de archivo 2")
    files = os.listdir(route)
    if "archivo1.txt" in files and "archivo2.txt" in files:
        print_msg("tick", "Listado de archivos en directorio exitoso.")
    else:
        print_msg("cross", "No se listaron todos los archivos en el directorio.")
  except Exception as e:
    print_msg("cross", str(e))
    
def test_create_hard_link():
  try:
    print("=== Prueba de creación de hard link ===")
    sanear_directorios()
    route1 = os.path.join(MOUNT_POINT, "ar1.txt")
    route2 = os.path.join(MOUNT_POINT, "ar2.txt")
    with open(route1, 'w') as f:
        f.write("Contenido de prueba")
    if os.path.exists(route2):
        os.remove(route2)
    os.link(route1, route2)
    if os.path.exists(route2):
        print_msg("tick", "Hard link se crea exitosamente.")
    else:
        print_msg("cross", "No se creó el hard link.")
  except Exception as e:
    print_msg("cross", str(e))

def test_chown_dir():
    try:
        print("=== Prueba de cambio de propietario de directorio ===")
        sanear_directorios()
        route = os.path.join(MOUNT_POINT, "directorio_prueba_2")
        os.mkdir(route)
        
        new_uid = 1001  
        new_gid = 1001  
        
        os.chown(route, new_uid, new_gid)
        output = os.popen(f"ls -ld {route}").read().split()
        if int(output[2]) == new_uid and int(output[3]) == new_gid:
            print_msg("tick", "Propietario del directorio se cambia exitosamente.")
        else:
            print_msg("cross", "No se cambió el propietario del directorio.")
    except Exception as e:
        print_msg("cross", str(e))

  

def test_chmod_file_in_dir():
  try:
    print("=== Prueba de cambio de permisos de archivo en directorio ===")
 
    route = os.path.join(MOUNT_POINT, "directorio_prueba_3")
    os.mkdir(route)
    assert "directorio_prueba_3" in os.listdir(MOUNT_POINT)
    file_route = os.path.join(route, "archivo_prueba_3.txt")
    f = open(file_route, 'w')
    f.close()
    assert "archivo_prueba_3.txt" in os.listdir(route)
    os.chmod(file_route, 0o777)
    if re.match(r"^-rwxrwxrwx", os.popen(f"ls -l {file_route}").read().split()[0]):
      print_msg("tick", "Permisos de archivo en directorio se cambian exitosamente.")
    else:
      print_msg("cross", "No se cambiaron los permisos de archivo en directorio.")
  except Exception as e:
    print_msg("cross", str(e)) 

def test_chown_file_in_dir():
    try:
        print("=== Prueba de cambio de propietario de archivo en directorio ===")
        
        route = os.path.join(MOUNT_POINT, "directorio_prueba_4")
        os.mkdir(route)
        assert "directorio_prueba_4" in os.listdir(MOUNT_POINT)
        file_route = os.path.join(route, "archivo_prueba_4.txt")
        f = open(file_route, 'w')
        f.close()
        
        new_uid = 1001  
        new_gid = 1001  
        
        os.chown(file_route, new_uid, new_gid)
        output = os.popen(f"ls -l {file_route}").read().split()
        if int(output[2]) == new_uid and int(output[3]) == new_gid:
            print_msg("tick", "Propietario del archivo en directorio se cambia exitosamente.")
        else:
            print_msg("cross", "No se cambió el propietario del archivo en directorio.")
    except Exception as e:
        print_msg("cross", str(e))

#

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
  
  
  print("\n\n[[[[[ CHANLLENGE ]]]]]")
  
  print("\n\n[[[[[ Prueba de hard link ]]]]]")
  test_create_hard_link()
  
  print("\n\n[[[[[ Pruebas de listado de archivos ]]]]]")
  test_list_files_in_dir()
  
  
  print("\n\n[[[[[ Pruebas de permisos y propietarios ]]]]]")
  test_chmod_file()
  test_chown_file()
  test_chown_dir()
  test_chmod_file_in_dir()
  test_chown_file_in_dir()
  
  
