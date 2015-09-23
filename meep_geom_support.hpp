/*---------------------------*/
/* START: meep_geom_supp.hpp */
/*---------------------------*/
#include <cmath>
#include "meep.hpp"
/* #include "meep_geom_supp0.hpp" // Deactivated line in combined file!!! */
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace meep;

#define max2(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min2(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MEEP_GEOM_SUPPORT_H	/* Specifying  the availability of customized geometry support header files(s) for Meep  */

/* class block2d: 
 * Linkable two-dimensional blocks */
class block2d{
        public:
                block2d(const vec &cen, const vec &dim);	/* Constructor */
                bool contains(const vec &pt);	/* Testing whether a point lyting within the block */
                block2d* get_next_block();	/* Pointer to next block in linked list */
                void set_next_block(block2d* next_block);	/* Linking another block to present block */
		void print_state();	/* Diagnostic output of internal values */
		vec src_det_loc();	/* Location of source/detector in object for Meep processing */
        private:
                double x_cen, y_cen, len, wid;	/* Block propertis: centre coordinates & dimensions  */
                block2d *next;	/* Pointer to next block in sequece */
};

/* class bindingBlock:
 * Linkable analyte-binding blocks */
class bindingBlock{
        public:
                bindingBlock(const vec &cen, const vec &dim, const double t);	/* Constructor */
                bindingBlock* get_next_site();	/* Pointer to next binding site in linked list  */
                void set_next_site(bindingBlock *next_site);	/* Linking another site to present site */
                block2d* sklt();	/* Pointer to skeleton block2d object of site */
                block2d* anlt();	/* Pointer to analyte block2d object of site*/
		void print_state();	/* Diagnostic output of internal values */
        private:
		block2d *skeleton, *analyte;	/* Pointers to block2d objects for the skeleton and analyte components  */
                bindingBlock *next;	/* Pointer to the next binding site in sequence */
};

/* class skeletonSites:
 * Linked list of block2d objects */
class skeletonSites{
        public:
                skeletonSites();	/* Conostructor */
                void add_block(const vec &cen, const vec &dim);	/* Adding a new block to the present list */
                bool sk_pt(const vec &pt);	/* Testing whether a point contained in any of the blocks in list  */
		void print_state();	/* Diagnostic output of internal values */
                void delete_blocks();	/* De-alocation of memory for blocks in list -- without destroying the list itself  */
        private:
                block2d *start;	/* Pointer to the first block in the linked list */
		/* This may be appended to contain other types of linkable geometric objects, e.g. circles etc.  */
};

/* class bindSites:
 * Linked list of bindingBlock objects */
class bindSites{
        public:
                bindSites();	/* Constructor  */
                void add_site(const vec &cen, const  vec &dim, const double t);	/* Adding a new binding site to the present list */
                bool sk_pt(const vec &pt);	/* Testing whether a point contained in any of the skeleton blocks in list  */
                bool an_pt(const vec &pt);	/* Testing whether a point contained in any of the analyte coatings in list  */
		void print_state();	/* Diagnostic output of internal values */
                void delete_sites();	/* De-alocation of memory for binding sites in list -- without destroying the list itself  */
		bindingBlock* site(int site_index);	/* Pointer to binding site of specified index -- for array style access, if needed */
        private:
                bindingBlock *start;	/* Pointer to the first binding site in the linked list */
                int site_count;	/* Count of the number of binding sites in list */

};

/*-----------------------------*/
/* END: meep_geom_support.hpp  */
/*-----------------------------*/
