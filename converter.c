#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml2/libxml/xmlschemastypes.h>


struct Person
{
    char name[15];
    char surname[15];
    char gender[2];
    char occupacy[15];
    char education[4];
    char email[30];
    char bankAccountNumber[13];
    char iban[28];
    char currency[4];
    char accountType[14];
    int balance;
    char loanAvaiable[5];
};
int readCSVWriteBIN(char *input_file, char *output_file);
int readBINWriteXML(char *input_file, char *output_file);
void validate(char *input_file, char *output_file);
char* getWord(char* line);
char* getBinWord(FILE *bin);
int getBigEnd(int num);
static struct Person *persons = NULL;
xmlDocPtr doc = NULL;


int main(int argc, char const *argv[])
{
    char input_file[20], output_file[20];
    int n;

    if(argc < 4) {
        printf("Usage: %s <input_file> <output_file> <n>\n", argv[0]);
        return 1;
    }

    strcpy(input_file, argv[1]);
    strcpy(output_file, argv[2]);
    n = atoi(argv[3]);

    switch (n)
    {
    case 1:
        if (readCSVWriteBIN(input_file, output_file) != 0)
        {
            return 1; //CSV read or Bin write error
        }
        break;
    case 2:
        if (readBINWriteXML(input_file, output_file) != 0)
        {
            return 2; //Bin read or XMl write error
        }
        break;
    case 3:
        validate(input_file,output_file);
        break;    
    default:
        printf("Unknown error");
        return -1;
        break;
    }
    
    return 0;
}
void validate(char *input_file, char *output_file){
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
	
    char *XMLFileName = input_file; // write your xml file here
    char *XSDFileName = output_file; // write your xsd file here
    
    
    xmlLineNumbersDefault(1); //set line numbers, 0> no substitution, 1>substitution
    ctxt = xmlSchemaNewParserCtxt(XSDFileName); //create an xml schemas parse context
    schema = xmlSchemaParse(ctxt); //parse a schema definition resource and build an internal XML schema
    xmlSchemaFreeParserCtxt(ctxt); //free the resources associated to the schema parser context
    
    doc = xmlReadFile(XMLFileName, NULL, 0); //parse an XML file
    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt;  //structure xmlSchemaValidCtxt, not public by API
        int ret;
        
        ctxt = xmlSchemaNewValidCtxt(schema); //create an xml schemas validation context 
        ret = xmlSchemaValidateDoc(ctxt, doc); //validate a document tree in memory
        if (ret == 0) //validated
        {
            printf("%s validates\n", XMLFileName);
        }
        else if (ret > 0) //positive error code number
        {
            printf("%s fails to validate\n", XMLFileName);
        }
        else //internal or API error
        {
            printf("%s validation generated an internal error\n", XMLFileName);
        }
        xmlSchemaFreeValidCtxt(ctxt); //free the resources associated to the schema validation context
        xmlFreeDoc(doc);
    }
    // free the resource
    if(schema != NULL)
        xmlSchemaFree(schema); //deallocate a schema structure

    xmlSchemaCleanupTypes(); //cleanup the default xml schemas types library
    xmlCleanupParser(); //cleans memory allocated by the library itself 
    xmlMemoryDump(); //memory dump
}

