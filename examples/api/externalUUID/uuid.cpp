/* 
 *
 * declare function se:uuid() as xs:string external;
 * se:uuid()
 * 
 * <result>1E87C960-0000-4000-8700-000051000003</result>
 */

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sedna_ef.h"


#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" {
//This is needed to ensure that functions will be exported as C functions
//and to let MSVC know what functions to export
#ifdef _WIN32
char const DLLEXPORT *ef_names[];
#endif
SEDNA_SEQUENCE_ITEM DLLEXPORT *uuid(SEDNA_EF_INIT *init, SEDNA_EF_ARGS *args, char * error_msg_buf);
void DLLEXPORT uuid_init(SEDNA_EF_INIT *init, char * error_msg_buf);
void DLLEXPORT uuid_deinit(SEDNA_EF_INIT *init, char * error_msg_buf);
}

char const *ef_names[] = { "uuid", NULL};
SEDNA_SEQUENCE_ITEM *item = NULL;


void uuid_init(SEDNA_EF_INIT *init, char * error_msg_buf)
{
	item = (SEDNA_SEQUENCE_ITEM*)init->sedna_malloc(sizeof(SEDNA_SEQUENCE_ITEM));
	item->data.type = SEDNATYPE_string;
	item->data.val_string = (SEDNA_string)malloc(1024);
}

void uuid_deinit(SEDNA_EF_INIT *init, char * error_msg_buf)
{
	free(item->data.val_string);
//	init->sedna_free(item);
}


SEDNA_SEQUENCE_ITEM *uuid(SEDNA_EF_INIT *init, SEDNA_EF_ARGS *args, char * error_msg_buf)
{
	SEDNA_SEQUENCE_ITEM *res = NULL, *last;
	
	if (args->length != 0)
	{
		sprintf(error_msg_buf, "bad number of arguments!");
		return NULL;
	}
	// sprintf(error_msg_buf, "arguments is zero");
	// return NULL;
	
//	uuid_t value;
//	uuid_generate(value);
//	uuid_unparse_upper(value, item->data.val_string );
	boost::uuids::random_generator generator;
	boost::uuids::uuid uuid1 = generator();

//	 uuid_generate(value);
//	 uuid_unparse_upper(value, str );

//    printf("%s",str);

	if(item == NULL){
		sprintf(error_msg_buf, "item is null");
		return NULL;
	}
//	sprintf(str,"aiueo");
//	item->data.val_string = str;
	std::string struuid = boost::lexical_cast<std::string>(uuid1);
	strcpy(item->data.val_string, struuid.c_str());
	item->next = NULL;

	return item;
}

