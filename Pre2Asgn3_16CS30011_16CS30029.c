#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

int main ( int argc, char ** argv ){

	int rank,size;
	int NRA,NCA,NRB,NCB;
	int m=199,a=7,last=1;

	/*printf("Input size of matrix A and B: \n");
	scanf("%d %d %d %d", &NRA,&NCA,&NRB,&NCB );
	if ( NCA!=NRB ){
		printf("ERROR: #column A not equal to #rows B\n");
		exit(1);
	}*/ 
	NRA = NCA = NRB = NCB = 3;
	int A[NRA][NCA],B[NRB][NCB],C[NRA][NCB];

	int averow,extra,offset,mtype,numworkers;
	int i,j,k,rc;
	MPI_Status status;
	int source,dest,rows;

	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
 	MPI_Comm_size( MPI_COMM_WORLD, &size );

 	if ( size < 2 ) {
		printf("ERROR: Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, 1 );
		exit(1);
	}
	numworkers = size-1;

	if ( rank==0 ){

		for (i=0; i<NRA; i++)
 			for (j=0; j<NCA; j++)
 				{ last = (last*a)%m; A[i][j]= last; }
 		for (i=0; i<NCA; i++)
 			for (j=0; j<NCB; j++)
 				{ last = (last*a)%m; B[i][j]= last; }

 		printf("Started with %d tasks.\n",numworkers );
 		
 		averow = NRA/numworkers;
 		extra = NRA%numworkers;
 		offset = 0;
 		mtype = 1;
 		for (dest=1; dest<=numworkers; dest++) {
 			rows = (dest <= extra) ? averow+1 : averow;
 			printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
 			MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
 			MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
 			MPI_Send(&A[offset][0], rows*NCA, MPI_INT, dest, mtype,MPI_COMM_WORLD);
 			MPI_Send(&B, NCA*NCB, MPI_INT, dest, mtype, MPI_COMM_WORLD);
 			offset = offset + rows;
		}

		mtype = 2;
 		for (i=1; i<=numworkers; i++) {
 			source = i;
 			MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
 			MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
			MPI_Recv(&C[offset][0], rows*NCB, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
 			printf("Received results from task %d\n",source);
 		}

 		for (i=0; i<NRA; i++) {
 			printf("\n");
 			for (j=0; j<NCB; j++) printf("%6d ", C[i][j]);
 		}
 		printf ("\nDone.\n");

	} else {

		mtype = 1;
 		MPI_Recv(&offset, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD, &status);
 		MPI_Recv(&rows, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD, &status);
 		MPI_Recv(&A, rows*NCA, MPI_INT, 0, mtype, MPI_COMM_WORLD, &status);
 		MPI_Recv(&B, NCA*NCB, MPI_INT, 0, mtype, MPI_COMM_WORLD, &status);
 		for (k=0; k<NCB; k++)
 			for (i=0; i<rows; i++) {
 				C[i][k] = 0;
 				for (j=0; j<NCA; j++) C[i][k] = C[i][k] + A[i][j] * B[j][k];
 			}
 		mtype = 2;
 		MPI_Send(&offset, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD);
 		MPI_Send(&rows, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD);
 		MPI_Send(&C, rows*NCB, MPI_INT, 0, mtype, MPI_COMM_WORLD);

	}

	MPI_Finalize();

}