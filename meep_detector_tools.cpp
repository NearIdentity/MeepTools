#include "meep_detector_tools.hpp"

/*-------------------------*/
/* meep_detector_tools.cpp */
/*-------------------------*/

/* class angleResolvedDetectors2D */

/* Constructor for angleResolvedDetectors2D Class */
angleResolvedDetectors2D::angleResolvedDetectors2D(const double yT, const double yR, const int alph, const double resl, const double d_theta, const double min_freq, const double max_freq, const int freq_num, double angle0, double nT, double nR, angleUnit unit){

	/* Constant(s) and Parallelization Control Variables */
	this->Pi=4.0*atan(1.0);	/* Constant value for usage in trigonometric and Fourier calculations */
	this->tolerance=1.0e-10;
	this->process_rank=my_rank();	/* Meep-generated MPI process rank */
	this->total_processes=count_processors();	/* Meep-generated number of MPI processes */
	this->process_rank_reversed=this->total_processes-1-this->process_rank;	/* Reversed process rank for master-lightened load distribution */
	this->total_num_freqs=freq_num;	/* Number of frequency samples for entire calculation */
	this->allocated_num_freqs=(((this->total_processes)>1) && ((this->process_rank_reversed)<(freq_num % this->total_processes))) ? ((freq_num/this->total_processes)+1):(freq_num/this->total_processes);	/* Number of frequency samples to be manipulated by local process */
	if(freq_num<(this->total_processes))
		master_printf("# Warning [angleResolvedDetectors2D::angleResolvedDetectors2D(...)]: Total number of frequencies smaller than number of processes available -- %d/%d processes unused\n", (this->total_processes - freq_num), this->total_processes);

	/* Upload of Statically Allocated Variables from Constructor Function Call */
	double angle_factor=(unit==degree) ? atan(1.0)/45.0:1.0;
	this->y_T=yT;	/* y-level of transmission detector line */
	this->y_R=yR;	/* y-level of reflection detector line */
	this->alpha=alph;	/* Number of photonic crystal unit cells in the x-direction, i.e. supercell x-period */
	this->res=resl;	/* Spatial resolution in terms of mesh points per unit cell-length */
	this->delta_theta=(d_theta*angle_factor<=1.0/(alph*min2(nT,nR)*min2(min_freq,max_freq))) ? d_theta*angle_factor:1.0/(alph*min2(nT,nR)*min2(min_freq,max_freq));	/* Minimum resolvable angle */
	this->theta0=angle0*angle_factor;	/* Source angle, i.e. angle of incidence */
	this->freq_min=min2(min_freq,max_freq);	/* Minimum frequency */
	this->freq_max=max2(min_freq,max_freq);	/* Maximum frequency */
	this->num_x_reduced=int(alph*resl);	/* Number of data-points in sampling window along x-direction */
	this->delta_freq=(max_freq-min_freq)/(freq_num-1);	/* Minimum resolvable frequency */
	this->n_T=nT;	/* Refractive index of transmission detector region */
	this->n_R=nR;	/* Refractive index of reflection detector region */
	this->done_reference=false;	/* Completion status of referencing stage of calculations */
	this->done_simulation=false;	/* Completion status of simulation stage of calculations */
	this->delta_t=1.0/(4.0*max2(max_freq,min_freq));	/* Time-step for updates: with double the maximum frequency as Nyquist frequency */
	this->t_last_update=0.0;	/* Record of last update time of temporal Fourier Transforms */
	this->fields_dt=0.0;	/* Intrinsic time-step size of Meep simulation: assuming (this->fields_dt) < (this->delta_t) */
	this->monitor_file0=fopen("reference_convergence_monitor.csv","w");	/* Field convergence monitor output file -- reference stage */
	fprintf(this->monitor_file0, "time, EnergyDensity_transmission, EnergyDensity_reflection\n");
	this->monitor_file1=fopen("simulation_convergence_monitor.csv","w");	/* Field convergence monitor output file -- simulation stage */
	fprintf(this->monitor_file1, "time, EnergyDensity_transmission, EnergyDensity_reflection\n");

	/* Dynamic Allocation and Initialization (to Zero) of Flux Variables */
	if((this->allocated_num_freqs)>0){
		this->ExT0=new complex<double>*[this->allocated_num_freqs];
		this->EzT0=new complex<double>*[this->allocated_num_freqs];
		this->HxT0=new complex<double>*[this->allocated_num_freqs];
		this->HzT0=new complex<double>*[this->allocated_num_freqs];
		this->ExT1=new complex<double>*[this->allocated_num_freqs];
		this->EzT1=new complex<double>*[this->allocated_num_freqs];
		this->HxT1=new complex<double>*[this->allocated_num_freqs];
		this->HzT1=new complex<double>*[this->allocated_num_freqs];
		this->ExR0=new complex<double>*[this->allocated_num_freqs];
		this->EzR0=new complex<double>*[this->allocated_num_freqs];
		this->HxR0=new complex<double>*[this->allocated_num_freqs];
		this->HzR0=new complex<double>*[this->allocated_num_freqs];
		this->ExR1=new complex<double>*[this->allocated_num_freqs];
		this->EzR1=new complex<double>*[this->allocated_num_freqs];
		this->HxR1=new complex<double>*[this->allocated_num_freqs];
		this->HzR1=new complex<double>*[this->allocated_num_freqs];
	}
	for(int j0=0; j0<(this->allocated_num_freqs); j0++){
		this->ExT0[j0]=new complex<double>[this->num_x_reduced];
		this->EzT0[j0]=new complex<double>[this->num_x_reduced];
		this->HxT0[j0]=new complex<double>[this->num_x_reduced];
		this->HzT0[j0]=new complex<double>[this->num_x_reduced];
		this->ExT1[j0]=new complex<double>[this->num_x_reduced];
		this->EzT1[j0]=new complex<double>[this->num_x_reduced];
		this->HxT1[j0]=new complex<double>[this->num_x_reduced];
		this->HzT1[j0]=new complex<double>[this->num_x_reduced];
		this->ExR0[j0]=new complex<double>[this->num_x_reduced];
		this->EzR0[j0]=new complex<double>[this->num_x_reduced];
		this->HxR0[j0]=new complex<double>[this->num_x_reduced];
		this->HzR0[j0]=new complex<double>[this->num_x_reduced];
		this->ExR1[j0]=new complex<double>[this->num_x_reduced];
		this->EzR1[j0]=new complex<double>[this->num_x_reduced];
		this->HxR1[j0]=new complex<double>[this->num_x_reduced];
		this->HzR1[j0]=new complex<double>[this->num_x_reduced];
		for(int j1=0; j1<(this->num_x_reduced); j1++){
			this->ExT0[j0][j1]=0;
			this->EzT0[j0][j1]=0;
			this->HxT0[j0][j1]=0;
			this->HzT0[j0][j1]=0;
			this->ExT1[j0][j1]=0;
			this->EzT1[j0][j1]=0;
			this->HxT1[j0][j1]=0;
			this->HzT1[j0][j1]=0;
			this->ExR0[j0][j1]=0;
			this->EzR0[j0][j1]=0;
			this->HxR0[j0][j1]=0;
			this->HzR0[j0][j1]=0;
			this->ExR1[j0][j1]=0;
			this->EzR1[j0][j1]=0;
			this->HxR1[j0][j1]=0;
			this->HzR1[j0][j1]=0;
		}
	}
}

