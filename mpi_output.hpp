#ifndef MPI_OUTPUT_HPP
#define MPI_OUTPUT_HPP

#include <cstdio>

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <tuple>

#include <mpi.h>

class MPI_Output
{
private:
	int world;
	int rank, size, buf_size;
	std::chrono::system_clock::time_point begin, end;
	std::vector<std::pair<long long, std::string>> buf;

	void gather_message ( std::vector<std::tuple<long long, int, std::string>>& mes, bool sort_id );
public:
	MPI_Output( int* argc, char*** argv, const int& world = MPI_COMM_WORLD );
	~MPI_Output();

	void add_message ( std::string mes );

	void print ( bool sort_id = false, bool dateflag = false, std::ostream& os = std::cout );
};

MPI_Output::MPI_Output( int* argc, char*** argv, const int& world )
	: world(world)
{
	int flag;
	MPI_Initialized(&flag);

	if( flag == 0 ) MPI_Init(argc, argv);
	
	MPI_Comm_rank(this->world, &rank);
	MPI_Comm_size(this->world, &size);

	begin = std::chrono::system_clock::now();
}

MPI_Output::~MPI_Output()
{
}

void MPI_Output::gather_message ( std::vector<std::tuple<long long, int, std::string>>& mes, bool sort_id )
{
	int* each_size = new int[size];
	int own_size = buf.size();

	MPI_Gather(&own_size, 1, MPI_INT, each_size, 1, MPI_INT, 0, world);

	if( rank == 0 ){
		for( int i = 0; i < buf.size(); ++i )
			mes.emplace_back(buf[i].first, 0, buf[i].second);
	}
	
	for ( int i = 1; i < size; ++i ){
		if( rank == 0 ){
			for( int j = 0; j < each_size[i]; ++j ){
				int buf_size;
				char *buf;
				long long time;
				MPI_Status status;
				
				MPI_Recv(&time, 1, MPI_LONG_LONG_INT, i, i, world, &status);
				MPI_Recv(&buf_size, 1, MPI_INT, i, i, world, &status);
				
				buf = new char[buf_size+1];
				for( int k = 0; k < buf_size+1; ++k ) buf[k] = 0;
				MPI_Recv(buf, buf_size, MPI_CHAR, i, i, world, &status);
				mes.emplace_back(time, i, std::string(buf));
				delete [] buf;
			}
		}
		else if( rank == i ){
			for( int j = 0; j < buf.size(); ++j ){
				long long time = buf[j].first;
				std::string str = buf[j].second;
				int buf_size = str.size();
				
				MPI_Send(&time, 1, MPI_LONG_LONG_INT, 0, i, world);
				MPI_Send(&buf_size, 1, MPI_INT, 0, i, world);
				MPI_Send(str.c_str(), str.size(), MPI_CHAR, 0, i, world);
			}
		}
		MPI_Barrier(world);
	}

	typedef std::tuple<long long, int, std::string> data;
	std::sort( mes.begin(), mes.end(), [&]( const data& d1, const data& d2 ) -> bool {
			if( sort_id ){
				return (std::get<1>(d1) == std::get<1>(d2) ?
						std::get<0>(d1) < std::get<0>(d2) : std::get<1>(d1) < std::get<1>(d2));
			}
			else{
				return (std::get<0>(d1) == std::get<0>(d2) ?
						std::get<1>(d1) < std::get<1>(d2) : std::get<0>(d1) < std::get<0>(d2));
			}
		});
	
	delete [] each_size;
}

void MPI_Output::add_message ( std::string mes )
{
	auto cur = std::chrono::system_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::microseconds>((cur-begin)).count();
	
	buf.emplace_back(diff, mes);
}

void MPI_Output::print ( bool sort_id, bool dateflag, std::ostream& os )
{
	std::vector<std::tuple<long long, int, std::string>> output;
	gather_message(output, sort_id);

	if( rank != 0 ) return;

	os << "  Time Trace    " << " Rank No. " << " : " << "Message" << std::endl;
	for( int i = 0; i < output.size(); ++i ){
		long long t;
		int id;
		std::string str;

		std::tie(t, id, str) = output[i];
		
		int space_size = 0;
		if( dateflag ){
			auto d = begin + std::chrono::microseconds(t);
			auto date = std::chrono::system_clock::to_time_t(d);
			std::string buf = std::string(ctime(&date));
			
			os << "[" << buf.substr(0,buf.size()-1) << "]" << " from rank " << std::setw(6) << id << " : ";
			space_size = 1+(buf.size()-1)+1+11+6+3;
		}
		else{
			os << "[" << std::setw(12) << t << "]" << " from rank " << std::setw(6) << id << " : ";
			space_size = 1+12+1+11+6+3;
		}
		
		const std::string space(space_size, ' ');
		str += "\n";
		int prev_pos = 0;
		while(1){
			int pos = str.find("\n", prev_pos);
			if( pos == -1 ) break;

			os << (prev_pos == 0 ? "" : space.c_str()) << (str.substr(prev_pos, pos-prev_pos+1));
			prev_pos = pos+1;
		}
	}
}

#endif
