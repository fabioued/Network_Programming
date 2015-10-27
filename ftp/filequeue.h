struct filequeue
{
	struct filenode* start;
	struct filenode* end;
	int nfile;
};
struct filenode
{
	struct filenode* next;
	char* filename;
};

typedef struct filequeue filequeue;
typedef struct filenode filenode;

void push(filequeue* FQ, filenode* FN)
{
	if(FQ->end == NULL){
		FQ->start = FN;
		FQ->end = FN;
		FQ->nfile++;
	}
	else{
		FQ->end->next = FN;
		FQ->end = FN;
		FQ->nfile++;
	}
}

filenode* pop(filequeue* FQ)
{
	filenode* temp;
	if(FQ->start == NULL)
		return NULL;
	else{
		temp = FQ->start;
		if((FQ->start = temp->next) == NULL)
			FQ->end = NULL;
		FQ->nfile--;
		return temp;
	}
}

int nfile(filequeue* FQ)
{
	return FQ->nfile;
}

filenode* createnode(char* filename)
{
	filenode* temp = (filenode*)malloc(sizeof(filenode));
	temp->filename = (char*)malloc(strlen(filename));
	strcpy(temp->filename, filename);
	temp->next = NULL;
	return temp;
}

void deletenode(filenode* FN)
{
	memset(FN->filename, 0, sizeof(FN->filename));
	free(FN->filename);
	memset(FN, 0, sizeof(FN));
	free(FN);
}