/* Setting Up Tolerance Limits for Numerical Equivalene */
void angleResolvedDetectors2D::set_tolerance(const double tol){
	this->tolerance=tol;
}

/* Probing Field Values at Various Points Along the x-Direction */
complex<double> angleResolvedDetectors2D::probe_field(fields &fld, component cmp, const int jx, const double y){
	if(jx==0)	/* Special case of x==0.0 */
		return 0.5*fld.get_field(cmp,vec(0.5*this->res,y))+0.5*fld.get_field(cmp,vec(this->alpha-0.5*this->res,y));
	return fld.get_field(cmp,vec(jx*this->res,y));
}

/* Time-Step Update for Referencing Stage -- Accumulating Temporal Fourier Transform Data [Optimization Priority Function!] */
void angleResolvedDetectors2D::update_reference(const double t, fields &fld){
	if(this->t_last_update==0.0)	/* Determining intrinsic FDTD time-step at start of time-stepping */
		this->fields_dt=fld.dt;
	/* Update Set to Occur Only at Time-Steps Necessary for Ensuring Sufficient Temporal Nyquist Frequency */
	if(((this->delta_t-0.5*this->fields_dt)<(t-this->t_last_update)) && ((t-this->t_last_update)<((this->delta_t+0.5*this->fields_dt)))){
		this->t_last_update=t;
	}
	else
		return;
	/* Fourier Transforming for Each x-Point and Allocated Frequency -- Outer Looping Over x-Points Helping Reduce Function Calls to this->probe_field(...) */
	for(int j_x=0; j_x<(this->num_x_reduced); j_x++){
		complex<double> exT0=this->probe_field(fld,Ex,j_x,this->y_T), ezT0=this->probe_field(fld,Ez,j_x,this->y_T), hxT0=this->probe_field(fld,Hx,j_x,this->y_T), hzT0=this->probe_field(fld,Hz,j_x,this->y_T), exR0=this->probe_field(fld,Ex,j_x,this->y_R), ezR0=this->probe_field(fld,Ez,j_x,this->y_R), hxR0=this->probe_field(fld,Hx,j_x,this->y_R), hzR0=this->probe_field(fld,Hz,j_x,this->y_R);	/* Storage variables for reduction of number of calls to this->probe_field(...) */
		for(int j_freq=0; j_freq<(this->allocated_num_freqs); j_freq++){
			int j_freq_absolute=( ((this->total_processes)>1) && ((this->process_rank_reversed)<(this->total_num_freqs % this->total_processes)) ) ? (this->process_rank_reversed * ((this->total_num_freqs / this->total_processes) + 1) + j_freq) : ( (this->total_num_freqs % this->total_processes)*((this->total_num_freqs / this->total_processes) + 1) + (this->process_rank_reversed - (this->total_num_freqs % this->total_processes))*(this->total_num_freqs / this->total_processes) + j_freq);
			double omega=this->freq_min + j_freq_absolute*this->delta_freq, arg_temporal=+2*this->Pi*omega*t;
			complex<double> Fourier_phase_temporal(cos(arg_temporal), sin(arg_temporal));
			this->ExT0[j_freq][j_x]+=exT0*Fourier_phase_temporal;
			this->EzT0[j_freq][j_x]+=ezT0*Fourier_phase_temporal;
			this->HxT0[j_freq][j_x]+=hxT0*Fourier_phase_temporal;
			this->HzT0[j_freq][j_x]+=hzT0*Fourier_phase_temporal;
			this->ExR0[j_freq][j_x]+=exR0*Fourier_phase_temporal;
			this->EzR0[j_freq][j_x]+=ezR0*Fourier_phase_temporal;
			this->HxR0[j_freq][j_x]+=hxR0*Fourier_phase_temporal;
			this->HzR0[j_freq][j_x]+=hzR0*Fourier_phase_temporal;
		}
	}
	fprintf(this->monitor_file0, "%f, %e, %e\n", t, fld.get_field(EnergyDensity, vec(this->alpha/2,this->y_T)), fld.get_field(EnergyDensity, vec(this->alpha/2,this->y_R)));	/* Output to convergence monitor file */
}

