/*-------------------------------------
	meep_control_support.hpp 
---------------------------------------*/

#define MEEP_CONTROL_SUPPORT_H

enum status{success, failure};
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <limits>
/*
#include "meep.hpp"
using namespace meep;
*/
using std::string;

class intVar{
	public:
		intVar(char *name, int init_val, char *desc);
		intVar(char *name, int init_val, int min_val, int max_val, char *desc);
		intVar(char *name, int init_val, int num_vals, int *accpt_vals, char *desc);
		char* variable_name();
		int value();
		intVar* next();
		void set_next_variable(intVar *next);
		status set(int vlu);
		void print_state();
	private:
		char *key, *description;
		int val, *range, num_values_list, *values_list;
		intVar *next_intVar;
		bool initialized;
};

class fltVar{
	public:
		fltVar(char *name, double init_val, char *desc);
		fltVar(char *name, double init_val, double min_val, double max_val, char *desc);
		fltVar(char *name, double init_val, int num_vals, double *accpt_vals, char *desc);
		char* variable_name();
		double value();
		fltVar* next();
		void set_next_variable(fltVar *next);
		status set(double vlu);
		void print_state();
	private:
		char *key, *description;
		int num_values_list;
		double val, *range, *values_list;
		fltVar *next_fltVar;
		bool initialized;
};

class specialVar{
	public:
		specialVar(char *name, char *init_val, char *desc);
		specialVar(char *name, char *init_val, int num_vals, char **accpt_vals, char *desc);
		char* variable_name();
		char* value();
		specialVar* next();
		void set_next_variable(specialVar *next);
		status set(char *val);
		void print_state();
	private:
		char *key, *val, *description, **values_list;
		int num_values_list;
		specialVar *next_specialVar;
		bool initialized;
};

class controlVariables{
	public:
		controlVariables();
		void add_int(char *name, int init_val, char *desc);
		void add_int(char *name, int init_val, int min_val, int max_val, char *desc);
		void add_int(char *name, int init_val, int num_vals, int *accpt_vals, char *desc);
		void add_double(char *name, double init_val, char *desc);
		void add_double(char *name, double init_val, double min_val, double max_val, char *desc);
		void add_double(char *name, double init_val, int num_vals, double *accpt_vals, char *desc);
		void add_special(char *name, char *init_val, char *desc);
		void add_special(char *name, char *init_val, int num_vals, char **accpt_vals, char *desc);
		int value_int(char *name);
		double value_double(char *name);
		char* value_special(char *name);
		status parse_runtime_params(int argc, char **argv);
		status set(char *name, int vlu);
		status set(char *name, double vlu);
		status set(char *name, char *vlu);
		void overview(bool help_section);
		int plus_infinity_int();
		int minus_infinity_int();
		double plus_infinity_double();
		double minus_infinity_double();
	private:
		intVar *start_int;
		fltVar *start_flt;
		specialVar *start_special;
		int count_int, count_flt, count_special;
		status parse_var(char *name, char *arg);
};

/*-------------------------------------
	meep_control_support.hpp 
---------------------------------------*/

