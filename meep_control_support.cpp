/*--------------------------------------
	meep_control_support0.cpp 
----------------------------------------*/
#include "meep_control_support.hpp"
#include "meep.hpp"
#define MEEP_CONTROL_SUPPORT_DEF
#ifdef MEEP_H
using meep::master_printf;
#endif

/* class intVar */

intVar::intVar(char *name, int init_val, char *desc){
	this->key=name;
	this->val=init_val;
	this->description=desc;
	this->range=NULL;
	this->values_list=NULL;
	this->initialized=true;
	this->next_intVar=NULL;
}

intVar::intVar(char *name, int init_val, int min_val, int max_val, char *desc){
	this->key=name;
	this->description=desc;
	this->initialized=false;
	if( (min_val<=init_val) && (init_val<=max_val) ){
		this->range=new int[2];
		this->range[0]=min_val;
		this->range[1]=max_val;
		this->val=init_val;
		this->initialized=true;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [intVar::intVar(...) -- %s]: Initial value (%d) outside of specified range (min = %d, max = %d).\n", name, init_val, min_val, max_val);
		#else
		printf("# Warning [intVar::intVar(...) -- %s]: Initial value (%d) outside of specified range (min = %d, max = %d).\n", name, init_val, min_val, max_val);
		#endif
	}
	this->values_list=NULL;
	this->next_intVar=NULL;
}


intVar::intVar(char *name, int init_val, int num_vals, int *accpt_vals, char *desc){
	this->key=name;
	this->description=desc;
	this->initialized=false;
	if(num_vals>0){
		this->num_values_list=num_vals;
		this->values_list=new int[num_vals];
		for(int j=0; j<num_vals; j++){
			values_list[j]=accpt_vals[j];
			if(init_val==accpt_vals[j]){
				this->val=init_val;
				this->initialized=true;
			}
		}
		if(!(this->initialized)){
			#ifdef MEEP_H
			master_printf("# Warning [intVar::intVar(...) -- %s]: Initial value (%d) outside of specified list of admissible values.\n", name, init_val);
			#else
			printf("# Warning [intVar::intVar(...) -- %s]: Initial value (%d) outside of specified list of admissible values.\n", name, init_val);
			#endif
		}
	}
	else{
		this->num_values_list=0;
		this->values_list=NULL;
		#ifdef MEEP_H
		master_printf("# Warning [intVar::intVar(...) -- %s]: Object not initialized due to negative number specified for count of admissible values.\n", name);
		#else
		printf("# Warning [intVar::intVar(...) -- %s]: Object not initialized due to negative number specified for count of admissible values.\n", name);
		#endif
	}
	this->range=NULL;
	this->next_intVar=NULL;
}

char* intVar::variable_name(){
	return this->key;
}

int intVar::value(){
	if(this->initialized)		
		return this->val;
	/* Error Messages for Attenpt to Access Un-Initialized Value -- No Value to be Returned to Function Call */	
	#ifdef MEEP_H
	master_printf("# Error [intVar::value() -- %s]: Access to un-initialized value denied. No value returned [cf. system error message(s) as applicable].\n", this->key);
	#else
	printf("# Error [intVar::value() -- %s]: Access to un-initialized value denied. No value returned [cf. system error message(s) as applicable].\n", this->key);
	#endif
	return ((std::numeric_limits<int>::max()+1) + (std::numeric_limits<int>::min()-1));	/* Deliberate setting of return value to generate overflow error for accessing unitialized variable */
}

intVar* intVar::next(){
	return this->next_intVar;
}

void intVar::set_next_variable(intVar *next){
	this->next_intVar=next;
}

status intVar::set(int vlu){
	if(this->range!=NULL){
		if( (this->range[0]<=vlu) && (vlu<=this->range[1]) ){
			this->val=vlu;
			this->initialized=true;
			return success;
		}
		#ifdef MEEP_H
		master_printf("# Warning [intVar::intVar(...) -- %s]: Specified value (%d) outside of admissible range (min = %d, max = %d).\n", this->key, vlu, this->range[0], this->range[1]);
		#else
		printf("# Warning [intVar::intVar(...) -- %s]: Specified value (%d) outside of admissible range (min = %d, max = %d).\n", this->key, vlu, this->range[0], this->range[1]);
		#endif
		return failure;
	}
	else if(this->values_list!=NULL){
		for(int j=0; j<(this->num_values_list); j++)
			if(vlu==values_list[j]){
				this->val=vlu;
				this->initialized=true;
				return success;
			}
		#ifdef MEEP_H
		master_printf("# Warning [intVar::set(...) -- %s]: Specified value (%d) outside of list of admissible values.\n", this->key, vlu);
		#else
		printf("# Warning [intVar::set(...) -- %s]: Specified value (%d) outside of list of admissible values.\n", this->key, vlu);
		#endif
		return failure;
	}
	else{
		this->val=vlu;
		this->initialized=true;
		return success;
	}
}