/* Time-Step Update for Simulation Stage -- Accumulating Temporal Fourier Transform Data [Optimization Priority Function!] */
void angleResolvedDetectors2D::update_simulation(const double t, fields &fld){
	if(this->t_last_update==0.0)	/* Determining intrinsic FDTD time-step at start of time-stepping */
		this->fields_dt=fld.dt;
	/* Update Set to Occur Only at Time-Steps Necessary for Ensuring Sufficient Temporal Nyquist Frequency */
	if(((this->delta_t-0.5*this->fields_dt)<(t-this->t_last_update)) && ((t-this->t_last_update)<((this->delta_t+0.5*this->fields_dt)))){
		this->t_last_update=t;
	}
	else
		return;
	/* Fourier Transforming for Each x-Point and Allocated Frequency -- Outer Looping Over x-Points Helping Reduce Function Calls to this->probe_field(...) */
	for(int j_x=0; j_x<(this->num_x_reduced); j_x++){
		complex<double> exT1=this->probe_field(fld,Ex,j_x,this->y_T), ezT1=this->probe_field(fld,Ez,j_x,this->y_T), hxT1=this->probe_field(fld,Hx,j_x,this->y_T), hzT1=this->probe_field(fld,Hz,j_x,this->y_T), exR1=this->probe_field(fld,Ex,j_x,this->y_R), ezR1=this->probe_field(fld,Ez,j_x,this->y_R), hxR1=this->probe_field(fld,Hx,j_x,this->y_R), hzR1=this->probe_field(fld,Hz,j_x,this->y_R);	/* Storage variables for reduction of number of calls to this->probe_field(...) */
		for(int j_freq=0; j_freq<(this->allocated_num_freqs); j_freq++){
			int j_freq_absolute=( ((this->total_processes)>1) && ((this->process_rank_reversed)<(this->total_num_freqs % this->total_processes)) ) ? (this->process_rank_reversed * ((this->total_num_freqs / this->total_processes) + 1) + j_freq) : ( (this->total_num_freqs % this->total_processes)*((this->total_num_freqs / this->total_processes) + 1) + (this->process_rank_reversed - (this->total_num_freqs % this->total_processes))*(this->total_num_freqs / this->total_processes) + j_freq);
			double omega=this->freq_min + j_freq_absolute*this->delta_freq, arg_temporal=+2*this->Pi*omega*t;
			complex<double> Fourier_phase_temporal(cos(arg_temporal), sin(arg_temporal));
			this->ExT1[j_freq][j_x]+=exT1*Fourier_phase_temporal;
			this->EzT1[j_freq][j_x]+=ezT1*Fourier_phase_temporal;
			this->HxT1[j_freq][j_x]+=hxT1*Fourier_phase_temporal;
			this->HzT1[j_freq][j_x]+=hzT1*Fourier_phase_temporal;
			this->ExR1[j_freq][j_x]+=exR1*Fourier_phase_temporal;
			this->EzR1[j_freq][j_x]+=ezR1*Fourier_phase_temporal;
			this->HxR1[j_freq][j_x]+=hxR1*Fourier_phase_temporal;
			this->HzR1[j_freq][j_x]+=hzR1*Fourier_phase_temporal;
		}
	}
	fprintf(this->monitor_file1, "%f, %e, %e\n", t, fld.get_field(EnergyDensity, vec(this->alpha/2,this->y_T)), fld.get_field(EnergyDensity, vec(this->alpha/2,this->y_R)));	/* Output to convergence monitor file */
}

