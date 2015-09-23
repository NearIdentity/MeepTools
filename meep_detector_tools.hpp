/*-------------------------*/
/* meep_detector_tools.hpp */
/*-------------------------*/

#include <cmath>
#include "meep.hpp"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <limits>

using namespace meep;

#define max2(a,b) \
	({ __typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b; })
#define min2(a,b) \
	({ __typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a < _b ? _a : _b; })

enum updateStage {reference, simulation};
enum angleUnit {degree, radian};

class angleResolvedDetectors2D{
	public:
		angleResolvedDetectors2D(const double yT, const double yR, const int alph, const double resl, const double d_theta, const double min_freq, const double max_freq, const int freq_num, double angle0, double nT=1.0, const double nR=1.0, angleUnit unit=degree);
		void update(const double t, fields &fld, updateStage stage);
		void finalize_update(updateStage stage);
		void print_angle_unresolved_T();
		void print_angle_unresolved_R();
		void print_angle_resolved_T(char *file_name_prefix);
		void print_angle_resolved_R(char *file_name_prefix);
		void set_tolerance(const double tol);
		~angleResolvedDetectors2D();
	private:
		/* Variables (and Constants) */
		complex<double> **ExR0, **ExR1, **EzR0, **EzR1, **HxR0, **HxR1, **HzR0, **HzR1, **ExT0, **ExT1, **EzT0, **EzT1, **HxT0, **HxT1, **HzT0, **HzT1;
		double freq_min, freq_max, delta_freq, delta_t, fields_dt, delta_theta, theta0, t_last_update, res, y_T, y_R, n_0, n_T, n_R, Pi, tolerance;
		int alpha, total_num_freqs, allocated_num_freqs, num_x_reduced, sign, process_rank, process_rank_reversed, total_processes;
		bool done_reference, done_simulation;
		FILE *monitor_file0, *monitor_file1;
		/* Functions */
		complex<double> probe_field(fields &fld, component cmp, const int jx, const double y);
		void update_reference(const double t, fields &fld);
		void update_simulation(const double t, fields &fld);
		double num_k(const double freq, const double angle, const double n);
		double factor_Bloch_sum(const double freq, const double angle, const double n);
		double angle_summed_T0(const int jfrq, const double frq);
		void print_single_frequency_T(const int jfreq, const double freq, FILE *T_file);
		double angle_summed_R0(const int jfrq, const double frq);
		void print_single_frequency_R(const int jfreq, const double freq, FILE *R_file);
};
	
/*-------------------------*/
/* meep_detector_tools.hpp */
/*-------------------------*/
