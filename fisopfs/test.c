#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>


void create_simple(char* path, int* result){
    result =  creat(path, S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO);
    assert(result > 2);
}

void create_already_exists(char* path, int* result){
    result =  creat(path, S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO);
    assert(result == 0);
}


void creation_tests() {
    // si el archivo ya existe devuelve 0
    // si el archivo devuleve -1 es por un error
    // si devuelve 1 o 2 puede que se haya cerrado el stdouto stderr
    printf("======== PRUEBAS DE CREACIÓN DE ARCHIVIOS ===========");

    char *path = "test.txt";

    int result; 
    
    create_simple(path, &result);
    create_already_exists(path, &result);

    close(path);
}


int main() {
    
    creation_test();

    return 0;
}