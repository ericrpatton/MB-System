/*--------------------------------------------------------------------
 *    The MB-system:	mbeditviz.h		4/27/2007
 *
 *    Copyright (c) 2007-2020 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 *
 * MBeditviz is an interactive swath bathymetry editor and patch
 * test tool for  MB-System.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global control parameters shared with
 * the Motif interface code.
 *
 * Author:	D. W. Caress
 * Date:	April 27, 2007
 *
 *
 */

/*--------------------------------------------------------------------*/

/* start this include */
#ifndef MB_EDITVIZ_DEF

/* header file flag */
#define MB_EDITVIZ_DEF 1

#ifndef MB_STATUS_DEF
#include "mb_status.h"
#endif

#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

#ifndef MB_PROCESS_DEF
#include "mb_process.h"
#endif

#ifndef MB_INFO_DEF
#include "mb_info.h"
#endif

#ifdef MBEDITVIZ_DECLARE_GLOBALS
#define MBVIEW_EXTERNAL
#else
#define MBVIEW_EXTERNAL extern
#endif

/* MBeditviz defines */
#define MBEV_GRID_NONE 0
#define MBEV_GRID_NOTVIEWED 1
#define MBEV_GRID_VIEWED 2
#define MBEV_GRID_ALGORITH_SIMPLE 0
#define MBEV_GRID_ALGORITH_FOOTPRINT 1
#define MBEV_GRID_WEIGHT_TINY 0.0000001
#define MBEV_ALLOC_NUM 24
#define MBEV_ALLOCK_NUM 1024
#define MBEV_NODATA -10000000.0
#define MBEV_NUM_ESF_OPEN_MAX 25

typedef enum {
     MBEV_GRID_ALGORITHM_SIMPLEMEAN = 0,
     MBEV_GRID_ALGORITHM_FOOTPRINT = 1,
     MBEV_GRID_ALGORITHM_SHOALBIAS = 2,
 } gridalgorithm_t;

typedef enum {
     MBEV_OUTPUT_MODE_EDIT = 0,
     MBEV_OUTPUT_MODE_BROWSE = 1,
 } output_mode_t;


/* usage of footprint based weight */
#define MBEV_USE_NO 0
#define MBEV_USE_YES 1
#define MBEV_USE_CONDITIONAL 2

/* mbeditviz structures */
struct mbev_ping_struct {
	int time_i[7];
	double time_d;
	int multiplicity;
	double navlon;
	double navlat;
	double navlonx;
	double navlaty;
	double portlon;
	double portlat;
	double stbdlon;
	double stbdlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sonardepth;
	double draft;
	double roll;
	double pitch;
	double heave;
	double ssv;
	int beams_bath;
	char *beamflag;
	char *beamflagorg;
	int *beamcolor;
	double *bath;
  double *amp;
	double *bathacrosstrack;
	double *bathalongtrack;
	double *bathcorr;
	double *bathlon;
	double *bathlat;
	double *bathx;
	double *bathy;
	double *angles;
	double *angles_forward;
	double *angles_null;
	double *ttimes;
	double *bheave;
	double *alongtrack_offset;
};
struct mbev_file_struct {
	int load_status;
	int load_status_shown;
	bool locked;
	bool esf_exists;
	char name[MB_PATH_MAXLINE];
	char path[MB_PATH_MAXLINE];
	int format;
	int raw_info_loaded;
	int processed_info_loaded;
	struct mb_info_struct raw_info;
	struct mb_info_struct processed_info;
	struct mb_process_struct process;
	bool esf_open;
  bool esf_changed;
	char esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;
	int num_pings;
	int num_pings_alloc;
	struct mbev_ping_struct *pings;
	double beamwidth_xtrack;
	double beamwidth_ltrack;
	int topo_type;

	int n_async_heading;
	int n_async_heading_alloc;
	double *async_heading_time_d;
	double *async_heading_heading;
	int n_async_sonardepth;
	int n_async_sonardepth_alloc;
	double *async_sonardepth_time_d;
	double *async_sonardepth_sonardepth;
	int n_async_attitude;
	int n_async_attitude_alloc;
	double *async_attitude_time_d;
	double *async_attitude_roll;
	double *async_attitude_pitch;
	int n_sync_attitude;
	int n_sync_attitude_alloc;
	double *sync_attitude_time_d;
	double *sync_attitude_roll;
	double *sync_attitude_pitch;
};
struct mbev_grid_struct {
	int status;
	char projection_id[MB_PATH_MAXLINE];
	void *pjptr;
	double bounds[4];
	double boundsutm[4];
	double dx;
	double dy;
	int n_columns;
	int n_rows;
	double min;
	double max;
	double smin;
	double smax;
	float nodatavalue;
	float *sum;
	float *wgt;
	float *val;
	float *sgm;
};

