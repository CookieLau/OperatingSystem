#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#define null NULL
typedef struct tnode{
    char c;
    struct tnode* next;
    struct tnode* last;
}tnode,*pnode;
pnode head;
pnode space(){
    pnode p=malloc(sizeof(tnode));
    p->next=null;
    p->last=null;
    return p;
}
int tmp[200];
void add(char a[]){
    int i=32;
    int j;
    j=strlen(a);
    for(;i<=126;i++){
        if(tmp[i]==0){
            a[j++]=i;
            tmp[i]=1;
        }else{
            continue;
        }
    }
}
void create(char a[]){
    int j;
    j=strlen(a);
    int i;
    pnode t;
    for(i=0;i<j;i++){
        pnode p=space();
        p->c=a[i];
        if(head->next==null){
            head->next=p;
            p->next=p;
            p->last=p;
            t=p;
        }else{
            p->next=t->next;
            t->next->last = p;
            p->last=t;
            t->next=p;
            t = p;
        }
    }
}
char ans[200];
void delete(){
    pnode p=head->next;
    pnode t;
    int n,m,k;
    k=head->next->c;
    while(p->next!=p){
        n=p->c;
        m=n;
        t=p->next;
        printf("p->next's c is %d\n",t->c);
        if(p->next==null)
            printf("null");
        p->next->last=p->last;
        p->last->next=p->next;
        free(p);
        
        while(--n){
        	if (t->next == null ) {
				printf("Fucking null!\n");
			} 
            t=t->next;
        }
        
        ans[m]=t->c;
        p=t;
    }
    char tempc;
    tempc = ans[k];
    ans[k]=p->c;
    p->c = tempc;
}
int main(){
    char key[200],c;
    gets(key);
    
    int i=0,j=0;
    memset(tmp,0,sizeof(tmp));
    head = space();
    head->next = null;
    while(key[i]!='\0'){
        if(tmp[key[i]]==0 && (int)key[i]<=126 &&(int)key[i]>=32){
            tmp[key[i]]=1;
            key[j++]=key[i];
            i++;
        }else{
            i++;
        }
    }
    key[j]='\0';
    
    add(key);
    printf("%s\n",key);
    
  
    memset(ans,'\0',sizeof(ans));
    //head=space();
    
    create(key);
   printf("11111");
     pnode start = head->next;
    /*while (start->next != head->next) {
    	printf("%c\n",start->c);
		start = start->next; 
	}*/
    

    delete();
    printf("11111");

    
    FILE* in,*out;
    in=fopen("in.txt","r");
    out=fopen("in_crpys.txt","w");
    
    while(EOF!=(c=fgetc(in))){
        if((int)c>=32&&(int)c<=126){
            c=ans[c];
        }
        fprintf(out,"%c",c);
        printf("%c\n",c);
    }
    for(i=30;i<=130;i++){
        if(ans[i]!='\0'){
            printf("%c",ans[i]);
        }
    }
    
    fclose(in);
    fclose(out);
}

