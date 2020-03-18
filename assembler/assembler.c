#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"data_header.h"
#include"memory.h"
#include"opcode_header.h"

struct opcodes{
	char name[50];
	char opc[2];
	int isdata;
	int islabel;
	int isaddress;
	int size;
}optab[246];
int a[1],b[1],c[1],d[1],e[1],h[1],l[1];//registers

int s[1],z[1],ac[1],p[1],cy[1];//flags

struct memory mem[10000];//memory array

struct symbol{
	char label[50];
	int add;
}symtab[100];
int lsym=0;

int start_add=0x2000;//startig address is 2000H

//output file here
char out[200][3];
int opos=0;

void checking_optab()
{   int i=0;
	for(i=0;i<246;i++)
	{
		printf("%s %c%c %d %d %d %d  %d\n",optab[i].name,optab[i].opc[0],optab[i].opc[1],optab[i].isdata,optab[i].islabel,optab[i].isaddress,strlen(optab[i].name),optab[i].size);
    //    printf("%s  %d %d %d\n",optab[i].name,optab[i].isdata,optab[i].islabel,optab[i].isaddress);
    //printf("%s %s %d %d %d %d  %d\n",optab[i].name,optab[i].opc,optab[i].isdata,optab[i].islabel,optab[i].isaddress,strlen(optab[i].name),optab[i].size);
	
	}
}
int findsubstr(char str1[],char st[])
{
	int len1=strlen(str1),len2=strlen(st);
	int i=0;
	for(i=0;i<len1-len2+1;i++)
	{    int j=i,f=0;
		for(;j<i+len2;j++)
		{
			if(str1[j]!=st[j-i]){
			f=1;break;}
		}
		if(f==0)return i;
	}
	return -1;
}
void generate_optab(){
	
	//opcode geneartion
	   FILE* fh;
	fopen_s(&fh,"opcode.txt", "r");
	//check if file exists
	if (fh == NULL){
	    printf("file does not exists");
	    return ;
	}
	
	int count=0;
	//read line by line
    char line[300];
	while (fgets(line, 300, fh) != NULL)  {
	    //printf("%s  %d\n",line,strlen(line));
	    int pos,len=strlen(line);
	    for(pos=0;pos<len;pos++)
	    {
	    	if(line[pos]=='.')
	    	break;
		}
		char last=line[len-2];
		optab[count].size=((int)last) -48;
		pos+=2;
		int x=0;
		for(;pos<len-6;pos++)
		{
			optab[count].name[x]=line[pos];
			x++;
		}
		char op[2];
		pos++;
		optab[count].opc[0]=line[pos];optab[count].opc[1]=line[pos+1];optab[count].opc[2]='\0';
		optab[count].isdata=findsubstr(optab[count].name,"Data");
		if(optab[count].isdata!=-1)
		optab[count].name[optab[count].isdata]='\0';
		optab[count].islabel=findsubstr(optab[count].name,"Label");
		if(optab[count].islabel!=-1)
		optab[count].name[optab[count].islabel]='\0';
		optab[count].isaddress=findsubstr(optab[count].name,"Address");
		if(optab[count].isaddress!=-1)
		optab[count].name[optab[count].isaddress]='\0';
		count++;
	}
    fclose(fh);
//   checking_optab();
}