void intVar::print_state(){
	#ifdef MEEP_H
	master_printf("\n\t%s (int) = ", this->key);
	#else
	printf("\n\t%s (int) = ", this->key);
	#endif
	if(this->initialized){
		#ifdef MEEP_H
		master_printf("%d : ", this->val);
		#else
		printf("%d :", this->val);
		#endif
	}
	else{
		#ifdef MEEP_H
		master_printf("[un-initialized] : ");
		#else
		printf("[un-initialized] :");
		#endif
	}
	#ifdef MEEP_H
	master_printf("\n\t\t%s", this->description);
	#else
	printf("\n\t\t%s", this->description);
	#endif
	if(this->range!=NULL){
		#ifdef MEEP_H
		master_printf("\n\t\tAdmissible range of values: min = %d, max = %d\t", this->range[0], this->range[1]);
		#else
		printf("\n\t\tAdmissible range of values: min = %d, max = %d\t", this->range[0], this->range[1]);
		#endif
	}
	if(this->values_list!=NULL){
		#ifdef MEEP_H
		master_printf("\n\t\tAdmissible values: {");
		#else
		printf("\n\t\tAdmissible values: {");
		#endif
		for(int j=0; j<(this->num_values_list); j++){
			#ifdef MEEP_H
			master_printf("%d,", this->values_list[j]);
			#else
			printf("%d,", this->values_list[j]);
			#endif
		}
		#ifdef MEEP_H
		master_printf("\b}");
		#else
		printf("\b}");
		#endif
	}
	#ifdef MEEP_H
	master_printf("\n");
	#else
	printf("\n");
	#endif
}


/* class fltVar */


fltVar::fltVar(char *name, double init_val, char *desc){
	this->key=name;
	this->val=init_val;
	this->description=desc;
	this->range=NULL;
	this->values_list=NULL;
	this->initialized=true;
	this->next_fltVar=NULL;
}

fltVar::fltVar(char *name, double init_val, double min_val, double max_val, char *desc){
	this->key=name;
	this->description=desc;
	this->initialized=false;
	if( (min_val<=init_val) && (init_val<=max_val) ){
		this->range=new double[2];
		this->range[0]=min_val;
		this->range[1]=max_val;
		this->val=init_val;
		this->initialized=true;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [fltVar::fltVar(...) -- %s]: Initial value (%f) outside of specified range (min = %f, max = %f).\n", name, init_val, min_val, max_val);
		#else
		printf("# Warning [fltVar::fltVar(...) -- %s]: Initial value (%f) outside of specified range (min = %f, max = %f).\n", name, init_val, min_val, max_val);
		#endif
	}
	this->values_list=NULL;
	this->next_fltVar=NULL;
}


fltVar::fltVar(char *name, double init_val, int num_vals, double *accpt_vals, char *desc){
	this->key=name;
	this->description=desc;
	this->initialized=false;
	if(num_vals>0){
		this->num_values_list=num_vals;
		this->values_list=new double[num_vals];
		for(int j=0; j<num_vals; j++){
			values_list[j]=accpt_vals[j];
			if(init_val==accpt_vals[j]){
				this->val=init_val;
				this->initialized=true;
			}
		}
		if(!(this->initialized)){
			#ifdef MEEP_H
			master_printf("# Warning [fltVar::fltVar(...) -- %s]: Initial value (%f) outside of specified list of admissible values.\n", name, init_val);
			#else
			printf("# Warning [fltVar::fltVar(...) -- %s]: Initial value (%f) outside of specified list of admissible values.\n", name, init_val);
			#endif
		}
	}
	else{
		this->num_values_list=0;
		this->values_list=NULL;
		#ifdef MEEP_H
		master_printf("# Warning [fltVar::fltVar(...) -- %s]: Object not initialized due to negative number specified for count of admissible values.\n", name);
		#else
		printf("# Warning [fltVar::fltVar(...) -- %s]: Object not initialized due to negative number specified for count of admissible values.\n", name);
		#endif
	}
	this->range=NULL;
	this->next_fltVar=NULL;
}