/* Generalizd Update Function to be Run at Every FDTD Time-Step [Optimization Priority Function!] */
void angleResolvedDetectors2D::update(const double t, fields &fld, updateStage stage){
	if(stage==reference)
		this->update_reference(t,fld);
	else if(stage==simulation)
		this->update_simulation(t,fld);
	all_wait();	
}

/* Generalized Function to Prepare Class for Completion of Update of Reference/Simulation Stages of Calculations */
void angleResolvedDetectors2D::finalize_update(updateStage stage){
	/* Tasks -- 1. Setting Necessary bool Variables to Required Values, 2. Closing Convergence Monitor File, 3. Resetting Update Time, If Needed */
	if(stage==reference){
		this->done_reference=true;
		fclose(this->monitor_file0);
		this->t_last_update=0.0;
	}
	else if((stage==simulation) && this->done_reference){
		this->done_simulation=true;
		fclose(this->monitor_file1);
	}
	else	/* Incorrect finalization for one of the stages detected ... Ideally, code ot to reach here */
		master_printf("# Error [angleResolvedDetectors2D::finalize_update(...)]: Completion of referencing time-steps not explicitly specified\n");
	all_wait();
}

/* Output of Angle-Unresolved Transmission [Multi-Processing Leading to Unsorted Output] */
void angleResolvedDetectors2D::print_angle_unresolved_T(){
	if(!this->done_reference || !this->done_simulation){	/* May not proceed unless both referencing and simulation stages finalized */
		master_printf("# Error [angleResolvedDetectors2D::print_angle_unresolved_T()]: Completion of referencing and/or simulation time-steps not explicitly specified\n");
		return;
	}
	master_printf("transmission_angle_unresolved:,omega,T\n");
	all_wait();
	for(int j_freq=0; j_freq<(this->allocated_num_freqs); j_freq++){
		int j_freq_absolute=( ((this->total_processes)>1) && ((this->process_rank_reversed)<(this->total_num_freqs % this->total_processes)) ) ? (this->process_rank_reversed * ((this->total_num_freqs / this->total_processes) + 1) + j_freq) : ( (this->total_num_freqs % this->total_processes)*((this->total_num_freqs / this->total_processes) + 1) + (this->process_rank_reversed - (this->total_num_freqs % this->total_processes))*(this->total_num_freqs / this->total_processes) + j_freq);
		double T0_flux=0, T1_flux=0, omega=this->freq_min + j_freq_absolute * this->delta_freq;
		for(int j_x=0; j_x<(this->num_x_reduced); j_x++){	/* Flux calcualtion through detector line */
			T0_flux+=real(this->EzT0[j_freq][j_x]*conj(this->HxT0[j_freq][j_x])-this->ExT0[j_freq][j_x]*conj(this->HzT0[j_freq][j_x]));
			T1_flux+=real(this->EzT1[j_freq][j_x]*conj(this->HxT1[j_freq][j_x])-this->ExT1[j_freq][j_x]*conj(this->HzT1[j_freq][j_x]));
		}
		printf("transmission_angle_unresolved:,%f,%e\n",omega,T1_flux/T0_flux);
	}
	all_wait();
}

