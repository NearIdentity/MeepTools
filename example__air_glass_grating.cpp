#include "meep_geom_support.hpp"
#include "meep_control_support.hpp"
#include "meep_detector_tools.hpp"
#include <cmath>
#include <string>
#include <stdio.h>
#include <string.h>

const double const_pi=4.0*atan(1.0), n_air=1.0, n_glass=1.5;
controlVariables ctrl=controlVariables();

double h_PML, h_sep, d, freq_min, freq_max, res, duration_factor, theta_degrees, angle_res_degrees;
int num_freqs, alpha;
char *file_name_prefix;

double size_x, size_y, pw_freq_width, freq_centre;
char *eps_file_name, *flux_file_name;

void create_control_variables(controlVariables &cv){
	int flags_fftw[]={-1,+1};
	cv.add_int("num_freqs",41,1,cv.plus_infinity_int(),"Number of frequency samples in spectrum");
	cv.add_int("alpha",1,1,cv.plus_infinity_int(),"Number of periods of the geometry in the x-direction");
	cv.add_double("d",1.0,0.0,cv.plus_infinity_double(),"Depth of diffraction grating layer");
	cv.add_double("h_sep",1.0,0.0,cv.plus_infinity_double(),"Source-detector separation");
	cv.add_double("h_PML",2.0,0.0,cv.plus_infinity_double(),"PML thickness");
	cv.add_double("res",80.0,0.0,cv.plus_infinity_double(),"Resolution -- number of mesh points per unit of periodicity");
	cv.add_double("freq_min",2.5,0.0,cv.plus_infinity_double(),"Minimum frequency for spectral analysis");
	cv.add_double("freq_max",3.5,0.0,cv.plus_infinity_double(),"Maximum frequency for spectral analysis");
	cv.add_double("theta_degrees",45.0,0.0,90.0,"Angle of incidence -- in degrees");
	cv.add_double("angle_res_degrees",1.0,0.0,90.0,"Angular resolution -- in degrees");
	cv.add_double("duration_factor",5.0,0.01,cv.plus_infinity_double(),"Factor to multiply default duration time (default duration time defined as \"num_freqs/(freq_max-freq_min)/2\")");	
	cv.add_special("file_name_prefix","air_glass_grating","File-name prefix for epsilon-profile, flux-data and angle-resolved spectrum files");
}

void set_variables(controlVariables &cv){
	alpha=cv.value_int("alpha");
	num_freqs=cv.value_int("num_freqs");
	d=cv.value_double("d");
	h_PML=cv.value_double("h_PML");
	h_sep=cv.value_double("h_sep");
	freq_min=cv.value_double("freq_min");
	freq_max=cv.value_double("freq_max");
	res=cv.value_double("res");
	duration_factor=cv.value_double("duration_factor");
	theta_degrees=cv.value_double("theta_degrees");
	angle_res_degrees=cv.value_double("angle_res_degrees");
	file_name_prefix=cv.value_special("file_name_prefix");

	size_x=(double) alpha;
	size_y=2*h_PML+d+5*h_sep;
	freq_centre=0.5*(max2(freq_min,freq_max)+min2(freq_min,freq_max));
	pw_freq_width=max2(freq_min,freq_max)-min2(freq_min,freq_max);

	int file_name_prefix_len=string(cv.value_special("file_name_prefix")).length();
	eps_file_name=new char[file_name_prefix_len+6];
	strcpy(eps_file_name,"eps__");
	strcat(eps_file_name,cv.value_special("file_name_prefix"));
	flux_file_name=new char[file_name_prefix_len+7];
	strcpy(flux_file_name,"flux__");
	strcat(flux_file_name,cv.value_special("file_name_prefix"));
}

double air(const vec &p) {
	return n_air*n_air;
}

double air_glass_grating(const vec &p){
	if( (p.y()>=h_PML+3*h_sep) && (p.y()<=h_PML+3*h_sep+d) && (p.x()<=0.5*alpha) )
		return n_glass*n_glass;
	return n_air*n_air;
}

/* Ampltiude Function for Linear Dipole Current Source */
complex<double> src_spatial_modulator(const vec &p) {
	vec pos_src_line(0,h_PML+h_sep), k_src(n_air*freq_centre*sin(theta_degrees*const_pi/180.0),n_air*freq_centre*cos(theta_degrees*const_pi/180.0));
	complex<double> const_i(0,1);
	return exp((k_src.x()*(p.x()-pos_src_line.x())+k_src.y()*(p.y()-pos_src_line.y()))*2*const_pi*const_i);
}