char* fltVar::variable_name(){
	return this->key;
}

double fltVar::value(){
	if(this->initialized)		
		return this->val;
	/* Error Messages for Attenpt to Access Un-Initialized Value -- No Value to be Returned to Function Call */	
	#ifdef MEEP_H
	master_printf("# Error [fltVar::value() -- %s]: Access to un-initialized value denied. No value returned [cf. system error message(s) as applicable].\n", this->key);
	#else
	printf("# Error [fltVar::value() -- %s]: Access to un-initialized value denied. No value returned [cf. system error message(s) as applicable].\n", this->key);
	#endif
	return ((std::numeric_limits<double>::max()+1) + (std::numeric_limits<double>::min()-1)); /* Deliberate setting of return value to generate overflow error for accessing unitialized variable */
}

fltVar* fltVar::next(){
	return this->next_fltVar;
}

void fltVar::set_next_variable(fltVar *next){
	this->next_fltVar=next;
}

status fltVar::set(double vlu){
	if(this->range!=NULL){
		if( (this->range[0]<=vlu) && (vlu<=this->range[1]) ){
			this->val=vlu;
			this->initialized=true;
			return success;
		}
		#ifdef MEEP_H
		master_printf("# Warning [fltVar::fltVar(...) -- %s]: Specified value (%f) outside of admissible range (min = %f, max = %f).\n", this->key, vlu, this->range[0], this->range[1]);
		#else
		printf("# Warning [fltVar::fltVar(...) -- %s]: Specified value (%f) outside of admissible range (min = %f, max = %f).\n", this->key, vlu, this->range[0], this->range[1]);
		#endif
		return failure;
	}
	else if(this->values_list!=NULL){
		for(int j=0; j<(this->num_values_list); j++)
			if(vlu==values_list[j]){
				this->val=vlu;
				this->initialized=true;
				return success;
			}
		#ifdef MEEP_H
		master_printf("# Warning [fltVar::set(...) -- %s]: Specified value (%f) outside of list of admissible values.\n", this->key, vlu);
		#else
		printf("# Warning [fltVar::set(...) -- %s]: Specified value (%f) outside of list of admissible values.\n", this->key, vlu);
		#endif
		return failure;
	}
	else{
		this->val=vlu;
		this->initialized=true;
		return success;
	}
}

void fltVar::print_state(){
	#ifdef MEEP_H
	master_printf("\n\t%s (double) = ", this->key);
	#else
	printf("\n\t%s (double) = ", this->key);
	#endif
	if(this->initialized){
		#ifdef MEEP_H
		master_printf("%f :", this->val);
		#else
		printf("%f : ", this->val);
		#endif
	}
	else{
		#ifdef MEEP_H
		master_printf("[un-initialized] : ");
		#else
		printf("[un-initialized] : ");
		#endif
	}
	#ifdef MEEP_H
	master_printf("\n\t\t%s", this->description);
	#else
	printf("\n\t\t%s", this->description);
	#endif
	if(this->range!=NULL){
		#ifdef MEEP_H
		master_printf("\n\t\tAdmissible range of values: min = %f, max = %f\t", this->range[0], this->range[1]);
		#else
		printf("\n\t\tAdmissible range of values: min = %f, max = %f\t", this->range[0], this->range[1]);
		#endif
	}
	if(this->values_list!=NULL){
		#ifdef MEEP_H
		master_printf("\n\t\tAdmissible values: {");
		#else
		printf("\n\t\tAdmissible values: {");
		#endif
		for(int j=0; j<(this->num_values_list); j++){
			#ifdef MEEP_H
			master_printf("%f,", this->values_list[j]);
			#else
			printf("%f,", this->values_list[j]);
			#endif
		}
		#ifdef MEEP_H
		master_printf("\b}");
		#else
		printf("\b}");
		#endif
	}
	#ifdef MEEP_H
	master_printf("\n");
	#else
	printf("\n");
	#endif
}

/* class specialVar */

specialVar::specialVar(char *name, char *init_val, char *desc){
	this->key=name;
	this->val=init_val;
	this->description=desc;
	this->values_list=NULL;
	this->initialized=true;
	this->next_specialVar=NULL;
}