/* Output of Angle-Unresolved Reflection [Multi-Processing Leading to Unsorted Output] */
void angleResolvedDetectors2D::print_angle_unresolved_R(){
	if(!this->done_reference || !this->done_simulation){	/* May not proceed unless both referencing and simulation stages finalized */
		master_printf("# Error [angleResolvedDetectors2D::print_angle_unresolved_R()]: Completion of referencing and/or simulation time-steps not explicitly specified\n");
		return;
	}
	master_printf("reflection_angle_unresolved:,omega,R\n");
	all_wait();
	for(int j_freq=0; j_freq<(this->allocated_num_freqs); j_freq++){
		int j_freq_absolute=( ((this->total_processes)>1) && ((this->process_rank_reversed)<(this->total_num_freqs % this->total_processes)) ) ? (this->process_rank_reversed * ((this->total_num_freqs / this->total_processes) + 1) + j_freq) : ( (this->total_num_freqs % this->total_processes)*((this->total_num_freqs / this->total_processes) + 1) + (this->process_rank_reversed - (this->total_num_freqs % this->total_processes))*(this->total_num_freqs / this->total_processes) + j_freq);
		double R0_flux=0, R1_flux=0, omega=this->freq_min + j_freq_absolute * this->delta_freq;
		for(int j_x=0; j_x<(this->num_x_reduced); j_x++){	/* Flux calcualtion through detector line */
			R0_flux+=real(this->EzR0[j_freq][j_x]*conj(this->HxR0[j_freq][j_x])-this->ExR0[j_freq][j_x]*conj(this->HzR0[j_freq][j_x]));
			R1_flux+=real((this->EzR1[j_freq][j_x]-this->EzR0[j_freq][j_x])*conj(this->HxR1[j_freq][j_x]-this->HxR0[j_freq][j_x])-(this->ExR1[j_freq][j_x]-this->ExR0[j_freq][j_x])*conj(this->HzR1[j_freq][j_x]-this->HzR0[j_freq][j_x]));
		}
		printf("reflection_angle_unresolved:,%f,%e\n",omega,-R1_flux/R0_flux);
	}
	all_wait();
}

/* Number of k-Space Values Necessary for Calculations for a Given Angle of Detection to Ensure Constant Angular Spacing for Integration */
double angleResolvedDetectors2D::num_k(const double freq, const double angle, const double n){
	return round(0.5*(1.0/(this->alpha*n*freq*cos(angle)*this->delta_theta)-1.0));
}

/* Bloch Sum Factor Necessary for Extending x-Sampling Window Beyond "Alpha" Unit Cells (by Using Bloch's Theorem) */
double angleResolvedDetectors2D::factor_Bloch_sum(const double freq, const double angle, const double n){
	double num_k_vals=this->num_k(freq,angle,n), arg_main=2*this->Pi*this->alpha*(n*freq*sin(angle)-this->n_R*(0.5*(this->freq_max+this->freq_min))*sin(this->theta0));
	if(abs(cos(arg_main)-1.0)<=(this->tolerance))	/* Special case of zero-valued argument */
		return (2*num_k_vals+1);	/* Technically, this being "return (2*num_k_vals+1)*(2*num_k_vals+1);", but later division by (2*num_k_vals+1) avoided by returning already reduced value */
	else{ 
		double sqrt_factor=(cos(num_k_vals*arg_main)-cos((num_k_vals+1)*arg_main))/(1.0-cos(arg_main));
		return sqrt_factor*sqrt_factor/(2*num_k_vals+1);	/* Division by (2*num_k_vals+1) incorporated here, as opposed to in flux summation */
	}
}