int readBINWriteXML(char *input_file, char *output_file){
    FILE *bin;
    int numLines = 0;
    char *c = (char*) malloc(sizeof(char));
    
    bin = fopen(input_file,"rb");
    if (bin == NULL) {
        printf("Unable to open file\n");
        return 1;
    }

    //count lines
    while(fread(c, 1, 1, bin) == 1){
        if (*c == '\n') {
            numLines++;
        }
    }

    free(c);
    //return to start
    fseek(bin,0,SEEK_SET);
    
    //to be sure
    persons = NULL;
    //allocate memory
    persons =  malloc(numLines * sizeof(struct Person));
    if(persons == NULL){
        printf("Could not allocate memory\n");
        return 2;
    }

    //read binary file and put the words in struct array
    char *temp;
    int tempBalance;
    for (int i = 0; i < numLines; i++)
    {
        temp = getBinWord(bin);
        strcpy(persons[i].name,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].surname,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].gender,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].occupacy,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].education,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].email,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].bankAccountNumber,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].iban,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].accountType,temp);
        temp = getBinWord(bin);
        strcpy(persons[i].currency,temp);
        memcpy(&tempBalance, getBinWord(bin), 4);
        persons[i].balance = tempBalance;
        temp = getBinWord(bin);
        strcpy(persons[i].loanAvaiable,temp);
    }
    
    fclose(bin);
    
    //create xml document file
    doc = xmlNewDoc(BAD_CAST "1.0");

    //set root 
    xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "records");
    xmlDocSetRootElement(doc,root);
    //creates other parents
    xmlNodePtr row, customerInfo, bankInfo, balancePtr;
    //tempNum is for all integer values
    int id = 1, tempNum;
    //to convert int to str
    char tempStr[4];
    for (int i = 0; i < numLines; i++)
    {
        //creates row
        row = xmlNewChild(root, NULL, BAD_CAST "row", NULL);
        //converts int to str
        sprintf(tempStr,"%d", id);
        xmlNewProp(row, "id", BAD_CAST tempStr);
        id++;
        //creates customer info element
        customerInfo = xmlNewChild(row, NULL, BAD_CAST "customer_info", NULL);
        xmlNewChild(customerInfo, NULL, BAD_CAST "name", persons[i].name);
        xmlNewChild(customerInfo, NULL, BAD_CAST "surname", persons[i].surname);
        xmlNewChild(customerInfo, NULL, BAD_CAST "gender", persons[i].gender);
        xmlNewChild(customerInfo, NULL, BAD_CAST "occupacy", persons[i].occupacy);
        xmlNewChild(customerInfo, NULL, BAD_CAST "level_of_education", persons[i].education);
        xmlNewChild(customerInfo, NULL, BAD_CAST "email", persons[i].email);
        
        //creates bank info element
        bankInfo = xmlNewChild(row, NULL, BAD_CAST "bank_info", NULL);
        xmlNewChild(bankInfo, NULL, BAD_CAST "bank_account_number", persons[i].bankAccountNumber);
        xmlNewChild(bankInfo, NULL, BAD_CAST "iban", persons[i].iban);
        xmlNewChild(bankInfo, NULL, BAD_CAST "account_type", persons[i].accountType);
        
        //converts balance int to str
        sprintf(tempStr,"%d", persons[i].balance);
        balancePtr = xmlNewChild(bankInfo, NULL, BAD_CAST "total_balance_available", tempStr);
        xmlNewProp(balancePtr, "currency_unit", BAD_CAST persons[i].currency);
        //convert little endian to big endian
        tempNum = getBigEnd(persons[i].balance);
        //converts big endian to str
        sprintf(tempStr,"%d", tempNum);
        xmlNewProp(balancePtr,"bigEnd_version" ,BAD_CAST tempStr);

        xmlNewChild(bankInfo, NULL, BAD_CAST "available_for_loan", persons[i].loanAvaiable);
    }
    
    //saves xml file
    xmlSaveFormatFileEnc(output_file, doc, "UTF-8", 1);
    
    return 0;

}

int getBigEnd(int num){
    //this code reverse the byte sequance and creates big endian
    char numBytes[4], reversedBytes[4];
    int bigEnd;
    memcpy(&numBytes, &num, 4);
    for (int i = 0; i < 4; i++)
    {
        reversedBytes[i] = numBytes[3-i];
    }
    memcpy(&bigEnd,&reversedBytes,4);

    return bigEnd;
}