/*--------------------------------------------------------------------*/

/* mbeditviz global control parameters */

/* status parameters */
MBVIEW_EXTERNAL int mbev_status;
MBVIEW_EXTERNAL int mbev_error;
MBVIEW_EXTERNAL int mbev_verbose;

/* gui parameters */
MBVIEW_EXTERNAL int mbev_message_on;

/* mode parameters */
MBVIEW_EXTERNAL int mbev_mode_output;

/* data parameters */
MBVIEW_EXTERNAL int mbev_num_files;
MBVIEW_EXTERNAL int mbev_num_files_alloc;
MBVIEW_EXTERNAL int mbev_num_files_loaded;
MBVIEW_EXTERNAL int mbev_num_pings_loaded;
MBVIEW_EXTERNAL int mbev_num_esf_open;
MBVIEW_EXTERNAL int mbev_num_soundings_loaded;
MBVIEW_EXTERNAL int mbev_num_soundings_secondary;
MBVIEW_EXTERNAL double mbev_bounds[4];
MBVIEW_EXTERNAL struct mbev_file_struct *mbev_files;
MBVIEW_EXTERNAL struct mbev_grid_struct mbev_grid;
MBVIEW_EXTERNAL size_t mbev_instance;

/* gridding parameters */
MBVIEW_EXTERNAL double mbev_grid_bounds[4];
MBVIEW_EXTERNAL double mbev_grid_boundsutm[4];
MBVIEW_EXTERNAL double mbev_grid_cellsize;
MBVIEW_EXTERNAL gridalgorithm_t mbev_grid_algorithm;
MBVIEW_EXTERNAL int mbev_grid_interpolation;
MBVIEW_EXTERNAL int mbev_grid_n_columns;
MBVIEW_EXTERNAL int mbev_grid_n_rows;

/* global patch test parameters */
MBVIEW_EXTERNAL double mbev_rollbias;
MBVIEW_EXTERNAL double mbev_pitchbias;
MBVIEW_EXTERNAL double mbev_headingbias;
MBVIEW_EXTERNAL double mbev_timelag;
MBVIEW_EXTERNAL double mbev_snell;

/* sparse voxel filter parameters */
MBVIEW_EXTERNAL int mbev_sizemultiplier;
MBVIEW_EXTERNAL int mbev_nsoundingthreshold;

/* selected sounding parameters */
MBVIEW_EXTERNAL struct mb3dsoundings_struct mbev_selected;

/* timer function */
MBVIEW_EXTERNAL int timer_function_set;