specialVar::specialVar(char *name, char *init_val, int num_vals, char **accpt_vals, char *desc){
	this->key=name;
	this->description=desc;
	this->initialized=false;
	if(num_vals>0){
		this->num_values_list=num_vals;
		this->values_list=new char*[num_vals]; 
		for(int j=0; j<num_vals; j++){
			values_list[j]=accpt_vals[j];
			if(string(init_val).compare(accpt_vals[j])==0){ 
				this->val=init_val;
				this->initialized=true;
			}
		}
		if(!(this->initialized)){
			#ifdef MEEP_H
			master_printf("# Warning [specialVar::specialVar(...) -- %s]: Initial value (%s) outside of specified list of admissible values.\n", name, init_val);
			#else
			printf("# Warning [specialVar::specialVar(...) -- %s]: Initial value (%s) outside of specified list of admissible values.\n", name, init_val);
			#endif
		}
	}
	else{
		this->num_values_list=0;
		this->values_list=NULL;
		#ifdef MEEP_H
		master_printf("# Warning [specialVar::specialVar(...) -- %s]: Object not initialized due to negative number specified for count of admissible values.\n", name);
		#else
		printf("# Warning [specialVar::specialVar(...) -- %s]: Object not initialized due to negative number specified for count of admissible values.\n", name);
		#endif
	}
	this->next_specialVar=NULL;
}

char* specialVar::variable_name(){
	return this->key;
}

char* specialVar::value(){
	if(this->initialized)		
		return this->val;
	/* Error Messages for Attenpt to Access Un-Initialized Value -- No Value to be Returned to Function Call */	
	#ifdef MEEP_H
	master_printf("# Error [specialVar::value() -- %s]: Access to un-initialized value denied. No value returned [cf. system error message(s) as applicable].\n", this->key);
	#else
	printf("# Error [specialVar::value() -- %s]: Access to un-initialized value denied. No value returned [cf. system error message(s) as applicable].\n", this->key);
	#endif
	return NULL;
}

specialVar* specialVar::next(){
	return this->next_specialVar;
}

void specialVar::set_next_variable(specialVar *next){
	this->next_specialVar=next;
}

status specialVar::set(char *vlu){
	if(this->values_list!=NULL){
		for(int j=0; j<(this->num_values_list); j++){
			if(string(vlu).compare(values_list[j])==0){
				this->val=vlu;
				this->initialized=true;
				return success;
			}
		}
		#ifdef MEEP_H
		master_printf("# Warning [specialVar::set(...) -- %s]: Specified value (%s) outside of list of admissible values.\n", this->key, vlu);
		#else
		printf("# Warning [specialVar::set(...) -- %s]: Specified value (%s) outside of list of admissible values.\n", this->key, vlu);
		#endif
		return failure;
	}
	else{
		this->val=vlu;
		this->initialized=true;
		return success;
	}
}

void specialVar::print_state(){
	#ifdef MEEP_H
	master_printf("\n\t%s (char*) = ", this->key);
	#else
	printf("\n\t%s (char*) = ", this->key);
	#endif
	if(this->initialized){
		#ifdef MEEP_H
		master_printf("%s : ", this->val);
		#else
		printf("%s : ", this->val);
		#endif
	}
	else{
		#ifdef MEEP_H
		master_printf("[un-initialized] : ");
		#else
		printf("[un-initialized] : ");
		#endif
	}
	#ifdef MEEP_H
	master_printf("\n\t\t%s", this->description);
	#else
	printf("\n\t\t%s", this->description);
	#endif
	if(this->values_list!=NULL){
		#ifdef MEEP_H
		master_printf("\n\t\tAdmissible values: {");
		#else
		printf("\n\t\tAdmissible values: {");
		#endif
		for(int j=0; j<(this->num_values_list); j++){
			#ifdef MEEP_H
			master_printf("%s,", this->values_list[j]);
			#else
			printf("%s,", this->values_list[j]);
			#endif
		}
		#ifdef MEEP_H
		master_printf("\b}");
		#else
		printf("\b}");
		#endif
	}
	#ifdef MEEP_H
	master_printf("\n");
	#else
	printf("\n");
	#endif
}

/* class controlVariables */

controlVariables::controlVariables(){
	this->start_int=NULL;
	this->start_flt=NULL;
	this->start_special=NULL;
	this->count_int=0;
	this->count_flt=0;
	this->count_special=0;
}