int validlabel(char label[])
{
	int len=strlen(label);
	int i;
	for(i=0;i<246;i++)
	{
		if(findsubstr(optab[i].name,label)!=-1)
		return -1;
	}
	return 1;
}
int process_instruction_pass1(char line[],int start)
{
	int len=strlen(line);
	//search for label
   
    char *lb ;
    lb = (char *)malloc(50 * sizeof(char));
	int i=0,z=0;
	for(i=0;i<strlen(line);i++)
	{
		if(line[i]==' ')
		{
		z=1;break;
	    }
		else
		lb[i]=line[i];
	}
	lb[i]='\0';
    //	printf(" here is %s\n",lb);
	if(validlabel(lb)==1 && z==1)
	{//insert into symtab
	 strcpy(symtab[lsym].label,lb);
	 symtab[lsym].add=start;
	 lsym++;
     //	printf("inserted %s %d %X\n",lb,strlen(lb),start);
	}
	
	//now search for opcodes
	
	for(i=0;i<246;i++)
	{
		if(findsubstr(line,optab[i].name)!=-1)
		{
			start+=optab[i].size;
			break;
		//	printf("check address %X  %d   %s\n",start,optab[i].size,optab[i].name);
		}
	}
	
return start;//need to change here	
free(lb);
}
void pass1()
{
	    //scan input file from user

    FILE* fh;
	fopen_s(&fh,"input_program.txt", "r");
	if (fh == NULL){
	    printf("file does not exists");
	    return ;
	}
    char line[300];
    int start=start_add;
	while (fgets(line,300, fh) != NULL)  
	{
         start=process_instruction_pass1(line,start);    	
	}
    fclose(fh);
    int i;
    for(i=0;i<lsym;i++)
    {
   	printf("%X %s %d\n",symtab[i].add,symtab[i].label,strlen(symtab[i].label));
	}

}
int findsymbol(char st[])
{
	int len=strlen(st);
	int i=0;
	//printf("len b4 is %d\n",strlen(st));
//	st[strlen(st)-1]='\0';
	//printf("to search %s %d %c\n",st,strlen(st),st[strlen(st)-1]);
	for(;i<lsym;i++)
	{
		if(strcmp(st,symtab[i].label)==0)
		{   
	//	    printf("%X %s\n",symtab[i].add,symtab[i].label);
			return symtab[i].add;
		}
	}
	return -1;
}
void pass2()
{
	 //machine code
     FILE *fp;
     fp = fopen("machine_code.txt", "w+");
     //fprintf(fp,"%s",);
     
    FILE* fh;
	fopen_s(&fh,"input_program.txt", "r");
	if (fh == NULL){
	    printf("file does not exists");
	    return ;
	}
    char line[300];
    int start=start_add;
	while (fgets(line,300, fh) != NULL)  
	{
		//search for opcode
		int i=0,pos,f=0;
		for(;i<246;i++){
			pos=findsubstr(line,optab[i].name);
			if(pos!=-1)
			{f=1;
				pos+=strlen(optab[i].name);//start of data/address/label
				fprintf(fp,"%c%c\n",optab[i].opc[0],optab[i].opc[1]);
			break;}
			 
		}
		if(f==0)printf("--------Opcode not found-------%s\n",line);
		else{
			if(optab[i].isdata!=-1)
			{
				fprintf(fp,"%c%c\n",line[pos],line[pos+1]);
			}
		    if(optab[i].isaddress!=-1)
			{
				fprintf(fp,"%c%c\n",line[pos+2],line[pos+3]);
				fprintf(fp,"%c%c\n",line[pos],line[pos+1]);
			}
			if(optab[i].islabel!=-1)
			{
				char label[50];
				int l=0;
				line[strlen(line)-1]='\0';
				for(;pos<strlen(line) && line[pos]!=' ';pos++)
				{   
					label[l]=line[pos];
			//		printf("%d %c %c\n",l,label[l],line[pos]);
					l++;
				}
				label[l]='\0';
			//	printf("len %d\n",strlen(label));
				int sym=findsymbol(label);
				fprintf(fp,"%X\n",sym%256);
                fprintf(fp,"%X\n",sym/256);
			 // fprintf(fp,"%X\n",sym);				
			}
		}
	}
    fclose(fh);
    fclose(fp);

}
int findopcode(char st[])
{
	int i=0;
	for(;i<246;i++)
	{
		if(optab[i].opc[0]==st[0] && optab[i].opc[1]==st[1] )
		return i;
	}
	return -1;
}
void filloutput()
{    FILE* fh;
	fopen_s(&fh,"machine_code.txt", "r");
	if (fh == NULL){
	    printf("file does not exists");
	    return ;
	}
    char line[300];
    //int start=start_add;
	while (fgets(line,300, fh) != NULL)  
	{ line[strlen(line)-1]='\0';
    // printf("%s %d\n",line,strlen(line));  
	 out[opos][0]=line[0];
	 if(strlen(line)==2)
	 {
	 out[opos][1]=line[1];
	 out[opos][2]='\0';
	 }
	 else
	 out[opos][1]='\0';
	 
	// printf("%s %d\n",out[opos],strlen(out[opos]));
	 opos++;
	}
    fclose(fh);
}
int convert(char a,char b)
{
	FILE* fp;
	fp=fopen("conversion.txt","w");
	fprintf(fp,"%c%c",a,b);
	fclose(fp);
	int x;
	FILE* fh;
	fh=fopen("conversion.txt","r");
	fscanf(fh,"%X",&x);
	fclose(fh);
	return x;
}
int findmem(int x)
{
	int i=0;
	for(;i<10000;i++)
	{if(mem[i].add==x){
	return i;
}
	}
	return -1;
}
void process_data(int pos,int data)
{
	if(pos==167)
	{
		int addr=h[0];
		addr=(addr*256)+l[0];
		int y=findmem(addr);
		mem[y].val=data;
	}
	else
	insert_data(data,pos,a,b,c,d,e,h,l,s,z,ac,p,cy);
}
void generate_address()
{
	int i=0;
	int m=0x2000,r=0x1;
	for(;i<10000;i++)
	{
		mem[i].add=m;
		mem[i].val=m%256;m=m+r;
	}
	a[0]=0x0;b[0]=0x0;c[0]=0x0;d[0]=0x0;e[0]=0x0;h[0]=0x0;l[0]=0x0;
}
void process_add(int pos,int data)
{   int place=findmem(data);
    //printf("HERE2 %d",place);
	insert_address(data,pos,a,b,c,d,e,h,l,mem,place);
}
void process_t(int pos)
{
	if(pos>=97 && pos<=159)
	{   char t=optab[pos].name[7];
	    char f=optab[pos].name[4];
	    int addr=h[0];
		addr=(addr*256)+l[0];
		int y=findmem(addr);
		mov_instruction(f,t,a,b,c,d,e,h,l,mem,y);
	}
	if(pos>=1 && pos<=39)
	{   char j=optab[pos].name[strlen(optab[pos].name)-1];//check dis 1
		int addr=h[0];
		addr=(addr*256)+l[0];
		int y=findmem(addr);
		arithmetic(j,pos,a,b,c,d,e,h,l,mem,s,z,ac,p,cy,y);
	}
}
void memoryfile()
{
	FILE *fp;
     fp = fopen("memory.txt", "w+");
     int i=0;
     for(;i<10000;i++)
     {  if(mem[i].add==0x4000)break;
     	fprintf(fp,"%X %X\n",mem[i].add,mem[i].val);
	 }
     printf("\nA %X\n",a[0]);
     printf("B %X\n",b[0]);
     printf("C %X\n",c[0]);
     printf("D%X\n",d[0]);
     printf("E %X\n",e[0]);
     printf("H %X\n",h[0]);
     printf("l %X\n",l[0]);
     printf("carry %d\n",cy[0]);
	 fclose(fp);
}
int process_jump(int i,int jmp_i,int jmp,int cur)
{
	int d;
	if(jmp>cur)d=jmp-cur;
	else
	d=cur-jmp;
	int shift=convert2(d);
	int prev=i;
	if(jmp>cur)i+=shift;
	else
	i-=shift;
	
	if(jmp_i==80)//jc
	{   //printf("JUMP %d %d  %d  %d %d\n",cy[0],shift,d,prev,i);
		if(cy[0]==0)return -1;
		
	}
	if(jmp_i==82)//jmp
	{
	}
	if(jmp_i==83)//jnc
	{
		if(cy[0]==1)return -1;
	}
	if(jmp_i==84)//jnz
	{
		if(z[0]==1)return -1;
	}
	if(jmp_i==88)//jz
	{
		if(z[0]==0)return -1;
	}
	return i;
	
}
int main() 
{
   //generate address
   generate_address();
   //optab geneartion 
    generate_optab();
   //pass1 
   pass1();
   //pass2
   pass2();
   
   //fill output matrix
   filloutput();
   
   
   int start=start_add;
   int i=0;
   for(;i<opos;)
   {
   
    int ops=findopcode(out[i]);
	if(ops==-1){printf("Error opcode not found\n");
	 }
    	printf("%X %s %d\n",start,optab[ops].name,ops); 
	 if(optab[ops].isdata!=-1)
	 {
	 	int data=convert(out[i+1][0],out[i+1][1]);
	 //	printf("Data is found %X %s %X\n",start,out[i],data);
	    process_data(ops,data);
	 }
	 else if(optab[ops].isaddress!=-1)
	 {
	 	int a1=convert(out[i+2][0],out[i+2][1]),a2=convert(out[i+1][0],out[i+1][1]);
	 	int data=(a1*256) + a2;
	 //	printf("Here %X %d\n",data,data);
	 	process_add(ops,data);
	 }
	 else if(optab[ops].islabel!=-1)
	 {
	 	//here jump to be put
	 	int a1=convert(out[i+2][0],out[i+2][1]),a2=convert(out[i+1][0],out[i+1][1]);
	 	int data=(a1*256) + a2;
	 	
	 	int jmp=process_jump(i,ops,data,start);
	 	if(jmp!=-1)
		{
		i=jmp;continue;
		}
	 }
	 else{
	 	process_t(ops);
	 }
	 
	 
	 i=i+optab[ops].size;
	 start+=optab[ops].size;   
   }  
   
   
   
   //generate memory file; 
   memoryfile();
	return 0;
}