void simulate_RT(component c, double a){
	/* Memory Allocation, Structures and Fields */
	grid_volume v = vol2d(size_x,size_y,a); /* Grid volume for computations */
	structure s0(v,air,pml(h_PML,Y));     /* Reference case: no scatterers; PML termination in the y-direction */
	structure s(v,air_glass_grating,pml(h_PML,Y));	/* Structure to be simulated; PML termination in the y-direction */
	fields f0(&s0); /* Fields for reference case */
	fields f(&s);   /* Fields for simulation structure */
	h5file *eps_file_ptr=f.open_h5file(eps_file_name);
	f.output_hdf5(Dielectric, v.surroundings(), eps_file_ptr, true);    /* Outputting dielectric function as .h5 file; <fields>.output_hdf5(<field_type>,<?>) */

	/* Flux Lines for Transmissions and Reflection Detectors */
	volume flux_line_trans(vec(0,h_PML+4*h_sep+d),vec(size_x,h_PML+4*h_sep+d));
	volume flux_line_refl(vec(0,h_PML+2*h_sep),vec(size_x,h_PML+2*h_sep));

	/* Appropriate Bloch Boundary Conditions */
	double k_x=n_air*freq_centre*sin(theta_degrees*const_pi/180.0);
	f0.use_bloch(vec(k_x,0.0));
	f.use_bloch(vec(k_x,0.0));

	/* Light Sources */
	gaussian_src_time src(freq_centre, 0.5/pw_freq_width, 0, 5/pw_freq_width);      /* Time-domain definition of source */
	volume src_line(vec(0,h_PML+h_sep),vec(size_x,h_PML+h_sep));

	f0.add_volume_source(c,src,src_line,src_spatial_modulator,1.0);
	f.add_volume_source(c,src,src_line,src_spatial_modulator,1.0);
	master_printf("# Line source(s) added ...\n");

	/* Fluxes for Transmission, Reflection */
	dft_flux f_t0 = f0.add_dft_flux_plane(flux_line_trans,min2(freq_min,freq_max),max2(freq_min,freq_max),num_freqs);
	dft_flux f_t = f.add_dft_flux_plane(flux_line_trans,min2(freq_min,freq_max),max2(freq_min,freq_max),num_freqs);
	dft_flux f_r0 = f0.add_dft_flux_plane(flux_line_refl,min2(freq_min,freq_max),max2(freq_min,freq_max),num_freqs);
	dft_flux f_r = f.add_dft_flux_plane(flux_line_refl,min2(freq_min,freq_max),max2(freq_min,freq_max),num_freqs);

	angleResolvedDetectors2D *ard = new angleResolvedDetectors2D(h_PML+4*h_sep, h_PML+2*h_sep, size_x, a, angle_res_degrees, freq_min, freq_max, num_freqs, theta_degrees, n_air, n_air, degree);

	master_printf("# Simulating reference structure ...\n");
	double t_final_src_0=f0.last_source_time(), t_final_sim_0=t_final_src_0+duration_factor*num_freqs/pw_freq_width/2;
	master_printf("\tparameter__user_inaccessible:\tt_final_src_0 = %f\n",t_final_src_0);
	master_printf("\tparameter__user_inaccessible:\tt_final_sim_0 = %f\n",t_final_sim_0);
	while(f0.time() < t_final_sim_0){ /* Time-stepping -- reference structure */
		f0.step(); 
		double t=f0.time();
		ard->update(t,f0,reference);
	}
	f_r0.save_hdf5(f0, flux_file_name, "reflection");
	ard->finalize_update(reference);

	master_printf("# Simulating test structure ...\n");
	double t_final_src=f.last_source_time(), t_final_sim=t_final_src+duration_factor*num_freqs/pw_freq_width/2;
	master_printf("\tparameter__user_inaccessible:\tt_final_src = %f\n",t_final_src);
	master_printf("\tparameter__user_inaccessible:\tt_final_sim = %f\n",t_final_sim);
	f_r.load_hdf5(f, flux_file_name, "reflection");
	f_r.scale_dfts(-1.0);
	while(f.time() < t_final_sim){  /* Time-stepping -- simulated structure */
		f.step();
		double t=f.time();
		ard->update(t,f,simulation);
	}
	f.output_hdf5(c, v.surroundings());     /* Outputting electric field as .h5 file; <fields>.output_hdf5(<field_type>,<?>) */
	ard->finalize_update(simulation);

	double *flux_t = f_t.flux();    /* Calculating flux -- integrating? */
	double *flux_t0 = f_t0.flux();  /* Calculating flux -- integrating? */
	double *flux_r = f_r.flux();    /* Calculating flux -- integrating? */
	double *flux_r0 = f_r0.flux();  /* Calculating flux -- integrating? */

	double *T;      /* Array to store transmission coefficients (frequency-dependent) */
	double *R;      /* Array to store reflection coefficients (frequency-dependent) */
	T = new double[num_freqs];
	R = new double[num_freqs];
	for (int i=0; i<num_freqs; ++i){	/* Calculating transmission, reflection coefficients */
		T[i] = flux_t[i] / flux_t0[i];
		R[i] = -flux_r[i] / flux_r0[i];
	}
	double dfreq = pw_freq_width / (num_freqs-1);

	master_printf("transmission:, omega, T\n");
	master_printf("reflection:, omega, R\n");
	master_printf("addition_check:, omega, R+T\n");
	for (int l=0; l<num_freqs; ++l){	/* Printing transmission coefficient values */
		master_printf("transmission:, %f, %f\n",freq_min+l*dfreq,T[l]);
		master_printf("reflection:, %f, %f\n",freq_min+l*dfreq,R[l]);
		master_printf("addition_check:, %f, %f\n",freq_min+l*dfreq,T[l]+R[l]);
	}

	ard->print_angle_unresolved_T();
	ard->print_angle_unresolved_R();
	ard->print_angle_resolved_T(file_name_prefix);
	ard->print_angle_resolved_R(file_name_prefix);

	delete [] eps_file_name;
	delete [] flux_file_name;


	delete [] flux_t;       /* "Garbage collection" at end of code execution */
	delete [] flux_t0;      /* "Garbage collection" at end of code execution */
	delete [] flux_r;       /* "Garbage collection" at end of code execution */
	delete [] flux_r0;      /* "Garbage collection" at end of code execution */

	delete [] T;    /* "Garbage collection" at end of code execution */
	delete [] R;    /* "Garbage collection" at end of code execution */

	delete ard;
}

int main(int argc, char **argv){
	initialize mpi(argc,argv);
	create_control_variables(ctrl);
	if(ctrl.parse_runtime_params(argc, argv)==success){
		set_variables(ctrl);
		simulate_RT(Ez,res);
	}
	return 0;
}
