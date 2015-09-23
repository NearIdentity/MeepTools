/*---------------------------*/
/* START: meep_geom_supp.cpp */
/*---------------------------*/
#include "meep_geom_support.hpp" 
#define MEEP_GEOMETRY_SUPPORT_DEF
/* class block2d */

block2d::block2d(const vec &cen, const vec &dim){
        this->x_cen=cen.x();
        this->y_cen=cen.y();
        this->len=dim.x();
        this->wid=dim.y();
        this->next=NULL;
}

bool block2d::contains(const vec &pt){
        return ( (this->x_cen-0.5*this->len <= pt.x()) && (this->x_cen+0.5*this->len >= pt.x()) && (this->y_cen-0.5*this->wid <= pt.y()) && (this->y_cen+0.5*this->wid >= pt.y()) );
}

block2d* block2d::get_next_block(){
        return this->next;
}

void block2d::set_next_block(block2d* next_block){
        this->next=next_block;
}

void block2d::print_state(){
	master_printf("# x_cen = %f,\ty_cen = %f,\tlen = %f,\twid = %f", this->x_cen, this->y_cen, this->len, this->wid);
	if(this->next==NULL)
		master_printf("\tnext = NULL\n");
	else
		master_printf("\tnext = ...\n");
}

vec block2d::src_det_loc(){
	return vec(this->x_cen+0.1*this->len,this->y_cen+0.1*this->wid);
}


/* class bindingBlock */

bindingBlock::bindingBlock(const vec &cen, const vec &dim, const double t){
        const vec dim_an(dim.x()+2*t,dim.y()+2*t);
        this->skeleton=new block2d(cen, dim);
        if(t>0.0)
                this->analyte = new block2d(cen, dim_an);
        else
                this->analyte = NULL;
        this->next=NULL;
}

bindingBlock* bindingBlock::get_next_site(){
        return this->next;
}

void bindingBlock::set_next_site(bindingBlock *next_site){
        this->next=next_site;
}

block2d* bindingBlock::sklt(){
        return this->skeleton;
}

block2d* bindingBlock::anlt(){
        return this->analyte;
}

void bindingBlock::print_state(){
	master_printf("# Skeleton block:\n\t");
	(this->skeleton)->print_state();
	master_printf("# Analyte block:\n\t");
	if(this->analyte==NULL)
		master_printf("# None\n");
	else
		(this->analyte)->print_state();
}

/* class skeletonSites */

skeletonSites::skeletonSites(){
        start=NULL;     /* Linked list of skeleton blocks initialized only with a "placeholder" block */
}

void skeletonSites::add_block(const vec &cen, const vec &dim){
        block2d *new_block=new block2d(cen, dim);
        if(this->start==NULL)   /* Purging any placeholder blocks from the initialization process */
                start=new_block;
        else{   /* Case of at least one skeleton block already present */
                block2d* current_block=this->start;
                while(current_block->get_next_block()!=NULL)    /* Finding last skeleton block */
                        current_block=current_block->get_next_block();
                current_block->set_next_block(new_block);
        }
}

bool skeletonSites::sk_pt(const vec &pt){
        block2d *current_block=this->start;
        if(current_block==NULL) /* Case of no skeleton blocks in structure */
                return false;
        while(current_block!=NULL){   /* Traversing all present skeleton structures */
                if(current_block->contains(pt))
                        return true;
                current_block=current_block->get_next_block();
        }
        return false;
}

void skeletonSites::print_state(){
	block2d *current=this->start;
	master_printf("# State check of skeletonSites ...\n");
	while(current!=NULL){
		current->print_state();
		current=current->get_next_block();
	}
}

void skeletonSites::delete_blocks(){
        while(this->start!=NULL){
                block2d *first=this->start;
                this->start=(this->start)->get_next_block();
                delete first;
        }

}

/* class bindSites */

bindSites::bindSites(){
        this->start=NULL;
        this->site_count=0;
}

void bindSites::add_site(const vec &cen, const vec &dim, const double t){
        bindingBlock *new_site = new bindingBlock(cen, dim, t);
        this->site_count++;
	if(this->start==NULL)   /* Case of no binding sites present in ensemble */
                this->start=new_site;
        else{   /* Case of >0 binding sites present in ensemble */
                bindingBlock *last_site=this->start;
                while(last_site->get_next_site()!=NULL){
                        last_site=last_site->get_next_site();
                }
                last_site->set_next_site(new_site); /* Appending new binding site after last one in existing sequence */
        }
}

bool bindSites::sk_pt(const vec &pt){
        bindingBlock *current_site=this->start;
        while(current_site!=NULL){
                if((current_site->sklt())->contains(pt))
                        return true;
                current_site=current_site->get_next_site();
        }
        return false;
}

bool bindSites::an_pt(const vec &pt){
        bindingBlock *current_site=this->start;
        while(current_site!=NULL){
                if( (current_site->anlt()!=NULL) && ((current_site->anlt())->contains(pt)) && !((current_site->sklt())->contains(pt)) )
                        return true;
                current_site=current_site->get_next_site();
        }
        return false;
}

void bindSites::print_state(){
	bindingBlock *current=this->start;
	master_printf("# State check of bindSites ...\n");
	while(current!=NULL){
		current->print_state();
		current=current->get_next_site();
	}
}

void bindSites::delete_sites(){
        while(this->start!=NULL){       /*  */
                bindingBlock *first=this->start;        /* Isolation of first site in sequence */
                this->start=(this->start)->get_next_site();     /* Moving start of sequence to next-to-start site */
                delete first->sklt();   /* Deleting skeleton block of isolated first site */
                if(first->anlt()!=NULL) /* Deleting analyte block og isolated first site, if necessary */
                        delete first->anlt();
                delete first;   /* Deleting isolated first site */
        }
	this->site_count=0;
}

bindingBlock* bindSites::site(int site_index){
	if(site_index>=(this->site_count))
		return NULL;
	bindingBlock *current=this->start;
	for(int j=0; j<site_index; j++)
		current=current->get_next_site();
	return current;
}

/*--------------------------*/
/* END: meep_geom_supp.cpp  */
/*--------------------------*/
