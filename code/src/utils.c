#include <stdio.h>

char getCode(int option){
    return option < 4 ? 0x23 : 0x16;
}

char getSubCode(int option){
    if(option == 1){
        return 0xC1;
    }else if(option == 2){
        return 0xC2;
    }else if(option == 3){
        return 0xC3;
    }else if(option == 4){
        return 0xD1;
    }else if(option == 5){
        return 0xD2;
    }else if (option == 6){
        return 0xD3;
    }
    else if (option == 7){
        return 0xD5;
    }
    else{
        return 0xD6;
    }
}

char* getCodeMeaning(char subcode){
    if(subcode == 0xC1){
        return "Temperatura Interna";
    } else if(subcode == 0xC2){
        return "Temperatura de Referência";
    } else if(subcode == 0xC3){
        return "Comando de usuário recebido";
    } else return "Valor";
}

int contains(char item, char *list, int size){
    for(int i=0; i<size; i++){
        if(item == list[i])
            return 1;
    }
    return 0;
}
