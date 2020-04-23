
/*
#ifndef KERNEL_H_
#define KERNEL_H_

#ifdef __cplusplus
extern "C"{
	void liberar_memoria(float *v_c, float *image_c, float *normM_c, float *out_c);
	void select_device();
	void reservar_memoria( float *image, int bands, int image_size, float *v_c, float *image_c, float *normM_c, float *out_c);
	void act(float *v, int bands, float *normM, int image_size, int i, int rows, float *v_c, float *image_c, float *normM_c,float *out_c);
}
#endif

void liberar_memoria(float *v_c, float *image_c, float *normM_c, float *out_c);
void select_device();
void reservar_memoria( float *image, int bands, int image_size, float *v_c, float *image_c, float *normM_c, float *out_c);
void act(float *v, int bands, float *normM, int image_size, int i, int rows, float *v_c, float *image_c, float *normM_c,float *out_c);



#endif
*/

#ifndef KERNEL_H_
#define KERNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

void liberar_memoria(float *v_c, float *image_c, float *normM_c);
void select_device();
void reservar_memoria(int bands, int image_size, float **v_c, float **image_c, float **normM_c);
void actualizar_normM(float *v, int bands, float *normM, int image_size, int i, int rows, float *v_c, float *image_c, float *normM_c);
void copy_normM(float *normM, float *normM_c, int image_size);
void copiar_image(float *image,float *image_c, int bands, int image_size);


#ifdef __cplusplus
}
#endif


#endif