/* Angle-Summed Transmission Detector Reference Flux Calculations */
double angleResolvedDetectors2D::angle_summed_T0(const int jfrq, const double frq){
	double T0=0;
	for(double theta=-0.5*this->Pi+this->delta_theta; theta<+0.5*this->Pi-0.5*this->delta_theta; theta+=this->delta_theta){
		complex<double> Ex0=0, Ez0=0, Hx0=0, Hz0=0;
		for(int j_x=0; j_x<(this->num_x_reduced); j_x++){
			double arg_spatial=-2*this->Pi*(this->n_R*frq*sin(theta))*(j_x/this->res);
			complex<double> Fourier_phase_spatial(cos(arg_spatial), sin(arg_spatial));
			Ex0+=this->ExT0[jfrq][j_x]*Fourier_phase_spatial;
			Ez0+=this->EzT0[jfrq][j_x]*Fourier_phase_spatial;
			Hx0+=this->HxT0[jfrq][j_x]*Fourier_phase_spatial;
			Hz0+=this->HzT0[jfrq][j_x]*Fourier_phase_spatial;
		}
		T0+=real(Ez0*conj(Hx0)-Ex0*conj(Hz0))*cos(theta)*this->factor_Bloch_sum(frq,theta,this->n_R);
	}
	return T0*this->n_R*this->n_R*this->num_k(frq,0.5*this->Pi-this->delta_theta,this->n_R)*this->delta_theta;	/* Multiplication by this->num_k(...) to ensure correct normalization of transmittance, despite variable number of unit cells considered at different angles; extra this->n_R used for "strength reduction" here instead of (this->n_T/this->n_R) for every inner loop of this->print_single_frequency_T(...) */
} 

/* Output of Angle-Resolved Transmission for a Single Frequency */
void angleResolvedDetectors2D::print_single_frequency_T(const int jfreq, const double freq, FILE *T_file){
	double T0=this->angle_summed_T0(jfreq,freq), T_sum=0, max_num_x=this->num_k(freq,0.5*this->Pi-this->delta_theta,this->n_T);
	for(double theta=-0.5*this->Pi+this->delta_theta; theta<+0.5*this->Pi-0.5*this->delta_theta; theta+=this->delta_theta){
		complex<double> Ex1=0, Ez1=0, Hx1=0, Hz1=0;
		for(int j_x=0; j_x<(this->num_x_reduced); j_x++){
			double arg_spatial=-2*this->Pi*(this->n_T*freq*sin(theta))*(j_x/this->res);
			complex<double> Fourier_phase_spatial(cos(arg_spatial), sin(arg_spatial));
			Ex1+=this->ExT1[jfreq][j_x]*Fourier_phase_spatial;
			Ez1+=this->EzT1[jfreq][j_x]*Fourier_phase_spatial;
			Hx1+=this->HxT1[jfreq][j_x]*Fourier_phase_spatial;
			Hz1+=this->HzT1[jfreq][j_x]*Fourier_phase_spatial;
		}
		double T1=real(Ez1*conj(Hx1)-Ex1*conj(Hz1))*this->n_T*this->n_T*cos(theta)*this->factor_Bloch_sum(freq,theta,this->n_T)*max_num_x;	/* "Strength reduction" in effect: extra factor of (this->n_T) to be balanced by extra (this->n_R) incorporated into variable 'T0'; substitute for (this->n_T/this->n_R) at every step of inner loop; multiplication by variable 'max_num_x' to ensure correct normalization for transmittance despite variable angle-dependent number of unit cells integrated over */
		fprintf(T_file,"transmission_angle_resolved:,%f,%f,%e\n",freq,+theta*180.0/this->Pi,T1/T0);
		T_sum+=T1/T0;
	}
	printf("transmission_angle_summed:,%f,%e\n", freq, T_sum*this->delta_theta);
}

