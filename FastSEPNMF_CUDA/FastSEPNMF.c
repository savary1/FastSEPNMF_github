//FastSEPNMF AROA AYUSO   DAVID SAVARY   2020

#include "kernel.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ReadWrite.h"
#include <sys/resource.h>

long int maxValExtractArray(float *normMAux, long int *b_pos, long int b_pos_size);
float maxVal(float *vector, long int image_size);


int main (int argc, char* argv[]){

	struct timeval t0, tfin, t1, t2;
	float secs_fin, t_sec, t_usec, t_norm, t_cost_square, t_copia_v, t_act, t_copia_norm, t_normalizar;
	
    int rows, cols, bands; 
    int datatype;
    int endmembers;
    int normalize;
    long int i, j, b_pos_size, d, k;
    float max_val, a, b, faux, faux2, max_min_val;

    if (argc != 5){
		printf("******************************************************************\n");
		printf("	ERROR in the input parameters:\n");
		printf("	The correct sintax is:\n");
		printf("	./FastSEPNMF image.bsq image.hdr numEndmembers normalize          \n");
		printf("******************************************************************\n");
		return(0);
	}
	else {
		endmembers = atoi(argv[3]);
        normalize = atoi(argv[4]);
	}
	

	secs_fin = t_sec = t_usec = t_norm = t_cost_square = t_copia_v = t_act = t_copia_norm = t_normalizar = 0;

    /**************************** #INIT# - Load Image and allocate memory******************************/
   
  	readHeader(argv[2], &cols, &rows, &bands, &datatype);
    printf("\nLines = %d  Samples = %d  Nbands = %d  Data_type = %d\n", rows, cols, bands, datatype);


    long int image_size = cols*rows;
    float *image;   															//input image 		
    float *U = (float *) malloc (bands * endmembers * sizeof(float));       	//selected endmembers
	float *normM_aux = (float *) malloc (image_size * sizeof(float));			//aux array to find the positions of (a-normM)/a <= 1e-6
	long int *b_pos = (long int *) malloc (image_size * sizeof(long int));	   	//valid positions of normM that meet (a-normM)/a <= 1e-6                                                                                                                                                            
	float *v;																	//used to update normM in every iteration                                                                                                                                                                    
    long int J[endmembers];                                                 	//selected endmembers positions in input image
	
	/**************************  CUDA  **********************************/	
	float *normM1_c;
	float *h_projections;  
	int *h_index;
	float *v_c, *image_c, *normM_c;
	float *d_projections;  
	int *d_index;
	int localSize = 1024;
	int globalSize_reduction = ceil((double)image_size/2/1024);

	
	selectDevice();	
	reservarMemoria(bands, image_size, &v_c, &image_c, &normM_c, &normM1_c, &image, &v, &d_projections, &d_index, &h_projections, &h_index, globalSize_reduction);
		
	LoadImageImagenes(argv[1], image, cols, rows, bands, datatype);
	
	/**************************** #END# - Load Image and allocate memory*******************************/
	
	gettimeofday(&t0,NULL);

    /**************************** #INIT# - Normalize image****************************************/
	gettimeofday(&t1,NULL);
    if (normalize == 1){
		normalizeImgC(image, image_size, bands, image_c, rows, normM_c, normM1_c);
    }
	else{
		calculateNormM(image, image_size, bands, rows, image_c, normM_c, normM1_c);
	}
	

	gettimeofday(&t2,NULL);
	t_sec  = (float)  (t2.tv_sec - t1.tv_sec);
  	t_usec = (float)  (t2.tv_usec - t1.tv_usec);
	t_norm = t_sec + t_usec/1.0e+6;
	
	/**************************** #END# - Normalize image****************************************/
	
	calculateMaxVal(image_size, normM_c, d_projections, h_projections);
	
	max_val = -1;
	for (j = 0; j < globalSize_reduction; j++){
		if(h_projections[j] > max_val){
			max_val = h_projections[j];
		}
	}
    /**************************** #INIT# - FastSEPNMF algorithm****************************************/ 
	
	i = 0;
	while(i < endmembers){
		
		calculateMaxVal(image_size, normM_c, d_projections, h_projections);
	
		a = -1;
		for (j = 0; j < globalSize_reduction; j++){
			if(h_projections[j] > a){
				a = h_projections[j];
			}
		}
		 
		if(a/max_val <= 1e-9){
			break;
		}
			
		calculateMaxValExtract_2(image_size, normM_c, normM1_c,  d_projections, h_projections, d_index, h_index, a);	
		
		max_min_val= -1;
		for (j = 0; j < globalSize_reduction; j++){
			if(h_projections[j] > max_min_val){
				max_min_val = h_projections[j];
				J[i] = h_index[j];
			}
		}	
		
		
		for(j = 0 ; j < bands; j++){
			U[i*bands + j] = image[J[i] + j*image_size];
		}

		for(j = 0; j < i; j++){
			faux = 0;
			for(k = 0; k < bands; k++){
				faux += U[j*bands + k] * U[i*bands + k];
			}			
			#pragma ivdep
			for(k = 0; k < bands; k ++){
				faux2 = U[j*bands + k] * faux;
				U[i*bands + k] = U[i*bands + k] - faux2;
			}				
		}
		faux = 0;
		for(j = 0; j < bands; j++){
			faux += U[i*bands + j]*U[i*bands + j];
		}
		faux = sqrt(faux);
		for(j = 0; j < bands; j++){	
			U[i*bands + j] = U[i*bands + j]/faux;
			v[j] = U[i*bands + j];
		}
		
		for(j = i - 1; j >= 0; j--){
			faux = 0;
			for(k = 0; k < bands; k++){
				faux += v[k] * U[j*bands + k];
			}
			for(k = 0; k < bands; k ++){
				faux2 = U[j*bands + k] * faux;
				v[k] = v[k] - faux2;
			}
		}
		
		gettimeofday(&t1,NULL);
		
		actualizarNormM(v, bands, image_size, i, rows, v_c, image_c, normM_c);

		gettimeofday(&t2,NULL);
		t_sec  = (float)  (t2.tv_sec - t1.tv_sec);
		t_usec = (float)  (t2.tv_usec - t1.tv_usec);
		t_cost_square = t_cost_square + t_sec + t_usec/1.0e+6;
		i = i + 1;
	}
	/**************************** #END# - FastSEPNMF algorithm*****************************************/
	
	gettimeofday(&tfin,NULL);
	t_sec  = (float)  (tfin.tv_sec - t0.tv_sec);
  	t_usec = (float)  (tfin.tv_usec - t0.tv_usec);
	secs_fin = t_sec + t_usec/1.0e+6;

	printf("Endmembers:\n");
    for(i = 0; i < endmembers; i++){
		printf("%ld \t- %ld \t- Coordenadas: (%ld,%ld)\n", i, J[i],(J[i] / cols),(J[i] % cols));
    }

	printf("Total time:	\t%.5f segundos\n", secs_fin);
	printf("T norm:	\t\t%.5f segundos\n", t_norm);
	printf("T square loop:	\t%.5f segundos\n", t_cost_square);
	 
    
    free(U);
	free(normM_aux);
	free(b_pos);

	liberarMemoria(v_c, image_c, normM_c, image, normM1_c, v, d_projections, d_index, h_projections, h_index);

    return 0;
}

long int maxValExtractArray(float *normM_aux, long int *b_pos, long int b_pos_size){
	float max_val = -1;
	long int pos = -1;
	long int i;
    for (i = 0; i < b_pos_size; i++){ 
        if(normM_aux[b_pos[i]] > max_val){
            max_val = normM_aux[b_pos[i]];
			pos = i;
        }
    }
	return pos;
}


float maxVal(float *vector, long int image_size){
	float max_val = -1;
	long int i;

    for (i = 0; i < image_size; i++){
        if(vector[i] > max_val){
            max_val = vector[i];
        }
    }

	return max_val;
}

