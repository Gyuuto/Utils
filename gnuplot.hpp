#ifndef GNUPLOT_HPP
#define GNUPLOT_HPP

#include <string>
#include <vector>
#include <numeric>

class PlotParams
{
public:
	std::string legend;

	bool with_line;
	bool with_point;
	
	int line_type;
	double line_size;
	int point_type;
	double point_size;

	int color;

	PlotParams () :legend(""), with_line(true), with_point(true),
				   line_type(-1), line_size(1.0), point_type(-1), point_size(1.0),
				   color(-1)
	{}
};

class Gnuplot
{
private:
	FILE* fp_plot;

	std::string filename;
	int cnt, num_holdon;
	bool ploted;
	
	bool grid, logscale_x, logscale_y, stacked;

	void replot();
public:
	Gnuplot( bool set_eps = true, const std::string& font_name = "TimesNewRoman", const int font_size = 18 );
	~Gnuplot();
	
	void toggle_grid ();
	void toggle_logscale_x ();
	void toggle_logscale_y ();
	void set_stacked ( double width = 0.75 );

	void set_xrange ( const double min_x, const double max_x );
	void set_yrange ( const double min_y, const double max_y );

	void set_xlabel ( const std::string& label );
	void set_ylabel ( const std::string& label );

	void set_xtics ( const double beg, const double end, const double inc );
	void set_ytics ( const double beg, const double end, const double inc );
	void set_format ( const std::string dir, const std::string format );
	void set_key ( const std::string& pos );

	void set_xtics_offset ( const double x, const double y );
	void set_ytics_offset ( const double x, const double y );
	
	void set_output_name ( const std::string& name );
	
	void plot ( const std::vector<double>& y, const PlotParams& params, bool hold_on = false );
	void plot ( const std::vector<double>& x, const std::vector<double>& y, const PlotParams& params, bool hold_on = false, bool wrap = false );

	void reset ();
};

Gnuplot::Gnuplot( bool set_eps, const std::string& font_name, const int font_size )
	:cnt(0), num_holdon(0), ploted(false), stacked(false), grid(false), logscale_x(false), logscale_y(false)
{
	fp_plot = popen("gnuplot", "w");

	if( set_eps ){
		fprintf(fp_plot, "set terminal postscript color eps\n");
		fprintf(fp_plot, "set terminal postscript font \'%s, %d\'\n", font_name.c_str(), font_size);
	}
	else{
		fprintf(fp_plot, "set terminal x11\n");
		fprintf(fp_plot, "set terminal x11 font \'%s, %d\'\n", font_name.c_str(), font_size);
	}
	fflush(fp_plot);
}
Gnuplot::~Gnuplot()
{
	pclose(fp_plot);

	for( int i = 0; i < cnt; ++i ){
		std::string cmd = std::string("rm ") + "gnuplot_tmp" + std::to_string(i) + ".dat";
		system(cmd.c_str());
	}
}
	
void Gnuplot::toggle_grid ()
{
	if( grid == false ) fprintf(fp_plot, "set grid ytics xtics mytics mxtics\n");
	else fprintf(fp_plot, "unset grid\n");
	grid = !grid;
	replot();
}

void Gnuplot::set_stacked ( double width )
{
	fprintf(fp_plot, "set style data histograms\n");
	fprintf(fp_plot, "set style histogram rowstacked\n");
	fprintf(fp_plot, "set style fill solid border lc rgb \'black\'\n");
	fprintf(fp_plot, "set boxwidth %.6E relative\n", width);
	stacked = true;
	replot();
}

void Gnuplot::toggle_logscale_x ()
{
	if( logscale_x == false ) fprintf(fp_plot, "set logscale x\n");
	else fprintf(fp_plot, "unset logscale x\n");
	logscale_x = !logscale_x;
	replot();
}
void Gnuplot::toggle_logscale_y ()
{
	if( logscale_y == false ) fprintf(fp_plot, "set logscale y\n");
	else fprintf(fp_plot, "unset logscale y\n");
	logscale_y = !logscale_y;
	replot();
}

void Gnuplot::set_xrange ( const double min_x, const double max_x )
{
	fprintf(fp_plot, "set xrange [%.16E:%.16E]\n", min_x, max_x);
	replot();
}
void Gnuplot::set_yrange ( const double min_y, const double max_y )
{
	fprintf(fp_plot, "set yrange [%.16E:%.16E]\n", min_y, max_y);
	replot();
}

void Gnuplot::set_xlabel ( const std::string& label )
{
	fprintf(fp_plot, "set xlabel \'%s\'\n", label.c_str());
	replot();
}
void Gnuplot::set_ylabel ( const std::string& label )
{
	fprintf(fp_plot, "set ylabel \'%s\'\n", label.c_str());
	replot();
}

