
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>


typedef struct macTableEntry{
    int vlanid;
    char mac_addr[13
    ];
    int port;
    int type; // 1 -> static    2 -> dynamic
    double age;
    time_t timeStamp;
}MAC_TABLE_ENTRY;

typedef struct macTableNode{
    MAC_TABLE_ENTRY mac_entry;
    struct macTableNode *next; 
}MAC_TABLE_NODE;

typedef struct macTable{
    int count;
    MAC_TABLE_NODE* head;
    MAC_TABLE_NODE* tail;
}MAC_TABLE;


int hextodec_quad(char hex){
    char hexvalues[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    int i;
    for(i=0;i<16;i++){
        if(hex == hexvalues[i]){
            return i;
        }
    }
    return -1;
}

int hextodec_byte (char *hex){
    int dec = 0;
    dec = 16*hextodec_quad(hex[0]) + hextodec_quad(hex[1]);
    return dec;
}


char* readChars(FILE *f,int n){
    int i;
    char *fields;

    fields = malloc(sizeof(char)*(n+1));
    for(i=0;i<n;i++){
        fields[i] = fgetc(f);
    }
    fields[n] = '\0';
    return fields;
}

int hextodec(char *hex, int nbits){
    int i;
    int dec = 0;
    for(i=0;i<nbits;i++){
        dec += hextodec_quad(hex[i]) * pow(16,nbits-i-1);
    }
    return dec;
}

void readFramePayload(FILE* fptr){
    int chr = fgetc(fptr);
    printf("Payload : \n");
    while(chr != EOF){
        printf("%c",chr);
        chr = fgetc(fptr);
    }
    printf("\n");
}

void printMAC(char* mac_addr){
    for(int i=0; i<12; i++){
        if(i%2==0 && i!=0)
            printf(" : ");
        printf("%c",mac_addr[i]);
    }
    //printf("\n");
}

int isUnicast(char *dest_mac_addr){
    int byt = hextodec_quad(dest_mac_addr[1]);
    if(byt%2==0){
        return 1;
    }else if(!strcmp(dest_mac_addr, "ffffffffffff")){
        return 2;
    }
    return 0;
}

int isTagged(char *TPid){
    if(!strcmp(TPid, "8100")){
        return 1;
    }else{
        return 0;
    }
}

void VlanUnpack(FILE* fptr, int *priority,int *vlanid){
    char *vlantag;
    int priot;
    vlantag = readChars(fptr, 4);
    priot = (hextodec_quad(vlantag[0]) & 14)>>1;
    //printf("VlanTag[0]: %c\n",vlantag[0]);
    //printf("priot: %d\n",priot>>1);
    *priority = priot;
    *vlanid = hextodec_quad(vlantag[1])*256 + hextodec_quad(vlantag[2])*16 + hextodec_quad(vlantag[3])*1;

    printf("Priority: %d\n",*priority);
    printf("Vlan ID: %d\n", *vlanid);
}

typedef struct frame{
    char *dest_mac_addr;
    char *src_mac_addr;
    int tagged;
    char vlantag[4];
    char *TPiD;
    char *type;
    int casting,priority, vlanid;
}FRAME;

void PrintIP(int *addr){
    for(int i=0;i<4;i++){
        printf("%d",addr[i]);
        if(i!=3){
            printf(" . ");
        }
    }
    printf("\n");
}

void IP_addr(FILE* fptr, int* ip_addr){
    char *addr = readChars(fptr, 8);
    int i,j=0;
    for(i=0;i<8;i+=2){
        ip_addr[j++] = hextodec_quad(addr[i]) * 16 + hextodec_quad(addr[i+1]);
    }
}

void Layer2(FILE* fptr, FRAME *f){
    
    printf("\nLayer 2 information: \n\n");
    f->dest_mac_addr = readChars(fptr, 12);
    f->src_mac_addr = readChars(fptr, 12);

    printf("The Destination MAC: ");
    printMAC(f->dest_mac_addr);
    printf("\n");
    printf("The Source MAC: ");
    printMAC(f->src_mac_addr);
    printf("\n");

    f->casting = isUnicast(f->dest_mac_addr);
    if(f->casting){
        if(f->casting == 1){
            printf("Destination : Unicast.\n");
        }else{
            printf("Destination : Broadcast.\n");
        }
    }else{
        printf("Destination : Multicast.\n");
    }

    f->TPiD = readChars(fptr, 4);

    f->tagged = isTagged(f->TPiD);
    if(f->tagged){
        printf("VLAN tagged = True.\n");
        VlanUnpack(fptr, &f->priority, &f->vlanid);
        f->type = readChars(fptr,4);
        if(f->vlanid == 0){
            printf("Priority tagged frame.\tVlanId = %d\n",f->vlanid);
        }
        if(!strcmp(f->type,"0800")){
            printf("Type = %s\tIPv4\n",f->type);
        }else if (!strcmp(f->type,"0806")){
            printf("Type = %s\tARP\n",f->type);
            //ARP(fptr);
        }
    }else{
        f->type = f->TPiD;
        printf("VLAN tagged = False.\n");
        if(!strcmp(f->type,"0800")){
            printf("Type = %s\tIPv4\n",f->type);
        }else if (!strcmp(f->type,"0806")){
            printf("Type = %s\tARP\n",f->type);
            //ARP(fptr);
        }
    }
    //readFramePayload(fptr);
}

void ReplaceEntry(MAC_TABLE *table, int i, MAC_TABLE_NODE* node){

}

void purgeEntry(MAC_TABLE *table) {
    if (table->head == NULL) return; 

    MAC_TABLE_NODE *curr = table->head;
    MAC_TABLE_NODE *prev = NULL;

    while (curr != NULL) {
        double age = difftime(time(NULL), curr->mac_entry.timeStamp);
        
        if (age > 300) {
            MAC_TABLE_NODE *toDelete = curr;
            if (prev == NULL) { 
                table->head = curr->next;
                curr = table->head;
            } else {
                prev->next = curr->next;
                curr = curr->next;
            }
            
            if (toDelete == table->tail) {
                table->tail = prev;
            }
            
            free(toDelete);
            table->count--;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}

void usp_mac(char *src_mac, char *usp){
    for(int i=0;i<6;i++){
        usp[i] = src_mac[i+6];
    }
}

int hashFunc(char *src_mac){
    char usp[6];
    usp_mac(src_mac,usp);
    printf("USP: %s\n",usp);
    long int hvalue = hextodec(usp,6);

    long int key = ((hvalue * 29 * 11) / 19 ) % 20;
    return key; 
}

void InitializeHash(MAC_TABLE_NODE **htable, int count){
    for(int i=0;i<count;i++){
        htable[i] = NULL;
    }
}

void AddToTable(MAC_TABLE_NODE *node, MAC_TABLE_NODE **htable, int key){
    MAC_TABLE_NODE *temp = htable[key];
    for(int i=0; temp != NULL; i++){
        if(!strcmp(node->mac_entry.mac_addr, temp->mac_entry.mac_addr)){
            
            temp->mac_entry.timeStamp = time(NULL);
        }
        temp = temp->next;
    }
    temp = node;
    temp->next = NULL;
}

void AddMACEntry(char *filename, MAC_TABLE_NODE **htable){
    FILE* fptr = fopen(filename,"r");
    int key;
    MAC_TABLE_NODE *node = malloc(sizeof(MAC_TABLE_NODE));
    FRAME f;
    Layer2(fptr, &f);
    strcpy(node->mac_entry.mac_addr, f.src_mac_addr);
    node->mac_entry.port = (rand() % 24);
    node->mac_entry.timeStamp = time(NULL);
    node->mac_entry.vlanid = f.tagged ? f.vlanid : 1;
    node->next = NULL;

    key = hashFunc(f.src_mac_addr);
    printf("The key value: %d\n",key);

    if(htable[key] == NULL){
        htable[key] = node;
    }else{
        AddToTable(node, htable, key);
    }

    fclose(fptr);
    printf("\nEntry Added\n\n");
}

void printMAC_table(MAC_TABLE *table){
    MAC_TABLE_NODE *temp = table->head;

    printf("\t\tMAC table\n");
    printf("S.no.\tVLAN ID\tMAC Address\tPort\tAge\n");
    for(int i=0; temp != NULL;i++){
        temp->mac_entry.age = difftime(time(NULL), temp->mac_entry.timeStamp);
        purgeEntry(table);
        printf("%d\t%d\t",i+1, temp->mac_entry.vlanid);
        printMAC(temp->mac_entry.mac_addr);
        printf("\t%d\t%.2f\n",temp->mac_entry.port, temp->mac_entry.age);
        temp = temp->next;
    }
}

/*
int main(){
    MAC_TABLE table;
    table.count = 0;
    table.head = NULL;
    table.tail = NULL;
    int ch;
    char filename[20];
    
    while(1){
        printf("1) Read ethernet Frame\n2) Print MAC table\n3) Exit\nEnter your choice: ");
        scanf("%d", &ch);

        if(ch == 1){
            printf("Enter the filename: ");
            scanf("%s", filename);
             
            AddMACEntry(filename, &table);
        }else if(ch == 2){
            printMAC_table(&table);
        }else if(ch==3){
            break;
        }else{
            printf("Invalid Choice");
        }
    }   
}*/

int main(){
    MAC_TABLE_NODE *hashtable[20];
    MAC_TABLE table;
    table.count = 0;
    table.head = NULL;
    table.tail = NULL;
    
    int ch;
    char filename[20];
    
    while(1){
        printf("1) Read ethernet Frame\n2) Print MAC table\n3) Exit\nEnter your choice: ");
        scanf("%d", &ch);

        if(ch == 1){
            printf("Enter the filename: ");
            scanf("%s", filename);
             
            AddMACEntry(filename, &table);

        }
    }
}