/* Output of Angle-Resolved Transmission for All Frequencies */
void angleResolvedDetectors2D::print_angle_resolved_T(char *file_name_prefix){	/* May not proceed unless both referencing and simulation stages finalized */
	if(!this->done_reference || !this->done_simulation){
		master_printf("# Error [angleResolvedDetectors2D::print_angle_resolved_T()]: Completion of referencing and/or simulation time-steps not explicitly specified\n");
		return;
	}
	master_printf("transmission_angle_summed:,omega,T\n");
	all_wait();
	/* Tasks for Each Allocated Frequency -- 1. Output File Creation/Opening, 2. Output Writing, 3. Output File Closing & Finalization */
	for(int j_freq=0; j_freq<(this->allocated_num_freqs); j_freq++){
		int j_freq_absolute=( ((this->total_processes)>1) && ((this->process_rank_reversed)<(this->total_num_freqs % this->total_processes)) ) ? (this->process_rank_reversed * ((this->total_num_freqs / this->total_processes) + 1) + j_freq) : ( (this->total_num_freqs % this->total_processes)*((this->total_num_freqs / this->total_processes) + 1) + (this->process_rank_reversed - (this->total_num_freqs % this->total_processes))*(this->total_num_freqs / this->total_processes) + j_freq), file_name_length=string(file_name_prefix).length()+12;
		double omega=this->freq_min + j_freq_absolute*this->delta_freq;
		char *file_name=new char[file_name_length];
		file_name_length=sprintf(file_name,"%s_T%05d.csv",file_name_prefix,j_freq_absolute);
		FILE *current_T_file=fopen(file_name,"w");
		fprintf(current_T_file,"transmission_angle_resolved:,omega,theta(degree),T\n");
		this->print_single_frequency_T(j_freq,omega,current_T_file);
		fclose(current_T_file);
		delete [] file_name;
	}
	all_wait();
}

/* Angle-Summed Reflection Detector Reference Flux Calculations */
double angleResolvedDetectors2D::angle_summed_R0(const int jfrq, const double frq){
	double R0=0;
	for(double theta=+0.5*this->Pi-this->delta_theta; theta>-0.5*this->Pi+0.5*this->delta_theta; theta-=this->delta_theta){
		complex<double> Ex0=0, Ez0=0, Hx0=0, Hz0=0;
		for(int j_x=0; j_x<(this->num_x_reduced); j_x++){
			double arg_spatial=-2*this->Pi*(this->n_R*frq*sin(theta))*(j_x/this->res);
			complex<double> Fourier_phase_spatial(cos(arg_spatial), sin(arg_spatial));
			Ex0+=this->ExR0[jfrq][j_x]*Fourier_phase_spatial;
			Ez0+=this->EzR0[jfrq][j_x]*Fourier_phase_spatial;
			Hx0+=this->HxR0[jfrq][j_x]*Fourier_phase_spatial;
			Hz0+=this->HzR0[jfrq][j_x]*Fourier_phase_spatial;
		}
		R0+=real(Ez0*conj(Hx0)-Ex0*conj(Hz0))*cos(theta)*this->factor_Bloch_sum(frq,theta,this->n_R);
	}
	return R0*this->n_R*this->delta_theta;
}

/* Output of Angle-Resolved Reflection for a Single Frequency */
void angleResolvedDetectors2D::print_single_frequency_R(const int jfreq, const double freq, FILE *R_file){
	double R0=this->angle_summed_R0(jfreq,freq), R_sum=0;
	for(double theta=+0.5*this->Pi-this->delta_theta; theta>-0.5*this->Pi+0.5*this->delta_theta; theta-=this->delta_theta){
		complex<double> Ex0=0, Ez0=0, Hx0=0, Hz0=0, Ex1=0, Ez1=0, Hx1=0, Hz1=0;
		for(int j_x=0; j_x<(this->num_x_reduced); j_x++){
			double arg_spatial=-2*this->Pi*(this->n_R*freq*sin(theta))*(j_x/this->res);
			complex<double> Fourier_phase_spatial(cos(arg_spatial), sin(arg_spatial));
			Ex0+=this->ExR0[jfreq][j_x]*Fourier_phase_spatial;
			Ez0+=this->EzR0[jfreq][j_x]*Fourier_phase_spatial;
			Hx0+=this->HxR0[jfreq][j_x]*Fourier_phase_spatial;
			Hz0+=this->HzR0[jfreq][j_x]*Fourier_phase_spatial;
			Ex1+=this->ExR1[jfreq][j_x]*Fourier_phase_spatial;
			Ez1+=this->EzR1[jfreq][j_x]*Fourier_phase_spatial;
			Hx1+=this->HxR1[jfreq][j_x]*Fourier_phase_spatial;
			Hz1+=this->HzR1[jfreq][j_x]*Fourier_phase_spatial;
		}
		double R1=real((Ez1-Ez0)*conj(Hx1-Hx0)-(Ex1-Ex0)*conj(Hz1-Hz0))*this->n_R*cos(theta)*this->factor_Bloch_sum(freq,theta,this->n_R);
		fprintf(R_file,"reflection_angle_resolved:,%f,%f,%e\n",freq,-theta*180.0/this->Pi,-R1/R0);
		R_sum-=R1/R0;
	}
	printf("reflection_angle_summed:,%f,%e\n", freq, R_sum*this->delta_theta);
}

