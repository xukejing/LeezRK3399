#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
using namespace std;

//Problem size
const int N = 450;

//global variables
double A[N][N];
double B[N][N];
double AB[N][N];
double AB_serial[N][N];

//timing variables
double time1,time2,time3;

//Function to fill matrices at random
void fill_matrices(){
  srand(time(NULL));

    for(int i = 0; i <N; i++){
      for (int j = 0; j < N; j++){
	A[i][j] = rand() %4;
	B[i][j] = rand() %4;
      }
    }
}

//Function to print matrix
void print_matrix(double mat[][N]){
  for (int i = 0; i < N; i++){
    for (int j = 0; j<N; j++){
      cout << mat[i][j] << " ";
    }
    cout << endl;
  }
  cout << endl;
}

//Serial version of solution
void serial_version(){
  for (int i = 0; i <N; i++){
    for (int j = 0; j < N;j++){
      AB_serial[i][j] = 0;
      for (int k = 0; k <N; k++){
	AB_serial[i][j] += A[i][k]*B[k][j];
      }
    }
  }
}

//Compute interval multiplication
void compute_interval(int start,int interval){
  for(int i = start; i <start+interval;i++){
    for (int j = 0; j <N; j++){
      AB[i][j] = 0;
      for (int k = 0; k <N; k++){
	AB[i][j] += A[i][k]*B[k][j];
      }
    }
  }
}

int main(int argc, char** argv){
  int interval, remainder;
  MPI::Init(argc,argv);
  MPI::Comm & comm = MPI::COMM_WORLD;
  int comm_sz = comm.Get_size();
  int my_rank = comm.Get_rank();

  //compute interval size
  //rank 0 responsible for remainder
  interval = N/comm_sz;
  remainder = N%comm_sz;

  //Record start time
  comm.Barrier();
  time1 = MPI::Wtime();

  //Rank 0 fills the matrices and computes the remainder
  if(my_rank ==0){
    fill_matrices();
    compute_interval(comm_sz*interval,remainder);
  }

  //Broadcast Matrix B and scatter relevant portions of Matrix A
  comm.Bcast(B,N*N,MPI::DOUBLE,0);
  comm.Scatter(A,interval*N,MPI::DOUBLE,A[my_rank*interval],interval*N,MPI::DOUBLE,0);

  //Each processor cumputes the interval they are responsible for
  compute_interval(my_rank*interval,interval);

  //Gather results
  comm.Gather(AB[my_rank*interval],interval*N,MPI::DOUBLE,AB,interval*N,MPI::DOUBLE,0);

  //Record parallel finish time
  comm.Barrier();
  time2 = MPI::Wtime();

  if (my_rank ==0){

    //serial computation
    serial_version();

    //Record serial finish time
    time3 = MPI::Wtime();

    //Print times
    cout << "Problem size " << N << endl;
    cout << comm_sz << " processors computed in time: " << time2-time1 << endl;
    cout << "Serial version computed in time: " << time3-time2 << endl;
    cout << "Efficiency of: " << (time3-time2)/((time2-time1)*comm_sz) << endl;

    /*Code to print matrices and results
    cout << "Matrix A: " << endl;
    print_matrix(A);
    cout << "multiplied Matrix B:" << endl;
    print_matrix(B);
    cout << "gives matrix AB:" << endl;
    print_matrix(AB);
    cout << "serial version gives:" << endl;
    print_matrix(AB_serial);*/
  }
  
  MPI::Finalize();
}
