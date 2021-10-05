#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PILA_SIZE 70
#define NUM_VAR 30

void generarCabecera(FILE *f);
void pushBoolean(FILE *f, int boolean);
void printBoolean(FILE *f);
void generarOperacion(FILE *f, char *tipo); //tipo="add" o "sub"
void generarOperacionI(FILE *f, char *tipo);  //tipo="imul" o "idiv"
void declararVariable(FILE *f, char nombre);
void doVariable(FILE *f, char nombre, char *tipo);  //tipo="push" o "pop"
void pushInt(FILE *f, int n);
void generarPie(FILE *f);
void printInt(FILE *f);

int main(int argc, char **argv){
  FILE *input=NULL, *output=NULL;
  char inputName[30], outputName[30];
  char string[30];  //línea leída de linea
  int linea=1, i;  //número de línea del programa
  char pilaVariables[PILA_SIZE];  //b->boolean, i->int
  int punteroPila=0;
  char tablaSimbolos[NUM_VAR][3];  //NombreVariable-TipoVariable('b'; 'i'; 0 para no asignado)-ValorVariable
  int punteroTablaSimbolos=0;
  int error=0;  //==1 si hay error

	if(argc==1){
		input=stdin;
		output=stdout;
    strcpy(inputName, "stdin");
    strcpy(outputName, "stdout");
    printf("////////////////////////////////\n");
    printf("Escriba \"EOF\" para terminar\n");
    printf("////////////////////////////////\n");
	} else if(argc>=2){
		strcpy(inputName, argv[1]);
		strcpy(outputName, argv[1]);
		outputName[strlen(outputName)-3]='a';
		outputName[strlen(outputName)-2]='s';
		outputName[strlen(outputName)-1]='m';

		input=fopen(inputName, "r");
		output=fopen(outputName, "w");
		if(input==NULL || output==NULL){
			fclose(input);
			fclose(output);
			fprintf(stderr, "<?>: Error: Error de apertura de ficheros\n");
      return 1;
		}
	}

  fprintf(output,"segment .bss\n");
  while (error!=1 && fscanf(input, "%s", string)!=EOF && string[0]!='}'){
    if(string[0]>='A' && string[0]<='Z' && strlen(string)==1){
      for(i=0;i<punteroTablaSimbolos;i++){
        if(string[0]==tablaSimbolos[i][0]){
          error=1;
        }
      }
      if(error==0){
        tablaSimbolos[punteroTablaSimbolos][0]=string[0];
        tablaSimbolos[punteroTablaSimbolos][1]=0; //aun no se conoce el tipo de la variable, se pone a 0
        punteroTablaSimbolos++;
        declararVariable(output, string[0]);
      } else {
        fprintf(stderr,"<%s>: Línea %d: Error: variable %c repetida\n", inputName, linea, string[0]);
        error=1;
      }
    } else {  //Las variables solo pueden ser una letra mayuscula
      fprintf(stderr,"<%s>: Línea %d: Error: variable incorrecta\n", inputName, linea);
      error=1;
    }
    linea++;
  }
  if(error==1){
    fclose(input);
    fclose(output);
    if(strcmp(outputName, "stdout")!=0){
      if(remove(outputName)){
        return 1;
      }
    }
    return 1;
  }

  linea++;
	generarCabecera(output);
  while(fscanf(input, "%s", string)!=EOF && strcmp(string,"EOF")!=0){
    if(strcmp(string, "true")==0 || strcmp(string, "false")==0){
      pilaVariables[punteroPila]='b';
      if(strcmp(string, "true")==0){
        pushBoolean(output, 1);
      } else {
        pushBoolean(output, 0);
      }

      punteroPila++;
      if(punteroPila>PILA_SIZE){
        fprintf(stderr,"<%s>: Línea %d: Error: Desbordamiento de pila\n", inputName, linea);
        error=1;
      }
    } else if(strcmp(string, "and")==0 || strcmp(string, "or")==0){
      if(pilaVariables[punteroPila-1]=='b' && pilaVariables[punteroPila-2]=='b'){
        if(strcmp(string, "and")==0){
          generarOperacion(output, "and");
        } else if(strcmp(string, "or")==0){
          generarOperacion(output, "or");
        }
        punteroPila--;
      } else {
        fprintf(stderr,"<%s>: Línea %d: Error: Conflicto de tipos\n", inputName, linea);
        error=1;
      }
    } else if(strcmp(string, "+")==0 || strcmp(string, "-")==0){
      if(pilaVariables[punteroPila-1]=='i' && pilaVariables[punteroPila-2]=='i'){
        if(strcmp(string, "+")==0){
          generarOperacion(output, "add");
        } else {
          generarOperacion(output, "sub");
        }
        punteroPila--;
      } else {
        fprintf(stderr,"<%s>: Línea %d: Error: Conflicto de tipos\n", inputName, linea);
        error=1;
      }
    } else if(strcmp(string,"*")==0 || strcmp(string, "/")==0){
      if(pilaVariables[punteroPila-1]=='i' && pilaVariables[punteroPila-2]=='i'){
        if(strcmp(string, "*")==0){
          generarOperacionI(output, "imul");
        } else {
          generarOperacionI(output, "idiv");
        }
        punteroPila--;
      } else {
        fprintf(stderr,"<%s>: Línea %d: Error: Conflicto de tipos\n", inputName, linea);
        error=1;
      }
    } else if(atoi(string)!=0 || strcmp(string, "0")==0){
      pilaVariables[punteroPila]='i';
      pushInt(output, atoi(string));

      punteroPila++;
      if(punteroPila>PILA_SIZE){
        fprintf(stderr,"<%s>: Línea %d: Error: Desbordamiento de pila\n", inputName, linea);
        error=1;
      }
    } else if(strcmp(string,"=")==0){
      if(punteroPila>0){
        if(pilaVariables[punteroPila-1]=='b'){
          printBoolean(output);
        } else {
          printInt(output);
        }
        punteroPila--;
      } else {
        fprintf(stderr,"<%s>: Línea %d: Error: Pila vacía\n", inputName, linea);
        error=1;
      }
    } else if(strlen(string)==1 && string[0]>='A' && string[0]<='Z'){
      error=1;
      for(i=0;i<punteroTablaSimbolos;i++){
        if(tablaSimbolos[i][0]==string[0] && tablaSimbolos[i][1]!=0){
          error=0;
          doVariable(output, string[0], "push");
          pilaVariables[punteroPila]=tablaSimbolos[i][1];
          punteroPila++;
          if(punteroPila>PILA_SIZE){
            fprintf(stderr,"<%s>: Línea %d: Error: Desbordamiento de pila\n", inputName, linea);
            error=1;
          }
        }
      }
      if(error==1){
        fprintf(stderr,"<%s>: Línea %d: Error: Variable %c no inicializada\n", inputName, linea, string[0]);
      }
    } else if(strlen(string)==2 && string[0]=='=' && string[1]>='A' && string[0]<='Z'){
      if(punteroPila>0){
        error=1;
        for(i=0;i<punteroTablaSimbolos;i++){
          if(tablaSimbolos[i][0]==string[1]){
            if(tablaSimbolos[i][1]!=0 && tablaSimbolos[i][1]!=pilaVariables[punteroPila-1]){
              fprintf(stderr,"<%s>: Línea %d: Error: Conflicto de tipos con variable %c\n", inputName, linea, string[1]);
            } else if(tablaSimbolos[i][1]==0 || tablaSimbolos[i][1]==pilaVariables[punteroPila-1]){
              error=0;
              tablaSimbolos[i][1]=pilaVariables[punteroPila-1];
              doVariable(output, string[1], "pop");
              punteroPila--;
            }
          }
        }
      } else {
        fprintf(stderr,"<%s>: Línea %d: Error: Pila vacía\n", inputName, linea);
      }
      if(error==1){
        fprintf(stderr,"<%s>: Línea %d: Error: Variable %c no inicializada\n", inputName, linea, string[1]);
      }
    } else {
      fprintf(stderr,"<%s>: Línea %d: Error: Cadena no reconocida\n", inputName, linea);
      error=1;
    }
    linea++;

    if(error){
      fclose(input);
      fclose(output);
      if(strcmp(outputName, "stdout")!=0){
        if(remove(outputName)){
          return 1;
        }
      }
      return 1;
    }
  }
  generarPie(output);
  fclose(input);
  fclose(output);

  return 0;
}

