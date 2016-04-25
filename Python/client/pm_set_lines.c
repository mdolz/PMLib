#include "pmlib.h"
#define BUFFSIZE 1024

int pm_set_lines( char *lines_string ,line_t *lines ){


/*! This function creates a bit map with the lines selected in lines_string.
*/
   
   char *p, *n, sep=',';
   int fail=1, l, l1, i;
 //  char lines_string[30]="30-36,42,100,111-115\0";
   char num[]="1234567890\0";
   char cad[4];
   int lineas[128], indice=0;
  
   for(p=lines_string; *p!='\0'; p++){
	for(n=num; *n!='\0'; n++){
		if(*p=='-' || *p==',' || *p==*n){
			fail=0;
			break;			
		}
		else{
			fail=1;	
		}
	}
	if(fail==1){
		fprintf(stderr,"Incorrect format of input string\n");
		return (-1);		
	}
   }
   p=lines_string;
   while(*p!='\0'){
	if(*p!='-' && *p!=',') {//is an integer
		if(*(p+1)!='-' && *(p+1)!=',' && *(p+1)!='\0'){
			if(*(p+2)!='-' && *(p+2)!=',' && *(p+2)!='\0'){  //is an integer with three numbers 
				sprintf(cad, "%c%c%c\n", *p, *(p+1), *(p+2));
				l=atoi(cad);
				p+=3;
			}
			else{  //is an integer with two numbers
				sprintf(cad, "%c%c\n", *p, *(p+1));
				l=atoi(cad);
				p+=2;
			}
		}
		else{   //is an integer with only one number 
			sprintf(cad, "%c\n", *p);
			l=atoi(cad);
			p+=1;
		}
		if(*p=='\0'){
			lineas[indice]=l;
			indice++;
		}		
  	}
	else{ 
		if(*p=='-'){
			sep='-';
			if(*(p+1)!='-' && *(p+1)!=',' && *(p+1)!='\0'){
				if(*(p+2)!='-' && *(p+2)!=',' && *(p+2)!='\0'){
					if(*(p+3)!='-' && *(p+3)!=',' && *(p+3)!='\0'){   //is an integer with three numbers 
						sprintf(cad, "%c%c%c\n", *(p+1), *(p+2), *(p+3));
						l1=atoi(cad);
						p+=4;
						for(i=l; i<=l1; i++){
							lineas[indice]=i;
							indice++;
						}
					}
					else{  //is an integer with two numbers
						sprintf(cad, "%c%c\n", *(p+1), *(p+2));
						l1=atoi(cad);
						p+=3;
						for(i=l; i<=l1; i++){
							lineas[indice]=i;
							indice++;
						}
					}
				}
				else{   //is an integer with only one number 
					sprintf(cad, "%c\n", *(p+1));
					l1=atoi(cad);
					p+=2;
					for(i=l; i<=l1; i++){
						lineas[indice]=i;
						indice++;
					}
				}
			}
			else{
				fprintf(stderr,"Incorrect format of input string 2\n");
				return (-1);	
			}
		}
		else {
			if(sep=='-'){
				p+=1;
				sep=',';
			}
			else{
				if(sep==','){
					lineas[indice]=l;
					indice++;
					p+=1;
				}
			}
		}
		
	} 
  }
  LINE_CLR_ALL(lines);
  for(i=0;i<indice;i++){
	LINE_SET(lineas[i], lines);
  }  

  return (0);	
}