void Gnuplot::set_xtics ( const double beg, const double end, const double inc )
{
	fprintf(fp_plot, "set xtics %.16E,%.16E,%.16E\n", beg, inc, end);
	replot();
}
void Gnuplot::set_ytics ( const double beg, const double end, const double inc )
{
	fprintf(fp_plot, "set ytics %.16E,%.16E,%.16E\n", beg, inc, end);
	replot();
}

void Gnuplot::set_format ( const std::string dir, const std::string format )
{
	fprintf(fp_plot, "set format %s \"%s\"\n", dir.c_str(), format.c_str());
	replot();
}

void Gnuplot::set_key( const std::string& pos )
{
	fprintf(fp_plot, "set key %s\n", pos.c_str());
	replot();
}

void Gnuplot::set_xtics_offset ( const double x, const double y )
{
	fprintf(fp_plot, "set xtics offset %.16E,%.16E\n", x, y);
	replot();
}

void Gnuplot::set_ytics_offset ( const double x, const double y )
{
	fprintf(fp_plot, "set ytics offset %.16E,%.16E\n", x, y);
	replot();
}


void Gnuplot::set_output_name ( const std::string& name )
{
	filename = name;
}

void Gnuplot::plot ( const std::vector<double>& y, const PlotParams& params, bool hold_on )
{
	std::vector<double> x(y.size());
	std::iota(x.begin(), x.end(), 0.0);
	
	if( y.size() != 0 ) plot(x, y, params, hold_on);
}

void Gnuplot::plot ( const std::vector<double>& x, const std::vector<double>& y, const PlotParams& params, bool hold_on, bool wrap )
{
	std::string option = "";
	if( hold_on && wrap == false ) num_holdon++;
	else num_holdon = 1;
	
	if( stacked ){
		option += "w boxes";
		if( params.legend != "" ) option += " title \'" + params.legend + "\'";
		else option += " notitle";
		
		std::string filename = "gnuplot_tmp" + std::to_string(cnt++) + ".dat";
		FILE* fp = fopen(filename.c_str(), "w");
		for( int i = 0; i < y.size(); ++i ) fprintf(fp, "%d %.16E\n", (int)x[i], y[i]);
		fclose(fp);

		if( hold_on ){
			fprintf(fp_plot, "set output \'%s\'\n", this->filename.c_str());
			fprintf(fp_plot, "NUM_HOLDON = %d\n", num_holdon);
			fprintf(fp_plot, "replot \'%s\' using ($0*NUM_HOLDON+%d):($2) %s\n", filename.c_str(), num_holdon-1, option.c_str());
		}
		else{
			fprintf(fp_plot, "set output \'%s\'\n", this->filename.c_str());
			fprintf(fp_plot, "NUM_HOLDON = %d\n", num_holdon);
			fprintf(fp_plot, "plot \'%s\' using ($0*NUM_HOLDON):($2):xtic(1) %s\n", filename.c_str(), option.c_str());
			ploted = true;
		}
	}
	else{
		if( params.with_line && params.with_point ) option += " w lp";
		else if( params.with_line && !params.with_point ) option += " w l";
		else option += " w p";
	
		if( params.with_line ){
			option += " lw " + std::to_string(params.line_size);
			if( params.line_type != -1 ) option += " lt " + std::to_string(params.line_type);
		}

		if( params.with_point ){
			option += " ps " + std::to_string(params.point_size);
			if( params.point_type != -1 ) option += " pt " + std::to_string(params.point_type);
		}

		if( params.color != -1 ){
			char color[32];
			sprintf(color, "#%08X", params.color);
			option += " lc rgb \"";
			option += color;
			option += "\"";
		}

		if( params.legend != "" ) option += " title \'" + params.legend + "\'";
		else option += " notitle";

		std::string filename = "gnuplot_tmp" + std::to_string(cnt++) + ".dat";
		FILE* fp = fopen(filename.c_str(), "w");
		for( int i = 0; i < y.size(); ++i ) fprintf(fp, "%.16E %.16E\n", x[i], y[i]);
		fclose(fp);
	
		if( hold_on ){
			fprintf(fp_plot, "set output \'%s\'\n", this->filename.c_str());
			fprintf(fp_plot, "replot \'%s\'%s\n", filename.c_str(), option.c_str());
		}
		else{
			fprintf(fp_plot, "set output \'%s\'\n", this->filename.c_str());
			fprintf(fp_plot, "plot \'%s\'%s\n", filename.c_str(), option.c_str());
			ploted = true;
		}
	}
	fflush(fp_plot);
}

void Gnuplot::reset ()
{
	fprintf(fp_plot, "reset\n");
	grid = stacked = false;
	logscale_x = logscale_y = false;
	ploted = false;
	fflush(fp_plot);
}

void Gnuplot::replot ()
{
	if( ploted ) {
		fprintf(fp_plot, "set output \'%s\'\n\n", filename.c_str());
		fprintf(fp_plot, "replot\n");
		fflush(fp_plot);
	}
}

#endif