void generarCabecera(FILE *f){
  fprintf(f,"segment .text\n");
  fprintf(f,"global main\n");
  fprintf(f,"extern print_boolean, print_int, print_endofline\n");
  fprintf(f,"main:\n");
  fprintf(f,"\tmov ebp,esp\n");
}

void pushBoolean(FILE *f, int boolean){
	fprintf(f,"\tpush dword %d\n", boolean);
}

void printBoolean(FILE *f){
  fprintf(f,"\tcall print_boolean\n");
  fprintf(f,"\tcall print_endofline\n");
}

void generarOperacion(FILE *f, char *tipo){
  fprintf(f,"\tpop eax\n");
  fprintf(f,"\tpop ebx\n");
  fprintf(f,"\t%s ebx,eax\n", tipo);
  fprintf(f,"\tpush dword ebx\n");
}

void generarOperacionI(FILE *f, char *tipo){
  fprintf(f,"\tpop ecx\n");
  fprintf(f,"\tpop eax\n");
  fprintf(f,"\tmov edx,0\n");
  fprintf(f,"\t%s ecx\n", tipo);
  fprintf(f,"\tpush dword eax\n");
}

void pushInt(FILE *f, int n){
	fprintf(f,"\tpush dword %d\n", n);
}

void printInt(FILE *f){
  fprintf(f,"\tcall print_int\n");
  fprintf(f,"\tcall print_endofline\n");
}

void generarPie(FILE *f){
	fprintf(f,"\tmov esp,ebp\n");
  fprintf(f,"\tret\n");
}

void declararVariable(FILE *f, char nombre){
  fprintf(f, "\t_%c resw 1\n", nombre);
}

void doVariable(FILE *f, char nombre, char *tipo){
  fprintf(f, "\t%s dword [_%c]\n", tipo, nombre);
}