void controlVariables::add_int(char *name, int init_val, char *desc){
	intVar *curr=this->start_int;
	bool pre_existing=false;
	while((curr!=NULL) && (curr->next()!=NULL)){	/* Finding last node of an alreasdy initiliazed linked list or pre-existing node of same key */
		//printf("# Diagnostic Check: curr->variable_name()=%s, name=%s; string(curr->variable_name()).compare(name)=%d\n", curr->variable_name(), name, string(curr->variable_name()).compare(name));
		if(string(curr->variable_name()).compare(name)==0){
			pre_existing=true;
			break;
		}
		curr=curr->next();
	}
	if(!pre_existing){		
		intVar *intVar_new=new intVar(name, init_val, desc);
		if(this->start_int==NULL)	/* Special case: start of linked list  */
			this->start_int=intVar_new;
		else
			curr->set_next_variable(intVar_new);
		count_int++;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [controlVariables::add_int(...)]: Pre-existing int-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#else
		printf("# Warning [controlVariables::add_int(...)]: Pre-existing int-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#endif
	}
}

void controlVariables::add_int(char *name, int init_val, int min_val, int max_val, char *desc){
	intVar *curr=this->start_int;
	bool pre_existing=false;
	while(curr!=NULL){	/* Traversing linked list to find previous instance of variable name or last node, whichever occurring first */
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name already in existence */
			pre_existing=true;
			break;
		}
		if(curr->next()==NULL)	/* To stop at last node of linked list */
			break;
		curr=curr->next();
	}
	if(!pre_existing){
		intVar *intVar_new=new intVar(name, init_val, min_val, max_val, desc);
		if(this->start_int==NULL)	/* Special case: start of linked list  */
			this->start_int=intVar_new;
		else
			curr->set_next_variable(intVar_new);
		count_int++;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [controlVariables::add_int(...)]: Pre-existing int-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#else
		printf("# Warning [controlVariables::add_int(...)]: Pre-existing int-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#endif
	}
}

void controlVariables::add_int(char *name, int init_val, int num_vals, int *accpt_vals, char *desc){
	intVar *curr=this->start_int;
	bool pre_existing=false;
	while(curr!=NULL){	/* Traversing linked list to find previous instance of variable name or last node, whichever occurring first */
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name already in existence */
			pre_existing=true;
			break;
		}
		if(curr->next()==NULL)	/* To stop at last node of linked list */
			break;
		curr=curr->next();
	}
	if(!pre_existing){
		intVar *intVar_new=new intVar(name, init_val, num_vals, accpt_vals, desc);
		if(this->start_int==NULL)	/* Special case: start of linked list  */
			this->start_int=intVar_new;
		else
			curr->set_next_variable(intVar_new);
		count_int++;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [controlVariables::add_int(...)]: Pre-existing int-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#else
		printf("# Warning [controlVariables::add_int(...)]: Pre-existing int-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#endif
	}
}

void controlVariables::add_double(char *name, double init_val, char *desc){
	fltVar *curr=this->start_flt;
	bool pre_existing=false;
	while(curr!=NULL){	/* Traversing linked list to find previous instance of variable name or last node, whichever occurring first */
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name already in existence */
			pre_existing=true;
			break;
		}
		if(curr->next()==NULL)	/* To stop at last node of linked list */
			break;
		curr=curr->next();
	}
	if(!pre_existing){
		fltVar *fltVar_new=new fltVar(name, init_val, desc);
		if(this->start_flt==NULL)	/* Special case: start of linked list  */
			this->start_flt=fltVar_new;
		else
			curr->set_next_variable(fltVar_new);
		count_flt++;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [controlVariables::add_double(...)]: Pre-existing double-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#else
		printf("# Warning [controlVariables::add_double(...)]: Pre-existing double-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#endif
	}

}

void controlVariables::add_double(char *name, double init_val, double min_val, double max_val, char *desc){
	fltVar *curr=this->start_flt;	
	bool pre_existing=false;
	while(curr!=NULL){	/* Traversing linked list to find previous instance of variable name or last node, whichever occurring first */
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name already in existence */
			pre_existing=true;
			break;
		}
		if(curr->next()==NULL)	/* To stop at last node of linked list */
			break;
		curr=curr->next();
	}
	if(!pre_existing){
		fltVar *fltVar_new=new fltVar(name, init_val, min_val, max_val, desc);
		if(this->start_flt==NULL)	/* Special case: start of linked list  */
			this->start_flt=fltVar_new;
		else
			curr->set_next_variable(fltVar_new);
		count_flt++;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [controlVariables::add_double(...)]: Pre-existing double-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#else
		printf("# Warning [controlVariables::add_double(...)]: Pre-existing double-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#endif
	}
}

void controlVariables::add_double(char *name, double init_val, int num_vals, double *accpt_vals, char *desc){
	fltVar *curr=this->start_flt;	
	bool pre_existing=false;
	while(curr!=NULL){	/* Traversing linked list to find previous instance of variable name or last node, whichever occurring first */
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name already in existence */
			pre_existing=true;
			break;
		}
		if(curr->next()==NULL)	/* To stop at last node of linked list */
			break;
		curr=curr->next();
	}
	if(!pre_existing){
		fltVar *fltVar_new=new fltVar(name, init_val, num_vals, accpt_vals, desc);
		if(this->start_flt==NULL)
			this->start_flt=fltVar_new;
		else
			curr->set_next_variable(fltVar_new);
		count_flt++;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [controlVariables::add_double(...)]: Pre-existing double-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#else
		printf("# Warning [controlVariables::add_double(...)]: Pre-existing double-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#endif
	}
}

void controlVariables::add_special(char *name, char *init_val, char *desc){
	specialVar *curr=this->start_special;	
	bool pre_existing=false;
	while(curr!=NULL){	/* Traversing linked list to find previous instance of variable name or last node, whichever occurring first */
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name already in existence */
			pre_existing=true;
			break;
		}
		if(curr->next()==NULL)	/* To stop at last node of linked list */
			break;
		curr=curr->next();
	}
	if(!pre_existing){
		specialVar *specialVar_new=new specialVar(name, init_val, desc);
		if(this->start_special==NULL)	/* Special case: start of linked list  */
			this->start_special=specialVar_new;
		else
			curr->set_next_variable(specialVar_new);
		count_special++;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [controlVariables::add_special(...)]: Pre-existing special/char*-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#else
		printf("# Warning [controlVariables::add_special(...)]: Pre-existing special/char*-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#endif
	}
}

void controlVariables::add_special(char *name, char *init_val, int num_vals, char **accpt_vals, char *desc){
	specialVar *curr=this->start_special;	
	bool pre_existing=false;
	while(curr!=NULL){	/* Traversing linked list to find previous instance of variable name or last node, whichever occurring first */
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name already in existence */
			pre_existing=true;
			break;
		}
		if(curr->next()==NULL)	/* To stop at last node of linked list */
			break;
		curr=curr->next();
	}
	if(!pre_existing){
		specialVar *specialVar_new=new specialVar(name, init_val, num_vals, accpt_vals, desc);
		if(this->start_special==NULL)	/* Special case: start of linked list  */
			this->start_special=specialVar_new;
		else
			curr->set_next_variable(specialVar_new);
		count_special++;
	}
	else{
		#ifdef MEEP_H
		master_printf("# Warning [controlVariables::add_special(...)]: Pre-existing special/char*-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#else
		printf("# Warning [controlVariables::add_special(...)]: Pre-existing special/char*-valued variable \'%s\' -- new variable addition ineffectual.\n", curr->variable_name());
		#endif
	}
}

int controlVariables::value_int(char *name){
	intVar *curr=this->start_int;
	while( (curr!=NULL) && (string(curr->variable_name()).compare(name)!=0))
		curr=curr->next();
	if(((curr!=NULL)) && (string(curr->variable_name()).compare(name)==0))
		return curr->value();
	#ifdef MEEP_H
	master_printf("# Error [controlVariables::value_int(...)]: Non-existent int-valued variable of name \'%s\'.\n", name);
	#else
	printf("# Error [controlVariables::value_int(...)]: Non-existent int-valued variable of name \'%s\'.\n", name);
	#endif
	return (1/0);	/* Deliberate setting of return value to generate error for accessing unitialized variable */
}

double controlVariables::value_double(char *name){
	fltVar *curr=this->start_flt;
	while( (curr!=NULL) && (string(curr->variable_name()).compare(name)!=0))
		curr=curr->next();
	if(((curr!=NULL)) && (string(curr->variable_name()).compare(name)==0))
		return curr->value();
	#ifdef MEEP_H
	master_printf("# Error [controlVariables::value_double(...)]: Non-existent double-valued variable of name \'%s\'.\n", name);
	#else
	printf("# Error [controlVariables::value_double(...)]: Non-existent double-valued variable of name \'%s\'.\n", name);
	#endif
	return (1/0);	/* Deliberate setting of return value to generate error for accessing unitialized variable */
}

char* controlVariables::value_special(char *name){
	specialVar *curr=this->start_special;
	while( (curr!=NULL) && (string(curr->variable_name()).compare(name)!=0))
		curr=curr->next();
	if(((curr!=NULL)) && (string(curr->variable_name()).compare(name)==0))
		return curr->value();
	#ifdef MEEP_H
	master_printf("# Error [controlVariables::value_special(...)]: Non-existent special/char*-valued variable of name \'%s\'.\n", name);
	#else
	printf("# Error [controlVariables::value_special(...)]: Non-existent special/char*-valued variable of name \'%s\'.\n", name);
	#endif
	return "Error! #@*&~^!";
}

status controlVariables::set(char *name, int vlu){
	intVar *curr=this->start_int;
	/*bool pre_existing=false;
	while(curr!=NULL){	/* Setting off to traverse linked list to the end 
		printf("# Dignostic Check [controlVariables::set(...)]: curr->variable_name()=%s, name=%s, string(curr->variable_name()).compare(name)=%d.\n", curr->variable_name(), name, string(curr->variable_name()).compare(name))
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name found 
			pre_existing=true;
			break;
		}
		curr=curr->next();
	}
	if(pre_existing){*/
	while( (curr!=NULL) && (string(curr->variable_name()).compare(name)!=0))
		curr=curr->next();
	if(((curr!=NULL)) && (string(curr->variable_name()).compare(name)==0)){
		curr->set(vlu);
		return success;
	}
	#ifdef MEEP_H
	master_printf("# Warning [controlVariables::set(...)]: Unable to set variable \'%s\' to specified int value (%d).\n", name, vlu);
	#else
	printf("# Warning [controlVariables::set(...)]: Unable to set variable \'%s\' to specified int value (%d).\n", name, vlu);
	#endif		
	return failure;
}

status controlVariables::set(char *name, double vlu){
	fltVar *curr=this->start_flt;
	/*bool pre_existing=false;
	while(curr!=NULL){	/* Setting off to traverse linked list to the end 
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name found 
			pre_existing=true;
			break;
		}
		curr=curr->next();
	}
	if(pre_existing){*/
	while( (curr!=NULL) && (string(curr->variable_name()).compare(name)!=0))
		curr=curr->next();
	if(((curr!=NULL)) && (string(curr->variable_name()).compare(name)==0)){
		curr->set(vlu);
		return success;
	}
	#ifdef MEEP_H
	master_printf("# Warning [controlVariables::set(...)]: Unable to set variable \'%s\' to specified double value (%f).\n", name, vlu);
	#else
	printf("# Warning [controlVariables::set(...)]: Unable to set variable \'%s\' to specified double value (%f).\n", name, vlu);
	#endif		
	return failure;
}

status controlVariables::set(char *name, char *vlu){
	specialVar *curr=this->start_special;
	/*bool pre_existing=false;
	while(curr!=NULL){	/* Setting off to traverse linked list to the end 
		if(string(curr->variable_name()).compare(name)==0){	/* To stop if variable name found
			pre_existing=true;
			break;
		}
		curr=curr->next();
	}
	if(pre_existing){*/
	while( (curr!=NULL) && (string(curr->variable_name()).compare(name)!=0))
		curr=curr->next();
	if(((curr!=NULL)) && (string(curr->variable_name()).compare(name)==0)){
		curr->set(vlu);
		return success;
	}
	#ifdef MEEP_H
	master_printf("# Warning [controlVariables::set(...)]: Unable to set variable \'%s\' to specified char* value (%s).\n", name, vlu);
	#else
	printf("# Warning [controlVariables::set(...)]: Unable to set variable \'%s\' to specified char* value (%s).\n", name, vlu);
	#endif		
	return failure;
}

void controlVariables::overview(bool help_section){
	if(help_section){
		#ifdef MEEP_H
		master_printf("# Dispalying \'Help\' section ...\n\n");
		master_printf("# Execution syntax on Linux terminal: \n\t\'./<executable_name>.x <variable1_name> <variable1_value> <variable2_name> <variable2_value> ... \'\n");
		#else
		printf("# Dispalying \'Help\' section ...\n\n");
		printf("# Execution syntax on Linux terminal: \n\t\'./<executable_name>.x <variable1_name> <variable1_value> <variable2_name> <variable2_value> ... \'\n");
		#endif
	}
	else{
		#ifdef MEEP_H
		master_printf("# Information [controlVariables::overview(...)]: %d int-valued variables, %d double-valued variables and %d special-valued variables.\n", this->count_int, this->count_flt, this->count_special);
		#else
		printf("# Information [controlVariables::overview(...)]: %d int-valued variables, %d double-valued variables and %d special-valued variables.\n", this->count_int, this->count_flt, this->count_special);
		#endif
	}

	intVar *curr_int=this->start_int;
	fltVar *curr_flt=this->start_flt;
	specialVar *curr_special=this->start_special;
	while(curr_int!=NULL){
		curr_int->print_state();
		curr_int=curr_int->next();
	}
	while(curr_flt!=NULL){
		curr_flt->print_state();
		curr_flt=curr_flt->next();
	}
	while(curr_special!=NULL){
		curr_special->print_state();
		curr_special=curr_special->next();
	}
}

status controlVariables::parse_var(char *name, char *arg){
	intVar *curr_int=this->start_int;
	fltVar *curr_flt=this->start_flt;
	specialVar *curr_special=this->start_special;
	while( (curr_int!=NULL) && (string(curr_int->variable_name()).compare(name)!=0) )
		curr_int=curr_int->next();
	if( (curr_int!=NULL) && (string(curr_int->variable_name()).compare(name)==0) ){
		return curr_int->set(atoi(arg));
	}
	while( (curr_flt!=NULL) && (string(curr_flt->variable_name()).compare(name)!=0) )
		curr_flt=curr_flt->next();
	if( (curr_flt!=NULL) && (string(curr_flt->variable_name()).compare(name)==0) )
		return curr_flt->set(atof(arg));

	while( (curr_special!=NULL) && (string(curr_special->variable_name()).compare(name)!=0) )
		curr_special=curr_special->next();
	if( (curr_special!=NULL) && (string(curr_special->variable_name()).compare(name)==0) )
		return curr_special->set(arg);
	#ifdef MEEP_H
	master_printf("# Error [controlVariables::parse_var(...)]: Variable name \'%s\' not found in repository", name);
	#else
	printf("# Error [controlVariables::parse_var(...)]: Variable name \'%s\' not found in repository", name);
	#endif		
	return failure;
}

status controlVariables::parse_runtime_params(int argc, char **argv){
	for(int arg_index=1;arg_index<argc;arg_index++){
		if((string(argv[arg_index]).compare("-h")==0)||(string(argv[arg_index]).compare("--help")==0)){
			this->overview(true);
			return failure;
		}
		else if(arg_index++!=argc){
			if(this->parse_var(argv[arg_index-1], argv[arg_index])==failure)
				return failure;
		}
		else{
			#ifdef MEEP_H
			master_printf("# Error [controlVariables::parse_runtime_params(...)]: Runtime parameter parsing error near token \'%s\': cf. \'Help\' section by running executable file with flag \'-h\' or \'--help\'.\n", argv[arg_index-1]);
			#else
			printf("# Error [controlVariables::parse_runtime_params(...)]: Runtime parameter parsing error near token \'%s\': cf. \'Help\' section by running executable file with flag \'-h\' or \'--help\'.\n", argv[arg_index-1]);
			#endif	
			return failure;
		}
	}
	#ifdef MEEP_H
	master_printf("# Information [controlVariables::parse_runtime_params(...)]: Runtime parameters parsed.\n");
	#else
	printf("# Information [controlVariables::parse_runtime_params(...)]: Runtime parameters parsed.\n");
	#endif
	this->overview(false);
	return success;
}

int controlVariables::plus_infinity_int(){
	return std::numeric_limits<int>::max();
}

int controlVariables::minus_infinity_int(){
	return std::numeric_limits<int>::min();
}

double controlVariables::plus_infinity_double(){
	return std::numeric_limits<double>::max();
}

double controlVariables::minus_infinity_double(){
	return std::numeric_limits<double>::min();
}

/*-------------------------------------
	meep_control_support.cpp 
---------------------------------------*/