int do_mbeditviz_init(Widget parentwidget, XtAppContext appcon);
void do_mbeditviz_mode_change(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_openfile(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_fileselection_list(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_fileSelectionBox_openswath(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_quit(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_viewall(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_viewselected(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_regrid(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_updategrid(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_changecellsize(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_gridparameters(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_viewgrid(void);
int do_mbeditviz_mbview_dismiss_notify(size_t instance);
void do_mbeditviz_deleteselected(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_changeoutputmode(Widget w, XtPointer client_data, XtPointer call_data);
int do_mbeditviz_opendata(char *input_file_ptr, int format);
void do_mbeditviz_update_gui(void);
void do_mbeditviz_update_filelist(void);
void do_mbeditviz_pickonepoint_notify(size_t instance);
void do_mbeditviz_picktwopoint_notify(size_t instance);
void do_mbeditviz_pickarea_notify(size_t instance);
void do_mbeditviz_pickregion_notify(size_t instance);
void do_mbeditviz_picksite_notify(size_t instance);
void do_mbeditviz_pickroute_notify(size_t instance);
void do_mbeditviz_picknav_notify(size_t instance);
void do_mbeditviz_regrid_notify(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_enablesecondarypicks_notify(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_disablesecondarypicks_notify(Widget w, XtPointer client_data, XtPointer call_data);
void do_mbeditviz_colorchange_notify(size_t instance);
int do_mbeditviz_message_on(char *message);
int do_mbeditviz_message_off(void);
int do_error_dialog(char *s1, char *s2, char *s3);
void set_label_string(Widget w, String str);
void set_label_multiline_string(Widget w, String str);
void get_text_string(Widget w, String str);
int do_wait_until_viewed(void);
int do_mbeditviz_settimer(void);
int do_mbeditviz_workfunction(XtPointer client_data);

int mbeditviz_init(int argc, char **argv);
int mbeditviz_get_format(char *file, int *form);
int mbeditviz_open_data(char *path, int format);
int mbeditviz_import_file(char *path, int format);
int mbeditviz_load_file(int ifile);
int mbeditviz_apply_biasesandtimelag(struct mbev_file_struct *file, struct mbev_ping_struct *ping, double rollbias, double pitchbias,
                            double headingbias, double timelag, double *headingdelta, double *sonardepth, double *rolldelta,
                            double *pitchdelta);
int mbeditviz_snell_correction(double snell, double roll, double *beam_xtrack,
							   double *beam_ltrack, double *beam_z);
int mbeditviz_beam_position(double navlon, double navlat, double mtodeglon, double mtodeglat, double rawbath, double acrosstrack,
                            double alongtrack, double sonardepth, double rolldelta, double pitchdelta, double heading,
                            double *bathcorr, double *lon, double *lat);
int mbeditviz_unload_file(int ifile);
int mbeditviz_delete_file(int ifile);
double mbeditviz_erf(double x);
int mbeditviz_bin_weight(double foot_a, double foot_b, double scale, double pcx, double pcy, double dx, double dy, double *px,
                         double *py, double *weight, int *use);
int mbeditviz_get_grid_bounds(void);
int mbeditviz_setup_grid(void);
int mbeditviz_project_soundings(void);
int mbeditviz_make_grid(void);
int mbeditviz_grid_beam(struct mbev_file_struct *file, struct mbev_ping_struct *ping, int ibeam, int beam_ok, int apply_now);
int mbeditviz_make_grid_simple(void);
int mbeditviz_destroy_grid(void);
int mbeditviz_selectregion(size_t instance);
int mbeditviz_selectarea(size_t instance);
int mbeditviz_selectnav(size_t instance);
void mbeditviz_mb3dsoundings_dismiss(void);
void mbeditviz_mb3dsoundings_edit(int ifile, int iping, int ibeam, char beamflag, int flush);
void mbeditviz_mb3dsoundings_info(int ifile, int iping, int ibeam, char *infostring);
void mbeditviz_mb3dsoundings_bias(double rollbias, double pitchbias, double headingbias, double timelag, double snell);
void mbeditviz_mb3dsoundings_biasapply(double rollbias, double pitchbias, double headingbias, double timelag, double snell);
void mbeditviz_mb3dsoundings_flagsparsevoxels(int sizemultiplier, int nsoundingthreshold);
void mbeditviz_mb3dsoundings_colorsoundings(int color);
void mbeditviz_mb3dsoundings_optimizebiasvalues(int mode, double *rollbias, double *pitchbias, double *headingbias,
                                                double *timelag, double *snell);
void mbeditviz_mb3dsoundings_getbiasvariance(double local_grid_xmin, double local_grid_xmax, double local_grid_ymin,
                                             double local_grid_ymax, int local_grid_nx, int local_grid_ny, double local_grid_dx,
                                             double local_grid_dy, double *local_grid_first, double *local_grid_sum,
                                             double *local_grid_sum2, double *local_grid_variance, int *local_grid_num,
                                             double rollbias, double pitchbias, double headingbias, double timelag, double snell,
                                             int *variance_total_num, double *variance_total);

void BxUnmanageCB(Widget w, XtPointer client, XtPointer call);
void BxManageCB(Widget w, XtPointer client, XtPointer call);
void BxPopupCB(Widget w, XtPointer client, XtPointer call);
XtPointer BX_CONVERT(Widget w, char *from_string, char *to_type, int to_size, Boolean *success);
void BxExitCB(Widget w, XtPointer client, XtPointer call);
void BxSetValuesCB(Widget w, XtPointer client, XtPointer call);

/* end this include */
#endif

/*--------------------------------------------------------------------*/