/* Output of Angle-Resolved Reflection for All Frequencies */
void angleResolvedDetectors2D::print_angle_resolved_R(char *file_name_prefix){
	if(!this->done_reference || !this->done_simulation){	/* May not proceed unless both referencing and simulation stages finalized */
		master_printf("# Error [angleResolvedDetectors2D::print_angle_resolved_R()]: Completion of referencing and/or simulation time-steps not explicitly specified\n");
		return;
	}
	master_printf("reflection_angle_summed:,omega,R\n");
	all_wait();
	/* Tasks for Each Allocated Frequency -- 1. Output File Creation/Opening, 2. Output Writing, 3. Output File Closing & Finalization */
	for(int j_freq=0; j_freq<(this->allocated_num_freqs); j_freq++){
		int j_freq_absolute=( ((this->total_processes)>1) && ((this->process_rank_reversed)<(this->total_num_freqs % this->total_processes)) ) ? (this->process_rank_reversed * ((this->total_num_freqs / this->total_processes) + 1) + j_freq) : ( (this->total_num_freqs % this->total_processes)*((this->total_num_freqs / this->total_processes) + 1) + (this->process_rank_reversed - (this->total_num_freqs % this->total_processes))*(this->total_num_freqs / this->total_processes) + j_freq), file_name_length=string(file_name_prefix).length()+12;
		double omega=this->freq_min + j_freq_absolute*this->delta_freq;
		char *file_name=new char[file_name_length];
		file_name_length=sprintf(file_name,"%s_R%05d.csv",file_name_prefix,j_freq_absolute);
		FILE *current_R_file=fopen(file_name,"w");
		fprintf(current_R_file,"reflection_angle_resolved:,omega,theta(degree),R\n");
		this->print_single_frequency_R(j_freq,omega,current_R_file);
		fclose(current_R_file);
		delete [] file_name;
	}
	all_wait();
}
	
/* Destructor for angleResolvedDetectors2D Class */
angleResolvedDetectors2D::~angleResolvedDetectors2D(){
	for(int j=0; j<(this->allocated_num_freqs); j++){
		delete [] this->ExT0[j];
		delete [] this->EzT0[j];
		delete [] this->HxT0[j];
		delete [] this->HzT0[j];
		delete [] this->ExT1[j];
		delete [] this->EzT1[j];
		delete [] this->HxT1[j];
		delete [] this->HzT1[j];
		delete [] this->ExR0[j];
		delete [] this->EzR0[j];
		delete [] this->HxR0[j];
		delete [] this->HzR0[j];
		delete [] this->ExR1[j];
		delete [] this->EzR1[j];
		delete [] this->HxR1[j];
		delete [] this->HzR1[j];
	}
	delete [] this->ExT0;
	delete [] this->EzT0;
	delete [] this->HxT0;
	delete [] this->HzT0;
	delete [] this->ExT1;
	delete [] this->EzT1;
	delete [] this->HxT1;
	delete [] this->HzT1;
	delete [] this->ExR0;
	delete [] this->EzR0;
	delete [] this->HxR0;
	delete [] this->HzR0;
	delete [] this->ExR1;
	delete [] this->EzR1;
	delete [] this->HxR1;
	delete [] this->HzR1;
}

/*-------------------------*/
/* meep_detector_tools.cpp */
/*-------------------------*/