char* getBinWord(FILE *bin){
    //read binary until comma appears
    int wordIndex = 0;
    char *word = (char*) malloc(50 * sizeof(char));
    if(word == NULL){
        printf("Memory allocation error");
    }
    while (fread(&word[wordIndex],1,1,bin) == 1)
    {
        if(word[wordIndex] == ','||word[wordIndex] == '\n'){
            word[wordIndex] = '\0';
            break;
        }
        wordIndex++;
    }

    return word;
    
}

int readCSVWriteBIN(char *input_file, char *output_file){
    FILE *fp;
    char c;
    char line[256];
    int numLines = 0;

    fp = fopen(input_file,"r");
    if(fp == NULL){
        printf("File not found\n");
        return 1;
    }

    //count number of lines
    while((c = fgetc(fp)) != EOF){
        if (c == '\n')
        {
            numLines++;   
        }
    }

    //reset file pointer
    fseek(fp, 0, SEEK_SET);

    //skips first line
    while ((c = fgetc(fp)) != '\n'){}
    
    //first line is description no need for that
    numLines--;

    //to be sure
    persons = NULL;
    //allocate memory
    persons = malloc(numLines * sizeof(struct Person));
    if(persons == NULL){
        printf("Could not allocate memory\n");
        return 2;
    }

    //puts infos into struct array
    for (int i = 0; i < numLines; i++)
    {
        //read line
        fscanf(fp,"%[^\n]", line);
        //copy string into dest
        strcpy(persons[i].name, getWord(line));
        strcpy(persons[i].surname, getWord(line));
        strcpy(persons[i].gender, getWord(line));
        strcpy(persons[i].occupacy, getWord(line));
        strcpy(persons[i].education, getWord(line));
        strcpy(persons[i].email, getWord(line));
        strcpy(persons[i].bankAccountNumber, getWord(line));
        strcpy(persons[i].iban, getWord(line));
        strcpy(persons[i].accountType, getWord(line));
        strcpy(persons[i].currency, getWord(line));
        persons[i].balance = atoi(getWord(line));
        strcpy(persons[i].loanAvaiable, getWord(line));

        //go to other line
        getc(fp);
        
    }
    
    fclose(fp);

    //open binary file
    FILE *bin;
    bin = fopen(output_file, "wb");
    if(bin == NULL){
        printf("Could not open binary file\n");
        return 3;
    }

    //write binary
    for (int i = 0; i < numLines; i++) {
        
        fwrite(persons[i].name, 1, strlen(persons[i].name), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].surname, 1, strlen(persons[i].surname), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].gender, 1, strlen(persons[i].gender), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].occupacy, 1, strlen(persons[i].occupacy), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].education, 1, strlen(persons[i].education), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].email, 1, strlen(persons[i].email), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].bankAccountNumber, 1, strlen(persons[i].bankAccountNumber), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].iban, 1, strlen(persons[i].iban), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].accountType, 1, strlen(persons[i].accountType), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].currency, 1, strlen(persons[i].currency), fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(&persons[i].balance, sizeof(int), 1, fp);
        fwrite(",", sizeof(char), 1, fp);
        fwrite(persons[i].loanAvaiable, 1, strlen(persons[i].loanAvaiable), fp);
        fwrite("\n", sizeof(char), 1, fp);
    }

    fclose(bin);
    return 0;
}

char* getWord(char* line){
    //reads line until comma or EOF or \n
    int wordIndex = 0;
    char* word = (char*) malloc(50 * sizeof(char));
    if(word == NULL){
        printf("Memory allocation error");
    }
    for (int i = 0; i < strlen(line); i++)
    {
        if(line[i] == ',' || line[i] == '\n' || line[i] == '\0'){
            break;
        }

        word[wordIndex] = line[i];
        wordIndex++;
    }
    //moves the line pointer so it does not read unneccessary parts
    memmove(line, line + wordIndex+ 1, strlen(line) - wordIndex + 1);
    word[wordIndex] = '\0';
    return word;
}
